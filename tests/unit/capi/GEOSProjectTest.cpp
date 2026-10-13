// Test Suite for C-API LineString project functions

#include <tut/tut.hpp>
// geos
#include <geos_c.h>
#include <geos/constants.h>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

// Common data used in test cases.
struct test_capiproject_data : public capitest::utility {};

typedef test_group<test_capiproject_data> group;
typedef group::object object;

group test_capiproject_group("capi::GEOSProject");

//
// Test Cases
//

// Test basic usage
template<>
template<>
void object::test<1>
()
{
    geom1_ = GEOSGeomFromWKT("LINESTRING (0 0, 0 2)");
    geom2_ = GEOSGeomFromWKT("POINT (1 1)");


    double dist = GEOSProject(geom1_, geom2_);
    ensure_equals(dist, 1.0);

    double dist_norm = GEOSProjectNormalized(geom1_, geom2_);
    ensure_equals(dist_norm, 0.5);
}

// Test non-linestring geometry (first argument) correctly returns -1.0
template<>
template<>
void object::test<2>
()
{
    geom1_ = GEOSGeomFromWKT("POLYGON ((0 0, 0 1, 1 1, 1 0, 0 0))");
    geom2_ = GEOSGeomFromWKT("POINT (1 1)");

    double dist = GEOSProject(geom1_, geom2_);
    ensure_equals(dist, -1.0);

    double dist_norm = GEOSProjectNormalized(geom1_, geom2_);
    ensure_equals(dist_norm, -1.0);
}

// Test non-point geometry (second argument) correctly returns -1.0
// https://trac.osgeo.org/geos/ticket/1058
template<>
template<>
void object::test<3>
()
{
    geom1_ = GEOSGeomFromWKT("LINESTRING (0 0, 0 2)");
    geom2_ = GEOSGeomFromWKT("LINESTRING (0 0, 0 2)");

    double dist = GEOSProject(geom1_, geom2_);
    ensure_equals(dist, -1.0);

    double dist_norm = GEOSProjectNormalized(geom1_, geom2_);
    ensure_equals(dist_norm, -1.0);
}

// Test
// https://github.com/libgeos/geos/issues/475
template<>
template<>
void object::test<4>
()
{
    geom1_ = GEOSGeomFromWKT("LINESTRING (0 0, 0 0)");
    geom2_ = GEOSGeomFromWKT("POINT (0 0)");

    double dist = GEOSProject(geom1_, geom2_);
    ensure_equals(dist, 0.0);

    double dist_norm = GEOSProjectNormalized(geom1_, geom2_);
    ensure_equals(dist_norm, 0.0);
}

// Test invalid value warning
// https://github.com/shapely/shapely/issues/1796
template<>
template<>
void object::test<5>
()
{
    geom1_ = GEOSGeomFromWKT("LINESTRING (0 0, 1 1, 1 1, 2 2)");
    geom2_ = GEOSGeomFromWKT("POINT (0 1)");

#ifdef HAVE_FENV
    std::feclearexcept(FE_ALL_EXCEPT);
#endif
    double dist = GEOSProject(geom1_, geom2_);
#ifdef FE_INVALID
    ensure("FE_INVALID raised", !std::fetestexcept(FE_INVALID));
#endif
    ensure_equals("GEOSProject", dist, 0.7071, 0.0001);

#ifdef HAVE_FENV
    std::feclearexcept(FE_ALL_EXCEPT);
#endif
    double dist_norm = GEOSProjectNormalized(geom1_, geom2_);
#ifdef FE_INVALID
    ensure("FE_INVALID raised", !std::fetestexcept(FE_INVALID));
#endif
    ensure_equals("GEOSProjectNormalized", dist_norm, 0.25);
}

template<>
template<>
void object::test<6>
()
{
    set_test_name("CircularString input");
    useContext();

    geom1_ = fromWKT("CIRCULARSTRING (0 0, 1 1, 2 0)");
    geom2_ = fromWKT("POINT (1 1.1)");

    ensure_equals(GEOSProject_r(ctxt_, geom1_, geom2_), -1.0);
    ensure_equals(GEOSProjectNormalized_r(ctxt_, geom1_, geom2_), -1.0);

    useCurveConversion();

    double dist = GEOSProject_r(ctxt_, geom1_, geom2_);
    ensure_equals("GEOSProject result does not match", dist, geos::MATH_PI/2, 1e-2);

    double dist_norm = GEOSProjectNormalized_r(ctxt_, geom1_, geom2_);
    ensure_equals("GEOSProjectNormalized result does not match", dist_norm, 0.5, 1e-3);
}

// Empty point as second argument returns -1.0 instead of crashing
// https://github.com/libgeos/geos/issues/1534
template<>
template<>
void object::test<7>
()
{
    geom1_ = GEOSGeomFromWKT("LINESTRING (0 0, 0 2)");
    geom2_ = GEOSGeomFromWKT("POINT EMPTY");

    ensure_equals(GEOSProject(geom1_, geom2_), -1.0);
    ensure_equals(GEOSProjectNormalized(geom1_, geom2_), -1.0);
}

// Invalid second argument reports IllegalArgumentException
// https://github.com/libgeos/geos/issues/1534
template<>
template<>
void object::test<8>
()
{
    useContext();

    geom1_ = fromWKT("LINESTRING (0 0, 0 2)");
    geom2_ = fromWKT("LINESTRING (0 0, 0 2)");

    std::string errorMsg;
    GEOSContext_setErrorMessageHandler_r(ctxt_, [](const char* message, void* userdata) {
        static_cast<std::string*>(userdata)->append(message);
    }, &errorMsg);

    ensure_equals(GEOSProject_r(ctxt_, geom1_, geom2_), -1.0);
    ensure("error message contains IllegalArgumentException",
           errorMsg.find("IllegalArgumentException") != std::string::npos);
}

} // namespace tut
