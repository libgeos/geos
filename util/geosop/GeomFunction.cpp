/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2020 Martin Davis
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/geom/Geometry.h>
#include <geos/geom/Point.h>
#include <geos/geom/LineString.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/GeometryCollection.h>
#include <geos/geom/IntersectionMatrix.h>
#include <geos/geom/Envelope.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/prep/PreparedGeometry.h>
#include <geos/geom/prep/PreparedGeometryFactory.h>
#include <geos/algorithm/construct/LargestEmptyCircle.h>
#include <geos/algorithm/construct/MaximumInscribedCircle.h>
#include <geos/algorithm/MinimumBoundingCircle.h>
#include <geos/algorithm/distance/DiscreteHausdorffDistance.h>
#include <geos/algorithm/distance/DiscreteFrechetDistance.h>
#include <geos/algorithm/hull/ConcaveHull.h>
#include <geos/geom/util/Densifier.h>
#include <geos/geom/util/GeometryFixer.h>
#include <geos/operation/buffer/BufferBuilder.h>
#include <geos/operation/buffer/BufferOp.h>
#include <geos/operation/buffer/BufferParameters.h>
#include <geos/operation/buffer/OffsetCurve.h>
#include <geos/operation/linemerge/LineMerger.h>
#include <geos/operation/distance/DistanceOp.h>
#include <geos/operation/intersection/RectangleIntersection.h>
#include <geos/operation/intersection/Rectangle.h>
#include <geos/operation/relate/RelateOp.h>
#include <geos/operation/valid/MakeValid.h>
#include <geos/operation/overlayng/OverlayNG.h>
#include <geos/operation/polygonize/Polygonizer.h>
#include <geos/operation/polygonize/BuildArea.h>
#include <geos/operation/overlayng/CoverageUnion.h>
#include <geos/operation/union/CoverageUnion.h>
#include <geos/precision/GeometryPrecisionReducer.h>
#include <geos/simplify/DouglasPeuckerSimplifier.h>
#include <geos/simplify/TopologyPreservingSimplifier.h>
#include <geos/triangulate/DelaunayTriangulationBuilder.h>
#include <geos/triangulate/VoronoiDiagramBuilder.h>
#include <geos/triangulate/polygon/ConstrainedDelaunayTriangulator.h>

#include "GeomFunction.h"

#include <sstream>

using geos::operation::overlayng::OverlayNG;
using geos::algorithm::distance::DiscreteFrechetDistance;

/* static private */
std::map<std::string, GeomFunction*> GeomFunction::registry;
/* static private */
std::vector<GeomFunction*> GeomFunction::functionList;

class PreparedGeometryCache {
public:
    const PreparedGeometry* get(const Geometry* key) {
        if (m_key != key) {
            m_pg = PreparedGeometryFactory::prepare(key);
            m_key = key;
        }

        return m_pg.get();
    }

private:
    std::unique_ptr<const PreparedGeometry> m_pg;
    const Geometry* m_key;
};

PreparedGeometryCache prepGeomCache;

const std::string catMetric = "Metric";
const std::string catConst = "Construction";
const std::string catDist = "Distance";
const std::string catGeom = "Geometry";
const std::string catOverlay = "Overlay";
const std::string catRel = "Spatial Relationship";
const std::string catValid = "Validity";

