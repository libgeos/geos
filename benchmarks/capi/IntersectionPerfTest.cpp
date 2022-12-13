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
#include <deque>

using BinaryPredicate = decltype(&GEOSIntersects);
using BinaryOperation = decltype(&GEOSIntersection);

int main(int argc, char** argv) {
    if (argc < 2 || argc > 5) {
        std::cout << "perf_intersection reads geometries from a WKT file and" << std::endl;
        std::cout << "inserts them into an STR-tree. For each input geometry, it" << std::endl;
        std::cout << "queries the tree to find all intersecting geometries and" << std::endl;
        std::cout << "then computes their intersection." << std::endl;
        std::cout << std::endl;
        std::cout << "Usage: perf_intersection [wktfile] [n] [pred] [op]" << std::endl;
        return 0;
    }

    initGEOS(nullptr, nullptr);

    std::string fname{argv[1]};

    long max_geoms;
    if (argc >= 3 && std::string(argv[2]) != "all") {
        max_geoms = std::atol(argv[2]);
        std::cout << "Reading up to " << max_geoms << " geometries from " << fname << std::endl;
    } else {
        std::cout << "Reading geometries from " << fname << std::endl;
        max_geoms = std::numeric_limits<decltype(max_geoms)>::max();
    }

    std::string pred = "intersects";
    if (argc >= 4) {
        pred = argv[3];
    }

    BinaryPredicate predfn = GEOSIntersects;
    if (pred == "intersects") {
        predfn = GEOSIntersects;
    } else if (pred == "contains") {
        predfn = GEOSContains;
    } else if (pred == "covers") {
        predfn = GEOSCovers;
    } else if (pred == "within") {
        predfn = GEOSWithin;
    } else if (pred == "coveredby") {
        predfn = GEOSCoveredBy;
    } else if (pred == "touches") {
        predfn = GEOSTouches;
    } else if (pred == "overlaps") {
        predfn = GEOSOverlaps;
    } else if (pred == "crosses") {
        predfn = GEOSCrosses;
    } else if (pred == "equals") {
        predfn = GEOSEquals;
    } else {
        std::cerr << "Unknown predicate." << std::endl;
        return 1;
    }

    std::string overlay = "intersection";
    if (argc >= 5) {
        overlay = argv[4];
    }
    BinaryOperation overlayfn = GEOSIntersection;
    if (overlay == "intersection") {
        overlayfn = GEOSIntersection;
    } else if (overlay == "none") {
        overlayfn = nullptr;
    }

    std::vector<GEOSGeometry*> geoms;

    std::ifstream f(fname);
    std::string line;
    long i = 0;
    std::deque<long> lineNos;
    while(std::getline(f, line) && i < max_geoms) {
        auto geom = GEOSGeomFromWKT(line.c_str());
        if (geom) {
            i++;
            lineNos.push_back(i);
            GEOSGeom_setUserData(geom, &lineNos.back());
            geoms.push_back(geom);
        }
    }
    f.close();

    std::cout << "Read " << geoms.size() << " geometries." << std::endl;
    std::cout << "Testing according to predicate: " << pred << " and performing operation: " << overlay << std::endl;

    GEOSSTRtree* tree = GEOSSTRtree_create(10);

    for (const auto& g : geoms) {
        GEOSSTRtree_insert(tree, g, g);
    }

    geos::util::Profile sw("Intersection");
    sw.start();

    struct QueryContext {
        GEOSGeometry* queryGeom;
        BinaryPredicate pred;
        BinaryOperation overlay;
        std::size_t* treeHits;
        std::size_t* predHits;
    };

    std::size_t treeHits = 0;
    std::size_t predHits = 0;


    for (const auto& g : geoms) {
        QueryContext ctxt{g, predfn, overlayfn, &treeHits, &predHits};

        GEOSSTRtree_query(tree, g, [](void* item, void* data) {
            auto context = static_cast<QueryContext*>(data);
            ++*context->treeHits;

            GEOSGeometry* g1 = (GEOSGeometry*) context->queryGeom;
            GEOSGeometry* g2 = (GEOSGeometry*) item;

#if 0
            std::cout << "Eval pred btwn geom " << *static_cast<long*>(GEOSGeom_getUserData(g1)) << " and " << *static_cast<long*>(GEOSGeom_getUserData(g2)) << std::endl;
#endif

            if ((context->pred)(g1, g2) == 1) {
                ++*context->predHits;

                if (context->overlay) {
                    GEOSGeometry* g3 = (context->overlay)(g1, g2);
                    GEOSGeom_destroy(g3);
                }
            }
        }, &ctxt);
    }

    sw.stop();
    std::cerr << predHits << " out of " << treeHits << " bounding box intersections satisfied predicate: " << pred << " (" << 100 * static_cast<double>(predHits) / static_cast<double>(treeHits) << "%)" << std::endl;
    std::cout << sw.getTotFormatted() << std::endl;

    GEOSSTRtree_destroy(tree);

    for (auto& g : geoms) {
        GEOSGeom_destroy(g);
    }
}

