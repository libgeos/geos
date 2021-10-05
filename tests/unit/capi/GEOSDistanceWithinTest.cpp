//
// Test Suite for C-API GEOSDistanceWithin

#include <tut/tut.hpp>
// geos
#include <geos/constants.h>
#include <geos_c.h>
// std
#include <limits>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <math.h>
#include <memory>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

struct test_capigeosdistancewithin_data : public capitest::utility {
  void testGEOSDistanceWithin(const char* wkt1, const char* wkt2,
                              double distance, char expectedResult) {

    GEOSGeometry *input1 = GEOSGeomFromWKT(wkt1);
    ensure(input1 != nullptr);
    GEOSGeometry *input2 = GEOSGeomFromWKT(wkt2);
    ensure(input2 != nullptr);

    char ret = GEOSDistanceWithin(input1, input2, distance);

    GEOSGeom_destroy(input1);
    GEOSGeom_destroy(input2);

    ensure_equals("return code", (int)ret, (int)expectedResult);

  };
};

typedef test_group<test_capigeosdistancewithin_data> group;
typedef group::object object;

group test_capigeosdistancewithin_group("capi::GEOSDistanceWithin");

// point within distance should return true
template <>
template <>
void object::test<1>() {
  testGEOSDistanceWithin(
      "POINT(0 0)",
      "POINT(0 1)",
      1.0,
      1
    );
}

// point not within distance should return false
template <>
template <>
void object::test<2>() {
  testGEOSDistanceWithin(
      "POINT(0 0)",
      "POINT(0 1)",
      0.999999,
      0
    );
}

// point at same location should return true even if distance is 0
template <>
template <>
void object::test<3>() {
  testGEOSDistanceWithin(
      "POINT(0 0)",
      "POINT(0 0)",
      0.0,
      1
    );
}

// line within distance of another line should return true
template <>
template <>
void object::test<4>() {
  testGEOSDistanceWithin(
      "LINESTRING(0 0, 1 1)",
      "LINESTRING(0 1, 1 2)",
      1.0,
      1
    );
}

// line not within distance of another line should return false
template <>
template <>
void object::test<5>() {
  testGEOSDistanceWithin(
      "LINESTRING(0 0, 1 0)",
      "LINESTRING(0 1, 1 1)",
      0.999999,
      0
    );
}

// line that equals another line should return true even if distance is 0
template <>
template <>
void object::test<6>() {
  testGEOSDistanceWithin(
      "LINESTRING(0 0, 1 1)",
      "LINESTRING(0 0, 1 1)",
      0.0,
      1
    );
}

// line that intersects another line should return true even if distance is 0
template <>
template <>
void object::test<7>() {
  testGEOSDistanceWithin(
      "LINESTRING(0 0, 1 1)",
      "LINESTRING(1 1, 0 0)",
      0.0,
      1
    );
}

// line that shares segment with other line should return true even if distance is 0
template <>
template <>
void object::test<8>() {
  testGEOSDistanceWithin(
      "LINESTRING(0 0, 2 2)",
      "LINESTRING(0 0, 1 1)",
      0.0,
      1
    );
}


// point within distance of line should return true
template <>
template <>
void object::test<9>() {
  testGEOSDistanceWithin(
      "LINESTRING(0 0, 1 1)",
      "POINT( 0 1)",
      1.0,
      1
    );
}

// point not within distance of line should return false
template <>
template <>
void object::test<10>() {
  testGEOSDistanceWithin(
      "LINESTRING(0 0, 1 0)",
      "POINT(0 1)",
      0.999999,
      0
    );
}

// line within distance of point should return true
template <>
template <>
void object::test<11>() {
  testGEOSDistanceWithin(
      "POINT( 0 1)",
      "LINESTRING(0 0, 1 1)",
      1.0,
      1
    );
}

// line not within distance of point should return false
template <>
template <>
void object::test<12>() {
  testGEOSDistanceWithin(
      "POINT(0 1)",
      "LINESTRING(0 0, 1 0)",
      0.999999,
      0
    );
}



// point that intersects line should return true even if distance is 0
template <>
template <>
void object::test<13>() {
  testGEOSDistanceWithin(
      "LINESTRING(0 0, 1 1)",
      "POINT(0.5 0.5)",
      0.0,
      1
    );
}

// polygon within distance of other polygon should return true
template <>
template <>
void object::test<14>() {
  testGEOSDistanceWithin(
      "POLYGON((0 0, 1 1, 2 0, 0 0))",
      "POLYGON((0 3, 2 3, 1 2, 0 3))",
      1.0,
      1
    );
}

