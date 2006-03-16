#ifdef _MSC_VER
#ifdef GEOS_DEBUG_MSVC_USE_VLD
#include <vld.h>
#endif
#endif

// TUT
#include <tut.h>
#include <tut_reporter.h>
// STL
#include <iostream>
// GEOS
#include <geos/unload.h>

namespace tut
{
    test_runner_singleton runner;
}

void usage()
{
    std::cout << "Usage: <geos_tut> [regression] | [list] | [ group] [test]" << std::endl;
    std::cout << "       List all groups: geos_tut list" << std::endl;
    std::cout << "       Run all tests: geos_tut regression" << std::endl;
    std::cout << "       Run one group: geos_tut <group name>" << std::endl;
    std::cout << "       Run one test: geos_tut <group name> 3" << std::endl;
    std::cout << "\nGEOS homepage: http://geos.refractions.net" << std::endl;

}

int main(int argc,const char* argv[])
{
    tut::reporter visi;

    std::cout << "GEOS Test Suite Application" << std::endl;
    std::cout << "===========================" << std::endl;

    if( argc < 2 || argc > 3 )
    {
        usage();
        return 0;
    }

    tut::runner.get().set_callback(&visi);

    try
    {
        if( argc == 1 || (argc == 2 && std::string(argv[1]) == "regression") )
        {
            tut::runner.get().run_tests();
        }
        else if( argc == 2 && std::string(argv[1]) == "list" )
        {
            tut::groupnames gl = tut::runner.get().list_groups();
            tut::groupnames::const_iterator b = gl.begin();
            tut::groupnames::const_iterator e = gl.end();

            tut::groupnames::difference_type d = std::distance(b, e);

            std::cout << "Registered " << d << " test groups:\n" << std::endl;
            
            while( b != e )
            {
                std::cout << "  " << *b << std::endl;
                ++b;
            }
        }
        else if( argc == 2 && std::string(argv[1]) != "regression" )
        {
            tut::runner.get().run_tests(argv[1]);
        }
        else if( argc == 3 )
        {
            // TODO - mloskot - check if test group with given name exists
            // TODO - mloskot - check if test case with given number exists

            tut::runner.get().run_test(argv[1], ::atoi(argv[2]));
        }
    }
    catch( const std::exception& ex )
    {
        std::cerr << "!!! GEOS Test Suite raised exception: " << ex.what() << std::endl;
    }

    // XXX - mloskot - this should be removed in future!
    geos::io::Unload::Release();
    return 0;
}
