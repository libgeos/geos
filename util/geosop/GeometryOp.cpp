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
#include <geos/algorithm/Orientation.h>
#include <geos/algorithm/construct/LargestEmptyCircle.h>
#include <geos/algorithm/construct/MaximumInscribedCircle.h>
#include <geos/algorithm/BoundaryNodeRule.h>
#include <geos/algorithm/MinimumAreaRectangle.h>
#include <geos/algorithm/MinimumDiameter.h>
#include <geos/algorithm/MinimumBoundingCircle.h>
#include <geos/algorithm/distance/DiscreteHausdorffDistance.h>
#include <geos/algorithm/distance/DiscreteFrechetDistance.h>
#include <geos/algorithm/hull/ConcaveHull.h>
#include <geos/geom/util/Densifier.h>
#include <geos/geom/util/GeometryFixer.h>
#include <geos/linearref/LengthIndexedLine.h>
#include <geos/noding/GeometryNoder.h>
#include <geos/operation/buffer/BufferBuilder.h>
#include <geos/operation/buffer/BufferOp.h>
#include <geos/operation/buffer/BufferParameters.h>
#include <geos/operation/buffer/OffsetCurve.h>
#include <geos/operation/cluster/GeometryDistanceClusterFinder.h>
#include <geos/operation/cluster/GeometryIntersectsClusterFinder.h>
#include <geos/coverage/CoverageCleaner.h>
#include <geos/coverage/CoverageSimplifier.h>
#include <geos/coverage/CoverageValidator.h>
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
#include <geos/operation/overlayng/UnaryUnionNG.h>
#include <geos/operation/relate/RelateOp.h>
#include <geos/operation/union/CoverageUnion.h>
#include <geos/precision/GeometryPrecisionReducer.h>
#include <geos/simplify/DouglasPeuckerSimplifier.h>
#include <geos/simplify/TopologyPreservingSimplifier.h>
#include <geos/triangulate/DelaunayTriangulationBuilder.h>
#include <geos/triangulate/VoronoiDiagramBuilder.h>
#include <geos/triangulate/polygon/ConstrainedDelaunayTriangulator.h>

#include "GeometryOp.h"

#include <sstream>
#include <iomanip>

// Quiet MSVC warning we cannot fix without
// a bunch of function renaming,
// see https://github.com/libgeos/geos/issues/929
#ifdef _MSC_VER
#pragma warning (disable : 4573)
#endif

using geos::operation::overlayng::OverlayNG;
using geos::algorithm::distance::DiscreteFrechetDistance;
using geos::operation::relate::RelateOp;
using geos::algorithm::BoundaryNodeRule;

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

const std::string catCluster = "Clustering";
const std::string catConst = "Construction";
const std::string catCoverage = "Coverage";
const std::string catDist = "Distance";
const std::string catGeom = "Geometry";
const std::string catLinearref = "Linear Referencing";
const std::string catMetric = "Metric";
const std::string catOverlay = "Overlay";
const std::string catRel = "Spatial Relationship";
const std::string catValid = "Validity";

//-- GeometryOps are created lazily via a function
struct GeometryOpCreator {
    std::string name;
    std::function<GeometryOp *( std::string name )> create;
} ;

std::vector<const Geometry*> toList(const Geometry& geom) {
    std::vector<const Geometry*> geomList;
    for (std::size_t i = 0; i < geom.getNumGeometries(); i++) {
        geomList.emplace_back( geom.getGeometryN(i));
    }
    return geomList;
}

