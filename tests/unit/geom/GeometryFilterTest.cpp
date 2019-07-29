//
// Test Suite for geos::geom::GeometryFilter class.

#include <tut/tut.hpp>
// geos
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/GeometryFilter.h>
#include <geos/io/WKTReader.h>
// std
#include <memory>
#include <vector>

namespace tut {

struct test_geometryfilter_data {
    typedef geos::geom::Geometry::Ptr GeometryPtr; // owner
    typedef std::vector<geos::geom::Geometry const*> GeometryRefArray; // observer

    geos::geom::GeometryFactory::Ptr gf;
    geos::io::WKTReader reader;
    test_geometryfilter_data()
        : gf(geos::geom::GeometryFactory::create())
        , reader(gf.get())
    {
    }
};

typedef test_group<test_geometryfilter_data> group;
typedef group::object object;

group test_geometryfilter_group("geos::geom::GeometryFilter");

//
// Test Cases
//

// Split geometries into two categories: Lineal and all other types
template<>
template<>
void object::test<1>
()
{
    // collection of 4 geometries
    GeometryPtr g(reader.read("GEOMETRYCOLLECTION("
                              "POINT(0 0),"
                              "LINESTRING(0 0,1 1,1 2),"
                              "POLYGON((0 0,4 0,4 4,0 4,0 0)),"
                              "MULTILINESTRING((0 0,1 1,1 2),(2 3,3 2,5 4)))"));

    struct GeometrySplitter : public geos::geom::GeometryFilter {
        GeometryRefArray& lineal;
        GeometryRefArray& nonlineal;
        GeometrySplitter(GeometryRefArray& p_lineal, GeometryRefArray& p_nonlineal)
            : lineal(p_lineal), nonlineal(p_nonlineal)
        {
            ensure(lineal.empty());
            ensure(nonlineal.empty());
        }
        void
        filter_ro(geos::geom::Geometry const* g) override
        {
            if(g->isLineal()) {
                lineal.push_back(g);
            }
            else {
                nonlineal.push_back(g);
            }
        }
        void
        filter_rw(geos::geom::Geometry*) override {}
    };

    GeometryRefArray lineal;
    GeometryRefArray nonlineal;
    GeometrySplitter splitter(lineal, nonlineal);
    g->apply_ro(&splitter);

    // TODO: Verify the actual results
    // lineal:
    //   0: LineString
    //   1: MultiLineString
    //   2: LineString
    //   3: LineString
    // nonlineal:
    //   0: GeometryCollection
    //   1: Point
    //   2: Polygon
    ensure_equals(lineal.size(), 4ul);
    // TODO: shouldn't be 1 for POLYGON?
    ensure_equals(nonlineal.size(), 3ul);
    // TODO: is 7, shouldn't be not 4?
    //ensure_equals(lineal.size() + nonlineal.size(), g->getNumGeometries());
    ensure_equals(g->getNumGeometries(), 4ul);
    ensure_equals(lineal.size() + nonlineal.size(), 7ul);
}

} // namespace tut
