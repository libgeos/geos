// $Id$
// 
// Test Suite for C-API GEOSSimplify

// TUT
#include <tut.h>
// GEOS CAPI
#include <geos_c.h>
// C+
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
    struct test_capigeossimplify_data
	{
		static void notice(const char *fmt, ...)
		{
			std::fprintf( stdout, "NOTICE: ");

			va_list ap;
			va_start(ap, fmt);
			std::vfprintf(stdout, fmt, ap);
			va_end(ap);
		
			std::fprintf(stdout, "\n");
		}

		test_capigeossimplify_data()
		{
			initGEOS(notice, notice);
		}		

		~test_capigeossimplify_data()
		{
			finishGEOS();
		}

	};

	typedef test_group<test_capigeossimplify_data> group;
	typedef group::object object;

	group test_capigeossimplify_group("capi::GEOSSimplify");

    //
    // Test Cases
    //

    // Test GEOSSimplify
    template<>
    template<>
    void object::test<1>()
    {
	    GEOSGeometry* geom1 = GEOSGeomFromWKT("POLYGON EMPTY");

		ensure ( 0 != GEOSisEmpty(geom1) );

		GEOSGeometry* geom2 = GEOSSimplify(geom1, 43.2);

		ensure ( 0 != GEOSisEmpty(geom2) );
    }
    
} // namespace tut

