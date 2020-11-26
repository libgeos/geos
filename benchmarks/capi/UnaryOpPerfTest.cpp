/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2019 Daniel Baston <dbaston@gmail.com>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/profiler.h>
#include <geos_c.h>

#include <fstream>
#include <iostream>
#include <sstream>

int main(int argc, char** argv) {
    if (argc != 3 && argc != 4) {
        std::cout << "perf_unary reads geometries from a WKT file and" << std::endl;
        std::cout << "performs a unary operation on each. The number of" << std::endl;
        std::cout << "geometries processed can be limited by specifying n." << std::endl;
        std::cout << std::endl;
        std::cout << "The following operations are supported:" << std::endl;
        std::cout << "- valid" << std::endl;
        std::cout << "- union" << std::endl;
        std::cout << std::endl;
        std::cout << "Usage: perf_unary [wktfile] [operation] [n]" << std::endl;
        return 0;
    }

    initGEOS(nullptr, nullptr);

    std::string fname{argv[1]};
    std::string op{argv[2]};

    long max_geoms;
    if (argc == 4) {
        max_geoms = std::atol(argv[3]);
        std::cout << "Reading up to " << max_geoms << " geometries from " << fname << std::endl;
    } else {
        std::cout << "Reading geometries from " << fname << std::endl;
        max_geoms = -1;
    }

    std::vector<GEOSGeometry*> geoms;

    std::ifstream f(fname);
    std::string line;
    long i = 0;
    while(std::getline(f, line) && (max_geoms < 0 || i < max_geoms)) {
        auto g = GEOSGeomFromWKT(line.c_str());
        if (g != nullptr) {
            geoms.push_back(g);
            i++;
        }
    }
    f.close();

    std::cout << "Read " << geoms.size() << " geometries." << std::endl;

    geos::util::Profile sw(op);
    sw.start();

    if (op == "valid") {
        for (const auto& g : geoms) {
            GEOSisValid(g);
        }
    } else if (op == "union") {
        for (const auto& g : geoms) {
            GEOSUnaryUnion(g);
        }
    }

    sw.stop();
    std::cout << sw.getTotFormatted() << std::endl;

    for (auto& g : geoms) {
        GEOSGeom_destroy(g);
    }
}
