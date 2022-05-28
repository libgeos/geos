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
#include <geos/geom/GeometryFactory.h>
#include <geos/operation/valid/MakeValid.h>
#include <geos/operation/polygonize/Polygonizer.h>

#include "GeomFunction.h"

#include <sstream>

/* static private */
std::map<std::string, GeomFunction*> GeomFunction::registry;

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
    add("buffer", "cmputes the buffer of a Geometry", 1, 1,
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
    add("envelope",
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            return new Result( geom->getCentroid() );
        });
    add("interiorPoint",
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            return new Result( geom->getInteriorPoint() );
        });
    add("intersection", "computes intersection of two Geometries", 2, 0,
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            return new Result( geom->intersection( geomB.get() ) );
        });
    add("intersects", "tests if two Geometries intesect", 2, 0,
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            return new Result( geom->intersects( geomB.get() ) );
        });
    add("isValid", "tests if a Geometry is valid", 1, 0,
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
    add("unaryUnion",
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            return new Result( geom->Union() );
        });
    add("union", "computes union of two Geometries", 2, 0,
        [](const std::unique_ptr<Geometry>& geom, const std::unique_ptr<Geometry>& geomB, double d)->Result* {
            return new Result( geom->Union( geomB.get() ) );
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
        "computes " + name + " for a Geometry",
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

Result::~Result()
{
}

bool
Result::isGeometry() {
    return typeCode == typeGeometry;
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

    case typeGeometry:
        if (valGeom == nullptr)
            return "null";
        return valGeom->toString();
    }
}

std::string
Result::metadata() {
    switch (typeCode) {
    case typeBool:
        return "bool";

    case typeInt:
        return "int";

    case typeDouble:
        return "double";

    case typeGeometry:
        if (valGeom == nullptr)
            return "null";
        return valGeom->getGeometryType() + "( " + std::to_string( valGeom->getNumPoints() ) + " )";
    }
}
