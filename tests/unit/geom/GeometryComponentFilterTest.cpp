//
// Test Suite for geos::geom::GeometryComponentFilter class.

#include <tut/tut.hpp>
// geos
#include <geos/geom/Geometry.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/LineString.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/GeometryComponentFilter.h>
#include <geos/io/WKTReader.h>
// std
#include <memory>
#include <vector>

namespace tut {

struct test_geometrycomponentfilter_data {
    typedef geos::geom::Geometry::Ptr GeometryPtr; // owner
    typedef std::vector<geos::geom::Geometry const*> GeometryRefArray; // observer

    geos::geom::GeometryFactory::Ptr gf;
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

class NumPointsInFirstNComponents : public geos::geom::GeometryComponentFilter {
public:

    NumPointsInFirstNComponents(size_t n) : components_remaining(n), num_points(0) {}

    void
    filter_ro(const geos::geom::Geometry* g) override {
        num_points += g->getNumPoints();
        components_remaining--;
    }

    void
    filter_rw(geos::geom::Geometry* g) override {
        filter_ro(g);
    }

    size_t numPoints() {
        return num_points;
    }

    bool isDone() override {
        return components_remaining == 0;
    }
private:
    size_t components_remaining;
    size_t num_points;
};

// Split components into two categories: Lineal and all other types
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

    struct GeometryComponentSplitter : public geos::geom::GeometryComponentFilter {
        GeometryRefArray& lineal;
        GeometryRefArray& nonlineal;
        GeometryComponentSplitter(GeometryRefArray& p_lineal, GeometryRefArray& p_nonlineal)
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
    ensure_equals(lineal.size(), 5ul); // TODO: Why MultiLineString is in?
    // TODO: shouldn't be 1 for POLYGON?
    ensure_equals(nonlineal.size(), 3ul);
    // TODO: is 8 right?
    ensure_equals(lineal.size() + nonlineal.size(), 8ul);
}

template<>
template<>
void object::test<2>()
{
    // Test isDone() behavior for collections

    // collection of 4 geometries
    GeometryPtr g(reader.read("GEOMETRYCOLLECTION("
                              "POINT(0 0),"
                              "LINESTRING(0 0,1 1,1 2),"
                              "POLYGON((0 0,4 0,4 4,0 4,0 0)),"
                              "MULTILINESTRING((0 0,1 1,1 2),(2 3,3 2,5 4)))"));

    auto filter = NumPointsInFirstNComponents(3);
    g->apply_ro(&filter);

    // Current GeometryComponentFilter semantics have the filter visit the entire collection
    // in addition to each of its components. So the first 3 components are the the entire collection,
    // the Point, and the LineString
    ensure_equals(g->getNumPoints() + g->getGeometryN(0)->getNumPoints() + g->getGeometryN(1)->getNumPoints(),
            filter.numPoints());

    // isDone() behavior should be the same in apply_rw scenario.
    filter = NumPointsInFirstNComponents(2);
    g->apply_rw(&filter);
    ensure_equals(g->getNumPoints() + g->getGeometryN(0)->getNumPoints(),
            filter.numPoints());
}

template<>
template<>
void object::test<3>()
{
    // Test isDone() behavior for polygons

    // collection of 4 geometries
    GeometryPtr g(reader.read("POLYGON((0 0, 100 0, 100 100, 0 100, 0 0),"  // 5 pt exterior ring
                              "(1 1, 2 1, 2 2, 1 1),"                       // 4 pt interior ring
                              "(5 5, 5 8, 8 8, 7 7, 6 6, 5 5))"));          // 6 pt interior ring
    auto poly = dynamic_cast<geos::geom::Polygon*>(g.get());

    auto filter = NumPointsInFirstNComponents(3);
    poly->apply_ro(&filter);

    // Current GeometryComponentFilter semantics have the filter visit the entire polygon
    // in addition to each of its rings. So the first 3 components are the the entire polygon,
    // the the exterior ring, and the first interior ring.
    ensure_equals(poly->getNumPoints() + poly->getExteriorRing()->getNumPoints() + poly->getInteriorRingN(0)->getNumPoints(),
                  filter.numPoints());

    // isDone() behavior should be the same in apply_rw scenario.
    filter = NumPointsInFirstNComponents(2);
    g->apply_rw(&filter);
    ensure_equals(poly->getNumPoints() + poly->getExteriorRing()->getNumPoints(),
                  filter.numPoints());
}


} // namespace tut
