// $Id.c$
// 
// Test Suite for C-API GEOSCoordSeq

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
    struct test_capigeoscoordseq_data
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

		test_capigeoscoordseq_data()
		{
			initGEOS(notice, notice);
		}		

		~test_capigeoscoordseq_data()
		{
			finishGEOS();
		}

	};

	typedef test_group<test_capigeoscoordseq_data> group;
	typedef group::object object;

	group test_capigeoscoordseq_group("capi::GEOSCoordSeq");

    //
    // Test Cases
    //

    // Test construction and fill of a 3D CoordinateSequence
    template<>
    template<>
    void object::test<1>()
    {
		GEOSCoordSequence* cs = GEOSCoordSeq_create(5, 3);
		
		unsigned int size;
		unsigned int dims;

		ensure ( GEOSCoordSeq_getSize(cs, &size) );
		ensure_equals( size, 5u );

		ensure ( GEOSCoordSeq_getDimensions(cs, &dims) );
		ensure_equals( dims, 3u );

		for (unsigned int i=0; i<5; ++i)
		{
			double x = i*10;
			double y = i*10+1;
			double z = i*10+2;

			GEOSCoordSeq_setX(cs, i, x);
			GEOSCoordSeq_setY(cs, i, y);
			GEOSCoordSeq_setZ(cs, i, z);

			double xcheck, ycheck, zcheck;
			ensure( GEOSCoordSeq_getX(cs, i, &xcheck) );
			ensure( GEOSCoordSeq_getY(cs, i, &ycheck) );
			ensure( GEOSCoordSeq_getZ(cs, i, &zcheck) );

			ensure_equals( xcheck, x );
			ensure_equals( ycheck, y );
			ensure_equals( zcheck, z );
		}
    }	
    
} // namespace tut

