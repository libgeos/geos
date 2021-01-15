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

/* static */
void
GeomFunction::init()
{
    add("area",
        [](const std::vector<Argument> & args) {
            return args[0].getGeometry()->getArea();
        });
    add("boundary",
        [](const std::vector<Argument> & args) {
            return args[0].getGeometry()->getBoundary();
        });
    add("buffer", "computes the buffer of geometry A", 1, 1,
        [](const std::vector<Argument> & args) {
            return args[0].getGeometry()->buffer(args[1].getDouble());
        });
    add("centroid",
        [](const std::vector<Argument> & args) {
            return std::unique_ptr<Geometry>(args[0].getGeometry()->getCentroid());
        });
    add("copy",
        [](const std::vector<Argument> & args) {
            return args[0].getGeometry()->clone();
        });
    add("convexHull",
        [](const std::vector<Argument> & args) {
            return args[0].getGeometry()->convexHull();
        });
    add("contains", "tests if geometry A contains geometry B", 2, 0,
        [](const std::vector<Argument> & args) {
            return args[0].getGeometry()->contains(args[1].getGeometry());
        });
    add("covers", "tests if geometry A covers geometry B", 2, 0,
        [](const std::vector<Argument> & args) {
            return args[0].getGeometry()->covers(args[1].getGeometry());
        });

    add("distance", "computes distance between geometry A and B", 2, 0,
        [](const std::vector<Argument> & args) {
            return args[0].getGeometry()->distance(args[1].getGeometry());
        });

     add("envelope",
        [](const std::vector<Argument> & args) {
            return std::unique_ptr<Geometry>(args[0].getGeometry()->getCentroid());
        });

    add("interiorPoint",
        [](const std::vector<Argument> & args) {
            return std::unique_ptr<Geometry>(args[0].getGeometry()->getInteriorPoint());
        });

    add("intersects", "tests if geometry A and B intersect", 2, 0,
        [](const std::vector<Argument> & args) {
            return args[0].getGeometry()->intersects(args[1].getGeometry());
        });

    add("isSimple", "tests if geometry A is simple", 1, 0,
        [](const std::vector<Argument> & args) {
            return args[0].getGeometry()->isSimple();
        });

    add("isValid", "tests if geometry A is valid", 1, 0,
        [](const std::vector<Argument> & args) {
            return args[0].getGeometry()->isValid();
        });

    add("length",
        [](const std::vector<Argument> & args) {
            return args[0].getGeometry()->getLength();
        });

    add("makeValid",
        [](const std::vector<Argument> & args) {
            return geos::operation::valid::MakeValid().build(args[0].getGeometry());
        });

    add("maxInscribedCircle", "computes maximum inscribed circle radius of Polygon A up to a distance tolerance", 2, 0,
        [](const std::vector<Argument> & args) {
            geos::algorithm::construct::MaximumInscribedCircle mc(args[0].getGeometry(), args[1].getDouble());
            return std::unique_ptr<Geometry>(mc.getRadiusLine());
        });
    add("minBoundingCircle",
        [](const std::vector<Argument> & args) {
            geos::algorithm::MinimumBoundingCircle mc(args[0].getGeometry());
            return std::unique_ptr<Geometry>(mc.getCircle());
        });
    add("nearestPoints", "computes nearest points of geometry A and B", 2, 0,
        [](const std::vector<Argument> & args) {
            auto cs = geos::operation::distance::DistanceOp::nearestPoints(args[0].getGeometry(), args[1].getGeometry());
            auto factory = args[0].getGeometry()->getFactory();
            auto res = factory->createLineString( std::move(cs) );
            return std::unique_ptr<Geometry>(std::move(res));
        });
    add("normalize", "normalizes geometry A", 1, 0,
        [](const std::vector<Argument> & args) {
            auto res = args[0].getGeometry()->clone();
            res->normalize();
            return res;
        });

    add("lineMerge", "merges the lines of geometry A", 1, 0,
        [](const std::vector<Argument> & args) {
            geos::operation::linemerge::LineMerger lmrgr;
            lmrgr.add(args[0].getGeometry());

            std::vector<std::unique_ptr<LineString>> lines = lmrgr.getMergedLineStrings();

            std::vector<std::unique_ptr<const Geometry>> geoms;
            for(unsigned int i = 0; i < lines.size(); i++) {
                geoms.push_back( std::move(lines[i]) );
            }
            return geoms;
        });

    add("delaunay", "computes the Delaunay Triangulation of geometry A vertices", 1, 0,
        [](const std::vector<Argument> & args) {
            auto geom = args[0].getGeometry();

            geos::triangulate::DelaunayTriangulationBuilder builder;
            builder.setTolerance(0);
            builder.setSites(*geom);

            Geometry* out = builder.getTriangles(*(geom->getFactory())).release();

            std::vector<std::unique_ptr<const Geometry>> geoms;
            for(unsigned int i = 0; i < out->getNumGeometries(); i++) {
                geoms.push_back( std::unique_ptr< const Geometry>( out->getGeometryN(i) ) );
            }
            return geoms;
        });

    add("voronoi", "computes the Voronoi Diagram of geometry A vertices", 1, 0,
        [](const std::vector<Argument> & args) {
            auto geom = args[0].getGeometry();

            geos::triangulate::VoronoiDiagramBuilder builder;
            builder.setTolerance(0);
            builder.setSites( *geom );

            Geometry* out = builder.getDiagram(*(geom->getFactory())).release();

            std::vector<std::unique_ptr<const Geometry>> geoms;
            for(unsigned int i = 0; i < out->getNumGeometries(); i++) {
                geoms.push_back( std::unique_ptr< const Geometry>( out->getGeometryN(i) ) );
            }
            return geoms;
        });

    add("polygonize",
        [](const std::vector<Argument> & args) {
            geos::operation::polygonize::Polygonizer p;
            p.add(args[0].getGeometry());

            std::vector<std::unique_ptr<Polygon>> polys = p.getPolygons();
            std::vector<std::unique_ptr<const Geometry>> geoms;
            for(unsigned int i = 0; i < polys.size(); i++) {
                geoms.push_back( std::move(polys[i]) );
            }
            return geoms;
        });

    add("reducePrecision", "reduces precision of geometry to a precision scale factor", 1, 1,
        [](const std::vector<Argument> & args) {
            PrecisionModel pm(args[1].getDouble());
            return geos::precision::GeometryPrecisionReducer::reduce( *args[0].getGeometry(), pm );
        });
    add("relate", "computes DE-9IM matrix for geometry A and B", 2, 0,
        [](const std::vector<Argument> & args) {
            std::unique_ptr<geom::IntersectionMatrix> im(args[0].getGeometry()->relate(args[1].getGeometry() ));
            return im->toString();
        });
    add("reverse", "reverses geometry A", 1, 0,
        [](const std::vector<Argument> & args) {
            return args[0].getGeometry()->reverse();
        });
    add("simplifyDP", "simplifies geometry A using Douglas-Peucker with a distance tolerance", 1, 1,
        [](const std::vector<Argument> & args) {
            return geos::simplify::DouglasPeuckerSimplifier::simplify(args[0].getGeometry(), args[1].getDouble());
         });
    add("simplifyTP", "simplifies geometry A using Douglas-Peucker with a distance tolerance, preserving topology", 1, 1,
        [](const std::vector<Argument> & args) {
            return geos::simplify::TopologyPreservingSimplifier::simplify(args[0].getGeometry(), args[1].getDouble());
         });

    add("containsPrep", "tests if geometry A contains geometry B, using PreparedGeometry", 2, 0,
        [](const std::vector<Argument> & args) {
            auto geom = args[0].getGeometry();
            return prepGeomCache.get(geom)->contains( args[1].getGeometry() );
        });
    add("containsProperlyPrep", "tests if geometry A properly contains geometry B using PreparedGeometry", 2, 0,
        [](const std::vector<Argument> & args) {
            auto geom = args[0].getGeometry();
            return prepGeomCache.get(geom)->containsProperly( args[1].getGeometry() );
        });
    add("coversPrep", "tests if geometry A covers geometry B using PreparedGeometry", 2, 0,
        [](const std::vector<Argument> & args) {
            auto geom = args[0].getGeometry();
            return prepGeomCache.get(geom)->covers( args[1].getGeometry() );
        });
    add("intersectsPrep", "tests if geometry A intersects B using PreparedGeometry", 2, 0,
        [](const std::vector<Argument> & args) {
            auto geom = args[0].getGeometry();
            return prepGeomCache.get(geom)->intersects( args[1].getGeometry() );
        });

    add("distancePrep", "computes distance between geometry A and B using PreparedGeometry", 2, 0,
        [](const std::vector<Argument> & args) {
            auto geom = args[0].getGeometry();
            return prepGeomCache.get(geom)->distance( args[1].getGeometry() );
        });
    add("nearestPointsPrep", "computes nearest points of geometry A and B using PreparedGeometry", 2, 0,
        [](const std::vector<Argument> & args) {
            auto geom = args[0].getGeometry();
            auto cs = prepGeomCache.get(geom)->nearestPoints( args[1].getGeometry());
            auto factory = geom->getFactory();
            auto res = factory->createLineString( std::move(cs) );
            return std::unique_ptr<Geometry>(std::move(res));
        });

    add("difference", "computes difference of geometry A from B", 2, 0,
        [](const std::vector<Argument> & args) {
            return args[0].getGeometry()->difference(args[1].getGeometry());
        });
    add("intersection", "computes intersection of geometry A and B", 2, 0,
        [](const std::vector<Argument> & args) {
            return args[0].getGeometry()->intersection(args[1].getGeometry());
        });
    add("symDifference", "computes symmetric difference of geometry A and B", 2, 0,
        [](const std::vector<Argument> & args) {
            return args[0].getGeometry()->symDifference(args[1].getGeometry());
        });
    add("unaryUnion",
        [](const std::vector<Argument> & args) {
            return args[0].getGeometry()->Union();
        });
    add("union", "computes union of geometry A and B", 2, 0,
        [](const std::vector<Argument> & args) {
            return args[0].getGeometry()->Union( args[1].getGeometry() );
        });

    add("differenceSR", "computes difference of geometry A from B, snap-rounding to a precision scale factor", 2, 1,
        [](const std::vector<Argument> & args) {
            geom::PrecisionModel pm(args[2].getDouble());
            return OverlayNG::overlay(args[0].getGeometry(), args[1].getGeometry(), OverlayNG::DIFFERENCE, &pm);
        });
    add("intersectionSR", "computes intersection of geometry A and B, snap-rounding to a precision scale factor", 2, 1,
        [](const std::vector<Argument> & args) {
            geom::PrecisionModel pm(args[2].getDouble());
            return OverlayNG::overlay(args[0].getGeometry(), args[1].getGeometry(), OverlayNG::INTERSECTION, &pm);
        });
    add("symDifferenceSR", "computes symmetric difference of geometry A and B, snap-rounding to a precision scale factor", 2, 1,
        [](const std::vector<Argument> & args) {
            geom::PrecisionModel pm(args[2].getDouble());
            return OverlayNG::overlay(args[0].getGeometry(), args[1].getGeometry(), OverlayNG::SYMDIFFERENCE, &pm);
        });
    add("unionSR", "computes union of geometry A and B, snap-rounding to a precision scale factor", 2, 1,
        [](const std::vector<Argument> & args) {
            geom::PrecisionModel pm(args[2].getDouble());
            return OverlayNG::overlay(args[0].getGeometry(), args[1].getGeometry(), OverlayNG::UNION, &pm);
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

Result GeomFunction::execute(const std::vector<Argument> & args)
{
    return geomfun(args);
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
