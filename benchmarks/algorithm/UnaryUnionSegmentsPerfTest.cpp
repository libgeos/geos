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

#include <geos/triangulate/DelaunayTriangulationBuilder.h>
#include <geos/triangulate/VoronoiDiagramBuilder.h>
#include <geos/geom/CoordinateArraySequence.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/profiler.h>

#include <algorithm>
#include <random>
#include <vector>
#include <memory>

class SegmentUnaryUnionPerfTest {

public:
    void test(size_t num_lines) {
        using namespace geos::geom;

        std::default_random_engine e(12345);
        std::uniform_real_distribution<> dis(0, 100);


        std::vector<std::unique_ptr<LineString>> lines;

        for (size_t i = 0; i < num_lines; i++) {
            CoordinateArraySequence cas(2, 2);
            cas.setAt(Coordinate(dis(e), dis(e)), 0);
            cas.setAt(Coordinate(dis(e), dis(e)), 1);

            lines.emplace_back(gfact->createLineString(std::move(cas)));
        }

        auto g = gfact->createMultiLineString(std::move(lines));

        auto sw = profiler->get("union");
        sw->start();

        g->Union();

        sw->stop();

        std::cout << *sw << std::endl;
    }
private:
    decltype(geos::geom::GeometryFactory::create()) gfact = geos::geom::GeometryFactory::create();
    geos::util::Profiler* profiler = geos::util::Profiler::instance();
};

int main(int argc, char** argv) {
    SegmentUnaryUnionPerfTest tester;

    auto num_lines = std::atol(argv[1]);
    auto num_reps = argc > 2 ? std::atol(argv[2]) : 1;

    for (std::size_t i = 0; i < num_reps; i++) {
        tester.test(num_lines);
    }
}
