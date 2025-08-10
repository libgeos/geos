#include <tut/tut.hpp>
#include <tut/tut_macros.hpp>
#include <utility.h>

#include <geos/operation/grid/Traversal.h>
#include <geos/operation/grid/TraversalAreas.h>

using geos::operation::grid::Side;
using geos::operation::grid::Traversal;
using geos::operation::grid::TraversalAreas;
using geos::geom::CoordinateXY;
using geos::geom::Envelope;
using geos::geom::GeometryFactory;

namespace tut {
struct test_traversalareastest_data : GEOSTestBase {
    using TraversalVector = std::vector<const Traversal*>;

    Traversal make_traversal(const std::vector<CoordinateXY>& coords, void* parentage=nullptr)
    {
        Traversal t;
        if (!coords.empty()) {
            t.enter(coords.front(), Side::NONE, parentage);
        }

        for (std::size_t i = 1; i < coords.size() - 1; i++) {
            t.add(coords[i]);
        }

        t.exit(coords.back(), Side::NONE);

        return t;
    }

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

    std::vector<CoordinateXY> coords{ { 7, 0 }, { 7, 1 }, { 6, 1 }, { 6, 0 } };
    Traversal t = make_traversal(coords);
    TraversalVector traversals{ &t };

    ensure_equals(TraversalAreas::getLeftHandArea(b, traversals),  1);
    ensure_equals_geometry(TraversalAreas::getLeftHandRings(gfact, b, traversals).get(), "POLYGON ((6 0, 7 0, 7 1, 6 1, 6 0))");

