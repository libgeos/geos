//
// Test Suite for C-API GEOSCoverageUnion

#include <tut/tut.hpp>
// geos
#include <geos_c.h>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

// Common data used in test cases.
struct test_capicoverageunion_data : public capitest::utility {

    test_capicoverageunion_data() {
        m_reader = GEOSWKTReader_create();
    }

    ~test_capicoverageunion_data() {
        GEOSWKTReader_destroy(m_reader);
    }

    GEOSWKTReader* m_reader;
};


typedef test_group<test_capicoverageunion_data> group;
typedef group::object object;

group test_capicoverageunion_group("capi::GEOSCoverageUnion");

//
// Test Cases
//


template<>
template<> void object::test<1>
()
{
    // Adjacent inputs
    std::vector<std::string> wkt{
            "POLYGON ((0 0, 0 1, 1 1, 1 0, 0 0))",
            "POLYGON ((1 0, 1 1, 2 1, 2 0, 1 0))"
    };

    auto g1 = GEOSWKTReader_read(m_reader, wkt[0].c_str());
    auto g2 = GEOSWKTReader_read(m_reader, wkt[1].c_str());

    GEOSGeometry* geoms[2] = { g1, g2 };

    auto input = GEOSGeom_createCollection(GEOS_GEOMETRYCOLLECTION, geoms, 2);
    auto result = GEOSCoverageUnion(input);

    ensure( result != nullptr );
    ensure( GEOSGeomTypeId(result) == GEOS_POLYGON );

    GEOSGeom_destroy(input);
    GEOSGeom_destroy(result);
}

template<>
template<>
void object::test<2>
() {
    auto input = GEOSWKTReader_read(m_reader,
        "GEOMETRYCOLLECTION(POLYGON ((0 0, 0 1, 1 1, 1 0, 0 0)), POLYGON ((1 0, 0.9 1, 2 1, 2 0, 1 0)))");

    auto result = GEOSCoverageUnion(input);
    ensure( result == nullptr );
    GEOSGeom_destroy(input);
}

template<>
template<> void object::test<4>
()
{
    set_test_name("curved inputs");
    useContext();

    input_ = fromWKT("GEOMETRYCOLLECTION ( "
        "CURVEPOLYGON (COMPOUNDCURVE ( (4 0, 0 0, 0 4, 4 4), CIRCULARSTRING (4 4, 2 2, 4 0))), "
        "CURVEPOLYGON (CIRCULARSTRING (4 4, 6 2, 4 0, 2 2, 4 4)))");

    result_ = GEOSCoverageUnion_r(ctxt_, input_);
    ensure("curved geometry not supported", result_ == nullptr);

    // Input converted to line, output not converted to curve
    GEOSContext_setCurveToLineParams_r(ctxt_, curveToLineParams_);
    result_ = GEOSCoverageUnion_r(ctxt_, input_);
    ensure(result_);
    ensure_equals(GEOSGeomTypeId_r(ctxt_, result_), GEOS_POLYGON);
    GEOSGeom_destroy_r(ctxt_, result_);

    // Input converted to line, output converted to curve
    GEOSContext_setLineToCurveParams_r(ctxt_, lineToCurveParams_);
    result_ = GEOSCoverageUnion_r(ctxt_, input_);
    ensure_equals(GEOSGeomTypeId_r(ctxt_, result_), GEOS_CURVEPOLYGON);
}

} // namespace tut
