//
// Test Suite for geos::operation::buffer::BufferOp class.

// tut
#include <tut/tut.hpp>
#include <utility.h>

// geos
#include <geos/operation/buffer/OffsetCurve.h>
#include <geos/geom/Geometry.h>
#include <geos/io/WKTReader.h>
// #include <geos/io/WKTWriter.h>

// std
#include <memory>
#include <string>

namespace tut {
//
// Test Group
//

// Common data used by tests
struct test_offsetcurve_data {

    geos::io::WKTReader wktreader;
    // geos::io::WKTWriter wktwriter;

    test_offsetcurve_data() {};

    void checkOffsetCurve(const char* wkt, double distance, const char* wktExpected)
    {
        checkOffsetCurve(wkt, distance, wktExpected, 0.05);
    }

    void checkOffsetCurve(const char* wkt, double distance, const char* wktExpected, double tolerance)
    {
        std::string wktString(wkt);
        std::string wktExpect(wktExpected);
        std::unique_ptr<geos::geom::Geometry> geom = wktreader.read(wktString);
        std::unique_ptr<geos::geom::Geometry> result = geos::operation::buffer::OffsetCurve::getCurve(*geom, distance);
        //System.out.println(result);

        if (wktExpected == nullptr)
            return;

        std::unique_ptr<geos::geom::Geometry> expected = wktreader.read(wktExpected);
        ensure_equals_geometry(result.get(), expected.get(), tolerance);
    }

};

typedef test_group<test_offsetcurve_data> group;
typedef group::object object;

group test_offsetcurve_group("geos::operation::buffer::OffsetCurve");

//
// Test Cases
//
template<>
template<>
void object::test<1> ()
{

}


} // namespace tut
