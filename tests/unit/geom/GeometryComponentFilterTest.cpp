//
// Test Suite for geos::geom::GeometryComponentFilter class.

#include <tut.hpp>
// geos
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/GeometryComponentFilter.h>
#include <geos/io/WKTReader.h>
// std
#include <memory>
#include <vector>

namespace tut
{

struct test_geometrycomponentfilter_data
{
    typedef geos::geom::Geometry::AutoPtr GeometryPtr; // owner
    typedef std::vector<geos::geom::Geometry const*> GeometryRefArray; // observer

    geos::geom::GeometryFactory::unique_ptr gf;
    geos::io::WKTReader reader;
    test_geometrycomponentfilter_data()
        : gf(geos::geom::GeometryFactory::create())
        , reader(gf.get())
    {
    }
};

typedef test_group<test_geometrycomponentfilter_data> group;
typedef group::object object;

group test_geometrycomponentfilter_group("geos::geom::GeometryComponentFilter");

//
// Test Cases
//

// Split components into two categories: Lineal and all other types
template<>
template<>
void object::test<1>()
{
    // collection of 4 geometries
    GeometryPtr g(reader.read("GEOMETRYCOLLECTION("
        "POINT(0 0),"
        "LINESTRING(0 0,1 1,1 2),"
        "POLYGON((0 0,4 0,4 4,0 4,0 0)),"
        "MULTILINESTRING((0 0,1 1,1 2),(2 3,3 2,5 4)))"));

    struct GeometryComponentSplitter : public geos::geom::GeometryComponentFilter
    {
        GeometryRefArray& lineal;
        GeometryRefArray& nonlineal;
        GeometryComponentSplitter(GeometryRefArray& lineal, GeometryRefArray& nonlineal)
            : lineal(lineal), nonlineal(nonlineal)
        {
            ensure(lineal.empty());
            ensure(nonlineal.empty());
        }
        void filter_ro(geos::geom::Geometry const* g)
        {
            if (dynamic_cast<geos::geom::Lineal const*>(g))
                lineal.push_back(g);
            else
                nonlineal.push_back(g);
        }
        void filter_rw(geos::geom::Geometry*) {}
    };

    GeometryRefArray lineal;
    GeometryRefArray nonlineal;
    GeometryComponentSplitter splitter(lineal, nonlineal);
    g->apply_ro(&splitter);

    // TODO: Verify the actual results
    // lineal:
    //   0: LineString
    //   1: LinearRing
    //   2: MultiLineString
    //   3: LineString
    //   4: LineString
    // nonlineal:
    //   0: GeometryCollection
    //   1: Point
    //   2: Polygon
    ensure_equals(lineal.size(), 5); // TODO: Why MultiLineString is in?
    // TODO: shouldn't be 1 for POLYGON?
    ensure_equals(nonlineal.size(), 3);
    // TODO: is 8 right?
    ensure_equals(lineal.size() + nonlineal.size(), 8);
}

} // namespace tut
