// 
// Test Suite for C-API GEOSCoordSeq

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
    struct test_capigeoscoordseq_data
    {

        GEOSCoordSequence* cs_;

        static void notice(const char *fmt, ...)
        {
            std::fprintf( stdout, "NOTICE: ");

            va_list ap;
            va_start(ap, fmt);
            std::vfprintf(stdout, fmt, ap);
            va_end(ap);
        
            std::fprintf(stdout, "\n");
        }

        test_capigeoscoordseq_data() : cs_(0)
        {
            initGEOS(notice, notice);
        }       

        ~test_capigeoscoordseq_data()
        {
            GEOSCoordSeq_destroy(cs_);
            cs_ = 0;
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
        cs_ = GEOSCoordSeq_create(5, 3);
        
        unsigned int size;
        unsigned int dims;

        ensure ( 0 != GEOSCoordSeq_getSize(cs_, &size) );
        ensure_equals( size, 5u );

        ensure ( 0 != GEOSCoordSeq_getDimensions(cs_, &dims) );
        ensure_equals( dims, 3u );

        for (unsigned int i=0; i<5; ++i)
        {
            double x = i*10;
            double y = i*10+1;
            double z = i*10+2;

            GEOSCoordSeq_setX(cs_, i, x);
            GEOSCoordSeq_setY(cs_, i, y);
            GEOSCoordSeq_setZ(cs_, i, z);

            double xcheck, ycheck, zcheck;
            ensure( 0 != GEOSCoordSeq_getX(cs_, i, &xcheck) );
            ensure( 0 != GEOSCoordSeq_getY(cs_, i, &ycheck) );
            ensure( 0 != GEOSCoordSeq_getZ(cs_, i, &zcheck) );

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
        cs_ = GEOSCoordSeq_create(1, 3);
        
        unsigned int size;
        unsigned int dims;

        ensure ( 0 != GEOSCoordSeq_getSize(cs_, &size) );
        ensure_equals( size, 1u );

        ensure ( 0 != GEOSCoordSeq_getDimensions(cs_, &dims) );
        ensure_equals( dims, 3u );

        double x = 10;
        double y = 11;
        double z = 12;

        // X, Y, Z
        GEOSCoordSeq_setX(cs_, 0, x);
        GEOSCoordSeq_setY(cs_, 0, y);
        GEOSCoordSeq_setZ(cs_, 0, z);

        double xcheck, ycheck, zcheck;
        ensure( 0 != GEOSCoordSeq_getY(cs_, 0, &ycheck) );
        ensure( 0 != GEOSCoordSeq_getX(cs_, 0, &xcheck) );
        ensure( 0 != GEOSCoordSeq_getZ(cs_, 0, &zcheck) );

        ensure_equals( xcheck, x );
        ensure_equals( ycheck, y );
        ensure_equals( zcheck, z );
    }   

    // Test not swapped setOrdinate calls (see bug #133, fixed)
    template<>
    template<>
    void object::test<3>()
    {
        cs_ = GEOSCoordSeq_create(1, 3);
        
        unsigned int size;
        unsigned int dims;

        ensure ( 0 != GEOSCoordSeq_getSize(cs_, &size) );
        ensure_equals( size, 1u );

        ensure ( 0 != GEOSCoordSeq_getDimensions(cs_, &dims) );
        ensure_equals( dims, 3u );

        double x = 10;
        double y = 11;
        double z = 12;

        // X, Y, Z
        GEOSCoordSeq_setOrdinate(cs_, 0, 0, x);
        GEOSCoordSeq_setOrdinate(cs_, 0, 1, y);
        GEOSCoordSeq_setOrdinate(cs_, 0, 2, z);

        double xcheck, ycheck, zcheck;
        ensure( 0 != GEOSCoordSeq_getOrdinate(cs_, 0, 1, &ycheck) );
        ensure( 0 != GEOSCoordSeq_getOrdinate(cs_, 0, 0, &xcheck) );
        ensure( 0 != GEOSCoordSeq_getOrdinate(cs_, 0, 2, &zcheck) );

        ensure_equals( xcheck, x );
        ensure_equals( ycheck, y );
        ensure_equals( zcheck, z );
    }   

    // Test swapped setX calls (see bug #133, fixed)
    template<>
    template<>
    void object::test<4>()
    {
        cs_ = GEOSCoordSeq_create(1, 3);
        
        unsigned int size;
        unsigned int dims;

        ensure ( 0 != GEOSCoordSeq_getSize(cs_, &size) );
        ensure_equals( size, 1u );

        ensure ( 0 != GEOSCoordSeq_getDimensions(cs_, &dims) );
        ensure_equals( dims, 3u );

        double x = 10;
        double y = 11;
        double z = 12;

        // Y, X, Z
        GEOSCoordSeq_setY(cs_, 0, y);
        GEOSCoordSeq_setX(cs_, 0, x);
        GEOSCoordSeq_setZ(cs_, 0, z);

        double xcheck, ycheck, zcheck;
        ensure( 0 != GEOSCoordSeq_getY(cs_, 0, &ycheck) );
        ensure( 0 != GEOSCoordSeq_getX(cs_, 0, &xcheck) );
        ensure( 0 != GEOSCoordSeq_getZ(cs_, 0, &zcheck) );

        ensure_equals( xcheck, x );
        ensure_equals( ycheck, y );
        ensure_equals( zcheck, z );
    }   

    // Test swapped setOrdinate calls (see bug #133, fixed)
    template<>
    template<>
    void object::test<5>()
    {
        cs_ = GEOSCoordSeq_create(1, 3);
        
        unsigned int size;
        unsigned int dims;

        ensure ( 0 != GEOSCoordSeq_getSize(cs_, &size) );
        ensure_equals( size, 1u );

        ensure ( 0 != GEOSCoordSeq_getDimensions(cs_, &dims) );
        ensure_equals( dims, 3u );

        double x = 10;
        double y = 11;
        double z = 12;

        // Y, X, Z
        GEOSCoordSeq_setOrdinate(cs_, 0, 1, y);
        GEOSCoordSeq_setOrdinate(cs_, 0, 0, x);
        GEOSCoordSeq_setOrdinate(cs_, 0, 2, z);

        double xcheck, ycheck, zcheck;
        ensure( 0 != GEOSCoordSeq_getOrdinate(cs_, 0, 1, &ycheck) );
        ensure( 0 != GEOSCoordSeq_getOrdinate(cs_, 0, 0, &xcheck) );
        ensure( 0 != GEOSCoordSeq_getOrdinate(cs_, 0, 2, &zcheck) );

        ensure_equals( xcheck, x );
        ensure_equals( ycheck, y );
        ensure_equals( zcheck, z );
    }   

    // Test getDimensions call (see bug #135)
    template<>
    template<>
    void object::test<6>()
    {
        cs_ = GEOSCoordSeq_create(1, 2);
        
        unsigned int size;
        unsigned int dims;

        ensure ( 0 != GEOSCoordSeq_getSize(cs_, &size) );
        ensure_equals( size, 1u );

        ensure ( 0 != GEOSCoordSeq_getDimensions(cs_, &dims) );

	// The dimension passed to GEOSCoordSeq_create()
	// is a request for a minimum, not a strict mandate
	// for changing actual size.
	//
        ensure ( dims >= 2u );

    }   
    
} // namespace tut

