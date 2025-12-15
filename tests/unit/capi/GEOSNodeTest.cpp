//
// Test Suite for C-API GEOSNode

#include <iostream>
#include <tut/tut.hpp>
// geos
#include <geos_c.h>
#include <geos/geom/Geometry.h>

#include "capi_test_utils.h"
#include "utility.h"

namespace tut {
//
// Test Group
//

// Common data used in test cases.
struct test_capigeosnode_data : public capitest::utility {
    test_capigeosnode_data()
    {
    }
};

typedef test_group<test_capigeosnode_data> group;
typedef group::object object;

group test_capigeosnode_group("capi::GEOSNode");

//
// Test Cases
//

/// Self-intersecting line
template<>
template<>
void object::test<1>
()
{
    geom1_ = GEOSGeomFromWKT("LINESTRING(0 0, 10 10, 10 0, 0 10)");
    geom2_ = GEOSNode(geom1_);
    ensure(nullptr != geom2_);

    GEOSNormalize(geom2_);
    wkt_ = GEOSWKTWriter_write(wktw_, geom2_);
    std::string out(wkt_);

    ensure_equals(out,
        "MULTILINESTRING ((5 5, 10 10, 10 0, 5 5), (0 10, 5 5), (0 0, 5 5))"
        );
}

/// Overlapping lines
template<>
template<>
void object::test<2>
()
{
    geom1_ = GEOSGeomFromWKT("MULTILINESTRING((0 0, 2 0, 4 0),(5 0, 3 0, 1 0))");
    geom2_ = GEOSNode(geom1_);
    ensure(nullptr != geom2_);

    GEOSNormalize(geom2_);
    wkt_ = GEOSWKTWriter_write(wktw_, geom2_);
    std::string out(wkt_);

    ensure_equals(out,
        "MULTILINESTRING ((4 0, 5 0), (3 0, 4 0), (2 0, 3 0), (1 0, 2 0), (0 0, 1 0))"
        );
}

/// Equal lines
template<>
template<>
void object::test<3>
()
{
    geom1_ = GEOSGeomFromWKT("MULTILINESTRING((0 0, 2 0, 4 0),(0 0, 2 0, 4 0))");
    geom2_ = GEOSNode(geom1_);
    ensure(nullptr != geom2_);

    GEOSNormalize(geom2_);
    wkt_ = GEOSWKTWriter_write(wktw_, geom2_);
    std::string out(wkt_);

    ensure_equals(out, "MULTILINESTRING ((2 0, 4 0), (0 0, 2 0))");
}

// https://gis.stackexchange.com/questions/345341/get-location-of-postgis-geos-topology-exception/345482#345482
template<>
template<>
void object::test<4>
()
{
    std::string wkb = "010500000002000000010200000003000000dc874d65fcc25ec176032c6b350c5341b336429ffec25ec1f962bbd"
                      "9480c5341fc849518ffc25ec15be20f5f500c5341010200000006000000fa9bbfd3fcc25ec1b978232f390c5341"
                      "b336429ffec25ec1f962bbd9480c5341a77e6be5fec25ec1357c21334d0c5341c3eba27bfec25ec11be5a4c34a0"
                      "c5341b61d8cacfcc25ec1bcf273143c0c5341fa9bbfd3fcc25ec1b978232f390c5341";

    geom1_ = GEOSGeomFromHEX_buf((const unsigned char*) wkb.c_str(), wkb.size());
    geom2_ = GEOSNode(geom1_);

    // Noding currently fails for this case.
    // ensure(geom2_);
}


// https://github.com/libgeos/geos/issues/601
template<>
template<>
void object::test<5>
()
{
    geom1_ = GEOSGeomFromWKT("LINESTRING EMPTY");
    geom2_ = GEOSNode(geom1_);
    ensure(nullptr != geom2_);

    wkt_ = GEOSWKTWriter_write(wktw_, geom2_);
    std::string out(wkt_);

    ensure_equals(out, "LINESTRING EMPTY");
}

// Noding two XYZ LineStrings
template<>
template<>
void object::test<6>
()
{
    geom1_= GEOSGeomFromWKT("MULTILINESTRING Z ((0 0 0, 1 1 1), (0 1 5, 1 0 10))");
    result_ = GEOSNode(geom1_);
    expected_ = GEOSGeomFromWKT("MULTILINESTRING Z("
                                "(0 0 0, 0.5 0.5 4),"
                                "(0.5 0.5 4, 1 0 10),"
                                "(0 1 5, 0.5 0.5 4),"
                                "(0.5 0.5 4, 1 1 1))");

    GEOSNormalize(result_);
    GEOSNormalize(expected_);

    auto wkt_result = GEOSWKTWriter_write(wktw_, result_);
    auto wkt_expected = GEOSWKTWriter_write(wktw_, expected_);

    ensure_equals(std::string(wkt_result), std::string(wkt_expected));

    GEOSFree(wkt_result);
    GEOSFree(wkt_expected);
}

// Noding two XYM LineStrings
template<>
template<>
void object::test<7>
()
{
    geom1_= GEOSGeomFromWKT("MULTILINESTRING M ((0 0 0, 1 1 1), (0 1 5, 1 0 10))");
    result_ = GEOSNode(geom1_);
    expected_ = GEOSGeomFromWKT("MULTILINESTRING M("
                                "(0 0 0, 0.5 0.5 4),"
                                "(0.5 0.5 4, 1 0 10),"
                                "(0 1 5, 0.5 0.5 4),"
                                "(0.5 0.5 4, 1 1 1))");

    GEOSNormalize(result_);
    GEOSNormalize(expected_);

    auto wkt_result = GEOSWKTWriter_write(wktw_, result_);
    auto wkt_expected = GEOSWKTWriter_write(wktw_, expected_);

    ensure_equals(std::string(wkt_result), std::string(wkt_expected));

    GEOSFree(wkt_result);
    GEOSFree(wkt_expected);
}

// Noding two XYZ and XYM LineStrings
template<>
template<>
void object::test<8>
()
{
    geom1_= GEOSGeomFromWKT("GEOMETRYCOLLECTION (LINESTRING Z(0 0 0, 1 1 1), LINESTRING M(0 1 5, 1 0 10))");
    result_ = GEOSNode(geom1_);
    expected_ = GEOSGeomFromWKT("MULTILINESTRING ZM("
                                "(0 0 0 NaN, 0.5 0.5 0.5 7.5),"
                                "(0.5 0.5 0.5 7.5, 1 0 NaN 10),"
                                "(0 1 NaN 5, 0.5 0.5 0.5 7.5),"
                                "(0.5 0.5 0.5 7.5, 1 1 1 NaN))");

    GEOSNormalize(result_);
    GEOSNormalize(expected_);

    auto wkt_result = GEOSWKTWriter_write(wktw_, result_);
    auto wkt_expected = GEOSWKTWriter_write(wktw_, expected_);

    ensure_equals(std::string(wkt_result), std::string(wkt_expected));

    GEOSFree(wkt_result);
    GEOSFree(wkt_expected);
}

template<>
template<>
void object::test<9>()
{
    set_test_name("two arcs with two intersection points");

    input_ = fromWKT("MULTICURVE (CIRCULARSTRING (0 0, 1 1, 2 0), CIRCULARSTRING (0 1, 1 0, 2 1))");
    ensure(input_);

    result_ = GEOSNode(input_);
    ensure(result_ != nullptr);

    expected_ = fromWKT("MULTICURVE ("
                        "CIRCULARSTRING (0 0, 0.0340741737 0.2588190451, 0.1339745962 0.5, 1 1, 1.8660254038 0.5, 1.9659258263 0.2588190451, 2 0),"
                        "CIRCULARSTRING (0 1, 0.0340741737 0.7411809549, 0.1339745962 0.5, 1 0, 1.8660254038 0.5, 1.9659258263 0.7411809549, 2 1))");

    ensure_geometry_equals_exact(result_, expected_, 1e-8);
}

template<>
template<>
void object::test<10>()
{
    set_test_name("CIRCULARSTRING ZM intersecting CIRCULARSTRING M");

    input_ = fromWKT("MULTICURVE (CIRCULARSTRING ZM (-1 0 3 4, 0 1 2 5, 1 0 4 7), CIRCULARSTRING M (-1 2 9, 0 1 13, -1 0 17))");
    ensure(input_);

    result_ = GEOSNode(input_);
    ensure(result_ != nullptr);

    expected_ = fromWKT("MULTICURVE ZM ("
                        "CIRCULARSTRING ZM (-1 0 3 4, -1 1.2246467991e-16 5 4.75, -1 2.7755575616e-16 7 5.5, -1 1.2246467991e-16 7 5.5, -1 5.5511151231e-17 7 5.5, -0.7071067812 0.7071067812 5.25 7.375, -2.7755575616e-16 1 3.5 9.25, -3.8285686989e-16 1 3.5 9.25, -5.5511151231e-17 1 3.5 9.25, 0.7071067812 0.7071067812 3.75 8.125, 1 0 4 7),"
                        "CIRCULARSTRING ZM (-1 2 NaN 9, -0.2928932188 1.7071067812 NaN 9.125, -2.7755575616e-16 1 3.5 9.25, 0 1 3.5 9.25, -5.5511151231e-17 1 3.5 9.25, -0.2928932188 0.2928932188 5.25 7.375, -1 2.7755575616e-16 7 5.5, -1 0 7 5.5, -1 5.5511151231e-17 7 5.5, -1 0 NaN 11.25, -1 0 NaN 17))");

    ensure_equals_exact_geometry_xyzm(reinterpret_cast<Geometry*>(result_),
                                      reinterpret_cast<Geometry*>(expected_), 1e-8);
}

template<>
template<>
void object::test<11>()
{
    set_test_name("CIRCULARSTRING ZM / LINESTRING M interior intersection");

    input_ = fromWKT("MULTICURVE (CIRCULARSTRING ZM (-5 0 3 4, -4 3 2 5, 4 3 4 7), LINESTRING M (0 0 7, 0 10 13))");
    ensure(input_);

    result_ = GEOSNode(input_);
    ensure(result_ != nullptr);

    expected_ = fromWKT("MULTICURVE ZM ("
                        "CIRCULARSTRING ZM (-5 0 3 4, -3.5355 3.5355 3 6, 0 5 3 8, 3.5355 3.5355 3.5 7.5, 4 3 4 7),"
                        "LINESTRING ZM (0 0 NaN 7, 0 5 3 8),"
                        "LINESTRING ZM (0 5 3 8, 0 10 NaN 13))");

    ensure_equals_exact_geometry_xyzm(reinterpret_cast<Geometry*>(result_),
                                      reinterpret_cast<Geometry*>(expected_), 1e-4);
}

template<>
template<>
void object::test<12>()
{
    set_test_name("two cocircular CIRCULARSTRINGs");

    input_ = fromWKT("MULTICURVE ("
        "CIRCULARSTRING (-5 0, 0 5, 5 0),"
        "CIRCULARSTRING (-4 3, 0 5, 4 3))");

    result_ = GEOSNode(input_);

    expected_ = fromWKT("MULTICURVE ("
                        "CIRCULARSTRING (-5.0000000000000000 0.0000000000000000, -4.7434164902525691 1.5811388300841900, -4.0000000000000000 3.0000000000000000, 0.0000000000000003 5.0000000000000000, 4.0000000000000000 3.0000000000000000, 4.7434164902525691 1.5811388300841898, 5.0000000000000000 0.0000000000000000),"
                        "CIRCULARSTRING (-4.0000000000000000 3.0000000000000000, 0.0000000000000003 5.0000000000000000, 4.0000000000000000 3.0000000000000000))");

    ensure_equals_exact_geometry_xyzm(reinterpret_cast<Geometry*>(result_),
                                      reinterpret_cast<Geometry*>(expected_), 1e-4);
}

template<>
template<>
void object::test<13>()
{
    set_test_name("LINESTRING Z / LINESTRING Z endpoint intersection");

    input_ = fromWKT("MULTILINESTRING Z ((-5 0 3, 4 3 6), (0 0 7, 4 3 13))");
    ensure(input_);

    result_ = GEOSNode(input_);
    ensure(result_ != nullptr);

    expected_ = GEOSGeom_clone(input_);

    ensure_equals_exact_geometry_xyzm(reinterpret_cast<Geometry*>(result_),
                                      reinterpret_cast<Geometry*>(expected_), 1e-4);
}

template<>
template<>
void object::test<14>()
{
    set_test_name("CIRCULARSTRING Z / LINESTRING Z endpoint intersection");

    input_ = fromWKT("MULTICURVE (CIRCULARSTRING Z (-5 0 3, -4 3 5, 4 3 6), LINESTRING Z (0 0 7, 4 3 13))");
    ensure(input_);

    result_ = GEOSNode(input_);
    ensure(result_ != nullptr);

    expected_ = GEOSGeom_clone(input_);

    ensure_equals_exact_geometry_xyzm(reinterpret_cast<Geometry*>(result_),
                                      reinterpret_cast<Geometry*>(expected_), 1e-4);
}

template<>
template<>
void object::test<15>()
{
    set_test_name("LINESTRING Z / LINESTRING endpoint intersection");

    input_ = fromWKT("GEOMETRYCOLLECTION (LINESTRING Z (-5 0 3, 4 3 6), LINESTRING (0 0, 4 3))");
    ensure(input_);

    result_ = GEOSNode(input_);
    ensure(result_ != nullptr);

    expected_ = fromWKT("MULTILINESTRING Z ((-5 0 3, 4 3 6), (0 0 NaN, 4 3 NaN))");

    ensure_equals_exact_geometry_xyzm(reinterpret_cast<Geometry*>(result_),
                                      reinterpret_cast<Geometry*>(expected_), 1e-4);
}

template<>
template<>
void object::test<16>()
{
    set_test_name("CIRCULARSTRING Z / LINESTRING endpoint intersection");

    input_ = fromWKT("MULTICURVE (CIRCULARSTRING Z (-5 0 3, -4 3 5, 4 3 6), LINESTRING (0 0, 4 3))");
    ensure(input_);

    result_ = GEOSNode(input_);
    ensure(result_ != nullptr);

    expected_ = fromWKT("MULTICURVE Z (CIRCULARSTRING Z (-5 0 3, -1.5811388301 4.7434164903 4.5, 4 3 6), (0 0 NaN, 4 3 NaN))");

    ensure_equals_exact_geometry_xyzm(reinterpret_cast<Geometry*>(result_),
                                      reinterpret_cast<Geometry*>(expected_), 1e-4);
}

} // namespace tut

