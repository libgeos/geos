//
// Test Suite Runner
//
#ifdef _MSC_VER
#ifdef GEOS_DEBUG_MSVC_USE_VLD
#include <vld.h>
#endif
#endif

// tut
#include <tut/tut.hpp>
#include <tut/tut_reporter.hpp>
// std
#include <cstdlib>
#include <iomanip>
#include <iostream>

std::string RESOURCE_DIR;

namespace tut {
test_runner_singleton runner;
}

void
usage()
{
    using std::cout;
    using std::endl;

    const std::string module("geos_unit");

    //[list] | [ group] [test]
    cout << "Usage: " << module << " [OPTION] [TARGET]\n"
         << endl
         << "Targets:\n"
         << "  <none>                          run all tests in all groups\n"
         << "  <group name>                    run all tests in group\n"
         << "  <group name> <test num>         run single group test <num>\n"
         << endl
         << "Options:\n"
         << "  --list                          list all registered test groups\n"
//         << "  --verbose                       run unit tests verbosely; displays non-error information\n"
//         << "  --version                       print version information and exit\n"
         << "  --data                          specify a directory containing test data files\n"
         << "  --help                          print this message and exit\n"
         << endl
         << "Examples:\n"
//         << "  " << module << " -v\n"
         << "  " << module << " list\n"
         << "  " << module << " geos::geom::Envelope\n"
         << "  " << module << " geos::geom::Envelope 2\n"
         << endl
         << "GEOS homepage: https://libgeos.org" << endl;
}

int
main(int argc, const char* argv[]) {
    tut::reporter visi;

    bool listOnly = false;
    std::string grpname;
    std::string testName;

    for (std::size_t i = 1; i < static_cast<std::size_t>(argc); i++) {
        std::string arg = argv[i];
        if (arg == "--help") {
            usage();
            return EXIT_SUCCESS;
        } else if (arg == "--list") {
            listOnly = true;
        } else if (arg == "--data" && (i + 1) < static_cast<std::size_t>(argc)) {
            RESOURCE_DIR = argv[++i];
        } else if (arg[0] == '-') {
            std::cerr << "Invalid option: " << argv << std::endl;
            return EXIT_FAILURE;
        } else if (grpname.empty()) {
            grpname = arg;
        } else if (testName.empty()) {
            testName = arg;
        } else {
            std::cerr << "Unexpected positional argument: " << arg << std::endl;
            return EXIT_FAILURE;
        }
    }

    std::cout << "===============================\n"
              << "  GEOS Unit Test Suite\n"
              << "===============================\n";

    tut::runner.get().set_callback(&visi);

    try {
        if (listOnly) {
            tut::groupnames gl = tut::runner.get().list_groups();
            tut::groupnames::const_iterator b = gl.begin();
            tut::groupnames::const_iterator e = gl.end();

            tut::groupnames::difference_type d = std::distance(b, e);

            std::cout << "Registered " << d << " test groups:\n" << std::endl;

            while(b != e) {
                std::cout << "  " << *b << std::endl;
                ++b;
            }
            return EXIT_SUCCESS;
        } else {
            if (!testName.empty()) {
                tut::test_result result;
                tut::runner.get().run_test(grpname, std::atoi(testName.c_str()), result);
            } else if (!grpname.empty()) {
                tut::runner.get().run_tests(grpname);
            } else {
                tut::runner.get().run_tests();
            }
        }
    }
    catch(const tut::no_such_group& ex) {
        std::cerr << "!!! GEOS Test Suite - unknown test group " << ex.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch(const std::exception& ex) {
        std::cerr << "!!! GEOS Test Suite raised exception: " << ex.what() << std::endl;
        return EXIT_FAILURE;
    }

    return (visi.all_ok() ? EXIT_SUCCESS : EXIT_FAILURE);
}
