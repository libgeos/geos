// $Id.c$
// 
// Test Suite for C-API GEOSSimplify

// TUT
#include <tut.h>
// GEOS CAPI
#include <geos_c.h>
// STL
#include <memory>
// C
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

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
			va_list ap;

			fprintf( stdout, "NOTICE: ");
			
			va_start (ap, fmt);
			vfprintf( stdout, fmt, ap);
			va_end(ap);
			fprintf( stdout, "\n" );
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

		ensure ( GEOSisEmpty(geom1) );

		GEOSGeometry* geom2 = GEOSSimplify(geom1, 43.2);

		ensure ( GEOSisEmpty(geom2) );


    }	

    
} // namespace tut

