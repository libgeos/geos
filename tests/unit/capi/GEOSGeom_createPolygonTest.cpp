#include <tut/tut.hpp>
#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

struct test_geosgeom_createpolygon_data : public capitest::utility {};

typedef test_group<test_geosgeom_createpolygon_data> group;
typedef group::object object;

group test_geosgeom_createpolygon("capi::GEOSGeom_createPolygon");

template<>
template<>
void object::test<1>
()
{
    GEOSCoordSequence* shell_seq = GEOSCoordSeq_create(5, 2);
    GEOSCoordSequence* hole_seq = GEOSCoordSeq_create(5, 2);

    double shell_coords[] = {0,0, 0,10, 10,10, 10,0, 0,0};
    double hole_coords[] = {5,5, 5,6, 6,6, 6,5, 5,5};
    for (int i = 0; i < 5; i++) {
        GEOSCoordSeq_setXY(shell_seq, i, shell_coords[2*i], shell_coords[2*i+1]);
        GEOSCoordSeq_setXY(hole_seq, i, hole_coords[2*i], hole_coords[2*i+1]);
    }

    GEOSGeometry* shell = GEOSGeom_createLinearRing(shell_seq);
    GEOSGeometry* hole = GEOSGeom_createLinearRing(hole_seq);
    GEOSGeometry** holes = (GEOSGeometry**)malloc(sizeof(GEOSGeometry *));
    holes[0] = hole;

    GEOSGeometry* polygon = GEOSGeom_createPolygon(shell, holes, 1);
    GEOSGeometry* expected = GEOSGeomFromWKT("POLYGON((0 0, 0 10, 10 10, 10 0, 0 0),(5 5, 5 6, 6 6, 6 5, 5 5))");

    // GEOSWKTWriter* w = GEOSWKTWriter_create();
    // printf("%s\n", GEOSWKTWriter_write(w, polygon));
    // printf("%s\n", GEOSWKTWriter_write(w, expected));

    ensure_equals(GEOSEqualsExact(polygon, expected, 0), 1);

    GEOSGeom_destroy(polygon);
    GEOSGeom_destroy(expected);
    // WARNING! The GEOSGeom_createPolygon takes ownership of the
    // GEOSGeometry, but not the containing array!
    // maybe this should be changed...
    free(holes);
}

template<>
template<>
void object::test<2>
()
{
    GEOSCoordSequence* shell_seq = GEOSCoordSeq_create(5, 2);

    double shell_coords[] = {0,0, 0,10, 10,10, 10,0, 0,0};
    for (unsigned int i = 0; i < 5; i++) {
        GEOSCoordSeq_setXY(shell_seq, i, shell_coords[2*i], shell_coords[2*i+1]);
    }

    GEOSGeometry* shell = GEOSGeom_createLineString(shell_seq);
    GEOSGeometry** holes = nullptr;
    unsigned int nholes = 1;

    // Returns null on exception, wrong input type for shell
    GEOSGeometry* polygon = GEOSGeom_createPolygon(shell, holes, nholes);
    ensure(polygon == nullptr);

    // Shouldn't need this
    if (polygon)
        GEOSGeom_destroy(polygon);
}


} // namespace tut

