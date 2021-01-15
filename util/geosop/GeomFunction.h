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

#ifndef GEOS_GEOMFUNCTION_H
#define GEOS_GEOMFUNCTION_H

#include <geos/geom/GeometryFactory.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/geom/prep/PreparedGeometry.h>

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

    explicit Result(bool val);
    Result(int val);
    Result(std::string val);
    Result(double val);
    Result(std::unique_ptr<Geometry> val);
    Result(std::vector<std::unique_ptr<const Geometry>> val);
    Result(Geometry * val);

    bool isGeometry();
    bool isGeometryList();
    std::string metadata();
    std::string toString();

private:
    enum {
        typeBool = 1, typeInt, typeDouble, typeString, typeGeometry, typeGeomList
    } typeCode;
};

class Argument {

public:
    explicit Argument(double d) {
        m_value.dbl_val = d;
    }

    explicit Argument(int i) {
        m_value.int_val = i;
    }

    explicit Argument(const Geometry* g) {
        m_value.geom_val = g;
    }

    double getDouble() const {
        return m_value.dbl_val;
    }

    int getInt() const {
        return m_value.int_val;
    }

    const Geometry* getGeometry() const {
        return m_value.geom_val;
    }

private:

    union {
        double          dbl_val;
        int             int_val;
        const Geometry* geom_val;
    } m_value;
};

using geomFunSig = std::function<Result(const std::vector<Argument> &)>;

class GeomFunction {

public:
    static void init();
    static GeomFunction* find(std::string name);
    static std::vector<std::string> list();

    std::string name();
    bool isBinary();
    std::string signature();

    Result execute(const std::vector<Argument> & args);

private:
    static void add(std::string name, geomFunSig geomfun);
    static void add(std::string name,
                    std::string desc,
                    int nGeom,
                    int nParam,
                    geomFunSig geomfun);

    static std::map<std::string, GeomFunction*> registry;

    //static geos::geom::prep::PreparedGeometry *prepGeomCache;
    //static Geometry *cacheKey;

    //---------------------------

    std::string funName;
    std::string description;

    int numGeomParam;  // number of *required* geometry parameters (0,1,2)
    int numParam;  // number of none-geometry parameters (0 or 1 currently)
    //TODO: add result type?

    geomFunSig geomfun;

};

#endif // GEOS_GEOMFUNCTION_H
