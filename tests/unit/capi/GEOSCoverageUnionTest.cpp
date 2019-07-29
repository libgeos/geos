//
// Test Suite for C-API GEOSCoverageUnion

#include <tut/tut.hpp>
// geos
#include <geos_c.h>
// std
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace tut {
//
// Test Group
//

// Common data used in test cases.
struct test_capicoverageunion_data {
    static void
    notice(const char *fmt, ...) {
        std::fprintf(stdout, "NOTICE: ");

        va_list ap;
        va_start(ap, fmt);
        std::vfprintf(stdout, fmt, ap);
        va_end(ap);

        std::fprintf(stdout, "\n");
    }

    test_capicoverageunion_data() {
        m_context = initGEOS_r(notice, notice);
        m_reader = GEOSWKTReader_create_r(m_context);
    }

    ~test_capicoverageunion_data() {
        GEOSWKTReader_destroy_r(m_context, m_reader);
        finishGEOS_r(m_context);
    }

    GEOSContextHandle_t m_context;
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

    auto g1 = GEOSWKTReader_read_r(m_context, m_reader, wkt[0].c_str());
    auto g2 = GEOSWKTReader_read_r(m_context, m_reader, wkt[1].c_str());

    GEOSGeometry* geoms[2] = { g1, g2 };

    auto input = GEOSGeom_createCollection_r(m_context, GEOS_GEOMETRYCOLLECTION, geoms, 2);
    auto result = GEOSCoverageUnion_r(m_context, input);

    ensure( result != nullptr );
    ensure( GEOSGeomTypeId_r(m_context, result) == GEOS_POLYGON );

    GEOSGeom_destroy_r(m_context, input);
    GEOSGeom_destroy_r(m_context, result);
}

template<>
template<>
void object::test<2>
() {
    // Overlapping inputs (error)
    std::vector<std::string> wkt{
            "POLYGON ((0 0, 0 1, 1 1, 1 0, 0 0))",
            "POLYGON ((1 0, 0.9 1, 2 1, 2 0, 1 0))"
    };

    auto g1 = GEOSWKTReader_read_r(m_context, m_reader, wkt[0].c_str());
    auto g2 = GEOSWKTReader_read_r(m_context, m_reader, wkt[1].c_str());

    GEOSGeometry* geoms[2] = { g1, g2 };

    auto input = GEOSGeom_createCollection_r(m_context, GEOS_GEOMETRYCOLLECTION, geoms, 2);
    auto result = GEOSCoverageUnion_r(m_context, input);

    ensure( result == nullptr );

    GEOSGeom_destroy_r(m_context, input);
}


} // namespace tut

