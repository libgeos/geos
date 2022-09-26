//
// Test Suite for C-API GEOSCluster*

#include <tut/tut.hpp>
// geos
#include <geos_c.h>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

// Common data used in test cases.
struct test_capicluster_data : public capitest::utility {

    test_capicluster_data() {
        m_reader = GEOSWKTReader_create();
    }

    ~test_capicluster_data() {
        GEOSWKTReader_destroy(m_reader);
    }

    GEOSWKTReader* m_reader;
};


typedef test_group<test_capicluster_data> group;
typedef group::object object;

group test_capicluster_group("capi::GEOSCluster");

//
// Test Cases
//
template<>
template<> void object::test<1>
()
{
    GEOSGeometry* input = GEOSWKTReader_read(m_reader,
                                             "GEOMETRYCOLLECTION ("
                                             "POINT (0 1),"
                                             "LINESTRING (0 0, 0 0.1),"
                                             "LINESTRING (0 0, 1.0 1.0),"
                                             "POINT (0.9 1.0),"
                                             "POINT (0 7))");

    GEOSGeometry* result;
    result = GEOSClusterEnvelopeIntersects(GEOSGeom_clone(input));
    ensure_equals("two clusters by envelope intersection", GEOSGetNumGeometries(result), 2);
    GEOSGeom_destroy(result);

    result = GEOSClusterEnvelopeDistance(GEOSGeom_clone(input), 6);
    ensure_equals("one cluster by envelope distance", GEOSGetNumGeometries(result), 1);
    GEOSGeom_destroy(result);

    result = GEOSClusterGeometryIntersects(GEOSGeom_clone(input));
    ensure_equals("four clusters by geometry intersection", GEOSGetNumGeometries(result), 4);
    GEOSGeom_destroy(result);

    result = GEOSClusterGeometryDistance(GEOSGeom_clone(input), 0.2);
    ensure_equals("three clusters by distance", GEOSGetNumGeometries(result), 3);
    GEOSGeom_destroy(result);

    GEOSGeom_destroy(input);
}

template<>
template<>
void object::test<2>()
{
    GEOSGeometry* input = GEOSWKTReader_read(m_reader,
                                             "GEOMETRYCOLLECTION ("
                                             "POINT (0 0),"
                                             "POINT (-1 0),"
                                             "POINT (-1 -0.1),"
                                             "POINT (-1 0.1),"
                                             "POINT (1 0),"
                                             "POINT (2 0),"
                                             "POINT (3  0),"
                                             "POINT ( 3 -0.1),"
                                             "POINT ( 3 0.1)"
                                             ")");

    GEOSGeometry* result;
    result = GEOSClusterDBSCAN(GEOSGeom_clone(input), 1.01, 5);
    ensure_equals("two clusters with minPoints = 5", GEOSGetNumGeometries(result), 2);
    GEOSGeom_destroy(result);

    result = GEOSClusterDBSCAN(GEOSGeom_clone(input), 1.01, 2);
    ensure_equals("one cluster with minPoints = 2", GEOSGetNumGeometries(result), 1);
    GEOSGeom_destroy(result);

    GEOSGeom_destroy(input);
}


} // namespace tut

