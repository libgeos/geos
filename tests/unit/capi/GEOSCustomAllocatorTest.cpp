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
        static int dealloc_count;

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
            ++dealloc_count;
            std::free(ptr);
        }

        test_capicustomallocators_data()
        {
            GEOS_setAllocator(t_alloc);
            GEOS_setFreer(t_free);
        }       

        ~test_capicustomallocators_data()
        {
        }

    };

    int test_capicustomallocators_data::alloc_count = 0;
    int test_capicustomallocators_data::dealloc_count = 0;

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
        alloc_count = dealloc_count = 0;

        initGEOS(notice, notice);
        notice("%d allocs, %d deallocs after initGEOS", alloc_count, dealloc_count);
        finishGEOS();
        notice("%d allocs, %d deallocs after finishGEOS", alloc_count, dealloc_count);

        ensure(alloc_count > 0);
        ensure_equals(alloc_count, dealloc_count);
    }

    /// WKTReader 
    template<>
    template<>
    void object::test<2>()
    {
        alloc_count = dealloc_count = 0;

        int tmpcount = 0;
        initGEOS(notice, notice);
        ensure("initGEOS didn't alloc", alloc_count > tmpcount);

        notice("%d allocs, %d deallocs after initGEOS", alloc_count, dealloc_count);

        tmpcount = alloc_count;
        GEOSWKTReader* reader = GEOSWKTReader_create();
        ensure("GEOSWKTReader_create didn't alloc", alloc_count > tmpcount);

        notice("%d allocs, %d deallocs after GEOSWKTReader_create", alloc_count, dealloc_count);

        tmpcount = alloc_count;
        GEOSGeometry* g = GEOSWKTReader_read(reader, "POINT(0 0)");
        ensure("GEOSWKTReader_read didn't alloc", alloc_count > tmpcount);

        notice("%d allocs, %d deallocs after GEOSWKTReader_read", alloc_count, dealloc_count);

        tmpcount = dealloc_count;
        GEOSGeom_destroy(g);
        ensure("GEOSGeom_destroy didn't dealloc", dealloc_count > tmpcount);

        notice("%d allocs, %d deallocs after GEOSGeom_destroy", alloc_count, dealloc_count);

        tmpcount = dealloc_count;
        GEOSWKTReader_destroy(reader);
        ensure("GEOSWKTReader_destroy didn't dealloc", dealloc_count > tmpcount);

        notice("%d allocs, %d deallocs after GEOSWKTReader_destroy", alloc_count, dealloc_count);

        tmpcount = dealloc_count;
        finishGEOS();
        ensure("finishGEOS didn't dealloc", dealloc_count > tmpcount);

        notice("%d allocs, %d deallocs after finishGEOS", alloc_count, dealloc_count);

        ensure_equals(alloc_count, dealloc_count);
    }


} // namespace tut

