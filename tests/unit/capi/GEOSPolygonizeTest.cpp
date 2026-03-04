//
// Test Suite for C-API GEOSPolygonize*

#include <tut/tut.hpp>
// geos
#include <geos_c.h>

#include <array>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

// Common data used in test cases.
struct test_capigeospolygonize_data : public capitest::utility {
    std::vector<GEOSGeometry*> geoms_;

    GEOSGeometry* cutEdges_ = nullptr;
    GEOSGeometry* dangles_ = nullptr;
    GEOSGeometry* invalidRings_ = nullptr;


    ~test_capigeospolygonize_data() override {
        if (ctxt_) {
            for (auto& geom: geoms_) {
                GEOSGeom_destroy_r(ctxt_, geom);
            }
            GEOSGeom_destroy_r(ctxt_, cutEdges_);
            GEOSGeom_destroy_r(ctxt_, dangles_);
            GEOSGeom_destroy_r(ctxt_, invalidRings_);
        } else {
            for (auto& geom: geoms_) {
                GEOSGeom_destroy(geom);
            }
            GEOSGeom_destroy(cutEdges_);
            GEOSGeom_destroy(dangles_);
            GEOSGeom_destroy(invalidRings_);
        }
    }
};

typedef test_group<test_capigeospolygonize_data> group;
typedef group::object object;

group test_capigeospolygonize_group("capi::GEOSPolygonize");

//
// Test Cases
//

template<>
template<>
void object::test<1>
()
{
    geoms_.push_back(fromWKT("LINESTRING(1 3, 3 3, 3 1, 1 1, 1 3)"));
    geoms_.push_back(fromWKT("LINESTRING(1 3, 3 3, 3 1, 1 1, 1 3)"));

    cutEdges_ = GEOSPolygonizer_getCutEdges(geoms_.data(), static_cast<unsigned int>(geoms_.size()));

    ensure(cutEdges_);
    ensure_equals(GEOSGetNumGeometries(cutEdges_), static_cast<int>(geoms_.size()));
}

template<>
template<>
void object::test<2>
()
{
    // Example from JTS Developer's Guide, Chapter 6 - Polygonization
    geoms_.push_back(fromWKT("LINESTRING(0 0, 10 10)")); // isolated edge
    geoms_.push_back(fromWKT("LINESTRING(185 221, 100 100)")); // dangling edge
    geoms_.push_back(fromWKT("LINESTRING(185 221, 88 275, 180 316)"));
    geoms_.push_back(fromWKT("LINESTRING(185 221, 292 281, 180 316)"));
    geoms_.push_back(fromWKT("LINESTRING(189 98, 83 187, 185 221)"));
    geoms_.push_back(fromWKT("LINESTRING(189 98, 325 168, 185 221)"));

    cutEdges_ = GEOSPolygonizer_getCutEdges(geoms_.data(), static_cast<unsigned int>(geoms_.size()));
    ensure(cutEdges_);
    ensure_equals(GEOSGetNumGeometries(cutEdges_), 0);
}

template<>
template<>
void object::test<3>
()
{
    geoms_.push_back(fromWKT("LINESTRING (100 100, 100 300, 300 300, 300 100, 100 100)"));
    geoms_.push_back(fromWKT("LINESTRING (150 150, 150 250, 250 250, 250 150, 150 150)"));

    // GEOSPolygonize gives us a collection of two polygons
    result_ = GEOSPolygonize(geoms_.data(), static_cast<unsigned int>(geoms_.size()));
    ensure(result_);
    ensure_equals(GEOSGetNumGeometries(result_), 2);
    ensure_equals(GEOSGeomTypeId(result_), GEOS_GEOMETRYCOLLECTION);
    GEOSGeom_destroy(result_);

    // GEOSPolygonize_valid gives us a single polygon with a hole
    result_ = GEOSPolygonize_valid(geoms_.data(), static_cast<unsigned int>(geoms_.size()));

    ensure(result_);
    ensure_equals(GEOSGetNumGeometries(result_), 1);
    ensure_equals(GEOSGeomTypeId(result_), GEOS_POLYGON);
}

template<>
template<>
void object::test<4>
()
{
    geoms_.push_back(fromWKT("LINESTRING (0 0, 1 1)"));

    result_ = GEOSPolygonize(geoms_.data(), static_cast<unsigned int>(geoms_.size()));

    ensure(result_);
    ensure_equals(GEOSGetNumGeometries(result_), 0);
    ensure_equals(GEOSGeomTypeId(result_), GEOS_GEOMETRYCOLLECTION);
}