/* static */
void
GeomFunction::init()
{
    add("copy", Result::typeGeometry, catGeom,
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            (void) geomB; (void)d;  // prevent unused variable warning
            return new Result( geom->clone() );
        });
     add("envelope", Result::typeGeometry, catGeom,
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            (void) geomB; (void)d;  // prevent unused variable warning
            return new Result( geom->getCentroid() );
        });
    add("isEmpty", 1, 0, Result::typeBool, catGeom,
        "test if geometry is empty",
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            (void) geomB; (void)d;  // prevent unused variable warning
            return new Result( geom->isEmpty() );
        });
    add("normalize", 1, 0, Result::typeGeometry, catGeom,
        "normalize geometry",
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            (void) geomB; (void)d;  // prevent unused variable warning
            auto res = geom->clone();
            res->normalize();
            return new Result( std::move(res) );
        });

    add("lineMerge", 1, 0, Result::typeGeometry, catGeom,
        "merge the lines of geometry",
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            (void) geomB; (void)d;  // prevent unused variable warning
            geos::operation::linemerge::LineMerger lmrgr;
            lmrgr.add(geom.get());

            std::vector<std::unique_ptr<LineString>> lines = lmrgr.getMergedLineStrings();

            std::vector<std::unique_ptr<const Geometry>> geoms;
            for(unsigned int i = 0; i < lines.size(); i++) {
                geoms.push_back( std::move(lines[i]) );
            }
            return new Result( std::move(geoms) ) ;
        });

    add("reducePrecision", 1, 1, Result::typeGeometry, catGeom,
        "reduce precision of geometry to a precision scale factor",
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            (void)geomB;  // prevent unused variable warning
            PrecisionModel pm(d);
            return new Result( geos::precision::GeometryPrecisionReducer::reduce( *geom, pm ) );
        });
    add("reducePrecisionKeepCollapsed", 1, 1, Result::typeGeometry, catGeom,
        "reduce precision of geometry to a precision scale factor",
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            (void)geomB;  // prevent unused variable warning
            PrecisionModel pm(d);
            return new Result( geos::precision::GeometryPrecisionReducer::reduceKeepCollapsed( *geom, pm ) );
        });
    add("reducePrecisionPointwise", 1, 1, Result::typeGeometry, catGeom,
        "reduce precision of geometry to a precision scale factor",
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            (void)geomB;  // prevent unused variable warning
            PrecisionModel pm(d);
            return new Result( geos::precision::GeometryPrecisionReducer::reducePointwise( *geom, pm ) );
        });

    add("reverse", 1, 0, Result::typeGeometry, catGeom,
        "reverse geometry",
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            (void) geomB; (void)d;  // prevent unused variable warning
            return new Result( geom->reverse() );
        });

    //-------------------------------------

    add("area", Result::typeDouble, catMetric,
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            (void) geomB; (void)d;  // prevent unused variable warning
            return new Result( geom->getArea() );
        });
    add("length", Result::typeDouble, catMetric,
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            (void) geomB; (void)d;  // prevent unused variable warning
            return new Result( geom->getLength() );
        });
//-------------------------------------
    add("isSimple", 1, 0, Result::typeBool, catValid,
        "test if geometry is simple",
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            (void) geomB; (void)d;  // prevent unused variable warning
            return new Result( geom->isSimple() );
        });

    add("isValid", 1, 0, Result::typeBool, catValid,
        "test if geometry is valid",
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            (void) geomB; (void)d;  // prevent unused variable warning
            return new Result( geom->isValid() );
        });
    add("fixInvalid", 1, 0, Result::typeGeometry, catValid,
        "fix invalid geometry to be valid",
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            (void) geomB; (void)d;  // prevent unused variable warning
            return new Result( geos::geom::util::GeometryFixer::fix( geom.get() ) );
       });
    add("makeValid", 1, 0, Result::typeGeometry, catValid,
        "make geometry valid (original algorithm)",
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            (void) geomB; (void)d;  // prevent unused variable warning
            return new Result( geos::operation::valid::MakeValid().build( geom.get() ) );
       });
