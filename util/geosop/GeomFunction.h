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

#pragma once

#include <geos/geom/GeometryFactory.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/geom/prep/PreparedGeometry.h>

#include <vector>
#include <map>
#include <functional>

using namespace geos;
using namespace geos::geom;
using namespace geos::geom::prep;

class Result {
public:
    bool valBool;
    int valInt;
    double valDouble;
    std::string valStr;
    std::unique_ptr<Geometry> valGeom;
    std::vector<std::unique_ptr<const Geometry>> valGeomList;

    Result(bool val);
    Result(int val);
    Result(std::string val);
    Result(double val);
    Result(std::unique_ptr<Geometry> val);
    Result(std::vector<std::unique_ptr<const Geometry>> val);
    Result(Geometry * val);
    ~Result();

    static std::string code(int typeCode);

    bool isGeometry();
    bool isGeometryList();
    std::string metadata();
    std::string toString();

    enum {
        typeBool = 1, typeInt, typeDouble, typeString, typeGeometry, typeGeomList
    } typeCode;
};

typedef std::function<Result *( const std::unique_ptr<Geometry>&, const std::unique_ptr<Geometry>&, double d )>
    geomFunSig;

class GeomFunction {

public:
    static void init();
    static GeomFunction* find(std::string name);
    static std::vector<std::string> list();

    GeomFunction(std::string strName,
                    int nGeom,
                    int nParam,
                    int resType,
                    std::string cat,
                    std::string desc,
                    geomFunSig fun)
        :
        funName(strName),
        numGeomParam(nGeom),
        numParam(nParam),
        resultType(resType),
        category(cat),
        description(desc),
        geomfun(fun)
    {}

    ~GeomFunction()
    {}

    std::string name();
    bool isBinary();
    std::string signature();

    Result * execute( const std::unique_ptr<Geometry>& geomA,
        const std::unique_ptr<Geometry>& geomB, double d );

private:
    static void add(std::string name, int resultType, std::string category, geomFunSig geomfun);
    static void add(std::string name,
                    int nGeom,
                    int nParam,
                    int resultType,
                    std::string category,
                    std::string desc,
                    geomFunSig geomfun);

    static std::map<std::string, GeomFunction*> registry;
    static std::vector<GeomFunction*> functionList;

    //---------------------------

    std::string funName;

    int numGeomParam;  // number of *required* geometry parameters (0,1,2)
    int numParam;  // number of none-geometry parameters (0 or 1 currently)
    //TODO: add result type?
    int resultType;

    std::string category;
    std::string description;

    geomFunSig geomfun;

};