template<>
template<>
void object::test<5>
()
{
    geoms_.push_back(fromWKT("LINESTRING (0 0, 1 0, 1 1, 0 1, 0 0)"));
    geoms_.push_back(fromWKT("LINESTRING (1 1, 2 1, 2 2, 1 2, 1 1)"));

    result_ = GEOSPolygonize_valid(geoms_.data(), static_cast<unsigned int>(geoms_.size()));

    ensure(result_);
    ensure_equals(GEOSGetNumGeometries(result_), 2);
    ensure_equals(GEOSGeomTypeId(result_), GEOS_MULTIPOLYGON);
}

// Test GEOSPolygonize_full
template<>
template<>
void object::test<6>
()
{
    geom1_ = fromWKT("MULTILINESTRING ((0 0, 1 0, 1 1, 0 1, 0 0),  (0 0, 0.5 0.5),  (1 1, 2 2, 1 2, 2 1, 1 1))");

    result_ = GEOSPolygonize_full(geom1_, &cutEdges_, &dangles_, &invalidRings_);

    expected_ = fromWKT("GEOMETRYCOLLECTION(POLYGON ((0 0, 1 0, 1 1, 0 1, 0 0)))");
    GEOSGeometry* expected_cuts = fromWKT("GEOMETRYCOLLECTION EMPTY");
    GEOSGeometry* expected_dangles = fromWKT("GEOMETRYCOLLECTION(LINESTRING (0 0, 0.5 0.5))");
    GEOSGeometry* expected_invalidRings = fromWKT("GEOMETRYCOLLECTION(LINESTRING (1 1, 2 2, 1 2, 2 1, 1 1))");

    ensure_geometry_equals(result_, expected_);
    ensure_geometry_equals(cutEdges_, expected_cuts);
    ensure_geometry_equals(dangles_, expected_dangles);
    ensure_geometry_equals(invalidRings_, expected_invalidRings);

    GEOSGeom_destroy(expected_cuts);
    GEOSGeom_destroy(expected_dangles);
    GEOSGeom_destroy(expected_invalidRings);
}

template<>
template<>
void object::test<7>
()
{
    set_test_name("LINESTRING ZM inputs");

    geoms_.push_back(fromWKT("LINESTRING ZM (0 0 5 4, 2 0 6 5, 2 2 7 6)"));
    geoms_.push_back(fromWKT("LINESTRING ZM (2 2 7 6, 0 0 5 4)"));

    result_ = GEOSPolygonize(geoms_.data(), static_cast<unsigned int>(geoms_.size()));
    ensure(result_);

    expected_ = fromWKT("GEOMETRYCOLLECTION ZM (POLYGON ZM ((2 2 7 6, 2 0 6 5, 0 0 5 4, 2 2 7 6)))");
    ensure(expected_);

    ensure_geometry_equals_identical(result_, expected_);
}

template<>
template<>
void object::test<8>
()
{
    set_test_name("GEOSPolygonize curved inputs");
    useContext();

    geoms_.push_back(fromWKT("LINESTRING (0 0, 2 0)"));
    geoms_.push_back(fromWKT("CIRCULARSTRING (0 0, 1 1, 2 0)"));

    result_ = GEOSPolygonize_r(ctxt_, geoms_.data(), static_cast<unsigned int>(geoms_.size()));
    ensure(!result_);

    // Input converted to line, output not converted to curve
    GEOSContext_setCurveToLineParams_r(ctxt_, curveToLineParams_);
    result_ = GEOSPolygonize_r(ctxt_, geoms_.data(), static_cast<unsigned int>(geoms_.size()));
    ensure(result_);
    ensure_equals(GEOSGeomTypeId_r(ctxt_, result_), GEOS_GEOMETRYCOLLECTION);
    ensure_equals(GEOSGetNumGeometries_r(ctxt_, result_), 1);
    ensure_equals(GEOSGeomTypeId_r(ctxt_, GEOSGetGeometryN_r(ctxt_, result_, 0)), GEOS_POLYGON);
    GEOSGeom_destroy(result_);

    // Input converted to line, output converted to curve
    GEOSContext_setLineToCurveParams_r(ctxt_, lineToCurveParams_);
    result_ = GEOSPolygonize_r(ctxt_, geoms_.data(), static_cast<unsigned int>(geoms_.size()));
    ensure(result_);
    ensure_equals(GEOSGeomTypeId_r(ctxt_, result_), GEOS_GEOMETRYCOLLECTION);
    ensure_equals(GEOSGetNumGeometries_r(ctxt_, result_), 1);
    ensure_equals(GEOSGeomTypeId_r(ctxt_, GEOSGetGeometryN_r(ctxt_, result_, 0)), GEOS_CURVEPOLYGON);
}