//-------------------------------------

    add("boundary", Result::typeGeometry, catConst,
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            (void) geomB; (void)d;  // prevent unused variable warning
            return new Result( geom->getBoundary() );
        });
    add("buffer", 1, 1, Result::typeGeometry,
        catConst, "compute the buffer of geometry by a distance",
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            (void) geomB;  // prevent unused variable warning
            return new Result( geom->buffer( d ) );
        });
    add("offsetCurve", 1, 1, Result::typeGeometry,
        catConst, "compute the offset curve of geometry by a distance",
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            (void) geomB;  // prevent unused variable warning
            geos::operation::buffer::BufferParameters bp;

            geos::operation::buffer::OffsetCurve oc(*geom, d, bp);
            std::unique_ptr<Geometry> g3 = oc.getCurve();
            return new Result( g3.release() );
        });
    add("OLDoffsetCurve", 1, 1, Result::typeGeometry,
        catConst, "compute the offset curve of geometry by a distance",
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            (void) geomB;  // prevent unused variable warning
            geos::operation::buffer::BufferParameters bp;

            bool isLeftSide = true;
            if(d < 0) {
                isLeftSide = false;
                d = -d;
            }
            geos::operation::buffer::BufferBuilder bufBuilder(bp);
            return new Result( bufBuilder.bufferLineSingleSided(geom.get(), d, isLeftSide) );
        });
    add("centroid", Result::typeGeometry, catConst,
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            (void) geomB; (void)d;  // prevent unused variable warning
            return new Result( geom->getCentroid() );
        });
    add("convexHull", Result::typeGeometry, catConst,
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            (void) geomB; (void)d;  // prevent unused variable warning
            return new Result( geom->convexHull() );
        });
    add("concaveHull", Result::typeGeometry, catConst,
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            (void) geomB; (void)d;  // prevent unused variable warning
            geos::algorithm::hull::ConcaveHull hull(geom.get());
            hull.setMaximumEdgeLengthRatio( d );
            return new Result( hull.getHull() );
        });
    add("concaveHullHoles", Result::typeGeometry, catConst,
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            (void) geomB; (void)d;  // prevent unused variable warning
            geos::algorithm::hull::ConcaveHull hull(geom.get());
            hull.setMaximumEdgeLengthRatio( d );
            hull.setHolesAllowed(true);
            return new Result( hull.getHull() );
        });

    add("densify", 1, 1, Result::typeGeometry, catConst,
        "densify geometry to a distance ",
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            (void)geomB;  // prevent unused variable warning
            geos::geom::util::Densifier densifier( geom.get() );
            densifier.setDistanceTolerance( d );
            return new Result( densifier.getResultGeometry() );
        });



    add("interiorPoint", Result::typeGeometry, catConst,
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            (void) geomB; (void)d;  // prevent unused variable warning
            return new Result( geom->getInteriorPoint() );
        });

    add("largestEmptyCircle", 1, 1, Result::typeGeometry, catConst,
        "compute radius line of largest empty circle of geometry up to a distance tolerance",
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            (void) geomB; (void)d;  // prevent unused variable warning
            geos::algorithm::construct::LargestEmptyCircle lec( geom.get(), d );
            std::unique_ptr<Geometry> res = lec.getRadiusLine();
            return new Result( std::move(res) );
        });

    add("maxInscribedCircle", 1, 1, Result::typeGeometry, catConst,
        "compute maximum inscribed circle radius of Polygon up to a distance tolerance",
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            (void) geomB; (void)d;  // prevent unused variable warning
            geos::algorithm::construct::MaximumInscribedCircle mc( geom.get(), d );
            std::unique_ptr<Geometry> res = mc.getRadiusLine();
            return new Result( std::move(res) );
        });
    add("minBoundingCircle", Result::typeGeometry, catConst,
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            (void) geomB; (void)d;  // prevent unused variable warning
            geos::algorithm::MinimumBoundingCircle mc( geom.get() );
            std::unique_ptr<Geometry> res = mc.getCircle();
            return new Result( std::move(res) );
        });


    add("delaunay", 1, 0, Result::typeGeometry, catConst,
        "compute the Delaunay Triangulation of geometry vertices",
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            (void) geomB; (void)d;  // prevent unused variable warning
            geos::triangulate::DelaunayTriangulationBuilder builder;
            builder.setTolerance(0);
            builder.setSites( *geom );

            Geometry* out = builder.getTriangles(*(geom->getFactory())).release();

            std::vector<std::unique_ptr<const Geometry>> geoms;
            for(unsigned int i = 0; i < out->getNumGeometries(); i++) {
                geoms.push_back( std::unique_ptr< const Geometry>( out->getGeometryN(i) ) );
            }
            return new Result( std::move(geoms) ) ;
        });
    add("constrainedDelaunay", 1, 0, Result::typeGeometry, catConst,
        "constrained Delauanay triangulation of polygonal geometries",
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            (void)geomB;  (void)d; // prevent unused variable warning
            return new Result( geos::triangulate::polygon::ConstrainedDelaunayTriangulator::triangulate(geom.get()) );
         });

    add("voronoi", 1, 0, Result::typeGeometry, catConst,
        "Voronoi Diagram of geometry vertices",
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            (void) geomB; (void)d;  // prevent unused variable warning
            geos::triangulate::VoronoiDiagramBuilder builder;
            builder.setTolerance(0);
            builder.setSites( *geom );

            Geometry* out = builder.getDiagram(*(geom->getFactory())).release();

            std::vector<std::unique_ptr<const Geometry>> geoms;
            for(unsigned int i = 0; i < out->getNumGeometries(); i++) {
                geoms.push_back( std::unique_ptr< const Geometry>( out->getGeometryN(i) ) );
            }
            return new Result( std::move(geoms) ) ;
        });

    add("polygonize", Result::typeGeometry, catConst,
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            (void) geomB; (void)d;  // prevent unused variable warning
            geos::operation::polygonize::Polygonizer p;
            p.add(geom.get());

            std::vector<std::unique_ptr<Polygon>> polys = p.getPolygons();
            std::vector<std::unique_ptr<const Geometry>> geoms;
            for(unsigned int i = 0; i < polys.size(); i++) {
                geoms.push_back( std::move(polys[i]) );
            }
            return new Result( std::move(geoms) ) ;
        });
    add("buildArea", Result::typeGeometry, catConst,
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            (void) geomB; (void)d;  // prevent unused variable warning
            geos::operation::polygonize::BuildArea builder;
            auto result = builder.build(geom.get());
            return new Result( std::move(result) ) ;
        });
    add("simplifyDP", 1, 1, Result::typeGeometry, catConst,
        "simplify geometry using Douglas-Peucker with a distance tolerance",
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            (void)geomB;  // prevent unused variable warning
            return new Result( geos::simplify::DouglasPeuckerSimplifier::simplify(geom.get(), d) );
         });
    add("simplifyTP", 1, 1, Result::typeGeometry, catConst,
        "simplify geometry using Douglas-Peucker with a distance tolerance, preserving topology",
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            (void)geomB;  // prevent unused variable warning
            return new Result( geos::simplify::TopologyPreservingSimplifier::simplify(geom.get(), d) );
         });

