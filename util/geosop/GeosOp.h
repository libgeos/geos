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

#ifndef GEOS_GEOSOP_H
#define GEOS_GEOSOP_H

#include <geos/geom/GeometryFactory.h>
#include <geos/geom/PrecisionModel.h>

using namespace geos;
using namespace geos::geom;


class GeosOpArgs {

public:
    enum {
        fmtNone, fmtText, fmtWKB
    } format = fmtNone;

    bool isShowTime = false;
    bool isVerbose = false;
    int precision = -1;
    int repeatNum = 1;

    //std::string format;

    std::string srcA;
    int limitA = -1;
    bool isCollect = true;
    bool isExplode = false;

    std::string srcB;

    std::string opName;
    double opArg1 = 0.0;
    //std::string opArg2;
};

class GeosOp {

public:
    static std::string const opNames[];

    GeosOp(GeosOpArgs& args);
    ~GeosOp();
    void run();

private:

    GeosOpArgs& args;

    long opCount = 0;
    long vertexCount = 0;
    double totalTime = 0;

    std::vector<std::unique_ptr<Geometry>> geomA;

    std::vector<std::unique_ptr<Geometry>> geomB;

    std::vector<std::unique_ptr<Geometry>> readInput(std::string name, std::string src, int limit);
    std::vector<std::unique_ptr<Geometry>> loadInput(std::string name, std::string src, int limit);
    void execute();
    void executeUnary(GeomFunction * fun);
    void executeBinary(GeomFunction * fun);
    Result* executeOpRepeat(GeomFunction * fun,
        int indexA, const  std::unique_ptr<Geometry>& geomA,
        int indexB, const  std::unique_ptr<Geometry>& geomB);
    Result* executeOp(GeomFunction * fun,
        int indexA, const  std::unique_ptr<Geometry>& geomA,
        int indexB, const  std::unique_ptr<Geometry>& geomB);
    void output(Result* result);
    void outputExplode(std::unique_ptr<Geometry>& geom);
    void outputGeometry( const Geometry* geom);
    void outputGeometryList(std::vector<std::unique_ptr<const Geometry>> & val);
    void log(std::string s);
};

#endif // GEOS_GEOSOP_H