template<>
template<>
void object::test<9>
()
{
    set_test_name("GEOSPolygonize_valid curved inputs returning a single polygon");
    useContext();

    geoms_.push_back(fromWKT("LINESTRING (0 0, 20 0)"));
    geoms_.push_back(fromWKT("CIRCULARSTRING (0 0, 10 10, 20 0)"));
    geoms_.push_back(fromWKT("LINESTRING (9 1, 11 1, 11 2, 9 2, 9 1)"));
    GEOSSetSRID_r(ctxt_, geoms_.back(), 4326);

    result_ = GEOSPolygonize_valid_r(ctxt_, geoms_.data(), static_cast<unsigned int>(geoms_.size()));
    ensure(!result_);

    // Input converted to line, output not converted to curve
    GEOSContext_setCurveToLineParams_r(ctxt_, curveToLineParams_);
    result_ = GEOSPolygonize_valid_r(ctxt_, geoms_.data(), static_cast<unsigned int>(geoms_.size()));
    ensure(result_);
    ensure_equals(GEOSGeomTypeId_r(ctxt_, result_), GEOS_POLYGON);
    ensure_equals(GEOSGetNumGeometries_r(ctxt_, result_), 1);
    ensure_equals(GEOSGeomTypeId_r(ctxt_, GEOSGetGeometryN_r(ctxt_, result_, 0)), GEOS_POLYGON);
    ensure_equals(GEOSGetSRID_r(ctxt_, result_), 4326);
    GEOSGeom_destroy(result_);

    // Input converted to line, output converted to curve
    GEOSContext_setLineToCurveParams_r(ctxt_, lineToCurveParams_);
    result_ = GEOSPolygonize_valid_r(ctxt_, geoms_.data(), static_cast<unsigned int>(geoms_.size()));
    ensure(result_);
    ensure_equals(GEOSGeomTypeId_r(ctxt_, result_), GEOS_CURVEPOLYGON);
    ensure_equals(GEOSGetNumGeometries_r(ctxt_, result_), 1);
    ensure_equals(GEOSGeomTypeId_r(ctxt_, GEOSGetGeometryN_r(ctxt_, result_, 0)), GEOS_CURVEPOLYGON);
    ensure_equals(GEOSGetSRID_r(ctxt_, result_), 4326);
}

template<>
template<>
void object::test<10>
()
{
    set_test_name("GEOSPolygonize_valid curved inputs returning multiple polygons");
    useContext();

    geoms_.push_back(fromWKT("LINESTRING (0 0, 20 0)"));
    geoms_.push_back(fromWKT("CIRCULARSTRING (0 0, 10 10, 20 0)"));
    geoms_.push_back(fromWKT("LINESTRING (20 0, 30 0, 30 30, 20 0)"));
    GEOSSetSRID_r(ctxt_, geoms_.back(), 4326);

    result_ = GEOSPolygonize_valid_r(ctxt_, geoms_.data(), static_cast<unsigned int>(geoms_.size()));
    ensure(!result_);

    // Input converted to line, output not converted to curve
    GEOSContext_setCurveToLineParams_r(ctxt_, curveToLineParams_);
    result_ = GEOSPolygonize_valid_r(ctxt_, geoms_.data(), static_cast<unsigned int>(geoms_.size()));
    ensure(result_);
    ensure_equals(GEOSGeomTypeId_r(ctxt_, result_), GEOS_MULTIPOLYGON);
    ensure_equals(GEOSGetNumGeometries_r(ctxt_, result_), 2);
    ensure_equals(GEOSGeomTypeId_r(ctxt_, GEOSGetGeometryN_r(ctxt_, result_, 0)), GEOS_POLYGON);
    ensure_equals(GEOSGeomTypeId_r(ctxt_, GEOSGetGeometryN_r(ctxt_, result_, 1)), GEOS_POLYGON);
    ensure_equals(GEOSGetSRID_r(ctxt_, result_), 4326);
    GEOSGeom_destroy(result_);

    // Input converted to line, output converted to curve
    GEOSContext_setLineToCurveParams_r(ctxt_, lineToCurveParams_);
    result_ = GEOSPolygonize_valid_r(ctxt_, geoms_.data(), static_cast<unsigned int>(geoms_.size()));
    ensure(result_);
    ensure_equals(GEOSGeomTypeId_r(ctxt_, result_), GEOS_MULTISURFACE);
    ensure_equals(GEOSGetNumGeometries_r(ctxt_, result_), 2);
    ensure_equals(GEOSGeomTypeId_r(ctxt_, GEOSGetGeometryN_r(ctxt_, result_, 0)), GEOS_CURVEPOLYGON);
    ensure_equals(GEOSGeomTypeId_r(ctxt_, GEOSGetGeometryN_r(ctxt_, result_, 1)), GEOS_POLYGON);
    ensure_equals(GEOSGetSRID_r(ctxt_, result_), 4326);
}

