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

typedef std::function<Result *( const std::unique_ptr<Geometry>& )>
    geomFunSig_G;

typedef std::function<Result *( const std::unique_ptr<Geometry>&, double d )>
    geomFunSig_GD;

typedef std::function<Result *( const std::unique_ptr<Geometry>&, double d, double d2 )>
    geomFunSig_GDD;

typedef std::function<Result *( const std::unique_ptr<Geometry>&, const std::unique_ptr<Geometry>& )>
    geomFunSig_GG;

typedef std::function<Result *( const std::unique_ptr<Geometry>&, const std::unique_ptr<Geometry>&, double d )>
    geomFunSig_GGD;

class GeometryOp {

public:
    static GeometryOp* create(std::string name,
                    std::string cat,
                    std::string desc,
                    int resType,
                    geomFunSig_G fun) {
        return new GeometryOp(name, cat, desc, false, resType, fun);
    }

    static GeometryOp* create(std::string name,
                    std::string cat,
                    std::string desc,
                    geomFunSig_G fun) {
        return new GeometryOp(name, cat, desc, false, Result::typeGeometry, fun);
    }

    static GeometryOp* create(std::string name,
                    std::string cat,
                    std::string desc,
                    int resType,
                    geomFunSig_GD fun) {
        return new GeometryOp(name, cat, desc, false, resType, fun);
    }

    static GeometryOp* create(std::string name,
                    std::string cat,
                    std::string desc,
                    geomFunSig_GD fun) {
        return new GeometryOp(name, cat, desc, false, Result::typeGeometry, fun);
    }

    static GeometryOp* create(std::string name,
                    std::string cat,
                    std::string desc,
                    geomFunSig_GDD fun) {
        return new GeometryOp(name, cat, desc, false, Result::typeGeometry, fun);
    }

    static GeometryOp* create(std::string name,
                    std::string cat,
                    std::string desc,
                    int resType,
                    geomFunSig_GG fun) {
        return new GeometryOp(name, cat, desc, false, resType, fun);
    }

    static GeometryOp* create(std::string name,
                    std::string cat,
                    std::string desc,
                    geomFunSig_GG fun) {
        return new GeometryOp(name, cat, desc, false, Result::typeGeometry, fun);
    }

    static GeometryOp* create(std::string name,
                    std::string cat,
                    std::string desc,
                    int resType,
                    geomFunSig_GGD fun) {
        return new GeometryOp(name, cat, desc, false, resType, fun);
    }

    static GeometryOp* create(std::string name,
                    std::string cat,
                    std::string desc,
                    geomFunSig_GGD fun) {
        return new GeometryOp(name, cat, desc, false, Result::typeGeometry, fun);
    }

    static GeometryOp* createAgg(std::string name,
                    std::string cat,
                    std::string desc,
                    geomFunSig_G fun) {
        return new GeometryOp(name, cat, desc, true, Result::typeGeometry, fun);
    }

    static GeometryOp* createAgg(std::string name,
                    std::string cat,
                    std::string desc,
                    geomFunSig_GD fun) {
        return new GeometryOp(name, cat, desc, true, Result::typeGeometry, fun);
    }

    static GeometryOp* find(std::string name);
    static std::vector<std::string> listOps();

    GeometryOp(std::string strName,
                    std::string cat,
                    std::string desc,
                    bool isAgg,
                    int resType,
                    geomFunSig_G fun)
        :
        opName(strName),
        numGeomParam(1),
        numParam(0),
        _isAggregate(isAgg),
        resultType(resType),
        category(cat),
        description(desc),
        geomfun_G(fun)
    {}

    GeometryOp(std::string strName,
                    std::string cat,
                    std::string desc,
                    bool isAgg,
                    int resType,
                    geomFunSig_GD fun)
        :
        opName(strName),
        numGeomParam(1),
        numParam(1),
        _isAggregate(isAgg),
        resultType(resType),
        category(cat),
        description(desc),
        geomfun_GD(fun)
    {}

    GeometryOp(std::string strName,
                    std::string cat,
                    std::string desc,
                    bool isAgg,
                    int resType,
                    geomFunSig_GDD fun)
        :
        opName(strName),
        numGeomParam(1),
        numParam(2),
        _isAggregate(isAgg),
        resultType(resType),
        category(cat),
        description(desc),
        geomfun_GDD(fun)
    {}

    GeometryOp(std::string strName,
                    std::string cat,
                    std::string desc,
                    bool isAgg,
                    int resType,
                    geomFunSig_GG fun)
        :
        opName(strName),
        numGeomParam(2),
        numParam(0),
        _isAggregate(isAgg),
        resultType(resType),
        category(cat),
        description(desc),
        geomfun_GG(fun)
    {}

    GeometryOp(std::string strName,
                    std::string cat,
                    std::string desc,
                    bool isAgg,
                    int resType,
                    geomFunSig_GGD fun)
        :
        opName(strName),
        numGeomParam(2),
        numParam(1),
        _isAggregate(isAgg),
        resultType(resType),
        category(cat),
        description(desc),
        geomfun_GGD(fun)
    {}

    ~GeometryOp()
    {}

    std::string name();
    bool isBinary();
    bool isAggregate();
    int nParam();
    std::string signature();

    Result * execute( const std::unique_ptr<Geometry>& geomA,
        const std::unique_ptr<Geometry>& geomB,
        double d, double d2 );

private:

    std::string opName;

    int numGeomParam;  // number of *required* geometry parameters (0,1,2)
    int numParam;  // number of none-geometry parameters (0 or 1 currently)
    bool _isAggregate;
    int resultType;

    std::string category;
    std::string description;

    geomFunSig_G geomfun_G;
    geomFunSig_GD geomfun_GD;
    geomFunSig_GDD geomfun_GDD;
    geomFunSig_GG geomfun_GG;
    geomFunSig_GGD geomfun_GGD;
};
