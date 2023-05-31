//
// Test Suite for geos::coverage::TPVWSimplifier class.

#include <tut/tut.hpp>
#include <utility.h>

// geos
#include <geos/coverage/TPVWSimplifier.h>

using geos::coverage::TPVWSimplifier;

namespace tut {
//
// Test Group
//

// Common data used by all tests
struct test_tpvwsimplifier_data {

    WKTReader r;
    WKTWriter w;

    test_tpvwsimplifier_data() {
    }

    void
    checkNoop(
        const std::string& wkt,
        double tolerance)
    {
        std::unique_ptr<Geometry> geom = r.read(wkt);
        const MultiLineString* mls = static_cast<const MultiLineString*>(geom.get());
        std::unique_ptr<Geometry> actual = TPVWSimplifier::simplify(mls, tolerance);
        ensure_equals_geometry(actual.get(), geom.get());
    }

    void
    checkSimplify(
        const std::string& wkt,
        double tolerance,
        const std::string& wktExpected)
    {
        std::unique_ptr<Geometry> geom = r.read(wkt);
        const MultiLineString* mls = static_cast<const MultiLineString*>(geom.get());
        std::unique_ptr<Geometry> actual = TPVWSimplifier::simplify(mls, tolerance);
        std::unique_ptr<Geometry> expected = r.read(wktExpected);
        ensure_equals_geometry(actual.get(), expected.get());
    }

    void
    checkSimplify(
        const std::string& wkt,
        const std::vector<std::size_t> freeRingIndex,
        double tolerance,
        const std::string& wktExpected)
    {
        checkSimplify(wkt, freeRingIndex, "", tolerance, wktExpected);
    }

    void
    checkSimplify(
        const std::string& wkt,
        const std::vector<std::size_t> freeRingIndex,
        const std::string& wktConstraints,
        double tolerance,
        const std::string& wktExpected)
    {
        auto geom = r.read(wkt);
        const MultiLineString* lines = static_cast<const MultiLineString*>(geom.get());

        std::vector<bool> freeRings(lines->getNumGeometries(), false);
        for (std::size_t index : freeRingIndex) {
            freeRings[index] = true;
        }
        std::unique_ptr<Geometry> constraintsPtr(nullptr);
        if (wktConstraints.length() > 0) {
            constraintsPtr = r.read(wktConstraints);
        }
        const MultiLineString* constraints = static_cast<const MultiLineString*>(constraintsPtr.get());
        std::unique_ptr<Geometry> actual = TPVWSimplifier::simplify(lines, freeRings, constraints, tolerance);
        std::unique_ptr<Geometry> expected = r.read(wktExpected);

        // std::cout << "-- actual" << std::endl;
        // std::cout << w.write(*actual) << std::endl;
        // std::cout << "-- expected" << std::endl;
        // std::cout << w.write(*expected) << std::endl;

        ensure_equals_geometry(actual.get(), expected.get());
    }

};


typedef test_group<test_tpvwsimplifier_data> group;
typedef group::object object;

group test_tpvwsimplifier_data("geos::coverage::TPVWSimplifier");


// testSimpleNoop
template<>
template<>
void object::test<1> ()
{
    checkNoop(
        "MULTILINESTRING ((9 9, 3 9, 1 4, 4 1, 9 1), (9 1, 2 4, 9 9))",
        2);
}

// testSimple
template<>
template<>
void object::test<2> ()
{
    checkSimplify(
        "MULTILINESTRING ((9 9, 3 9, 1 4, 4 1, 9 1), (9 1, 6 3, 2 4, 5 7, 9 9))",
        2,
        "MULTILINESTRING ((9 9, 3 9, 1 4, 4 1, 9 1), (9 1, 2 4, 9 9))"
    );
}

// testFreeRing
template<>
template<>
void object::test<3> ()
{
    checkSimplify(
        "MULTILINESTRING ((1 9, 9 9, 9 1), (1 9, 1 1, 9 1), (7 5, 8 8, 2 8, 2 2, 8 2, 7 5))",
        { 2 },
        2,
        "MULTILINESTRING ((1 9, 1 1, 9 1), (1 9, 9 9, 9 1), (8 8, 2 8, 2 2, 8 2, 8 8))"
    );
}

// testNoFreeRing
template<>
template<>
void object::test<4> ()
{
    checkSimplify(
        "MULTILINESTRING ((1 9, 9 9, 9 1), (1 9, 1 1, 9 1), (5 5, 4 8, 2 8, 2 2, 4 2, 5 5), (5 5, 6 8, 8 8, 8 2, 6 2, 5 5))",
        {},
        2,
        "MULTILINESTRING ((1 9, 1 1, 9 1), (1 9, 9 9, 9 1), (5 5, 2 2, 2 8, 5 5), (5 5, 8 2, 8 8, 5 5))"
    );
}

// testConstraint
template<>
template<>
void object::test<5> ()
{
    checkSimplify(
        "MULTILINESTRING ((6 8, 2 8, 2.1 5, 2 2, 6 2, 5.9 5, 6 8))",
        {},
        "MULTILINESTRING ((1 9, 9 9, 6 5, 9 1), (1 9, 1 1, 9 1))",
        1,
        "MULTILINESTRING ((6 8, 2 8, 2 2, 6 2, 5.9 5, 6 8))"
    );
}


} // namespace tut