template<>
template<>
void object::test<11>
()
{
    set_test_name("GEOSPolygonizer_getCutEdges curved inputs");
    useContext();

    geoms_.push_back(fromWKT("CIRCULARSTRING (2 0, 1 1, 0 0, 1 -1, 2 0)"));
    geoms_.push_back(fromWKT("CIRCULARSTRING (0 0, 1 1, 2 0, 1 -1, 0 0)"));
    geoms_.push_back(fromWKT("LINESTRING (0 0, 2 0, 2 2, 0 2, 0 0)"));

    result_ = GEOSPolygonizer_getCutEdges_r(ctxt_, geoms_.data(), static_cast<unsigned int>(geoms_.size()));
    ensure(!result_);

    // Input converted to line, output not converted to curve
    GEOSContext_setCurveToLineParams_r(ctxt_, curveToLineParams_);
    result_ = GEOSPolygonizer_getCutEdges_r(ctxt_, geoms_.data(), static_cast<unsigned int>(geoms_.size()));
    ensure(result_);
    ensure_equals(GEOSGeomTypeId_r(ctxt_, result_), GEOS_GEOMETRYCOLLECTION);
    ensure_equals(GEOSGetNumGeometries_r(ctxt_, result_), 2);
    ensure_equals(GEOSGeomTypeId_r(ctxt_, GEOSGetGeometryN_r(ctxt_, result_, 0)), GEOS_LINESTRING);
    ensure_equals(GEOSGeomTypeId_r(ctxt_, GEOSGetGeometryN_r(ctxt_, result_, 1)), GEOS_LINESTRING);
    GEOSGeom_destroy(result_);

    // Input converted to line, output converted to curve
    GEOSContext_setLineToCurveParams_r(ctxt_, lineToCurveParams_);
    result_ = GEOSPolygonizer_getCutEdges_r(ctxt_, geoms_.data(), static_cast<unsigned int>(geoms_.size()));
    ensure(result_);
    ensure_equals(GEOSGeomTypeId_r(ctxt_, result_), GEOS_GEOMETRYCOLLECTION);
    ensure_equals(GEOSGetNumGeometries_r(ctxt_, result_), 2);
    ensure_equals(GEOSGeomTypeId_r(ctxt_, GEOSGetGeometryN_r(ctxt_, result_, 0)), GEOS_CIRCULARSTRING);
    ensure_equals(GEOSGeomTypeId_r(ctxt_, GEOSGetGeometryN_r(ctxt_, result_, 1)), GEOS_LINESTRING);
}

