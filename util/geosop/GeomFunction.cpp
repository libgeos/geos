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
#include <geos/geom/PrecisionModel.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/prep/PreparedGeometry.h>
#include <geos/geom/prep/PreparedGeometryFactory.h>
#include <geos/algorithm/construct/MaximumInscribedCircle.h>
#include <geos/algorithm/MinimumBoundingCircle.h>
#include <geos/geom/util/Densifier.h>
#include <geos/operation/linemerge/LineMerger.h>
#include <geos/operation/distance/DistanceOp.h>
#include <geos/operation/relate/RelateOp.h>
#include <geos/operation/valid/MakeValid.h>
#include <geos/operation/overlayng/OverlayNG.h>
#include <geos/operation/polygonize/Polygonizer.h>
#include <geos/precision/GeometryPrecisionReducer.h>
#include <geos/simplify/DouglasPeuckerSimplifier.h>
#include <geos/simplify/TopologyPreservingSimplifier.h>
#include <geos/triangulate/DelaunayTriangulationBuilder.h>
#include <geos/triangulate/VoronoiDiagramBuilder.h>

#include "GeomFunction.h"

#include <sstream>

using geos::operation::overlayng::OverlayNG;

/* static private */
std::map<std::string, GeomFunction*> GeomFunction::registry;

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

//static std::unique_ptr<const PreparedGeometry> prepGeomCache;
//static Geometry *cacheKey;