//--------------------------------
    add("contains", 2, 0, Result::typeBool, catRel,
        "test if geometry A contains geometry B",
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            (void)d;  // prevent unused variable warning
            return new Result( geom->contains( geomB.get() ) );
        });
    add("covers", 2, 0, Result::typeBool, catRel,
        "test if geometry A covers geometry B",
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            (void)d;  // prevent unused variable warning
            return new Result( geom->covers( geomB.get() ) );
        });
    add("equals", 2, 0, Result::typeBool, catRel,
        "test if geometry A equals geometry B",
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            (void)d;  // prevent unused variable warning
            return new Result( geom->equals( geomB.get() ) );
        });
    add("intersects", 2, 0, Result::typeBool, catRel,
        "test if geometry A and B intersect",
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            (void)d;  // prevent unused variable warning
            return new Result( geom->intersects( geomB.get() ) );
        });
    add("relate", 2, 0, Result::typeString, catRel,
        "compute DE-9IM matrix for geometry A and B",
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            (void)d;  // prevent unused variable warning
            std::unique_ptr<geom::IntersectionMatrix> im(geom->relate( geomB.get() ));
            return new Result( im->toString() );
        });

    add("containsPrep", 2, 0, Result::typeBool, catRel,
        "test if geometry A contains geometry B, using PreparedGeometry",
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            (void)d;  // prevent unused variable warning
            return new Result( prepGeomCache.get(geom.get())->contains( geomB.get() ) );
        });
    add("containsProperlyPrep", 2, 0, Result::typeBool, catRel,
        "test if geometry A properly contains geometry B using PreparedGeometry",
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            (void)d;  // prevent unused variable warning
            return new Result( prepGeomCache.get(geom.get())->containsProperly( geomB.get() ) );
        });
    add("coversPrep", 2, 0, Result::typeBool, catRel,
        "test if geometry A covers geometry B using PreparedGeometry",
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            (void)d;  // prevent unused variable warning
            return new Result( prepGeomCache.get(geom.get())->covers( geomB.get() ) );
        });
    add("intersectsPrep", 2, 0, Result::typeBool, catRel,
        "test if geometry A intersects B using PreparedGeometry",
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            (void)d;  // prevent unused variable warning
            return new Result( prepGeomCache.get(geom.get())->intersects( geomB.get() ) );
        });