/*
* Static array of operation definitions.
* All metadata for an operation is defined here.
* Operation objects are created on-demand via a
* lambda, for efficiency.
*
* To add an operation, add an entry to this array.
*
* Operations are sorted in category and then "logical" order.
* This makes it easy to read and add operations.
* It is a bit slower for lookup, but that is only done once per run.
*/
std::vector<GeometryOpCreator> opRegistry {

//=============  category: Geometry  ==================

{ "copy", [](std::string name) { return GeometryOp::create(name,
    catGeom, "copy geometry",
    [](const Geometry& geom) {
        return new Result( geom.clone() );
    });
}},
{ "envelope", [](std::string name) { return GeometryOp::create(name,
    catGeom, "envelope of geometry",
    [](const Geometry& geom) {
        return new Result( geom.getEnvelope() );
    });
}},
{"hasZ", [](std::string name) { return GeometryOp::create(name,
    catGeom,
    "test if geometry has Z ordinate",
    Result::typeBool,
    [](const Geometry& geom) {
        return new Result( geom.hasZ() );
    });
}},
{"hasM", [](std::string name) { return GeometryOp::create(name,
    catGeom,
    "test if geometry has M ordinate",
    Result::typeBool,
    [](const Geometry& geom) {
        return new Result( geom.hasM() );
    });
}},
{"isEmpty", [](std::string name) { return GeometryOp::create(name,
    catGeom, "test if geometry is empty",
    Result::typeBool,
    [](const Geometry& geom) {
        return new Result( geom.isEmpty() );
    });
}},
{"lineMerge",  [](std::string name) { return GeometryOp::create(name,
    catGeom,
    "merge the lines of geometry",
        [](const Geometry& geom) {
            geos::operation::linemerge::LineMerger lmrgr;
            lmrgr.add(&geom);

            std::vector<std::unique_ptr<LineString>> lines = lmrgr.getMergedLineStrings();

            std::vector<std::unique_ptr<const Geometry>> geoms;
            for(unsigned int i = 0; i < lines.size(); i++) {
                geoms.push_back( std::move(lines[i]) );
            }
            return new Result( std::move(geoms) ) ;
        });
}},
{"normalize", [](std::string name) { return GeometryOp::create(name,
    catGeom, "normalize geometry",
    [](const Geometry& geom) {
        auto res = geom.clone();
        res->normalize();
        return new Result( std::move(res) );
    });
}},
{"reducePrecision", [](std::string name) { return GeometryOp::create(name,
    catGeom,
    "reduce precision of geometry to a precision scale factor",
    [](const Geometry& geom, double d) {
        PrecisionModel pm(d);
        return new Result( geos::precision::GeometryPrecisionReducer::reduce( geom, pm ) );
    });
}},
{"reducePrecisionKeepCollapsed", [](std::string name) { return GeometryOp::create(name,
    catGeom,
    "reduce precision of geometry to a precision scale factor",
    [](const Geometry& geom, double d) {
        PrecisionModel pm(d);
        return new Result( geos::precision::GeometryPrecisionReducer::reduceKeepCollapsed( geom, pm ) );
    });
}},
{"reducePrecisionPointwise", [](std::string name) { return GeometryOp::create(name,
    catGeom,
    "reduce precision of geometry to a precision scale factor",
    [](const Geometry& geom, double d) {
        PrecisionModel pm(d);
        return new Result( geos::precision::GeometryPrecisionReducer::reducePointwise( geom, pm ) );
    });
}},
{"reverse", [](std::string name) { return GeometryOp::create(name,
    catGeom,
    "reverse geometry",
    [](const Geometry& geom) {
        return new Result( geom.reverse() );
    });
}},
//=============  category: Metric  ==================
{"area", [](std::string name) { return GeometryOp::create(name,
    catMetric, "area of geometry",
    Result::typeDouble,
    [](const Geometry& geom) {
        return new Result( geom.convexHull() );
    });
}},
{"length", [](std::string name) { return GeometryOp::create(name,
    catMetric, "length or perimeter of geometry",
    Result::typeDouble,
    [](const Geometry& geom) {
        return new Result( geom.getLength() );
    });
}},

//=============  category: Validation  ==================

{"isSimple", [](std::string name) { return GeometryOp::create(name,
    catValid,
    "test if geometry is simple",
    Result::typeBool,
    [](const Geometry& geom) {
        return new Result( geom.isSimple() );
    });
}},
{"isValid", [](std::string name) { return GeometryOp::create(name,
    catValid,
    "test if geometry is valid",
    Result::typeBool,
    [](const Geometry& geom) {
        return new Result( geom.isValid() );
    });
}},
{"fixInvalid", [](std::string name) { return GeometryOp::create(name,
    catValid,
    "fix invalid geometry to be valid",
    [](const Geometry& geom) {
        return new Result( geos::geom::util::GeometryFixer::fix( &geom ) );
    });
}},
{"makeValid", [](std::string name) { return GeometryOp::create(name,
    catValid,
    "make geometry valid (original algorithm)",
    [](const Geometry& geom) {
        return new Result( geos::operation::valid::MakeValid().build( &geom ) );
    });
}},

//=============  category: Construction  ==================

{"boundary", [](std::string name) { return GeometryOp::create(name,
    catConst,
    "compute geometry boundary",
    [](const Geometry& geom) {
        return new Result( geom.convexHull() );
    });
    }},
{"buffer", [](std::string name) { return GeometryOp::create(name,
    catConst,
    "compute the buffer of geometry by a distance",
    [](const Geometry& geom, double d) {
        return new Result( geom.buffer( d ) );
    });
    }},
{"bufferQuadSegs", [](std::string name) { return GeometryOp::create(name,
    catConst,
    "compute the buffer of geometry by a distance with quadrant segments",
    [](const Geometry& geom, double d, double quadrantSegments) {
        return new Result( geom.buffer( d, (int) quadrantSegments ) );
    });
    }},
{"bufferJoin", [](std::string name) { return GeometryOp::create(name,
    catConst,
    "compute the buffer of geometry by a distance, with join >0 = QS, 0 = Bevel, <0 = Mitre limit",
    [](const Geometry& geom, double d, double join) {
        geos::operation::buffer::BufferParameters param;
        if (join > 0) {
            param.setQuadrantSegments( (int) join );
            param.setJoinStyle(geos::operation::buffer::BufferParameters::JOIN_ROUND);
        }
        else if (join == 0) {
            param.setJoinStyle(geos::operation::buffer::BufferParameters::JOIN_BEVEL);
        }
        else if (join < 0) {
            param.setJoinStyle(geos::operation::buffer::BufferParameters::JOIN_MITRE);
            param.setMitreLimit( (int) -join );
        }
        std::unique_ptr<Geometry> g3 = geos::operation::buffer::BufferOp::bufferOp(&geom, d, param);
        return new Result( g3.release() );
    });
    }},
{"bufferSingleSided", [](std::string name) { return GeometryOp::create(name,
    catConst,
    "compute the buffer of geometry by a distance with the single-sided option",
    [](const Geometry& geom, double d) {
        geos::operation::buffer::BufferParameters param;
        param.setSingleSided( true );
        std::unique_ptr<Geometry> g3 = geos::operation::buffer::BufferOp::bufferOp(&geom, d, param);
        return new Result( g3.release() );
    });
    }},
{"offsetCurve", [](std::string name) { return GeometryOp::create(name,
    catConst,
    "compute the offset curve of geometry by a distance",
    [](const Geometry& geom, double d) {
        geos::operation::buffer::OffsetCurve oc(geom, d);
        std::unique_ptr<Geometry> g3 = oc.getCurve();
        return new Result( g3.release() );
    });
    }},
{"offsetCurveJoin", [](std::string name) { return GeometryOp::create(name,
    catConst,
    "compute the offset curve of geometry by a distance, with join >0 = QS, 0 = Bevel, <0 = Mitre limit",
    [](const Geometry& geom, double d, double join) {
        int quadSegs = 0;
        geos::operation::buffer::BufferParameters::JoinStyle joinStyle = geos::operation::buffer::BufferParameters::JOIN_ROUND;
        double miterLimit = 0;
        if (join > 0) {
            quadSegs = (int) join;
        }
        else if (join == 0) {
            joinStyle = geos::operation::buffer::BufferParameters::JOIN_BEVEL;
        }
        else if (join < 0) {
            joinStyle = geos::operation::buffer::BufferParameters::JOIN_MITRE;
            miterLimit = (int) -join;
        }
        std::unique_ptr<Geometry> g3 = geos::operation::buffer::OffsetCurve::getCurve(geom, d, quadSegs, joinStyle, miterLimit);
        return new Result( g3.release() );
    });
    }},
{"OLDoffsetCurve", [](std::string name) { return GeometryOp::create(name,
    catConst,
    "compute the offset curve of geometry by a distance",
    [](const Geometry& geom, double d) {
        geos::operation::buffer::BufferParameters bp;

        bool isLeftSide = true;
        if(d < 0) {
            isLeftSide = false;
            d = -d;
        }
        geos::operation::buffer::BufferBuilder bufBuilder(bp);
        return new Result( bufBuilder.bufferLineSingleSided(&geom, d, isLeftSide) );
    });
    }},
{"centroid", [](std::string name) { return GeometryOp::create(name,
    catConst,
    "compute geometry centroid",
    [](const Geometry& geom) {
        return new Result( geom.getCentroid() );
    });
    }},
{"convexHull", [](std::string name) { return GeometryOp::create(name,
    catConst,
    "compute geometry convex hull",
    [](const Geometry& geom) {
        return new Result( geom.convexHull() );
    });
    }},
{"concaveHull", [](std::string name) { return GeometryOp::create(name,
    catConst,
    "compute geometry concave hull for Edge Length Ratio",
    [](const Geometry& geom, double d) {
        geos::algorithm::hull::ConcaveHull hull(&geom);
        hull.setMaximumEdgeLengthRatio( d );
        return new Result( hull.getHull() );
    });
    }},
{"concaveHullByLength", [](std::string name) { return GeometryOp::create(name,
    catConst,
    "compute geometry concave hull for Edge Length",
    [](const Geometry& geom, double d) {
        geos::algorithm::hull::ConcaveHull hull(&geom);
        hull.setMaximumEdgeLength( d );
        return new Result( hull.getHull() );
    });
    }},
{"concaveHullHoles", [](std::string name) { return GeometryOp::create(name,
    catConst,
    "compute geometry concave hull allowing holes for Edge Length Ratio",
    [](const Geometry& geom, double d) {
        geos::algorithm::hull::ConcaveHull hull(&geom);
        hull.setMaximumEdgeLengthRatio( d );
        hull.setHolesAllowed(true);
        return new Result( hull.getHull() );
    });
    }},
{"densify", [](std::string name) { return GeometryOp::create(name,
    catConst,
    "densify geometry to a segment length ",
    [](const Geometry& geom, double d) {
        geos::geom::util::Densifier densifier( &geom );
        densifier.setDistanceTolerance( d );
        return new Result( densifier.getResultGeometry() );
    });
}},
{"interiorPoint", [](std::string name) { return GeometryOp::create(name,
    catConst,
    "compute interior point of geometry",
    [](const Geometry& geom) {
        return new Result( geom.getInteriorPoint() );
    });
}},
{"largestEmptyCircle", [](std::string name) { return GeometryOp::create(name,
    catConst,
    "compute radius line of largest empty circle between obstacles, up to a distance tolerance",
    [](const Geometry& geom, double d) {
        geos::algorithm::construct::LargestEmptyCircle lec( &geom, d );
        std::unique_ptr<Geometry> res = lec.getRadiusLine();
        return new Result( std::move(res) );
    });
}},
{"largestEmptyCircleBdy", [](std::string name) { return GeometryOp::create(name,
    catConst,
    "compute radius line of largest empty circle between obstacles with center in a boundary, up to a distance tolerance",
    [](const Geometry& geom, const Geometry& geomB, double d) {
        geos::algorithm::construct::LargestEmptyCircle lec( &geom, &geomB, d );
        std::unique_ptr<Geometry> res = lec.getRadiusLine();
        return new Result( std::move(res) );
    });
}},
{"maxInscribedCircle", [](std::string name) { return GeometryOp::create(name,
    catConst,
    "compute maximum inscribed circle radius of Polygon up to a distance tolerance",
    [](const Geometry& geom, double d) {
        geos::algorithm::construct::MaximumInscribedCircle mc( &geom, d );
        std::unique_ptr<Geometry> res = mc.getRadiusLine();
        return new Result( std::move(res) );
    });
    }},
{"minAreaRectangle", [](std::string name) { return GeometryOp::create(name,
    catConst,
    "compute minimum-area rectangle enclosing geometry",
    [](const Geometry& geom) {
        std::unique_ptr<Geometry> res = geos::algorithm::MinimumAreaRectangle::getMinimumRectangle(&geom);
        return new Result( std::move(res) );
    });
    }},
{"minBoundingCircle", [](std::string name) { return GeometryOp::create(name,
    catConst,
    "compute minimum bounding circle of geometry",
    [](const Geometry& geom) {
        geos::algorithm::MinimumBoundingCircle mc( &geom );
        std::unique_ptr<Geometry> res = mc.getCircle();
        return new Result( std::move(res) );
    });
    }},
{"maxDiameter", [](std::string name) { return GeometryOp::create(name,
   catConst,
    "compute maximum diameter line of geometry",
    [](const Geometry& geom) {
        geos::algorithm::MinimumBoundingCircle mc( &geom );
        std::unique_ptr<Geometry> res = mc.getMaximumDiameter();
        return new Result( std::move(res) );
    });
    }},
{"minDiameter", [](std::string name) { return GeometryOp::create(name,
    catConst,
    "compute minimum diameter line of geometry",
    [](const Geometry& geom) {
        geos::algorithm::MinimumDiameter md( &geom );
        std::unique_ptr<Geometry> res = md.getDiameter();
        return new Result( std::move(res) );
    });
}},

{"delaunay", [](std::string name) { return GeometryOp::create(name,
    catConst,
    "compute the Delaunay Triangulation of geometry vertices",
    [](const Geometry& geom) {
        geos::triangulate::DelaunayTriangulationBuilder builder;
        builder.setTolerance(0);
        builder.setSites( geom );

        Geometry* out = builder.getTriangles(*(geom.getFactory())).release();

        std::vector<std::unique_ptr<const Geometry>> geoms;
        for(unsigned int i = 0; i < out->getNumGeometries(); i++) {
            geoms.push_back( std::unique_ptr< const Geometry>( out->getGeometryN(i) ) );
        }
        return new Result( std::move(geoms) ) ;
    });
    }},
{"constrainedDelaunay", [](std::string name) { return GeometryOp::create(name,
    catConst,
    "constrained Delauanay triangulation of polygonal geometries",
    [](const Geometry& geom) {
        return new Result( geos::triangulate::polygon::ConstrainedDelaunayTriangulator::triangulate(&geom) );
        });
}},
{"voronoi", [](std::string name) { return GeometryOp::create(name,
    catConst,
    "Voronoi Diagram of geometry vertices",
    [](const Geometry& geom) {
        geos::triangulate::VoronoiDiagramBuilder builder;
        builder.setTolerance(0);
        builder.setSites( geom );

        Geometry* out = builder.getDiagram(*(geom.getFactory())).release();

        std::vector<std::unique_ptr<const Geometry>> geoms;
        for(unsigned int i = 0; i < out->getNumGeometries(); i++) {
            geoms.push_back( std::unique_ptr< const Geometry>( out->getGeometryN(i) ) );
        }
        return new Result( std::move(geoms) ) ;
    });
}},
{"polygonize", [](std::string name) { return GeometryOp::create(name,
    catConst,
    "polygonize lines",
    [](const Geometry& geom) {
        geos::operation::polygonize::Polygonizer p;
        p.add(&geom);

        std::vector<std::unique_ptr<Polygon>> polys = p.getPolygons();
        std::vector<std::unique_ptr<const Geometry>> geoms;
        for(unsigned int i = 0; i < polys.size(); i++) {
            geoms.push_back( std::move(polys[i]) );
        }
        return new Result( std::move(geoms) ) ;
    });
    }},
{"polygonizeValid", [](std::string name) { return GeometryOp::create(name,
    catConst,
    "polygonize lines into a valid polygonal geometry",
    [](const Geometry& geom) {
        geos::operation::polygonize::Polygonizer p(true);
        p.add(&geom);

        std::vector<std::unique_ptr<Polygon>> polys = p.getPolygons();
        std::vector<std::unique_ptr<const Geometry>> geoms;
        for(unsigned int i = 0; i < polys.size(); i++) {
            geoms.push_back( std::move(polys[i]) );
        }
        return new Result( std::move(geoms) ) ;
    });
    }},
{"buildArea", [](std::string name) { return GeometryOp::create(name,
    catConst,
    "build area from lines",
    [](const Geometry& geom) {
        geos::operation::polygonize::BuildArea builder;
        auto result = builder.build(&geom);
        return new Result( std::move(result) ) ;
    });
    }},
{"simplifyDP", [](std::string name) { return GeometryOp::create(name,
    catConst,
    "simplify geometry using Douglas-Peucker by a distance tolerance",
    [](const Geometry& geom, double d) {
        return new Result( geos::simplify::DouglasPeuckerSimplifier::simplify(&geom, d) );
        });
        }},
{"simplifyTP", [](std::string name) { return GeometryOp::create(name,
    catConst,
    "simplify geometry using Douglas-Peucker with a distance tolerance, preserving topology",
    [](const Geometry& geom, double d) {
        return new Result( geos::simplify::TopologyPreservingSimplifier::simplify(&geom, d) );
        });
}},
//=============  category: Distance  ==================

{"distance", [](std::string name) { return GeometryOp::create(name,
    catDist,
    "compute distance between geometry A and B",
    Result::typeDouble,
    [](const Geometry& geom, const Geometry& geomB) {
        return new Result( geom.distance( &geomB) );
    });
    }},
{"nearestPoints", [](std::string name) { return GeometryOp::create(name,
    catDist,
    "compute a line containing the nearest points of geometry A and B",
    [](const Geometry& geom, const Geometry& geomB) {
        std::unique_ptr<CoordinateSequence> cs = geos::operation::distance::DistanceOp::nearestPoints(&geom, &geomB);
        auto factory = geom.getFactory();
        auto res = factory->createLineString( std::move(cs) );
        return new Result( std::move(res) );
    });
    }},
{"frechetDistance", [](std::string name) { return GeometryOp::create(name,
    catDist,
    "compute discrete Frechet distance between geometry A and B",
    Result::typeDouble,
    [](const Geometry& geom, const Geometry& geomB) {
       return new Result( geos::algorithm::distance::DiscreteFrechetDistance::distance(geom, geomB ) );
    });
    }},
{"hausdorffDistance", [](std::string name) { return GeometryOp::create(name,
    catDist,
    "compute discrete Hausdorff distance between geometry A and B",
    Result::typeDouble,
    [](const Geometry& geom, const Geometry& geomB) {
        return new Result( geos::algorithm::distance::DiscreteHausdorffDistance::distance(geom, geomB ) );
    });
    }},
    /*
    // MD - can't get this to work for now
    add("frechetDistanceLine", 2, 0, Result::typeGeometry, catDist,
    "computes a line indicating the discrete Frechet distance between geometry A and B",
    [](const Geometry& geom, const Geometry& geomB, double d) {
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
        auto cs = std::unique_ptr<CoordinateSequence>(new CoordinateSequence(pts.release()));

        auto factory = geom->getFactory();
        auto res = factory->createLineString( std::move(cs) );
        return new Result( std::move(res) );
    });
    */
{"distancePrep", [](std::string name) { return GeometryOp::create(name,
    catDist,
    "compute distance between geometry A and B using PreparedGeometry",
    Result::typeDouble,
    [](const Geometry& geom, const Geometry& geomB) {
        return new Result( prepGeomCache.get(&geom)->distance( &geomB ) );
    });
    }},
{"nearestPointsPrep", [](std::string name) { return GeometryOp::create(name,
    catDist,
    "compute a line containing the nearest points of geometry A and B using PreparedGeometry",
    [](const Geometry& geom, const Geometry& geomB) {
        auto cs = prepGeomCache.get(&geom)->nearestPoints( &geomB );
        auto factory = geom.getFactory();
        auto res = factory->createLineString( std::move(cs) );
        return new Result( std::move(res) );
    });
}},

//=============  category: Spatial Relationship  ==================

{ "contains", [](std::string name) { return GeometryOp::create(name,
    catRel, "test if geometry A contains geometry B",
    Result::typeBool,
    [](const Geometry& geom, const Geometry& geomB) {
        return new Result( geom.contains( &geomB ) );
    });
}},
{ "coveredBy", [](std::string name) { return GeometryOp::create(name,
    catRel, "test if geometry A is covered by geometry B",
    Result::typeBool,
    [](const Geometry& geom, const Geometry& geomB) {
        return new Result( geom.coveredBy( &geomB ) );
    });
}},
{ "covers", [](std::string name) { return GeometryOp::create(name,
    catRel, "test if geometry A covers geometry B",
    Result::typeBool,
    [](const Geometry& geom, const Geometry& geomB) {
        return new Result( geom.covers( &geomB ) );
    });
}},
{ "crosses", [](std::string name) { return GeometryOp::create(name,
    catRel, "test if geometry A crosses geometry B",
    Result::typeBool,
    [](const Geometry& geom, const Geometry& geomB) {
        return new Result( geom.crosses( &geomB ) );
    });
}},
{ "disjoint", [](std::string name) { return GeometryOp::create(name,
    catRel, "test if geometry A is disjoint from geometry B",
    Result::typeBool,
    [](const Geometry& geom, const Geometry& geomB) {
        return new Result( geom.disjoint( &geomB ) );
    });
}},
{ "equals", [](std::string name) { return GeometryOp::create(name,
    catRel, "test if geometry A equals geometry B",
    Result::typeBool,
    [](const Geometry& geom, const Geometry& geomB) {
        return new Result( geom.equals( &geomB ) );
    });
}},
{ "intersects", [](std::string name) { return GeometryOp::create(name,
    catRel, "test if geometry A intersects geometry B",
    Result::typeBool,
    [](const Geometry& geom, const Geometry& geomB) {
        return new Result( geom.intersects( &geomB ) );
    });
}},
{ "overlaps", [](std::string name) { return GeometryOp::create(name,
    catRel, "test if geometry A overlaps geometry B",
    Result::typeBool,
    [](const Geometry& geom, const Geometry& geomB) {
        return new Result( geom.overlaps( &geomB ) );
    });
}},
{ "touches", [](std::string name) { return GeometryOp::create(name,
    catRel, "test if geometry A touches geometry B",
    Result::typeBool,
    [](const Geometry& geom, const Geometry& geomB) {
        return new Result( geom.touches( &geomB ) );
    });
}},
{ "within", [](std::string name) { return GeometryOp::create(name,
    catRel, "test if geometry A is within geometry B",
    Result::typeBool,
    [](const Geometry& geom, const Geometry& geomB) {
        return new Result( geom.within( &geomB ) );
    });
}},
{ "relate", [](std::string name) { return GeometryOp::create(name,
    catRel, "compute DE-9IM matrix for geometry A and B",
    Result::typeString,
    [](const Geometry& geom, const Geometry& geomB) {
        std::unique_ptr<geom::IntersectionMatrix> im(geom.relate( &geomB ));
        return new Result( im->toString() );
    });
}},
{ "relateBNR", [](std::string name) { return GeometryOp::create(name,
    catRel, "compute DE-9IM matrix for geometry A and B with a Boundary Node Rule (1=Mod2,2=Endpt,3=Multivalent,4=Monovalent)",
    Result::typeString,
    [](const Geometry& geom, const Geometry& geomB, double d) {
            int bnr = (int) d;
            std::unique_ptr<IntersectionMatrix> im;
            switch (bnr) {
                case 1: /* same as OGC */
                    im = RelateOp::relate(&geom, &geomB,
                                          BoundaryNodeRule::getBoundaryRuleMod2());
                    break;
                case 2:
                    im = RelateOp::relate(&geom, &geomB,
                                          BoundaryNodeRule::getBoundaryEndPoint());
                    break;
                case 3:
                    im = RelateOp::relate(&geom, &geomB,
                                          BoundaryNodeRule::getBoundaryMultivalentEndPoint());
                    break;
                case 4:
                    im = RelateOp::relate(&geom, &geomB,
                                          BoundaryNodeRule::getBoundaryMonovalentEndPoint());
                    break;
                default:
                    std::ostringstream ss;
                    ss << "Invalid Boundary Node Rule " << bnr;
                    throw std::runtime_error(ss.str());
            }
            return new Result( im->toString() );
    });
}},


{ "containsPrep", [](std::string name) { return GeometryOp::create(name,
    catRel, "test if geometry A contains geometry B, with A prepared",
    Result::typeBool,
    [](const Geometry& geom, const Geometry& geomB) {
        return new Result( prepGeomCache.get(&geom)->contains( &geomB ) );
    });
}},
{ "containsProperlyPrep", [](std::string name) { return GeometryOp::create(name,
    catRel, "test if geometry A properly contains geometry B, with A prepared",
    Result::typeBool,
    [](const Geometry& geom, const Geometry& geomB) {
        return new Result( prepGeomCache.get(&geom)->containsProperly( &geomB ) );
    });
}},
{ "coversPrep", [](std::string name) { return GeometryOp::create(name,
    catRel, "test if geometry A covers geometry B, with A prepared",
    Result::typeBool,
    [](const Geometry& geom, const Geometry& geomB) {
        return new Result( prepGeomCache.get(&geom)->covers( &geomB ) );
    });
}},
{ "intersectsPrep", [](std::string name) { return GeometryOp::create(name,
    catRel, "test if geometry A intersects geometry B, with A prepared",
    Result::typeBool,
    [](const Geometry& geom, const Geometry& geomB) {
        return new Result( prepGeomCache.get(&geom)->intersects( &geomB ) );
    });
}},
{ "orientationIndex", [](std::string name) { return GeometryOp::create(name,
    catRel, "orientation index for a line segment and a point",
    Result::typeInt,
    [](const Geometry& geom, const Geometry& geomB) {
        std::unique_ptr<CoordinateSequence> seqA = geom.getCoordinates();
        Coordinate p0 = seqA->getAt(0);
        Coordinate p1 = seqA->getAt(1);
        std::unique_ptr<CoordinateSequence> seqB = geomB.getCoordinates();
        Coordinate q = seqB->getAt(0);
        return new Result( algorithm::Orientation::index(p0, p1, q) );
    });
}},

//=============  category: Overlay  ==================

{"difference", [](std::string name) { return GeometryOp::create(name,
    catOverlay,
    "compute difference of geometry A from B",
    [](const Geometry& geom, const Geometry& geomB) {
       return new Result( geom.difference( &geomB ) );
    });
}},
{"intersection", [](std::string name) { return GeometryOp::create(name,
    catOverlay,
    "compute intersection of geometry A and B",
    [](const Geometry& geom, const Geometry& geomB) {
       return new Result( geom.intersection( &geomB ) );
    });
}},
{"symDifference", [](std::string name) { return GeometryOp::create(name,
    catOverlay,
    "compute symmetric difference of geometry A and B",
    [](const Geometry& geom, const Geometry& geomB) {
        return new Result( geom.symDifference( &geomB ) );
    });
}},
{"unaryUnion", [](std::string name) { return GeometryOp::createAgg(name,
    catOverlay,
    "compute aggregate union",
    [](const Geometry& geom) {
        return new Result( geom.Union() );
    });
}},
{"union", [](std::string name) { return GeometryOp::create(name,
    catOverlay,
    "compute union of geometry A and B",
    [](const Geometry& geom, const Geometry& geomB) {
        return new Result( geom.Union( &geomB ) );
});
}},
{"differenceSR", [](std::string name) { return GeometryOp::create(name,
    catOverlay,
    "compute difference of geometry A from B, snap-rounding to a precision scale factor",
    [](const Geometry& geom, const Geometry& geomB, double d) {
        geos::geom::PrecisionModel pm(d);
        return new Result( OverlayNG::overlay(&geom, &geomB, OverlayNG::DIFFERENCE, &pm) );
    });
}},
{"intersectionSR", [](std::string name) { return GeometryOp::create(name,
    catOverlay,
    "compute intersection of geometry A and B, snap-rounding to a precision scale factor",
    [](const Geometry& geom, const Geometry& geomB, double d) {
        geos::geom::PrecisionModel pm(d);
        return new Result( OverlayNG::overlay(&geom, &geomB, OverlayNG::INTERSECTION, &pm) );
    });
}},
{"symDifferenceSR", [](std::string name) { return GeometryOp::create(name,
    catOverlay,
    "compute symmetric difference of geometry A and B, snap-rounding to a precision scale factor",
   [](const Geometry& geom, const Geometry& geomB, double d) {
        geos::geom::PrecisionModel pm(d);
        return new Result( OverlayNG::overlay(&geom, &geomB, OverlayNG::SYMDIFFERENCE, &pm) );
    });
}},
{"unionSR", [](std::string name) { return GeometryOp::create(name,
    catOverlay,
    "compute union of geometry A and B, snap-rounding to a precision scale factor",
    [](const Geometry& geom, const Geometry& geomB, double d) {
        geos::geom::PrecisionModel pm(d);
        return new Result( OverlayNG::overlay(&geom, &geomB, OverlayNG::UNION, &pm) );
    });
}},
{"unaryUnionSR", [](std::string name) { return GeometryOp::createAgg(name,
    catOverlay,
    "compute aggregate union",
    [](const Geometry& geom, double d) {
        geos::geom::PrecisionModel pm(d);
        return new Result( geos::operation::overlayng::UnaryUnionNG::Union(&geom, pm ));
    });
}},
{"node", [](std::string name) { return GeometryOp::create(name,
    catOverlay,
    "compute fully noded geometry",
    [](const Geometry& geom) {
        return new Result( geos::noding::GeometryNoder::node( geom ) );
});
}},
{"clipRect", [](std::string name) { return GeometryOp::create(name,
    catOverlay,
    "clip geometry A to envelope of B",
    [](const Geometry& geom, const Geometry& geomB) {
        using geos::operation::intersection::Rectangle;
        using geos::operation::intersection::RectangleIntersection;
        const Envelope* env = geomB.getEnvelopeInternal();
        Rectangle rect(env->getMinX(), env->getMinY(), env->getMaxX(), env->getMaxY());
        return new Result( RectangleIntersection::clip( geom, rect) );
    });
}},

//=============  category: Linear Referencing  ==================

{"extractLine", [](std::string name) { return GeometryOp::create(name,
    catLinearref,
    "compute the line between two distances along linear geometry A",
    [](const Geometry& geom, double d, double d2) {
        auto lil = geos::linearref::LengthIndexedLine( &geom );
        auto res = lil.extractLine(d, d2);
        return new Result( std::move(res) );
    });
}},
{"interpolate", [](std::string name) { return GeometryOp::create(name,
    catLinearref,
    "compute a point interpolated along a distance from the start of geometry A",
    [](const Geometry& geom, double d) {
        auto lil = geos::linearref::LengthIndexedLine( &geom );
        auto coord = geos::geom::Coordinate( lil.extractPoint(d) );
        auto factory = geom.getFactory();
        auto res = factory->createPoint( std::move(coord) );
        return new Result( std::move(res) );
    });
}},
{"project", [](std::string name) { return GeometryOp::create(name,
    catLinearref,
    "compute the distance of point B projected onto line A from the start of the line",
    Result::typeDouble,
    [](const Geometry& geom, const Geometry& geomB) {
        auto inputPt = geos::geom::Coordinate( *geomB.getCoordinate() );
        return new Result( geos::linearref::LengthIndexedLine( &geom ).project( inputPt ) );
    });
}},

//=============  category: Clustering  ==================

{"clusterIntersecting", [](std::string name) { return GeometryOp::createAgg(name,
    catCluster, "cluster geometries based on intersection",
    [](const Geometry& geom) {
        geos::operation::cluster::GeometryIntersectsClusterFinder f;
        return new Result(f.clusterToCollection(geom));
    });
}},
{"clusterWithin", [](std::string name) { return GeometryOp::createAgg(name,
    catCluster, "cluster geometries based on distance",
    [](const Geometry& geom, double d) {
        geos::operation::cluster::GeometryDistanceClusterFinder f(d);
        return new Result(f.clusterToCollection(geom));
    });
}},

//=============  category: Polygonal Coverage  ==================

{"coverageClean", [](std::string name) { return GeometryOp::createAgg(name,
    catCoverage, "cleans a coverage, merging gaps narrower than a given width",
    [](const Geometry& geom, double gapWidth) {
        std::vector<const Geometry*> coverage = toList(geom);
        std::vector<std::unique_ptr<Geometry>> result
            = geos::coverage::CoverageCleaner::cleanGapWidth(coverage, gapWidth);;
        //-- convert list type (be nice to avoid this)
        std::vector<std::unique_ptr<const Geometry>> resultList;
        for (std::size_t i = 0; i < result.size(); i++) {
            resultList.emplace_back( std::move(result[i]) );
        }
        return new Result( std::move(resultList) );
    });
}},
{"coverageSimplify", [](std::string name) { return GeometryOp::createAgg(name,
    catCoverage, "simplify a polygonal coverage by a distance tolerance",
    [](const Geometry& geom, double d) {
        std::vector<const Geometry*> coverage = toList(geom);
        std::vector<std::unique_ptr<Geometry>> result
            = geos::coverage::CoverageSimplifier::simplify(coverage, d);
        //-- convert list type (be nice to avoid this)
        std::vector<std::unique_ptr<const Geometry>> resultList;
        for (std::size_t i = 0; i < result.size(); i++) {
            resultList.emplace_back( std::move(result[i]) );
        }
        return new Result( std::move(resultList) );
    });
}},
{"coverageUnionNG", [](std::string name) { return GeometryOp::createAgg(name,
    catCoverage, "union a polygonal coverage",
    [](const Geometry& geom) {
        return new Result( geos::operation::overlayng::CoverageUnion::geomunion(&geom) );
    });
}},
{"coverageUnion", [](std::string name) { return GeometryOp::createAgg(name,
    catCoverage, "union a polygonal coverage",
    [](const Geometry& geom) {
        return new Result( geos::operation::geounion::CoverageUnion::Union(&geom) );
});
}},
{"coverageValidate", [](std::string name) { return GeometryOp::createAgg(name,
    catCoverage, "validate a polygonal coverage",
    [](const Geometry& geom) {
        std::vector<const Geometry*> coverage = toList(geom);
        std::vector<std::unique_ptr<Geometry>> invalidList
            = geos::coverage::CoverageValidator::validate(coverage);

        // result may contain nulls, so remove them
        std::vector<std::unique_ptr<const Geometry>> resultList;
        for (std::size_t i = 0; i < invalidList.size(); i++) {
            if (invalidList[i] != nullptr) {
                resultList.emplace_back( std::move(invalidList[i]) );
            }
        }
        return new Result( std::move(resultList) );
    });
}}
};

