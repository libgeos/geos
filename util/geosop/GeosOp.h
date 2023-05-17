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

using namespace geos;
using namespace geos::geom;

class OpArguments {
public:
    int nArgs = 0;
    double arg1;
    double arg2;
};

class GeosOpArgs {

public:
    enum {
        fmtNone,
        fmtText,
        fmtWKB,
        fmtGeoJSON,
    } format = fmtText;

    bool isShowTime = false;
    bool isVerbose = false;
    bool isQuiet = false;
    int precision = -1;
    int repeatNum = 1;

    //std::string format;

    std::string srcA;
    int limitA = -1;
    int offsetA = -1;
    bool isCollect = true;
    bool isExplode = false;

    std::string srcB;

    std::string opName;
};

class GeosOp {

public:
    static std::string const opNames[];

    GeosOp(GeosOpArgs& args);
    ~GeosOp();
    void run(OpArguments& opArgs);

private:

    GeosOpArgs& args;

    long opCount = 0;
    std::size_t vertexCount = 0;
    double totalTime = 0;

    std::vector<std::unique_ptr<Geometry>> geomA;

    std::vector<std::unique_ptr<Geometry>> geomB;

    std::vector<std::unique_ptr<Geometry>> readInput(std::string name, std::string src, int limit, int offset);
    std::vector<std::unique_ptr<Geometry>> loadInput(std::string name, std::string src, int limit, int offset);
    GeometryOp* getOp();
    void execute(GeometryOp * op, OpArguments& opArgs);
    void executeUnary(GeometryOp * op, OpArguments& opArgs);
    void executeBinary(GeometryOp * op, OpArguments& opArgs);
    Result* executeOpRepeat(GeometryOp * op,
        unsigned int indexA, const  std::unique_ptr<Geometry>& geomA,
        unsigned int indexB, const  std::unique_ptr<Geometry>& geomB,
        OpArguments& opArgs);
    Result* executeOp(GeometryOp * op,
        unsigned int indexA, const  std::unique_ptr<Geometry>& geomA,
        unsigned int indexB, const  std::unique_ptr<Geometry>& geomB,
        OpArguments& opArgs);
    void output(Result* result);
    void outputExplode(std::unique_ptr<Geometry>& geom);
    void outputGeometry( const Geometry* geom);
    void outputGeometryList(std::vector<std::unique_ptr<const Geometry>> & val);
    void log(std::string s);
};