//----------------------------------------

    add("distance", 2, 0, Result::typeDouble, catDist,
        "compute distance between geometry A and B",
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            (void)d;  // prevent unused variable warning
            return new Result( geom->distance( geomB.get() ) );
        });
    add("nearestPoints", 2, 0, Result::typeGeometry, catDist,
        "compute a line containing the nearest points of geometry A and B",
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            (void)d;  // prevent unused variable warning
            std::unique_ptr<CoordinateSequence> cs = geos::operation::distance::DistanceOp::nearestPoints(geom.get(), geomB.get());
            auto factory = geom->getFactory();
            auto res = factory->createLineString( std::move(cs) );
            return new Result( std::move(res) );
        });
    add("frechetDistance", 2, 0, Result::typeDouble, catDist,
        "compute discrete Frechet distance between geometry A and B",
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            (void)d;  // prevent unused variable warning
            return new Result( geos::algorithm::distance::DiscreteFrechetDistance::distance(*geom, *geomB ) );
        });
        /*
        // MD - can't get this to work for now
     add("frechetDistanceLine", 2, 0, Result::typeGeometry, catDist,
        "computes a line indicating the discrete Frechet distance between geometry A and B",
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            (void)d;  // prevent unused variable warning
            DiscreteFrechetDistance dist(*geom, *geomB);
            //--- not supported for now
            //dist.setDensifyFraction(d);
            const std::array<geom::Coordinate, 2> ptArray = dist.getCoordinates();

            std::unique_ptr<std::vector<Coordinate>> pts(new std::vector<Coordinate>(2));
            (*pts)[0] = ptArray[0];
            (*pts)[1] = ptArray[1];
            //std::cout << ptArray[0] << std::endl;
            //std::cout << ptArray[1] << std::endl;
            auto cs = std::unique_ptr<CoordinateSequence>(new CoordinateArraySequence(pts.release()));

            auto factory = geom->getFactory();
            auto res = factory->createLineString( std::move(cs) );
            return new Result( std::move(res) );
        });
        */
    add("distancePrep", 2, 0, Result::typeDouble, catDist,
        "compute distance between geometry A and B using PreparedGeometry",
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            (void)d;  // prevent unused variable warning
            return new Result( prepGeomCache.get(geom.get())->distance( geomB.get() ) );
        });
    add("nearestPointsPrep", 2, 0, Result::typeGeometry, catDist,
        "compute a line containing the nearest points of geometry A and B using PreparedGeometry",
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            (void)d;  // prevent unused variable warning
            auto cs = prepGeomCache.get(geom.get())->nearestPoints( geomB.get() );
            auto factory = geom->getFactory();
            auto res = factory->createLineString( std::move(cs) );
            return new Result( std::move(res) );
        });

