#include <tut/tut.hpp>
#include <tut/tut_macros.hpp>
#include <utility.h>

#include <geos/operation/grid/TraversalAreas.h>

using geos::operation::grid::TraversalAreas;
using geos::geom::CoordinateXY;
using geos::geom::Envelope;
using geos::geom::GeometryFactory;

namespace tut {
struct test_traversalareastest_data : GEOSTestBase {
    using TraversalVector = std::vector<const std::vector<CoordinateXY>*>;

    const GeometryFactory& gfact = *GeometryFactory::getDefaultInstance();
};

typedef test_group<test_traversalareastest_data, 255> group;
typedef group::object object;

group test_traversalareastest_group("geos::operation::grid::TraversalAreas");


template<>
template<>
void object::test<1>()
{
    set_test_name("Exit from same side as entry");

    Envelope b{ 0, 10, 0, 10 };

    std::vector<CoordinateXY> traversal{ { 7, 0 }, { 7, 1 }, { 6, 1 }, { 6, 0 } };
    TraversalVector traversals{ &traversal };

    ensure_equals(TraversalAreas::getLeftHandArea(b, traversals),  1);
    ensure_equals_geometry(TraversalAreas::getLeftHandRings(gfact, b, traversals).get(), "POLYGON ((6 0, 7 0, 7 1, 6 1, 6 0))");

    std::reverse(traversal.begin(), traversal.end());
    ensure_equals(TraversalAreas::getLeftHandArea(b, traversals), 99);
    ensure_equals_geometry(TraversalAreas::getLeftHandRings(gfact, b, traversals).get(), "POLYGON ((0 0, 6 0, 6 1, 7 1, 7 0, 10 0, 10 10, 0 10, 0 0))");
}

template<>
template<>
void object::test<2>()
{
    set_test_name("Enter bottom, exit left");

    Envelope b{ 0, 10, 0, 10 };

    std::vector<CoordinateXY> traversal{ { 5, 0 }, { 5, 5 }, { 0, 5 } };
    TraversalVector traversals{ &traversal };

    ensure_equals(TraversalAreas::getLeftHandArea(b, traversals), 25);
    ensure_equals_geometry(TraversalAreas::getLeftHandRings(gfact, b, traversals).get(), "POLYGON ((0 0, 5 0, 5 5, 0 5, 0 0))");
}

template<>
template<>
void object::test<3>()
{
    set_test_name("Enter bottom, exit top");

    Envelope b{ 0, 10, 0, 10 };

    std::vector<CoordinateXY> traversal{ { 4, 0 }, { 4, 10 } };
    TraversalVector traversals{ &traversal };

    ensure_equals(TraversalAreas::getLeftHandArea(b, traversals), 40);
    ensure_equals_geometry(TraversalAreas::getLeftHandRings(gfact, b, traversals).get(), "POLYGON ((0 0, 4 0, 4 10, 0 10, 0 0))");
}

template<>
template<>
void object::test<4>()
{
    set_test_name("Multiple traversals (basic)");

    Envelope b{ 0, 10, 0, 10 };

    std::vector<CoordinateXY> t1 = { { 2, 10 }, { 2, 0 } };
    std::vector<CoordinateXY> t2 = { { 4, 0 }, { 4, 10 } };

    TraversalVector traversals{ &t1, &t2 };

    ensure_equals(TraversalAreas::getLeftHandArea(b, traversals), 20);
    ensure_equals_geometry(TraversalAreas::getLeftHandRings(gfact, b, traversals).get(), "POLYGON ((2 0, 4 0, 4 10, 2 10, 2 0))");
}

template<>
template<>
void object::test<5>()
{
    set_test_name("Multiple traversals");

    Envelope b{ 0, 10, 0, 10 };

    std::vector<CoordinateXY> t1 = { { 2, 0 }, { 2, 2 }, { 0, 2 } }; // 2x2 = 4
    std::vector<CoordinateXY> t2 = { { 3, 10 }, { 3, 0 } };
    std::vector<CoordinateXY> t3 = { { 5, 0 }, { 5, 10 } };                      // 2x10 = 20
    std::vector<CoordinateXY> t4 = { { 8, 10 }, { 10, 8 } };                     // 2x2/2 = 2
    std::vector<CoordinateXY> t5 = { { 10, 6 }, { 8, 6 }, { 8, 3 }, { 10, 3 } }; // 2x3 = 6
    std::vector<CoordinateXY> t6 = { { 10, 4 }, { 9, 4 }, { 9, 5 }, { 10, 5 } }; // 1x1 = 1 (subtracted)
    std::vector<CoordinateXY> t7 = { { 10, 3 }, { 8, 3 }, { 8, 0 } };            // 2x3 = 6

    TraversalVector traversals{ &t1, &t2, &t3, &t4, &t5, &t6, &t7 };

    ensure_equals(TraversalAreas::getLeftHandArea(b, traversals), 4 + 20 + 2 + 6 - 1 + 6);
    ensure_equals_geometry(TraversalAreas::getLeftHandRings(gfact, b, traversals).get(), "MULTIPOLYGON (((2 0, 2 2, 0 2, 0 0, 2 0)), "
                                                                            "((3 10, 3 0, 5 0, 5 10, 3 10)), "
                                                                            "((8 10, 10 8, 10 10, 8 10)), "
                                                                            "((10 6, 8 6, 8 3, 10 3, 10 3, 8 3, 8 0, 10 0, 10 4, 9 4, 9 5, 10 5, 10 6)))");
}

template<>
template<>
void object::test<6>()
{
    set_test_name("No traversals");

    Envelope b{ 0, 10, 0, 10 };

    TraversalVector traversals;

    ensure_THROW(TraversalAreas::getLeftHandArea(b, traversals), std::exception);
    ensure_THROW(TraversalAreas::getLeftHandRings(gfact, b, traversals), std::exception);
}

template<>
template<>
void object::test<7>()
{
    set_test_name("Point traversal");

    Envelope b{ 0, 10, 0, 10 };

    std::vector<CoordinateXY> t1{ { 4, 0 }, { 4, 0 } };
    TraversalVector traversals{ &t1 };

    ensure_THROW(TraversalAreas::getLeftHandArea(b, traversals), std::exception);
    ensure_THROW(TraversalAreas::getLeftHandRings(gfact, b, traversals), std::exception);
}

template<>
template<>
void object::test<8>()
{
    set_test_name("Closed ring ccw");

    Envelope b{ 0, 10, 0, 10 };

    std::vector<CoordinateXY> t1 = { { 1, 1 }, { 2, 1 }, { 2, 2 }, { 1, 2 }, { 1, 1 } };
    TraversalVector traversals{ &t1 };

    ensure_equals(TraversalAreas::getLeftHandArea(b, traversals), 1);
    ensure_equals_geometry(TraversalAreas::getLeftHandRings(gfact, b, traversals).get(), "POLYGON ((1 1, 2 1, 2 2, 1 2, 1 1))");
}

template<>
template<>
void object::test<9>()
{
    set_test_name("Closed ring ccw overlapping edge");

    Envelope b{ 0, 10, 00, 10 };

    std::vector<CoordinateXY> t1 = { { 1, 0 }, { 2, 1 }, { 1, 1 }, { 1, 0 } };
    TraversalVector traversals{ &t1 };

    ensure_equals(TraversalAreas::getLeftHandArea(b, traversals), 0.5);
    ensure_equals_geometry(TraversalAreas::getLeftHandRings(gfact, b, traversals).get(), "POLYGON ((1 0, 1 1, 2 1, 1 0))");
}

template<>
template<>
void object::test<10>()
{
    set_test_name("Closed ring cw");

    Envelope b{ 0, 10, 0, 10 };

    std::vector<CoordinateXY> t1 = { { 1, 1 }, { 1, 2 }, { 2, 2 }, { 2, 1 }, { 1, 1 } };
    TraversalVector traversals{ &t1 };

    ensure_equals(TraversalAreas::getLeftHandArea(b, traversals), 99);
    ensure_equals_geometry(TraversalAreas::getLeftHandRings(gfact, b, traversals).get(), "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0), (1 1, 1 2, 2 2, 2 1, 1 1))");
}

template<>
template<>
void object::test<11>()
{
    set_test_name("Closed ring cw with point traversal");

    Envelope b{ 0, 10, 0, 10 };

    std::vector<CoordinateXY> t1 = { { 1, 1 }, { 1, 2 }, { 2, 2 }, { 2, 1 }, { 1, 1 } };
    std::vector<CoordinateXY> t2 = { { 10, 5 }, { 10, 5 } };
    TraversalVector traversals{ &t1, &t2 };

    ensure_equals(TraversalAreas::getLeftHandArea(b, traversals), 99);
    ensure_equals_geometry(TraversalAreas::getLeftHandRings(gfact, b, traversals).get(), "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0), (1 1, 1 2, 2 2, 2 1, 1 1))");
}

template<>
template<>
void object::test<12>()
{
    set_test_name("Closed ring cw touching edge at node");

    Envelope b{ 0, 10, 0, 10 };

    std::vector<CoordinateXY> t1 = { { 0, 0 }, { 2, 2 }, { 3, 2 }, { 0, 0 } };
    TraversalVector traversals{ &t1 };

    ensure_equals(TraversalAreas::getLeftHandArea(b, traversals), 99);
    ensure_equals_geometry(TraversalAreas::getLeftHandRings(gfact, b, traversals).get(), "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0), (0 0, 2 2, 3 2, 0 0))");
}

template<>
template<>
void object::test<13>()
{
    set_test_name("Closed ring cw touching edge interior");

    Envelope b{ 0, 10, 0, 10 };

    std::vector<CoordinateXY> t1 = { { 1, 0 }, { 2, 2 }, { 3, 2 }, { 1, 0 } };
    TraversalVector traversals{ &t1 };

    ensure_equals(TraversalAreas::getLeftHandArea(b, traversals), 99);
    ensure_equals_geometry(TraversalAreas::getLeftHandRings(gfact, b, traversals).get(), "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0), (1 0, 2 2, 3 2, 1 0))");
}

template<>
template<>
void object::test<14>()
{
    set_test_name("Closed ring cw overlapping edge");

    Envelope b{ 0, 10, 0, 10 };

    std::vector<CoordinateXY> t1 = { { 1, 0 }, { 1, 1 }, { 2, 1 }, { 1, 0 } };
    TraversalVector traversals{ &t1 };

    ensure_equals(TraversalAreas::getLeftHandArea(b, traversals), 99.5);
    ensure_equals_geometry(TraversalAreas::getLeftHandRings(gfact, b, traversals).get(), "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0), (1 0, 1 1, 2 1, 1 0))");
}

template<>
template<>
void object::test<15>()
{
    set_test_name("Edge traversal (interior left)");

    Envelope b{ 0, 10, 0, 10 };

    std::vector<CoordinateXY> t1{ { 4, 0 }, { 10, 0 } };
    TraversalVector traversals{ &t1 };

    ensure_equals(TraversalAreas::getLeftHandArea(b, traversals), 100);
    // FIXME remove repeated point from result
    ensure_equals_geometry(TraversalAreas::getLeftHandRings(gfact, b, traversals).get(), "POLYGON ((0 0, 4 0, 10 0, 10 0, 10 10, 0 10, 0 0))");
}

template<>
template<>
void object::test<16>()
{
    set_test_name("Edge traversal (interior right)");

    Envelope b{ 2, 3, 2, 3 };

    std::vector<CoordinateXY> t1{ { 2, 2 }, { 2, 2.5 }, { 2, 2.5 } };
    TraversalVector traversals{ &t1 };

    ensure_equals(TraversalAreas::getLeftHandArea(b, traversals), 0);
    ensure_equals_geometry(TraversalAreas::getLeftHandRings(gfact, b, traversals).get(), "POLYGON EMPTY");
}

template<>
template<>
void object::test<17>() {
    set_test_name("lake with island");

    Envelope b(0, 10, 0, 10);
    std::vector<CoordinateXY> lake{{1, 1}, {1, 9}, {9, 9}, {9, 1}, {1, 1}};
    std::vector<CoordinateXY> island{{2, 2}, {4, 2}, {4, 4}, {2, 4}, {2, 2}};
    TraversalVector traversals{ &island, &lake };

    ensure_equals(TraversalAreas::getLeftHandArea(b, traversals), 40);
    //ensure_equals_geometry(TraversalAreas::getLeftHandRings(gfact, b, traversals).get(),
    //"MULTIPOLYGON (((0 0, 10 0, 10 10, 0 10, 0 0), (1 1, 9 1, 9 9, 1 9, 1 1)), ((2 2, 4 2, 4 4, 2 4, 2 2)))");
}


}
