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

#include <geos/geom/Coordinate.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/util/SineStarFactory.h>

#include <algorithm>
#include <random>
#include <vector>
#include <memory>

#include <fstream>
#include <iostream>
#include <sstream>

class GEOSPreparedContainsPerfTest {

public:
    void test(const std::vector<GEOSGeometry*>& geoms, std::size_t num_points) {
        using namespace geos::geom;

        geos::util::Profile sw("GEOSPreparedContains");
        geos::util::Profile sw_xy("GEOSPreparedContainsXY");

        std::size_t hits = 0;
        std::size_t hits_xy = 0;

        for (const auto& g: geoms) {
            double xmin, xmax, ymin, ymax;

            GEOSGeom_getXMin(g, &xmin);
            GEOSGeom_getXMax(g, &xmax);
            GEOSGeom_getYMin(g, &ymin);
            GEOSGeom_getYMax(g, &ymax);

            std::default_random_engine e(12345);
            std::uniform_real_distribution<> xdist(xmin, xmax);
            std::uniform_real_distribution<> ydist(ymin, ymax);

            std::vector<Coordinate> coords(num_points);
            std::generate(coords.begin(), coords.end(), [&xdist, &ydist, &e]() {
                return Coordinate(xdist(e), ydist(e));
            });


            sw.start();
            auto prep = GEOSPrepare(g);
            for (const auto& c : coords) {
                auto pt = GEOSGeom_createPointFromXY(c.x, c.y);
                if (GEOSPreparedContains(prep, pt)) {
                    hits++;
                }

                GEOSGeom_destroy(pt);
            }
            GEOSPreparedGeom_destroy(prep);
            sw.stop();

            sw_xy.start();
            prep = GEOSPrepare(g);
            for (const auto& c : coords) {
                if (GEOSPreparedContainsXY(prep, c.x, c.y)) {
                    hits_xy++;
                }
            }
            GEOSPreparedGeom_destroy(prep);
            sw_xy.stop();
        }

        std::cout << sw.name << ": " << hits << " hits from " << num_points << " points in " <<  sw.getTotFormatted() << std::endl;
        std::cout << sw_xy.name << ": " << hits_xy << " hits from " << num_points << " points in " <<  sw_xy.getTotFormatted() << std::endl;
    }
};

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cout << "perf_geospreparedcontains performs a specified number of point-in-polygon tests" << std::endl;
        std::cout << "on randomly generated points from the bounding box of each geometry provided" << std::endl;
        std::cout << "in a file as WKT." << std::endl;
        std::cout << std::endl;
        std::cout << "Usage: perf_geospreparedcontains [wktfile] [n]" << std::endl;
        return 0;
    }

    initGEOS(nullptr, nullptr);

    GEOSPreparedContainsPerfTest tester;

    std::size_t n = static_cast<std::size_t>(std::atoi(argv[2]));
    std::cout << "Performing " << n << " point-in-polygon tests." << std::endl;

    std::string fname{argv[1]};
    std::cout << "Reading shapes from " << fname << std::endl;

    std::vector<GEOSGeometry*> geoms;

    std::ifstream f(fname);
    std::string line;
    long i = 0;
    while(std::getline(f, line)) {
        if (line != "") {
            geoms.push_back(GEOSGeomFromWKT(line.c_str()));
            i++;
        }
    }
    f.close();

    std::cout << "Read " << geoms.size() << " geometries." << std::endl;

    tester.test(geoms, n);

    for (auto& g : geoms) {
        GEOSGeom_destroy(g);
    }
}
