#include <tut/tut.hpp>
#include <unordered_set>

#include <geos/noding/ArcString.h>
#include <geos/noding/DirectionIndependentArcString.h>
#include <geos/geom/CircularString.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/io/WKTReader.h>

using namespace geos::geom;
using namespace geos::noding;

namespace tut {

struct test_directionindependentarcstring_data {
    geos::io::WKTReader reader_;

    static bool isSame(const ArcString& as0, const ArcString& as1) {
        std::unordered_set<DirectionIndependentArcString> map;
        map.emplace(as0);

        return !map.emplace(as1).second;
    }

    static bool isSame(const CircularString& cs0, const CircularString& cs1) {
        ArcString as0(cs0.getArcs(), cs0.getSharedCoordinates(), nullptr);
        ArcString as1(cs1.getArcs(), cs1.getSharedCoordinates(), nullptr);

        return isSame(as0, as1);
    }

    bool isSame(const std::string& wkt0, const std::string& wkt1) {
        auto cs0 = reader_.read<CircularString>(wkt0);
        auto cs1 = reader_.read<CircularString>(wkt1);

        return isSame(*cs0, *cs1);
    }
};

using group = test_group<test_directionindependentarcstring_data>;
using object = group::object;

group test_directionindependentarcstring_data("geos::noding::DirectionIndependentArcString");

template<>
template<>
void object::test<1>()
{
    set_test_name("equivalent: single-section arcs with different control points and opposite directions");

    ensure(isSame("CIRCULARSTRING (-5 0, -4 3, 5 0)", "CIRCULARSTRING (5 0, 4 3, -5 0)"));
}

template<>
template<>
void object::test<2>()
{
    set_test_name("equivalent: multi-section arcs with different control points and opposite directions");

    ensure(isSame("CIRCULARSTRING (-5 0, -4 3, 5 0, 6 1, 7 0)", "CIRCULARSTRING (7 0, 6 1, 5 0, 4 3, -5 0)"));
}

template<>
template<>
void object::test<3>()
{
    set_test_name("different: single-section arcs with same endpoints, covering different halves of circle");

    ensure(!isSame("CIRCULARSTRING (-5 0, -4 3, 5 0)", "CIRCULARSTRING (-5 0, -4 -3, 5 0)"));
}

template<>
template<>
void object::test<4>()
{
    set_test_name("equivalent: two degenerate (linear) arcs having different control points and opposite directions");

    ensure(isSame("CIRCULARSTRING (0 0, 4 4, 10 10)", "CIRCULARSTRING (10 10, 8 8, 0 0)"));
}

template<>
template<>
void object::test<5>()
{
    set_test_name("different: arc and degenerate arc having same endpoints");

    ensure(!isSame("CIRCULARSTRING (-1 0, 0 1, 1 0)", "CIRCULARSTRING (-1 0, 0 0, 1 0)"));
}

template<>
template<>
void object::test<6>()
{
    set_test_name("different: two arcs having same endpoints and different control points");

    ensure(!isSame("CIRCULARSTRING (-1 0, 0 1, 1 0)", "CIRCULARSTRING (-1 0, 0 0.9, 1 0)"));
}

template<>
template<>
void object::test<7>()
{
    set_test_name("different: two arcs sharing same path that later diverge");

    ensure(!isSame("CIRCULARSTRING (-1 0, 0 1, 1 0, 2 -1, 3 0)", "CIRCULARSTRING (-1 0, 0 1, 1 0, 2 -1, 3 0.1)"));
}

}