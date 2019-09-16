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
    if (argc != 2 && argc != 3) {
        std::cout << "perf_intersection reads geometries from a WKT file and" << std::endl;
        std::cout << "inserts them into an STR-tree. For each input geometry, it" << std::endl;
        std::cout << "queries the tree to find all intersecting geometries and" << std::endl;
        std::cout << "then computes their intersection." << std::endl;
        std::cout << std::endl;
        std::cout << "Usage: perf_intersection [wktfile] [n]" << std::endl;
        return 0;
    }

    initGEOS(nullptr, nullptr);

    std::string fname{argv[1]};

    long max_geoms;
    if (argc == 3) {
        max_geoms = std::atol(argv[2]);
        std::cout << "Reading up to " << max_geoms << " geometries from " << fname << std::endl;
    } else {
        std::cout << "Reading geometries from " << fname << std::endl;
        max_geoms = -1;
    }

    std::vector<GEOSGeometry*> geoms;

    std::ifstream f(fname);
    std::string line;
    long i = 0;
    while(std::getline(f, line) && i < max_geoms) {
        geoms.push_back(GEOSGeomFromWKT(line.c_str()));
        i++;
    }
    f.close();

    std::cout << "Read " << geoms.size() << " geometries." << std::endl;

    GEOSSTRtree* tree = GEOSSTRtree_create(10);

    for (const auto& g : geoms) {
        GEOSSTRtree_insert(tree, g, g);
    }

    geos::util::Profile sw("Intersection");
    sw.start();

    for (const auto& g : geoms) {
        GEOSSTRtree_query(tree, g, [](void* g2v, void* g1v) {
            GEOSGeometry* g1 = (GEOSGeometry*) g1v;
            GEOSGeometry* g2 = (GEOSGeometry*) g2v;
            if (GEOSIntersects(g1, g2) == 1) {
                GEOSGeometry* g3 = GEOSIntersection(g1, g2);
                GEOSGeom_destroy(g3);
            }
        }, g);
    }

    sw.stop();
    std::cout << sw.getTotFormatted() << std::endl;

    GEOSSTRtree_destroy(tree);

    for (auto& g : geoms) {
        GEOSGeom_destroy(g);
    }
}
