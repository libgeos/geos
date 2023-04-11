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
using geos::geom::Ordinate;
using geos::DoubleNotANumber;

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

    Coordinate cxyz;
    ensure_same("XYZ x", cxyz.x, cxy.x);
    ensure_same("XYZ y", cxyz.y, cxy.y);

    CoordinateXYM cxym;
    ensure_same("XYM x", cxym.x, cxy.x);
    ensure_same("XYM y", cxym.y, cxy.y);

    CoordinateXYZM cxyzm;
    ensure_same("XYZM x", cxyzm.x, cxy.x);
    ensure_same("XYZM y", cxyzm.y, cxy.y);
    ensure_same("XYZM z", cxyzm.z, cxyz.z);
    ensure_same("XYZM m", cxyzm.m, cxym.m);
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
    Coordinate::UnorderedSet coords;

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
    const double default_z = Coordinate().z;
    const double default_m = CoordinateXYM().m;

    CoordinateXY xy(1, 2);

    Coordinate xyz(xy);
    ensure_same(xyz.x, xy.x);
    ensure_same(xyz.y, xy.y);
    ensure_same(xyz.z, default_z);
    ensure_equals(xy, xyz);

    CoordinateXYM xym(xy);
    ensure_same(xym.x, xy.x);
    ensure_same(xym.y, xy.y);
    ensure_same(xym.m, default_m);

    CoordinateXYZM xyzm(xy);
    ensure_same(xyzm.x, xy.x);
    ensure_same(xyzm.y, xy.y);
    ensure_same(xyzm.z, default_z);
    ensure_same(xyzm.m, default_m);
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
    ensure_equals("XY", xy.toString(), std::string("1 2"));

    Coordinate xyz(1, 2, 3);
    ensure_equals("XYZ", xyz.toString(), std::string("1 2 3"));
    xyz.z = geos::DoubleNotANumber;
    ensure_equals("XYZ", xyz.toString(), std::string("1 2"));

    CoordinateXYM xym(1, 2, 3);
    ensure_equals("XYM", xym.toString(), std::string("1 2 3"));
    xym.m = geos::DoubleNotANumber;
    ensure_equals("XYM", xym.toString(), std::string("1 2"));

    CoordinateXYZM xyzm(1, 2, 3, 4);
    ensure_equals("XYZM", xyzm.toString(), std::string("1 2 3 4"));
    xyzm.z = geos::DoubleNotANumber;
    ensure_equals("XYZM", xyzm.toString(), std::string("1 2 nan 4"));
    xyzm.m = geos::DoubleNotANumber;
    ensure_equals("XYZM", xyzm.toString(), std::string("1 2"));
    xyzm.z = 3;
    ensure_equals("XYZM", xyzm.toString(), std::string("1 2 3"));
}

// Test object sizes
template<>
template<>
void object::test<14>
()
{
    ensure_equals("XY", sizeof(CoordinateXY), 16u);
    ensure_equals("XYZ", sizeof(Coordinate), 24u);
    ensure_equals("XYM", sizeof(CoordinateXYM), 24u);
    ensure_equals("XYZM", sizeof(CoordinateXYZM), 32u);
}

// Test 4D initialization from XYM
template<>
template<>
void object::test<15>
()
{
    double default_z = Coordinate().z;

    CoordinateXYM xym(1,2,3);
    CoordinateXYZM xyzm1(xym);
    CoordinateXYZM xyzm2;
    xyzm2 = xym;

    ensure_equals_xyzm(xyzm1, {1, 2, default_z, 3});
    ensure_equals_xyzm(xyzm2, {1, 2, default_z, 3});
}

// Test use of std::common_type
template<>
template<>
void object::test<16>()
{
    CoordinateXYM xym(1, 2, 3);
    Coordinate xyz(1, 2, 3);

    auto c = std::common_type_t<CoordinateXYM, Coordinate>(xyz);
    c.m = xym.m;

    ensure_equals(c.x, xyz.x);
    ensure_equals(c.y, xyz.y);
    ensure_equals(c.z, xyz.z);
    ensure_equals(c.m, xym.m);
}

// Test use of get()
template<>
template<>
void object::test<17>()
{
    CoordinateXY xy(1, 2);
    ensure_same(xy.get<Ordinate::X>(), 1);
    ensure_same(xy.get<Ordinate::Y>(), 2);
    ensure_same(xy.get<Ordinate::Z>(), DoubleNotANumber);
    ensure_same(xy.get<Ordinate::M>(), DoubleNotANumber);

    Coordinate xyz(1, 2, 3);
    ensure_same(xyz.get<Ordinate::X>(), 1);
    ensure_same(xyz.get<Ordinate::Y>(), 2);
    ensure_same(xyz.get<Ordinate::Z>(), 3);
    ensure_same(xyz.get<Ordinate::M>(), DoubleNotANumber);

    CoordinateXYM xym(1, 2, 3);
    ensure_same(xym.get<Ordinate::X>(), 1);
    ensure_same(xym.get<Ordinate::Y>(), 2);
    ensure_same(xym.get<Ordinate::Z>(), DoubleNotANumber);
    ensure_same(xym.get<Ordinate::M>(), 3);

    CoordinateXYZM xyzm(1, 2, 3, 4);
    ensure_same(xyzm.get<Ordinate::X>(), 1);
    ensure_same(xyzm.get<Ordinate::Y>(), 2);
    ensure_same(xyzm.get<Ordinate::Z>(), 3);
    ensure_same(xyzm.get<Ordinate::M>(), 4);
}


} // namespace tut

