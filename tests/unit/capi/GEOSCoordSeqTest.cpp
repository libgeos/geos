// $Id$
// 
// Test Suite for C-API GEOSCoordSeq

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
    struct test_capigeoscoordseq_data
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

		ensure ( 0 != GEOSCoordSeq_getSize(cs, &size) );
		ensure_equals( size, 5u );

		ensure ( 0 != GEOSCoordSeq_getDimensions(cs, &dims) );
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
			ensure( 0 != GEOSCoordSeq_getX(cs, i, &xcheck) );
			ensure( 0 != GEOSCoordSeq_getY(cs, i, &ycheck) );
			ensure( 0 != GEOSCoordSeq_getZ(cs, i, &zcheck) );

			ensure_equals( xcheck, x );
			ensure_equals( ycheck, y );
			ensure_equals( zcheck, z );
		}
    }	
    
    // Test not swapped setX/setY calls (see bug #133, fixed)
    template<>
    template<>
    void object::test<2>()
    {
		GEOSCoordSequence* cs = GEOSCoordSeq_create(1, 3);
		
		unsigned int size;
		unsigned int dims;

		ensure ( 0 != GEOSCoordSeq_getSize(cs, &size) );
		ensure_equals( size, 1u );

		ensure ( 0 != GEOSCoordSeq_getDimensions(cs, &dims) );
		ensure_equals( dims, 3u );

		double x = 10;
		double y = 11;
		double z = 12;

        // X, Y, Z
		GEOSCoordSeq_setX(cs, 0, x);
		GEOSCoordSeq_setY(cs, 0, y);
		GEOSCoordSeq_setZ(cs, 0, z);

		double xcheck, ycheck, zcheck;
		ensure( 0 != GEOSCoordSeq_getY(cs, 0, &ycheck) );
		ensure( 0 != GEOSCoordSeq_getX(cs, 0, &xcheck) );
		ensure( 0 != GEOSCoordSeq_getZ(cs, 0, &zcheck) );

		ensure_equals( xcheck, x );
		ensure_equals( ycheck, y );
		ensure_equals( zcheck, z );
    }	

    // Test not swapped setOrdinate calls (see bug #133, fixed)
    template<>
    template<>
    void object::test<3>()
    {
		GEOSCoordSequence* cs = GEOSCoordSeq_create(1, 3);
		
		unsigned int size;
		unsigned int dims;

		ensure ( 0 != GEOSCoordSeq_getSize(cs, &size) );
		ensure_equals( size, 1u );

		ensure ( 0 != GEOSCoordSeq_getDimensions(cs, &dims) );
		ensure_equals( dims, 3u );

		double x = 10;
		double y = 11;
		double z = 12;

        // X, Y, Z
		GEOSCoordSeq_setOrdinate(cs, 0, 0, x);
		GEOSCoordSeq_setOrdinate(cs, 0, 1, y);
		GEOSCoordSeq_setOrdinate(cs, 0, 2, z);

		double xcheck, ycheck, zcheck;
		ensure( 0 != GEOSCoordSeq_getOrdinate(cs, 0, 1, &ycheck) );
		ensure( 0 != GEOSCoordSeq_getOrdinate(cs, 0, 0, &xcheck) );
		ensure( 0 != GEOSCoordSeq_getOrdinate(cs, 0, 2, &zcheck) );

		ensure_equals( xcheck, x );
		ensure_equals( ycheck, y );
		ensure_equals( zcheck, z );
    }	

    // Test swapped setX calls (see bug #133, fixed)
    template<>
    template<>
    void object::test<4>()
    {
		GEOSCoordSequence* cs = GEOSCoordSeq_create(1, 3);
		
		unsigned int size;
		unsigned int dims;

		ensure ( 0 != GEOSCoordSeq_getSize(cs, &size) );
		ensure_equals( size, 1u );

		ensure ( 0 != GEOSCoordSeq_getDimensions(cs, &dims) );
		ensure_equals( dims, 3u );

		double x = 10;
		double y = 11;
		double z = 12;

        // Y, X, Z
		GEOSCoordSeq_setY(cs, 0, y);
		GEOSCoordSeq_setX(cs, 0, x);
		GEOSCoordSeq_setZ(cs, 0, z);

		double xcheck, ycheck, zcheck;
		ensure( 0 != GEOSCoordSeq_getY(cs, 0, &ycheck) );
		ensure( 0 != GEOSCoordSeq_getX(cs, 0, &xcheck) );
		ensure( 0 != GEOSCoordSeq_getZ(cs, 0, &zcheck) );

		ensure_equals( xcheck, x );
		ensure_equals( ycheck, y );
		ensure_equals( zcheck, z );
    }	

    // Test swapped setOrdinate calls (see bug #133, fixed)
    template<>
    template<>
    void object::test<5>()
    {
		GEOSCoordSequence* cs = GEOSCoordSeq_create(1, 3);
		
		unsigned int size;
		unsigned int dims;

		ensure ( 0 != GEOSCoordSeq_getSize(cs, &size) );
		ensure_equals( size, 1u );

		ensure ( 0 != GEOSCoordSeq_getDimensions(cs, &dims) );
		ensure_equals( dims, 3u );

		double x = 10;
		double y = 11;
		double z = 12;

        // Y, X, Z
		GEOSCoordSeq_setOrdinate(cs, 0, 1, y);
		GEOSCoordSeq_setOrdinate(cs, 0, 0, x);
		GEOSCoordSeq_setOrdinate(cs, 0, 2, z);

		double xcheck, ycheck, zcheck;
		ensure( 0 != GEOSCoordSeq_getOrdinate(cs, 0, 1, &ycheck) );
		ensure( 0 != GEOSCoordSeq_getOrdinate(cs, 0, 0, &xcheck) );
		ensure( 0 != GEOSCoordSeq_getOrdinate(cs, 0, 2, &zcheck) );

		ensure_equals( xcheck, x );
		ensure_equals( ycheck, y );
		ensure_equals( zcheck, z );
    }	
    
} // namespace tut

