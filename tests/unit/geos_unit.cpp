// 
// Test Suite Runner
//
#ifdef _MSC_VER
#ifdef GEOS_DEBUG_MSVC_USE_VLD
#include <vld.h>
#endif
#endif

// tut
#include <tut.hpp>
#include <tut_reporter.hpp>
// geos 
#include <geos/unload.h>
// std
#include <cstdlib>
#include <iomanip>
#include <iostream>

namespace tut
{
    test_runner_singleton runner;
}

void usage()
{
    using std::cout;
    using std::endl;

    const std::string module("geos_unit");

    //[list] | [ group] [test]
    cout << "Usage: " << module << " [OPTION] [TARGET]\n"
        << endl
        << "Targets:\n"
        << "  <none>                          run all tests in all groups\n"
        << "  <group name>                    run all tests from given group\n"
        << "  <group name> <test nr>          run single test with given number from given group\n"
        << endl
        << "Options:\n"
        << "  --list                          list all registered test groups\n"
        << "  --verbose                       run unit tests verbosely; displays non-error information\n"
        << "  --version                       print version information and exit\n"
        << "  --help                          print this message and exit\n"
        << endl
        << "Examples:\n"
        << "  " << module << " -v\n"
        << "  " << module << " list\n"
        << "  " << module << " geos::geom::Envelope\n"
        << "  " << module << " geos::geom::Envelope 2\n"
        << endl
        << "GEOS homepage: http://geos.osgeo.org" << endl;
}

int main(int argc, const char* argv[])
{
    tut::reporter visi;

    if ( (argc == 2 && std::string(argv[1]) == "--help") || argc > 3 )
    {
        usage();
        return 0;
    }

    std::cout << "===============================\n"
              << "  GEOS Test Suite Application\n"
              << "===============================\n";

    tut::runner.get().set_callback(&visi);

    try
    {
        if ( argc == 1 )
        {
            tut::runner.get().run_tests();
        }
        else if ( argc == 2 && std::string(argv[1]) == "--list" )
        {
            tut::groupnames gl = tut::runner.get().list_groups();
            tut::groupnames::const_iterator b = gl.begin();
            tut::groupnames::const_iterator e = gl.end();

            tut::groupnames::difference_type d = std::distance(b, e);

            std::cout << "Registered " << d << " test groups:\n" << std::endl;
            
            while ( b != e )
            {
                std::cout << "  " << *b << std::endl;
                ++b;
            }
        }
        else if ( argc == 2 && std::string(argv[1]) != "--list" )
        {
            tut::runner.get().run_tests(argv[1]);
        }
        else if( argc == 3 )
        {
            // TODO - mloskot - check if test group with given name exists
            // TODO - mloskot - check if test case with given number exists
            std::string grpname(argv[1]);
            if (grpname.empty())
                throw std::runtime_error("missing test group name");

            tut::test_result result;
            tut::runner.get().run_test(grpname, std::atoi(argv[2]), result);
        }
    }
    catch( const std::exception& ex )
    {
        std::cerr << "!!! GEOS Test Suite raised exception: " << ex.what() << std::endl;
    }

    // XXX - mloskot - this should be removed in future!
    geos::io::Unload::Release();

    return (visi.all_ok() ? EXIT_SUCCESS : EXIT_FAILURE);   
}
