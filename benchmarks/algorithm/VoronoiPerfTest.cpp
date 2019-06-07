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

class VoronoiPerfTest {

public:
    void test(size_t num_points) {
        using namespace geos::geom;

        std::default_random_engine e(12345);
        std::uniform_real_distribution<> dis(0, 100);

        std::unique_ptr<std::vector<Coordinate>> coords(new std::vector<Coordinate>(num_points));
        std::generate(coords->begin(), coords->end(), [&dis, &e]() {
            return Coordinate(dis(e), dis(e));
        });
        CoordinateArraySequence seq(coords.release());
        auto geom = gfact->createLineString(seq.clone());

        voronoi(seq);
        voronoi(*geom);

        delaunay(seq);
        delaunay(*geom);

        std::cout << std::endl;
    }
private:
    decltype(geos::geom::GeometryFactory::create()) gfact = geos::geom::GeometryFactory::create();
    geos::util::Profiler* profiler = geos::util::Profiler::instance();

    template<typename T>
    void voronoi(const T & sites) {
        auto sw = profiler->get(std::string("Voronoi from ") + typeid(T).name());
        sw->start();

        geos::triangulate::VoronoiDiagramBuilder vdb;
        vdb.setSites(sites);

        auto result = vdb.getDiagram(*gfact);

        sw->stop();
        std::cout << sw->name << ": " << result->getNumGeometries() << ": " << *sw << std::endl;
    }

    template<typename T>
    void delaunay(const T & seq) {
        auto sw = profiler->get(std::string("Delaunay from ") + typeid(T).name());
        sw->start();

        geos::triangulate::DelaunayTriangulationBuilder dtb;
        dtb.setSites(seq);

        auto result = dtb.getTriangles(*gfact);

        sw->stop();
        std::cout << sw->name << ": " << result->getNumGeometries() << ": " << *sw << std::endl;
    }
};

int main() {
    VoronoiPerfTest tester;

    //tester.test(100);
    //tester.test(1000);
    //tester.test(10000);
    for (auto i = 0; i < 5; i++) {
        tester.test(100000);
    }
}