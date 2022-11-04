//
// Test Suite for geos::geom::Coordinate class.

#include <tut/tut.hpp>
// geos
#include <geos/constants.h>
#include <geos/geom/Coordinate.h>
// std
#include <cmath>
#include <unordered_set>
#include <utility.h>

namespace tut {
//
// Test Group
//

using geos::geom::Coordinate;
using geos::geom::CoordinateXY;
using geos::geom::CoordinateXYM;
using geos::geom::CoordinateXYZM;
using geos::geom::CoordinateType;

// Common data used by tests
struct test_coordinate_data {
    const double x;
    const double y;
    const double z;

    test_coordinate_data()
        : x(1.234), y(2.345), z(3.456)
    {}

private:
    // Declare type as noncopyable
    test_coordinate_data(test_coordinate_data const& other) = delete;
    test_coordinate_data& operator=(test_coordinate_data const& rhs) = delete;
};

typedef test_group<test_coordinate_data> group;
typedef group::object object;

group test_coordinate_group("geos::geom::Coordinate");

//
// Test Cases
//

// Test of default constructors
template<>
template<>
void object::test<1>
()
{
    CoordinateXY cxy;
    ensure_same("XY x", cxy.x, Coordinate::DEFAULT_X);
    ensure_same("XY y", cxy.y, Coordinate::DEFAULT_Y);

    Coordinate coord;
    ensure_same("XYZ x", coord.x, Coordinate::DEFAULT_X);
    ensure_same("XYZ y", coord.y, Coordinate::DEFAULT_Y);
    ensure_same("XYZ z", coord.z, Coordinate::DEFAULT_Z);

    CoordinateXYM cxym;
    ensure_same("XYM x", cxym.x, Coordinate::DEFAULT_X);
    ensure_same("XYM y", cxym.y, Coordinate::DEFAULT_Y);
    ensure_same("XYM m", cxym.m, Coordinate::DEFAULT_M);

    CoordinateXYZM cxyzm;
    ensure_same("XYZM x", cxyzm.x, Coordinate::DEFAULT_X);
    ensure_same("XYZM y", cxyzm.y, Coordinate::DEFAULT_Y);
    ensure_same("XYZM z", cxyzm.z, Coordinate::DEFAULT_Z);
    ensure_same("XYZM m", cxyzm.m, Coordinate::DEFAULT_M);
}

// Test of overridden constructors
template<>
template<>
void object::test<2>
()
{
    CoordinateXY cxy(1, 2);
    ensure_equals(cxy.x, 1);
    ensure_equals(cxy.y, 2);

    Coordinate coord(1, 2, 3);
    ensure_equals(coord.x, 1);
    ensure_equals(coord.y, 2);
    ensure_equals(coord.z, 3);

    CoordinateXYM cxym(1, 2, 3);
    ensure_equals(cxym.x, 1);
    ensure_equals(cxym.y, 2);
    ensure_equals(cxym.m, 3);

    CoordinateXYZM cxyzm(1, 2, 3, 4);
    ensure_equals(cxyzm.x, 1);
    ensure_equals(cxyzm.y, 2);
    ensure_equals(cxyzm.z, 3);
    ensure_equals(cxyzm.m, 4);
}

// Test of copy constructor and assignment operator
template<>
template<>
void object::test<3>
()
{
    // Create exemplar object
    geos::geom::Coordinate original(x, y, z);
    ensure_equals(original.x, x);
    ensure_equals(original.y, y);
    ensure_equals(original.z, z);

    // Use copy ctor
    geos::geom::Coordinate copy(original);
    ensure_equals(copy.x, x);
    ensure_equals(copy.y, y);
    ensure_equals(copy.z, z);

    // Use assignment operator
    geos::geom::Coordinate assigned; // uses default ctor
    assigned = original;
    ensure_equals(assigned.x, x);
    ensure_equals(assigned.y, y);
    ensure_equals(assigned.z, z);
}

// Test of equality operators
template<>
template<>
void object::test<4>
()
{
    geos::geom::Coordinate original(x, y, z);
    ensure_equals(original.x, x);
    ensure_equals(original.y, y);
    ensure_equals(original.z, z);

    // Use copy ctor
    geos::geom::Coordinate copy(original);

    // Compare copy with original
    ensure_equals(original, copy);

    copy.x += 3.14; // permutation
    ensure(original != copy);
}

// Test of equals()
template<>
template<>
void object::test<5>
()
{
    geos::geom::Coordinate first(x, y);
    geos::geom::Coordinate second(x, y);

    ensure(first.equals(second));
}

// Test of equals2D()
template<>
template<>
void object::test<6>
()
{
    geos::geom::Coordinate first(x, y);
    geos::geom::Coordinate second(x, y);

    ensure(first.equals2D(second));
}

// Test of equals3D()
template<>
template<>
void object::test<7>
()
{
    geos::geom::Coordinate first(x, y);
    geos::geom::Coordinate second(x, y);

    // Test with z = NaN
    ensure(first.equals3D(second));

    // Test all valid coordinates
    first.z = second.z = z;
    ensure(first.equals3D(second));
}

// Test of distance()
template<>
template<>
void object::test<8>
()
{
    geos::geom::Coordinate first(x, y, z);
    geos::geom::Coordinate second(x, y, z);

    // Both coordinates are same
    ensure_equals(first.distance(second), 0);
    ensure_equals(first.distance(second), second.distance(first));

    // Move second coordinate
    second.x += 10.0;
    second.y += 10.0;
    second.z += 10.0;
    ensure(first.distance(second) != 0);
    ensure_equals(first.distance(second), second.distance(first));
    // TODO - mloskot - I'm not sure if this will work on every platform.
    // In case of any problems, just remove it.
    ensure_equals(first.distance(second), 14.142135623730951);
}

// Test of set/getNull()
template<>
template<>
void object::test<9>
()
{
    // Create not null coordinate
    Coordinate not_null_coord(x, y, z);
    ensure_equals(not_null_coord.x, x);
    ensure_equals(not_null_coord.y, y);
    ensure_equals(not_null_coord.z, z);

    // Make it null and check
    not_null_coord.setNull();
    ensure(not_null_coord.isNull());
    ensure(0 != std::isnan(not_null_coord.x));
    ensure(0 != std::isnan(not_null_coord.y));
    ensure(0 != std::isnan(not_null_coord.z));

    // Build in static null instance
    geos::geom::Coordinate null_coord;
    null_coord = geos::geom::Coordinate::getNull();
    ensure(null_coord.isNull());
    ensure(0 != std::isnan(null_coord.x));
    ensure(0 != std::isnan(null_coord.y));
    ensure(0 != std::isnan(null_coord.z));

    // XY setNull / isNull
    CoordinateXY xy(1, 2);
    ensure(!xy.isNull());
    xy.setNull();
    ensure(xy.isNull());
    ensure(std::isnan(xy.x));
    ensure(std::isnan(xy.y));

    // XYM setNull / isNull
    CoordinateXYM xym(1, 2, 3);
    ensure(!xym.isNull());
    xym.setNull();
    ensure(xym.isNull());
    ensure(std::isnan(xym.x));
    ensure(std::isnan(xym.y));
    ensure(std::isnan(xym.m));

    // XYZM setNull / isNull
    CoordinateXYZM xyzm(1, 2, 3, 4);
    ensure(!xyzm.isNull());
    xyzm.setNull();
    ensure(xyzm.isNull());
    ensure(std::isnan(xyzm.x));
    ensure(std::isnan(xyzm.y));
    ensure(std::isnan(xyzm.z));
    ensure(std::isnan(xyzm.m));
}

template<>
template<>
void object::test<10>
()
{
    std::unordered_set<Coordinate, Coordinate::HashCode> coords;

    coords.emplace(1, 2);
    ensure_equals(coords.size(), 1ul);

    coords.emplace(2, 1);
    ensure_equals(coords.size(), 2ul);

    // hash function defined consistently with equality operator
    // and considers X and Y only
    coords.emplace(1, 2, 3);
    ensure_equals(coords.size(), 2ul);
}

// Test 3D, 4D initialization from 2D
template<>
template<>
void object::test<11>
()
{
    CoordinateXY xy(1, 2);

    Coordinate xyz(xy);
    ensure_same(xyz.x, xy.x);
    ensure_same(xyz.y, xy.y);
    ensure_same(xyz.z, Coordinate::DEFAULT_Z);
    ensure_equals(xy, xyz);

    CoordinateXYM xym(xy);
    ensure_same(xym.x, xy.x);
    ensure_same(xym.y, xy.y);
    ensure_same(xym.m, Coordinate::DEFAULT_M);

    CoordinateXYZM xyzm(xy);
    ensure_same(xyzm.x, xy.x);
    ensure_same(xyzm.y, xy.y);
    ensure_same(xyzm.z, Coordinate::DEFAULT_Z);
    ensure_same(xyzm.m, Coordinate::DEFAULT_M);
}

// Test 3D, 4D assignment from 2D
template<>
template<>
void object::test<12>
()
{
    CoordinateXY xy(1, 2);

    Coordinate xyz;
    xyz = xy;
    ensure("XYZ", xyz.equals3D(Coordinate(xy)));

    CoordinateXYM xym;
    xym = xy;
    ensure("XYM", xym.equals3D(CoordinateXYM(xy)));

    CoordinateXYZM xyzm;
    xyzm = xy;
    ensure("XYZM", xyzm.equals4D(CoordinateXYZM(xy)));
}

// Test toString()
template<>
template<>
void object::test<13>
()
{
    CoordinateXY xy(1, 2);
    ensure_equals("XY", xy.toString(), std::string("(1 2)"));

    Coordinate xyz(1, 2, 3);
    ensure_equals("XYZ", xyz.toString(), std::string("(1 2 3)"));
    xyz.z = geos::DoubleNotANumber;
    ensure_equals("XYZ", xyz.toString(), std::string("(1 2)"));

    CoordinateXYM xym(1, 2, 3);
    ensure_equals("XYM", xym.toString(), std::string("(1 2 3)"));
    xym.m = geos::DoubleNotANumber;
    ensure_equals("XYM", xym.toString(), std::string("(1 2)"));

    CoordinateXYZM xyzm(1, 2, 3, 4);
    ensure_equals("XYZM", xyzm.toString(), std::string("(1 2 3 4)"));
    xyzm.z = geos::DoubleNotANumber;
    ensure_equals("XYZM", xyzm.toString(), std::string("(1 2 nan 4)"));
    xyzm.m = geos::DoubleNotANumber;
    ensure_equals("XYZM", xyzm.toString(), std::string("(1 2)"));
    xyzm.z = 3;
    ensure_equals("XYZM", xyzm.toString(), std::string("(1 2 3)"));
}



} // namespace tut

