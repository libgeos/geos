// 
// Test Suite for C-API GEOSRelatePatternMatch

#include <tut.hpp>
// geos
#include <geos_c.h>
// std
#include <cstdarg>
#include <cstdio>
#include <cstdlib>

namespace tut
{
    //
    // Test Group
    //

    // Common data used in test cases.
    struct test_capigeosrelatepatternmatch_data
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

        test_capigeosrelatepatternmatch_data()
        {
            initGEOS(notice, notice);
        }       

        ~test_capigeosrelatepatternmatch_data()
        {
            finishGEOS();
        }

    };

    typedef test_group<test_capigeosrelatepatternmatch_data> group;
    typedef group::object object;

    group test_capigeosrelatepatternmatch_group("capi::GEOSRelatePatternMatch");

    //
    // Test Cases
    //

    template<>
    template<>
    void object::test<1>()
    {
        char ret = GEOSRelatePatternMatch("000000000", "*********");
	ensure_equals(ret, char(1));
    }

    template<>
    template<>
    void object::test<2>()
    {
        char ret = GEOSRelatePatternMatch("000000000", "TTTTTTTTT");
	ensure_equals(ret, char(1));
    }
 
    template<>
    template<>
    void object::test<3>()
    {
        char ret = GEOSRelatePatternMatch("000000000", "000000000");
	ensure_equals(ret, char(1));
    }

    template<>
    template<>
    void object::test<4>()
    {
        char ret = GEOSRelatePatternMatch("000000000", "FFFFFFFFF");
	ensure_equals(ret, char(0));
    }

    template<>
    template<>
    void object::test<5>()
    {
        const char* mat = "012TF012F";

        char ret = GEOSRelatePatternMatch(mat, "TTTTFTTTF");
	ensure_equals(ret, char(1));

        ret = GEOSRelatePatternMatch(mat, "TT1TFTTTF");
	ensure_equals(ret, char(0));
    }

} // namespace tut