// polygon not within distance of other polygon should return true
template <>
template <>
void object::test<15>() {
  testGEOSDistanceWithin(
      "POLYGON((0 0, 1 1, 2 0, 0 0))",
      "POLYGON((0 3, 2 3, 1 2, 0 3))",
      0.999999,
      0
    );
}

// polygon that intersects polygon should return true even if distance is 0
template <>
template <>
void object::test<16>() {
  testGEOSDistanceWithin(
      "POLYGON((0 0, 1 1, 2 0, 0 0))",
      "POLYGON((0 3, 2 3, 1 0, 0 3))",
      0.0,
      1
    );
}

// polygon that is equal to polygon should return true even if distance is 0
template <>
template <>
void object::test<17>() {
  testGEOSDistanceWithin(
      "POLYGON((0 0, 1 1, 2 0, 0 0))",
      "POLYGON((1 1, 2 0, 0 0, 1 1))",
      0.0,
      1
    );
}

// point within distance of polygon should return true
template <>
template <>
void object::test<18>() {
  testGEOSDistanceWithin(
      "POLYGON((0 0, 1 1, 2 0, 0 0))",
      "POINT(1 2)",
      1.0,
      1
    );
}

// point not within distance of polygon should return false
template <>
template <>
void object::test<19>() {
  testGEOSDistanceWithin(
      "POLYGON((0 0, 1 1, 2 0, 0 0))",
      "POINT(1 2)",
      0.999999,
      0
    );
}


// polygon within distance of point should return true
template <>
template <>
void object::test<20>() {
  testGEOSDistanceWithin(
      "POINT(1 2)",
      "POLYGON((0 0, 1 1, 2 0, 0 0))",
      1.0,
      1
    );
}

// point not within distance of polygon should return false
template <>
template <>
void object::test<21>() {
  testGEOSDistanceWithin(
      "POINT(1 2)",
      "POLYGON((0 0, 1 1, 2 0, 0 0))",
      0.999999,
      0
    );
}

// polygon that intersects point should return true even if distance is 0
template <>
template <>
void object::test<22>() {
  testGEOSDistanceWithin(
      "POLYGON((0 0, 1 1, 2 0, 0 0))",
      "POINT(1 0)",
      0.0,
      1
    );
}

// polygon within distance of line should return true
template <>
template <>
void object::test<23>() {
  testGEOSDistanceWithin(
      "POLYGON((0 0, 1 1, 2 0, 0 0))",
      "LINESTRING(0 -1, 2 -1)",
      1.0,
      1
    );
}

// polygon not within distance of line should return false
template <>
template <>
void object::test<24>() {
  testGEOSDistanceWithin(
      "POLYGON((0 0, 1 1, 2 0, 0 0))",
      "LINESTRING(0 -1, 2 -1)",
      0.999999,
      0
    );
}

// polygon that intersects line should return true even if distance is 0
template <>
template <>
void object::test<25>() {
  testGEOSDistanceWithin(
      "POLYGON((0 0, 1 1, 2 0, 0 0))",
      "LINESTRING(0 -1, 0.5 0.5, 2 -1)",
      0.0,
      1
    );
}

// polygon that shares edge with line should return true even if distance is 0
template <>
template <>
void object::test<26>() {
  testGEOSDistanceWithin(
      "POLYGON((0 0, 1 1, 2 0, 0 0))",
      "LINESTRING(0 0, 1 1, 2 0)",
      0.0,
      1
    );
}


// empty geometries should return false (distance 1)
template <>
template <>
void object::test<27>() {
  testGEOSDistanceWithin(
      "POINT EMPTY",
      "LINESTRING EMPTY",
      1.0,
      0
    );
}

// empty geometries should return false (distance 0)
template <>
template <>
void object::test<28>() {
  testGEOSDistanceWithin(
      "POINT EMPTY",
      "LINESTRING EMPTY",
      0.0,
      0
    );
}

// empty geometries should return false (distance Infinity)
template <>
template <>
void object::test<29>() {
  testGEOSDistanceWithin(
      "POINT EMPTY",
      "LINESTRING EMPTY",
      geos::DoubleInfinity,
      0
    );
}

// empty geometry is never within any distance
template <>
template <>
void object::test<30>() {
  testGEOSDistanceWithin(
      "POINT EMPTY",
      "LINESTRING(0 0, 20 0)",
      geos::DoubleInfinity,
      0
    );
}

// empty geometry is never within any distance
template <>
template <>
void object::test<31>() {
  testGEOSDistanceWithin(
      "LINESTRING(0 0, 20 0)",
      "POINT EMPTY",
      geos::DoubleInfinity,
      0
    );
}




} // namespace tut