    std::reverse(coords.begin(), coords.end());
    t = make_traversal(coords);
    traversals = { &t };
    ensure_equals(TraversalAreas::getLeftHandArea(b, traversals), 99);
    ensure_equals_geometry(TraversalAreas::getLeftHandRings(gfact, b, traversals).get(), "POLYGON ((0 0, 6 0, 6 1, 7 1, 7 0, 10 0, 10 10, 0 10, 0 0))");
}

template<>
template<>
void object::test<2>()
{
    set_test_name("Enter bottom, exit left");

    Envelope b{ 0, 10, 0, 10 };

    std::vector<CoordinateXY> coords{ { 5, 0 }, { 5, 5 }, { 0, 5 } };
    Traversal t = make_traversal(coords);
    TraversalVector traversals{ &t };

    ensure_equals(TraversalAreas::getLeftHandArea(b, traversals), 25);
    ensure_equals_geometry(TraversalAreas::getLeftHandRings(gfact, b, traversals).get(), "POLYGON ((0 0, 5 0, 5 5, 0 5, 0 0))");
}

template<>
template<>
void object::test<3>()
{
    set_test_name("Enter bottom, exit top");

    Envelope b{ 0, 10, 0, 10 };

    std::vector<CoordinateXY> coords{ { 4, 0 }, { 4, 10 } };
    Traversal t = make_traversal(coords);
    TraversalVector traversals{ &t };

    ensure_equals(TraversalAreas::getLeftHandArea(b, traversals), 40);
    ensure_equals_geometry(TraversalAreas::getLeftHandRings(gfact, b, traversals).get(), "POLYGON ((0 0, 4 0, 4 10, 0 10, 0 0))");
}

template<>
template<>
void object::test<4>()
{
    set_test_name("Multiple traversals (basic)");

    Envelope b{ 0, 10, 0, 10 };

    std::vector<CoordinateXY> c1 = { { 2, 10 }, { 2, 0 } };
    std::vector<CoordinateXY> c2 = { { 4, 0 }, { 4, 10 } };
    Traversal t1 = make_traversal(c1);
    Traversal t2 = make_traversal(c2);
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

    std::vector<CoordinateXY> c1 = { { 2, 0 }, { 2, 2 }, { 0, 2 } }; // 2x2 = 4
    std::vector<CoordinateXY> c2 = { { 3, 10 }, { 3, 0 } };
    std::vector<CoordinateXY> c3 = { { 5, 0 }, { 5, 10 } };                      // 2x10 = 20
    std::vector<CoordinateXY> c4 = { { 8, 10 }, { 10, 8 } };                     // 2x2/2 = 2
    std::vector<CoordinateXY> c5 = { { 10, 6 }, { 8, 6 }, { 8, 3 }, { 10, 3 } }; // 2x3 = 6
    std::vector<CoordinateXY> c6 = { { 10, 4 }, { 9, 4 }, { 9, 5 }, { 10, 5 } }; // 1x1 = 1 (subtracted)
    std::vector<CoordinateXY> c7 = { { 10, 2 }, { 8, 2 }, { 8, 0 } };            // 2x2 = 4

    Traversal t1 = make_traversal(c1);
    Traversal t2 = make_traversal(c2);
    Traversal t3 = make_traversal(c3);
    Traversal t4 = make_traversal(c4);
    Traversal t5 = make_traversal(c5);
    Traversal t6 = make_traversal(c6);
    Traversal t7 = make_traversal(c7);

    TraversalVector traversals{ &t1, &t2, &t3, &t4, &t5, &t6, &t7 };

    ensure_equals(TraversalAreas::getLeftHandArea(b, traversals), 4 + 20 + 2 + 6 - 1 + 4);

    ensure_equals_geometry(TraversalAreas::getLeftHandRings(gfact, b, traversals).get(), "MULTIPOLYGON (((2 0, 2 2, 0 2, 0 0, 2 0)), ((3 10, 3 0, 5 0, 5 10, 3 10)), ((8 10, 10 8, 10 10, 8 10)), ((10 6, 8 6, 8 3, 10 3, 10 4, 9 4, 9 5, 10 5, 10 6)), ((10 2, 8 2, 8 0, 10 0, 10 2)))");
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

    std::vector<CoordinateXY> c1{ { 4, 0 }, { 4, 0 } };
    Traversal t1 = make_traversal(c1);
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

    std::vector<CoordinateXY> c1 = { { 1, 1 }, { 2, 1 }, { 2, 2 }, { 1, 2 }, { 1, 1 } };
    Traversal t1 = make_traversal(c1);
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

    std::vector<CoordinateXY> c1 = { { 1, 0 }, { 2, 1 }, { 1, 1 }, { 1, 0 } };
    Traversal t1 = make_traversal(c1);
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

    std::vector<CoordinateXY> c1 = { { 1, 1 }, { 1, 2 }, { 2, 2 }, { 2, 1 }, { 1, 1 } };
    Traversal t1 = make_traversal(c1);
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

    std::vector<CoordinateXY> c1 = { { 1, 1 }, { 1, 2 }, { 2, 2 }, { 2, 1 }, { 1, 1 } };
    std::vector<CoordinateXY> c2 = { { 10, 5 }, { 10, 5 } };
    Traversal t1 = make_traversal(c1);
    Traversal t2 = make_traversal(c2);
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

    std::vector<CoordinateXY> c1 = { { 0, 0 }, { 2, 2 }, { 3, 2 }, { 0, 0 } };
    Traversal t1 = make_traversal(c1);
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

    std::vector<CoordinateXY> c1 = { { 1, 0 }, { 2, 2 }, { 3, 2 }, { 1, 0 } };
    Traversal t1 = make_traversal(c1);
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

    std::vector<CoordinateXY> c1 = { { 1, 0 }, { 1, 1 }, { 2, 1 }, { 1, 0 } };
    Traversal t1 = make_traversal(c1);
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

    std::vector<CoordinateXY> c1{ { 4, 0 }, { 10, 0 } };
    Traversal t1 = make_traversal(c1);
    TraversalVector traversals{ &t1 };

    ensure_equals(TraversalAreas::getLeftHandArea(b, traversals), 100);
    ensure_equals_geometry(TraversalAreas::getLeftHandRings(gfact, b, traversals).get(), "POLYGON ((0 0, 4 0, 10 0, 10 10, 0 10, 0 0))");
}

template<>
template<>
void object::test<16>()
{
    set_test_name("Edge traversal (interior right)");

    Envelope b{ 2, 3, 2, 3 };

    std::vector<CoordinateXY> c1{ { 2, 2 }, { 2, 2.5 }, { 2, 2.5 } };
    Traversal t1 = make_traversal(c1);
    TraversalVector traversals{ &t1 };

    ensure_equals(TraversalAreas::getLeftHandArea(b, traversals), 0);
    ensure_equals_geometry(TraversalAreas::getLeftHandRings(gfact, b, traversals).get(), "POLYGON EMPTY");
}

template<>
template<>
void object::test<17>()
{
    set_test_name("interior and edge traversal");

    Envelope b(6, 7, 3, 4);
    std::vector<CoordinateXY> c1{{7, 3}, {6, 4}, {7, 4}};
    Traversal t1 = make_traversal(c1);
    TraversalVector traversals{ &t1 };

    ensure_equals(TraversalAreas::getLeftHandArea(b, traversals), 0.5);
    ensure_equals_geometry(TraversalAreas::getLeftHandRings(gfact, b, traversals).get(), "POLYGON ((6 3, 7 3, 6 4, 6 3))");
}

template<>
template<>
void object::test<18>()
{
    set_test_name("interior-edge segment-interior traversal");

    Envelope b(0, 10, 0, 10);
    std::vector<CoordinateXY> c1{{10, 5}, {8, 0}, {4, 0}, {0, 3}};
    Traversal t1 = make_traversal(c1);
    TraversalVector traversals{ &t1 };

    ensure_equals(TraversalAreas::getLeftHandArea(b, traversals), 11);
    ensure_equals_geometry(TraversalAreas::getLeftHandRings(gfact, b, traversals).get(), "MULTIPOLYGON (((10 5, 8 0, 10 0, 10 5)), ((4 0, 0 3, 0 0, 4 0)))");
}

template<>
template<>
void object::test<19>()
{
    set_test_name("interior-edge point-interior traversal");

    Envelope b(0, 10, 0, 10);
    std::vector<CoordinateXY> c1{{10, 5}, {8, 0}, {0, 3}};
    Traversal t1 = make_traversal(c1);
    TraversalVector traversals{ &t1 };

    ensure_equals(TraversalAreas::getLeftHandArea(b, traversals), 17);
    ensure_equals_geometry(TraversalAreas::getLeftHandRings(gfact, b, traversals).get(), "MULTIPOLYGON (((8 0, 10 5, 10 0, 8 0)), ((8 0, 0 0, 0 3, 8 0)))");
}

template<>
template<>
void object::test<20>()
{
    set_test_name("interior-edge point-interior traversal, with repeated points");

    Envelope b(0, 10, 0, 10);
    std::vector<CoordinateXY> c1{{10, 5}, {8, 0}, {8, 0}, {0, 3}};
    Traversal t1 = make_traversal(c1);
    TraversalVector traversals{ &t1 };

    ensure_equals(TraversalAreas::getLeftHandArea(b, traversals), 17);
    ensure_equals_geometry(TraversalAreas::getLeftHandRings(gfact, b, traversals).get(), "MULTIPOLYGON (((8 0, 10 5, 10 0, 8 0)), ((8 0, 0 0, 0 3, 8 0)))");
}

template<>
template<>
void object::test<21>()
{
    set_test_name("bouncing off multiple edges");

    Envelope b(0, 10, 0, 10);

    std::vector<CoordinateXY> c1{{10, 5}, {5, 10}, {2, 0}, {0, 5}};
    Traversal t1 = make_traversal(c1);
    TraversalVector traversals{ &t1 };

    ensure_equals(TraversalAreas::getLeftHandArea(b, traversals), 57.5);
    ensure_equals_geometry(TraversalAreas::getLeftHandRings(gfact, b, traversals).get(), "MULTIPOLYGON (((10 5, 5 10, 2 0, 10 0, 10 5)), ((2 0, 0 5, 0 0, 2 0)))");
}

template<>
template<>
void object::test<22>()
{
    set_test_name("enter top, bounce bottom, exit right");

    Envelope b(0, 10, 0, 10);

    std::vector<CoordinateXY> c1{{5, 10}, {5, 0}, {10, 2}};
    Traversal t1 = make_traversal(c1);
    TraversalVector traversals{ &t1 };

    ensure_equals(TraversalAreas::getLeftHandArea(b, traversals), 45);
    ensure_equals_geometry(TraversalAreas::getLeftHandRings(gfact, b, traversals).get(),
    "POLYGON ((5 10, 5 0, 10 2, 10 10, 5 10))"
    );
}

template<>
template<>
void object::test<23>()
{
    set_test_name("along top, then bottom to top");

    Envelope b(0, 10, 0, 10);
    std::vector<CoordinateXY> c1{{5, 10}, {0, 10}};
    std::vector<CoordinateXY> c2{{5, 0}, {5, 10}};
    Traversal t1 = make_traversal(c1);
    Traversal t2 = make_traversal(c2);
    TraversalVector traversals{ &t1, &t2 };

    ensure_equals(TraversalAreas::getLeftHandArea(b, traversals), 50);
    ensure_equals_geometry(TraversalAreas::getLeftHandRings(gfact, b, traversals).get(),
        "POLYGON ((5 10, 0 10, 0 0, 5 0, 5 10))");
}

template<>
template<>
void object::test<24>()
{
    set_test_name("two traversals, touching in interior");
    // This would occur when two touching holes are present in the cell, and the shell is not present.
    // Or it could occur when a shell and hole touch

    Envelope b(0, 10, 0, 10);
    std::vector<CoordinateXY> c1{{10, 0}, {5, 5}, {10, 10}};
    std::vector<CoordinateXY> c2{{0, 10}, {5, 5}, {0, 0}};
    Traversal t1 = make_traversal(c1, &c1);
    Traversal t2 = make_traversal(c2, &c2);
    TraversalVector traversals{ &t1, &t2 };

    ensure_equals(TraversalAreas::getLeftHandArea(b, traversals), 50);
    ensure_equals_geometry(TraversalAreas::getLeftHandRings(gfact, b, traversals).get(),
    "MULTIPOLYGON (((5 5, 0 10, 10 10, 5 5)), ((5 5, 10 0, 0 0, 5 5)))");
}

template<>
template<>
void object::test<25>()
{
    set_test_name("two traversals, touching in interior, plus complete hole");
    // This is the same as #24 but the complete hole forces use of the polygonizer

    Envelope b(0, 10, 0, 10);
    std::vector<CoordinateXY> c1{{10, 0}, {5, 5}, {10, 10}};
    std::vector<CoordinateXY> c2{{0, 10}, {5, 5}, {0, 0}};
    std::vector<CoordinateXY> c3{{5, 1}, {5, 2}, {6, 2}, {6, 1}, {5, 1}};
    Traversal t1 = make_traversal(c1, &c1);
    Traversal t2 = make_traversal(c2, &c2);
    Traversal t3 = make_traversal(c3, &c3);
    TraversalVector traversals{ &t1, &t2, &t3 };

    ensure_equals(TraversalAreas::getLeftHandArea(b, traversals), 49);
    ensure_equals_geometry(TraversalAreas::getLeftHandRings(gfact, b, traversals).get(),
    "MULTIPOLYGON (((0 10, 10 10, 5 5, 0 10)), ((0 0, 5 5, 10 0, 0 0), (5 1, 6 1, 6 2, 5 2, 5 1)))");
}

template<>
template<>
void object::test<26>()
{
    set_test_name("multiple holes touching at endpoints");

    Envelope b(0, 10, 0, 10);
    std::vector<CoordinateXY> c1{{0, 10}, {3, 1}, {0, 0}};
    std::vector<CoordinateXY> c2{{0, 10}, {10, 9}};
    std::vector<CoordinateXY> c3{{10, 7}, {0, 10}};
    Traversal t1 = make_traversal(c1, &c1);
    Traversal t2 = make_traversal(c2, &c2);
    Traversal t3 = make_traversal(c3, &c2);
    TraversalVector traversals{ &t1, &t2, &t3 };

    ensure_equals(TraversalAreas::getLeftHandArea(b, traversals), 75);
    ensure_equals_geometry(TraversalAreas::getLeftHandRings(gfact, b, traversals).get(),
    "MULTIPOLYGON (((0 10, 3 1, 0 0, 10 0, 10 7, 0 10)), ((0 10, 10 9, 10 10, 0 10)))");
}

template<>
template<>
void object::test<28>() {
    set_test_name("lake with island");

    Envelope b(0, 10, 0, 10);
    std::vector<CoordinateXY> c1{{1, 1}, {1, 9}, {9, 9}, {9, 1}, {1, 1}};
    std::vector<CoordinateXY> c2{{2, 2}, {4, 2}, {4, 4}, {2, 4}, {2, 2}};
    Traversal lake = make_traversal(c1, &c1);
    Traversal island = make_traversal(c2, &c2);
    TraversalVector traversals{ &island, &lake };

    ensure_equals(TraversalAreas::getLeftHandArea(b, traversals), 40);
    //ensure_equals_geometry(TraversalAreas::getLeftHandRings(gfact, b, traversals).get(),
    //"MULTIPOLYGON (((0 0, 10 0, 10 10, 0 10, 0 0), (1 1, 9 1, 9 9, 1 9, 1 1)), ((2 2, 4 2, 4 4, 2 4, 2 2)))");
}

}
