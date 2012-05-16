// 
// Test Suite for C-API custom allocators

#include <tut.hpp>
// geos
#include <geos_c.h>
// std
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <memory>

namespace tut
{
    //
    // Test Group
    //

    // Common data used in test cases.
    struct test_capicustomallocators_data
    {
        static int alloc_count;

        static void notice(const char *fmt, ...)
        {
            std::fprintf( stdout, "NOTICE: ");

            va_list ap;
            va_start(ap, fmt);
            std::vfprintf(stdout, fmt, ap);
            va_end(ap);
        
            std::fprintf(stdout, "\n");
        }

        static void *t_alloc(size_t sz)
        {
            ++alloc_count;
            void *ret = std::malloc(sz);
            //notice("t_alloc(%d) returning %p", sz, ret);
            return ret;
        }

        static void t_free(void *ptr)
        {
            //notice("t_free(%p)", ptr);
            --alloc_count;
            std::free(ptr);
        }

        test_capicustomallocators_data()
        {
            GEOS_setAllocator(t_alloc);
            GEOS_setFreer(t_free);

            alloc_count = 0;
        }       

        ~test_capicustomallocators_data()
        {
        }

    };

    int test_capicustomallocators_data::alloc_count = 0;

    typedef test_group<test_capicustomallocators_data> group;
    typedef group::object object;

    group test_capicustomallocators_group("capi::GEOSCustomAllocators");

    //
    // Test Cases
    //

    /// init and finish
    template<>
    template<>
    void object::test<1>()
    {
        alloc_count = 0;

        initGEOS(notice, notice);
        finishGEOS();

        ensure_equals(alloc_count, 0);
    }

    /// WKTReader 
    template<>
    template<>
    void object::test<2>()
    {
        alloc_count = 0;

        initGEOS(notice, notice);

        GEOSWKTReader* reader = GEOSWKTReader_create();
        GEOSGeometry* g = GEOSWKTReader_read(reader, "POINT(0 0)");
        GEOSGeom_destroy(g);
        GEOSWKTReader_destroy(reader);

        finishGEOS();

        ensure_equals(alloc_count, 0);
    }


} // namespace tut

