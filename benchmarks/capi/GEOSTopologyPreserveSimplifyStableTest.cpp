/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2024 Martin Davis 
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

/**
 * Tests the stability of topology-preserving simplification
 * (using GEOSTopologyPreserveSimplify).
 * The expectation is that repeated calls to GEOSTopologyPreserveSimplify(geom, tol)
 * produce identical results.
 * 
 * See https://github.com/libgeos/geos/issues/1107
*/
#include <geos_c.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <deque>

GEOSGeometry* readGeom(std::string& fname) {
    std::ifstream f(fname);
    std::string line;
    if (! std::getline(f, line)) {
        return nullptr;
    }
    auto geom = GEOSGeomFromWKT(line.c_str());
    f.close();
    return geom;
}

    //----------  Run test  ---------------
void run(std::string& fname, int nRuns, double tolerance)
{
    initGEOS(nullptr, nullptr);

    GEOSGeometry* geom = readGeom(fname);  

    int nErrors = 0;
    for (int i = 0; i < nRuns; i++) {
        GEOSGeometry* simp1 = GEOSTopologyPreserveSimplify(geom, tolerance);
        GEOSGeometry* simp2 = GEOSTopologyPreserveSimplify(geom, tolerance);
        bool isEqualExact = 1 == GEOSEqualsExact(simp1, simp2, 0.0);

        GEOSGeometry* diff = GEOSSymDifference(simp1, simp2);
        bool isDiffEmpty = 1 == GEOSisEmpty(diff);

        //std::cout << "Run " << i << std::endl;
        if (! isEqualExact || ! isDiffEmpty) {
            nErrors++;
            std::cout << "simplified results are not identical" << std::endl;
            std::cout << diff << std::endl;
        }

        GEOSGeom_destroy(simp1);
        GEOSGeom_destroy(simp2);
        GEOSGeom_destroy(diff);
    }
    std::cout << "Number of tests with non-identical results: " << nErrors << std::endl;
    if (nErrors > 0) {
        std::cout << "ERRORS FOUND " << std::endl;
    }

    GEOSGeom_destroy(geom);
}

int main(int argc, char** argv) {
    if (argc != 4) {
        std::cout << "Reads a geometry from a WKT file" << std::endl;
        std::cout << "and executes GEOSTopologyPreserveSimplify(geom, TOL) N times," << std::endl;
        std::cout << "checking that the results are identical" << std::endl;
        std::cout << std::endl;
        std::cout << "Usage: perf_geostpsimplifystable wktfile TOL N" << std::endl;
        return 0;
    }

    std::string fname{argv[1]};
    std::cout << "Reading geometry from " << fname << std::endl;

    double tolerance = atof(argv[2]);
    std::cout << "Tolerance: " << tolerance << std::endl;

    int nRuns = std::atoi(argv[3]);
    std::cout << "Performing " << nRuns << " tests." << std::endl;

    run(fname, nRuns, tolerance);
}