//----------------------------------------

    add("difference", 2, 0, Result::typeGeometry, catOverlay,
        "compute difference of geometry A from B",
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            (void)d;  // prevent unused variable warning
            return new Result( geom->difference( geomB.get() ) );
        });
    add("intersection", 2, 0, Result::typeGeometry, catOverlay,
        "compute intersection of geometry A and B",
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            (void)d;  // prevent unused variable warning
            return new Result( geom->intersection( geomB.get() ) );
        });
    add("symDifference", 2, 0, Result::typeGeometry, catOverlay,
        "compute symmetric difference of geometry A and B",
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            (void)d;  // prevent unused variable warning
            return new Result( geom->symDifference( geomB.get() ) );
        });
    add("unaryUnion", Result::typeGeometry, catOverlay,
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            (void)geomB; (void)d;  // prevent unused variable warning
            return new Result( geom->Union() );
        });
    add("union", 2, 0, Result::typeGeometry, catOverlay,
        "compute union of geometry A and B",
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            (void)d;  // prevent unused variable warning
            return new Result( geom->Union( geomB.get() ) );
        });
    add("unionCoverageNG", 1, 0, Result::typeGeometry, catOverlay,
        "union a polygonal coverage",
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            (void)geomB; (void)d;  // prevent unused variable warning
            return new Result( geos::operation::overlayng::CoverageUnion::geomunion(geom.get()) );
        });
    add("unionCoverage", 1, 0, Result::typeGeometry, catOverlay,
        "union a polygonal coverage",
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            (void)geomB; (void)d;  // prevent unused variable warning
            return new Result( geos::operation::geounion::CoverageUnion::Union(geom.get()) );
        });

    add("differenceSR", 2, 1, Result::typeGeometry, catOverlay,
        "compute difference of geometry A from B, snap-rounding to a precision scale factor",
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            geos::geom::PrecisionModel pm(d);
            return new Result( OverlayNG::overlay(geom.get(), geomB.get(), OverlayNG::DIFFERENCE, &pm) );
        });
    add("intersectionSR", 2, 1, Result::typeGeometry, catOverlay,
        "compute intersection of geometry A and B, snap-rounding to a precision scale factor",
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            geos::geom::PrecisionModel pm(d);
            return new Result( OverlayNG::overlay(geom.get(), geomB.get(), OverlayNG::INTERSECTION, &pm) );
        });
    add("symDifferenceSR", 2, 1, Result::typeGeometry, catOverlay,
        "compute symmetric difference of geometry A and B, snap-rounding to a precision scale factor",
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            geos::geom::PrecisionModel pm(d);
            return new Result( OverlayNG::overlay(geom.get(), geomB.get(), OverlayNG::SYMDIFFERENCE, &pm) );
        });
    add("unionSR", 2, 1, Result::typeGeometry, catOverlay,
        "compute union of geometry A and B, snap-rounding to a precision scale factor",
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            geos::geom::PrecisionModel pm(d);
            return new Result( OverlayNG::overlay(geom.get(), geomB.get(), OverlayNG::UNION, &pm) );
        });

    add("clipRect", 2, 0, Result::typeGeometry, catOverlay,
        "clip geometry A to envelope of B",
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            (void)d;  // prevent unused variable warning
            using geos::operation::intersection::Rectangle;
            using geos::operation::intersection::RectangleIntersection;
            const Envelope* env = geomB->getEnvelopeInternal();
            Rectangle rect(env->getMinX(), env->getMinY(), env->getMaxX(), env->getMaxY());
            return new Result( RectangleIntersection::clip( *geom, rect) );
        });

}

/* static */
GeomFunction*
GeomFunction::find(std::string name)
{
    if (registry.count(name) == 0)
        return nullptr;
    return registry[name];
}

/* static */
void
GeomFunction::add(std::string name, int resultType, std::string category, geomFunSig geomfun)
{
    add(name, 1, 0, resultType, category,
        "compute " + name + " of geometry",
        geomfun);
}