/* static */
void
GeomFunction::init()
{
    add("area",
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            return new Result( geom->getArea() );
        });
    add("boundary",
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            return new Result( geom->getBoundary() );
        });
    add("buffer", "computes the buffer of geometry A to a distance", 1, 1,
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            return new Result( geom->buffer( d ) );
        });
    add("centroid",
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            return new Result( geom->getCentroid() );
        });
    add("copy",
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            return new Result( geom->clone() );
        });
    add("convexHull",
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            return new Result( geom->convexHull() );
        });
    add("contains", "tests if geometry A contains geometry B", 2, 0,
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            return new Result( geom->contains( geomB.get() ) );
        });
    add("covers", "tests if geometry A covers geometry B", 2, 0,
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            return new Result( geom->covers( geomB.get() ) );
        });

    add("densify", "densifies geometry A to a distance ", 1, 1,
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            geom::util::Densifier densifier( geom.get() );
            densifier.setDistanceTolerance( d );
            return new Result( densifier.getResultGeometry() );
        });

    add("distance", "computes distance between geometry A and B", 2, 0,
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            return new Result( geom->distance( geomB.get() ) );
        });

     add("envelope",
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            return new Result( geom->getCentroid() );
        });

    add("interiorPoint",
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            return new Result( geom->getInteriorPoint() );
        });

    add("intersects", "tests if geometry A and B intersect", 2, 0,
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            return new Result( geom->intersects( geomB.get() ) );
        });

    add("isEmpty", "tests if geometry A is empty", 1, 0,
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            return new Result( geom->isEmpty() );
        });

    add("isSimple", "tests if geometry A is simple", 1, 0,
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            return new Result( geom->isSimple() );
        });

    add("isValid", "tests if geometry A is valid", 1, 0,
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            return new Result( geom->isValid() );
        });

    add("length",
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            return new Result( geom->getLength() );
        });

    add("makeValid",
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            return new Result( geos::operation::valid::MakeValid().build( geom.get() ) );
        });

    add("maxInscribedCircle", "computes maximum inscribed circle radius of Polygon A up to a distance tolerance", 2, 0,
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            geos::algorithm::construct::MaximumInscribedCircle mc( geom.get(), d );
            std::unique_ptr<Geometry> res = mc.getRadiusLine();
            return new Result( std::move(res) );
        });
    add("minBoundingCircle",
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            geos::algorithm::MinimumBoundingCircle mc( geom.get() );
            std::unique_ptr<Geometry> res = mc.getCircle();
            return new Result( std::move(res) );
        });

    add("nearestPoints", "computes a line containing the nearest points of geometry A and B", 2, 0,
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            std::unique_ptr<CoordinateSequence> cs = geos::operation::distance::DistanceOp::nearestPoints(geom.get(), geomB.get());
            auto factory = geom->getFactory();
            auto res = factory->createLineString( std::move(cs) );
            return new Result( std::move(res) );
        });
    add("normalize", "normalizes geometry A", 1, 0,
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            auto res = geom->clone();
            res->normalize();
            return new Result( std::move(res) );
        });

    add("lineMerge", "merges the lines of geometry A", 1, 0,
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            geos::operation::linemerge::LineMerger lmrgr;
            lmrgr.add(geom.get());

            std::vector<std::unique_ptr<LineString>> lines = lmrgr.getMergedLineStrings();

            std::vector<std::unique_ptr<const Geometry>> geoms;
            for(unsigned int i = 0; i < lines.size(); i++) {
                geoms.push_back( std::move(lines[i]) );
            }
            return new Result( std::move(geoms) ) ;
        });

    add("delaunay", "computes the Delaunay Triangulation of geometry A vertices", 1, 0,
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
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

    add("voronoi", "computes the Voronoi Diagram of geometry A vertices", 1, 0,
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
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

    add("polygonize",
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            geos::operation::polygonize::Polygonizer p;
            p.add(geom.get());

            std::vector<std::unique_ptr<Polygon>> polys = p.getPolygons();
            std::vector<std::unique_ptr<const Geometry>> geoms;
            for(unsigned int i = 0; i < polys.size(); i++) {
                geoms.push_back( std::move(polys[i]) );
            }
            return new Result( std::move(geoms) ) ;
        });

    add("reducePrecision", "reduces precision of geometry to a precision scale factor", 1, 1,
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            PrecisionModel pm(d);
            return new Result( geos::precision::GeometryPrecisionReducer::reduce( *geom, pm ) );
        });
    add("relate", "computes DE-9IM matrix for geometry A and B", 2, 0,
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            std::unique_ptr<geom::IntersectionMatrix> im(geom->relate( geomB.get() ));
            return new Result( im->toString() );
        });
    add("reverse", "reverses geometry A", 1, 0,
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            return new Result( geom->reverse() );
        });
    add("simplifyDP", "simplifies geometry A using Douglas-Peucker with a distance tolerance", 1, 1,
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            return new Result( geos::simplify::DouglasPeuckerSimplifier::simplify(geom.get(), d) );
         });
    add("simplifyTP", "simplifies geometry A using Douglas-Peucker with a distance tolerance, preserving topology", 1, 1,
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            return new Result( geos::simplify::TopologyPreservingSimplifier::simplify(geom.get(), d) );
         });


    add("containsPrep", "tests if geometry A contains geometry B, using PreparedGeometry", 2, 0,
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            return new Result( prepGeomCache.get(geom.get())->contains( geomB.get() ) );
        });
    add("containsProperlyPrep", "tests if geometry A properly contains geometry B using PreparedGeometry", 2, 0,
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            return new Result( prepGeomCache.get(geom.get())->containsProperly( geomB.get() ) );
        });
    add("coversPrep", "tests if geometry A covers geometry B using PreparedGeometry", 2, 0,
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            return new Result( prepGeomCache.get(geom.get())->covers( geomB.get() ) );
        });
    add("intersectsPrep", "tests if geometry A intersects B using PreparedGeometry", 2, 0,
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            return new Result( prepGeomCache.get(geom.get())->intersects( geomB.get() ) );
        });

    add("distancePrep", "computes distance between geometry A and B using PreparedGeometry", 2, 0,
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            return new Result( prepGeomCache.get(geom.get())->distance( geomB.get() ) );
        });
    add("nearestPointsPrep", "computes a line containing the nearest points of geometry A and B using PreparedGeometry", 2, 0,
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            auto cs = prepGeomCache.get(geom.get())->nearestPoints( geomB.get() );
            auto factory = geom->getFactory();
            auto res = factory->createLineString( std::move(cs) );
            return new Result( std::move(res) );
        });



    add("difference", "computes difference of geometry A from B", 2, 0,
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            return new Result( geom->difference( geomB.get() ) );
        });
    add("intersection", "computes intersection of geometry A and B", 2, 0,
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            return new Result( geom->intersection( geomB.get() ) );
        });
    add("symDifference", "computes symmetric difference of geometry A and B", 2, 0,
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            return new Result( geom->symDifference( geomB.get() ) );
        });
    add("unaryUnion",
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            return new Result( geom->Union() );
        });
    add("union", "computes union of geometry A and B", 2, 0,
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            return new Result( geom->Union( geomB.get() ) );
        });

    add("differenceSR", "computes difference of geometry A from B, snap-rounding to a precision scale factor", 2, 1,
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            geom::PrecisionModel pm(d);
            return new Result( OverlayNG::overlay(geom.get(), geomB.get(), OverlayNG::DIFFERENCE, &pm) );
        });
    add("intersectionSR", "computes intersection of geometry A and B, snap-rounding to a precision scale factor", 2, 1,
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            geom::PrecisionModel pm(d);
            return new Result( OverlayNG::overlay(geom.get(), geomB.get(), OverlayNG::INTERSECTION, &pm) );
        });
    add("symDifferenceSR", "computes symmetric difference of geometry A and B, snap-rounding to a precision scale factor", 2, 1,
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            geom::PrecisionModel pm(d);
            return new Result( OverlayNG::overlay(geom.get(), geomB.get(), OverlayNG::SYMDIFFERENCE, &pm) );
        });
    add("unionSR", "computes union of geometry A and B, snap-rounding to a precision scale factor", 2, 1,
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            geom::PrecisionModel pm(d);
            return new Result( OverlayNG::overlay(geom.get(), geomB.get(), OverlayNG::UNION, &pm) );
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
GeomFunction::add(std::string name, geomFunSig geomfun)
{
    add(name,
        "computes " + name + " for geometry A",
        1, 0, geomfun);
}

/* static */
void
GeomFunction::add(std::string name,
                    std::string desc,
                    int nGeomParam,
                    int nParam,
                    geomFunSig geomfun)
{
    auto fun = new GeomFunction();
    fun->funName = name;
    fun->description = desc;
    fun->geomfun = geomfun;
    fun->numGeomParam = nGeomParam;
    fun->numParam = nParam;

    registry.insert( std::pair<std::string, GeomFunction *>(name, fun) );
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
    std::string sig = funName + " A";
    if (isBinary()) {
        sig += " B";
    }
    if (numParam > 0) sig += " N";
    return sig;
}

std::vector<std::string>
GeomFunction::list()
{
    std::vector<std::string> list;
    for (auto itr = registry.begin(); itr != registry.end(); ++itr) {
        auto fun = itr->second;
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