GeometryOp* findFun(std::string name) {
    //-- brute force, but good enough for one-time lookup
    for (auto gfc : opRegistry) {
        if (name == gfc.name) {
            return gfc.create(gfc.name);
        }
    }
    return nullptr;
}

std::vector<std::string> GeometryOp::listOps()
{
    std::vector<std::string> list;
    std::string cat = "";
    for (auto gfc : opRegistry) {
        auto op = gfc.create(gfc.name);
        if (op->category != cat) {
            list.push_back( op->category + "  ------------------");
            cat = op->category;
        }
        auto desc = op->signature() + " - " + op->description;
        // TODO: add display of function signature
        list.push_back( desc );
    }
    return list;
}

/* static */
GeometryOp*
GeometryOp::find(std::string name)
{
    return findFun(name);
}

std::string GeometryOp::name()
{
    return opName;
}

bool GeometryOp::isBinary()
{
    return numGeomParam == 2;
}

bool GeometryOp::isAggregate()
{
    return _isAggregate;
}

int GeometryOp::nParam()
{
    return numParam;
}

std::string GeometryOp::signature() {
    std::string sig =  " A";
    sig += isBinary() ? " B" : "  ";
    sig += "  ";
    sig += opName;
    if (numParam > 0) sig += " N";
    if (numParam > 1) sig += " N";
    sig += _isAggregate ? " AGG" : "";
    sig += " > ";
    sig += Result::code(resultType);
    return sig;
}

Result * GeometryOp::execute( const Geometry& geomA, const Geometry& geomB, double d, double d2 )
{
    if (numGeomParam == 1) {
        if (numParam == 0) return geomfun_G(geomA);
        if (numParam == 1) return geomfun_GD(geomA, d);
        if (numParam == 2) return geomfun_GDD(geomA, d, d2);
    }
    if (numGeomParam == 2) {
        if (numParam == 0) return geomfun_GG(geomA, geomB);
        if (numParam == 1) return geomfun_GGD(geomA, geomB, d);
    }
    // TODO: error here?
    return nullptr;
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
Result::isBool() {
    return typeCode == typeBool;
}

bool
Result::toBool() {
    if (typeCode == typeBool)
        return valBool;
    return false;
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
        converter << std::setprecision(17) << valDouble;
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
    return "Value for Unknown type";
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
    return "Unknown type";
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