/* static */
void
GeomFunction::add(std::string name,
                    int nGeomParam,
                    int nParam,
                    int typeCode,
                    std::string category,
                    std::string desc,
                    geomFunSig geomfun)
{
    GeomFunction *fun = new GeomFunction(name, nGeomParam, nParam, typeCode,
        category, desc, geomfun );

    registry.insert( std::pair<std::string, GeomFunction *>(name, fun) );
    functionList.push_back(fun);
}

std::string GeomFunction::name()
{
    return funName;
}

bool GeomFunction::isBinary()
{
    return numGeomParam == 2;
}

std::string GeomFunction::signature() {
    std::string sig =  " A";
    sig += isBinary() ? " B" : "  ";
    sig += "  ";
    sig += funName;
    if (numParam > 0) sig += " N";
    sig += " >";
    sig += Result::code(resultType);
    return sig;
}

std::vector<std::string>
GeomFunction::list()
{
    std::vector<std::string> list;
    std::string cat = "";
    for (auto itr = functionList.begin(); itr != functionList.end(); ++itr) {
        auto fun = *itr;
        if (fun->category != cat) {
            list.push_back( fun->category + "  ------------------");
            cat = fun->category;
        }
        auto desc = fun->signature() + " - " + fun->description;
        // TODO: add display of function signature
        list.push_back( desc );
    }
    return list;
}

Result * GeomFunction::execute( const std::unique_ptr<Geometry>& geomA, const std::unique_ptr<Geometry>& geomB, double d )
{
    return geomfun( geomA, geomB, d );
}

//===============================================

Result::Result(bool val)
{
    valBool = val;
    typeCode = typeBool;
}

Result::Result(int  val)
{
    valInt = val;
    typeCode = typeInt;
}

Result::Result(double val)
{
    valDouble = val;
    typeCode = typeDouble;
}

Result::Result(std::string val)
{
    valStr = val;
    typeCode = typeString;
}

Result::Result(std::unique_ptr<geom::Geometry> val)
{
    valGeom = std::move(val);
    typeCode = typeGeometry;
}

Result::Result(Geometry * val)
{
    valGeom = std::unique_ptr<Geometry>(val);
    typeCode = typeGeometry;
}

Result::Result( std::vector<std::unique_ptr<const Geometry>> val )
{
    valGeomList = std::move(val);
    typeCode = typeGeomList;
}

Result::~Result()
{
}

bool
Result::isGeometry() {
    return typeCode == typeGeometry;
}

bool
Result::isGeometryList() {
    return typeCode == typeGeomList;
}

std::string
Result::toString() {
    std::stringstream converter;
    switch (typeCode) {
    case typeBool:
        converter << std::boolalpha << valBool;
        return converter.str();

    case typeInt:
        converter << valInt;
        return converter.str();

    case typeDouble:
        converter << valDouble;
        return converter.str();

    case typeString:
        return valStr;

    case typeGeometry:
        if (valGeom == nullptr)
            return "null";
        return valGeom->toString();

    case typeGeomList:
       return metadata();
    }
    return "Value for Unknonwn type";
}

std::string
Result::metadata() {
    switch (typeCode) {
    case typeBool:    return "bool";
    case typeInt:     return "int";
    case typeDouble:  return "double";
    case typeString:  return "string";

    case typeGeometry:
        if (valGeom == nullptr)
            return "null";
        return valGeom->getGeometryType() + "( " + std::to_string( valGeom->getNumPoints() ) + " )";

    case typeGeomList:
        return "Geometry[" + std::to_string( valGeomList.size()) + "]";
    }
    return "Unknonwn type";
}

std::string
Result::code(int code) {
    switch (code) {
    case typeBool:    return "B";
    case typeInt:     return "I";
    case typeDouble:  return "D";
    case typeString:  return "S";
    case typeGeometry: return "G";
    case typeGeomList: return "[G]";
    }
    return "U";
}
