//
// Test Suite for PreparedGeometry methods

// tut
#include <tut/tut.hpp>
#include <utility.h>
// geos
#include <geos/geom/prep/PreparedGeometryFactory.h>
#include <geos/geom/prep/PreparedGeometry.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Geometry.h>
#include <geos/io/WKTReader.h>
// std
#include <memory>
#include <thread>

using namespace geos::geom;
using geos::geom::prep::PreparedGeometry;

namespace tut {

//
// Test Group
//

struct test_preparedgeometry_data {
    typedef geos::geom::GeometryFactory GeometryFactory;

    geos::geom::GeometryFactory::Ptr factory;
    geos::io::WKTReader reader;
    std::unique_ptr<geos::geom::Geometry> g1;
    std::unique_ptr<geos::geom::Geometry> g2;
    std::unique_ptr<PreparedGeometry> pg1;
    std::unique_ptr<PreparedGeometry> pg2;

    test_preparedgeometry_data()
        : factory(GeometryFactory::create())
        , reader(factory.get())
        , g1(nullptr)
        , g2(nullptr)
        , pg1(nullptr)
        , pg2(nullptr)
    {}
};

typedef test_group<test_preparedgeometry_data> group;
typedef group::object object;

group test_preparedgeometry_data("geos::geom::prep::PreparedGeometry");

//
// Test Cases
//

// 1 - Check EMPTY elements are handled correctly
// See https://trac.osgeo.org/postgis/ticket/5224
template<>
template<>
void object::test<1>
()
{
    g1 = reader.read( "MULTIPOLYGON (((9 9, 9 1, 1 1, 2 4, 7 7, 9 9)), EMPTY)" );
    g2 = reader.read( "MULTIPOLYGON (((7 6, 7 3, 4 3, 7 6)), EMPTY)" );

    pg1 = prep::PreparedGeometryFactory::prepare(g1.get());

    ensure( pg1->intersects(g2.get()));
    ensure( pg1->contains(g2.get()));
    ensure( pg1->covers(g2.get()));
}

// Check prepared geometry can be used from multiple threads
template<>
template<>
void object::test<2>
()
{
    std::vector<std::unique_ptr<Geometry>> geoms;
    std::vector<std::unique_ptr<PreparedGeometry>> ppolys;
    std::vector<std::unique_ptr<PreparedGeometry>> plines;
    std::vector<std::thread> threads;

    constexpr std::size_t nrow = 10;
    constexpr std::size_t ncol = 10;
    constexpr std::size_t nthreads = 10;

    for (std::size_t i = 0; i < ncol; i++) {
        for (std::size_t j = 0; j < nrow; j++) {
            CoordinateXY c(static_cast<double>(i), static_cast<double>(j));
            auto pt = factory->createPoint(c);

            geoms.emplace_back(pt->buffer(1.5));
            ppolys.push_back(prep::PreparedGeometryFactory::prepare(geoms.back().get()));
            plines.push_back(prep::PreparedGeometryFactory::prepare(static_cast<Polygon*>(geoms.back().get())->getExteriorRing()));
        }
    }

    auto findIntersects = [&geoms](const PreparedGeometry* pg) {
        for (const auto& geom : geoms) {
            pg->intersects(geom.get());
            pg->distance(geom.get());
        }
    };

    // check PreparedPolygon
    for (std::size_t i = 0; i < nthreads; i++) {
        threads.emplace_back(findIntersects, ppolys[0].get());
    }

    for (auto& thread : threads) {
        thread.join();
    }

    // check PreparedLineString
    threads.clear();
    for (std::size_t i = 0; i < nthreads; i++) {
        threads.emplace_back(findIntersects, plines[0].get());
    }

    for (auto& thread : threads) {
        thread.join();
    }

}

} // namespace tut