template<>
template<>
void object::test<12>
() {
    set_test_name("GEOSPolygonize_full curved inputs, linear outputs");
    useContext();

    geom1_ = fromWKT("MULTICURVE ((0 0, 10 0, 10 10, 0 10), CIRCULARSTRING (0 10, -5 5, 0 0), (0 0, 5 5), CIRCULARSTRING (5 5, 6 6, 7 5), (10 10, 20 20), CIRCULARSTRING (20 20, 15 25, 10 20), (10 20, 20 10, 10 10))");

    result_ = GEOSPolygonize_full_r(ctxt_, geom1_, &cutEdges_, &dangles_, &invalidRings_);
    ensure(result_ == nullptr);

    GEOSContext_setCurveToLineParams_r(ctxt_, curveToLineParams_);
    result_ = GEOSPolygonize_full_r(ctxt_, geom1_, &cutEdges_, &dangles_, &invalidRings_);
    ensure(result_);
    ensure_equals(GEOSGeomTypeId_r(ctxt_, result_), GEOS_GEOMETRYCOLLECTION);
    ensure_equals(GEOSGetNumGeometries_r(ctxt_, result_), 1);
    ensure_equals(GEOSGeomTypeId_r(ctxt_, GEOSGetGeometryN_r(ctxt_, result_, 0)), GEOS_POLYGON);

    ensure(cutEdges_);
    ensure_equals(GEOSGeomTypeId_r(ctxt_, cutEdges_), GEOS_GEOMETRYCOLLECTION);
    ensure_equals(GEOSGetNumGeometries_r(ctxt_, cutEdges_), 0);

    ensure(invalidRings_);
    ensure_equals(GEOSGeomTypeId_r(ctxt_, invalidRings_), GEOS_GEOMETRYCOLLECTION);
    ensure_equals(GEOSGetNumGeometries_r(ctxt_, invalidRings_), 1);
    ensure_equals(GEOSGeomTypeId_r(ctxt_, GEOSGetGeometryN_r(ctxt_, invalidRings_, 0)), GEOS_LINESTRING);

    ensure(dangles_);
    ensure_equals(GEOSGeomTypeId_r(ctxt_, dangles_), GEOS_GEOMETRYCOLLECTION);
    ensure_equals(GEOSGetNumGeometries_r(ctxt_, dangles_), 2);
    ensure_equals(GEOSGeomTypeId_r(ctxt_, GEOSGetGeometryN_r(ctxt_, dangles_, 0)), GEOS_LINESTRING);
    ensure_equals(GEOSGeomTypeId_r(ctxt_, GEOSGetGeometryN_r(ctxt_, dangles_, 1)), GEOS_LINESTRING);
}

template<>
template<>
void object::test<13>
()
{
    set_test_name("GEOSPolygonize_full curved inputs, curved outputs");
    useContext();
    useCurveConversion();

    geom1_ = fromWKT("MULTICURVE ((0 0, 10 0, 10 10, 0 10), CIRCULARSTRING (0 10, -5 5, 0 0), (0 0, 5 5), CIRCULARSTRING (5 5, 6 6, 7 5), (10 10, 20 20), CIRCULARSTRING (20 20, 15 25, 10 20), (10 20, 20 10, 10 10))");

    result_ = GEOSPolygonize_full_r(ctxt_, geom1_, &cutEdges_, &dangles_, &invalidRings_);
    ensure(result_);

    expected_ = fromWKT("GEOMETRYCOLLECTION(CURVEPOLYGON (COMPOUNDCURVE( (0 0, 10 0, 10 10, 0 10), CIRCULARSTRING (0 10, -5 5, 0 0))))");
    ensure_geometry_equals(result_, expected_);

    ensure(cutEdges_);
    ensure_equals(GEOSGeomTypeId_r(ctxt_, cutEdges_), GEOS_GEOMETRYCOLLECTION);
    ensure_equals(GEOSGetNumGeometries_r(ctxt_, cutEdges_), 0);

    ensure(invalidRings_);
    ensure_equals(GEOSGeomTypeId_r(ctxt_, invalidRings_), GEOS_GEOMETRYCOLLECTION);
    ensure_equals(GEOSGetNumGeometries_r(ctxt_, invalidRings_), 1);
    ensure_equals(GEOSGeomTypeId_r(ctxt_, GEOSGetGeometryN_r(ctxt_, invalidRings_, 0)), GEOS_COMPOUNDCURVE);

    ensure(dangles_);
    ensure_equals(GEOSGeomTypeId_r(ctxt_, dangles_), GEOS_GEOMETRYCOLLECTION);
    ensure_equals(GEOSGetNumGeometries_r(ctxt_, dangles_), 2);
    ensure_equals(GEOSGeomTypeId_r(ctxt_, GEOSGetGeometryN_r(ctxt_, dangles_, 0)), GEOS_CIRCULARSTRING);
    ensure_equals(GEOSGeomTypeId_r(ctxt_, GEOSGetGeometryN_r(ctxt_, dangles_, 1)), GEOS_LINESTRING);
}

} // namespace tut

