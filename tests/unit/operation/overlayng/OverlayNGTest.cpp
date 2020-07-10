//
// Test Suite for geos::noding::snapround::SnapRoundingNoder class.

#include <tut/tut.hpp>
#include <utility.h>

// geos
#include <geos/io/WKTReader.h>
#include <geos/io/WKTWriter.h>
#include <geos/geom/Geometry.h>
#include <geos/operation/overlayng/OverlayNG.h>

// std
#include <memory>

using namespace geos::geom;
using namespace geos::operation::overlayng;
using geos::io::WKTReader;
using geos::io::WKTWriter;

namespace tut {
//
// Test Group
//

// Common data used by all tests
struct test_overlayng_data {

    WKTReader r;
    WKTWriter w;

    void
    testOverlay(const std::string& a, const std::string& b, const std::string& expected, int opCode, double scaleFactor)
    {
        PrecisionModel pm(scaleFactor);
        std::unique_ptr<Geometry> geom_a = r.read(a);
        std::unique_ptr<Geometry> geom_b = r.read(b);
        std::unique_ptr<Geometry> geom_expected = r.read(expected);
        std::unique_ptr<Geometry> geom_result = OverlayNG::overlay(geom_a.get(), geom_b.get(), opCode, &pm);
        ensure_equals_geometry(geom_expected.get(), geom_result.get());
    }

  // public static Geometry intersectionNoOpt(Geometry a, Geometry b, double scaleFactor) {
  //   PrecisionModel pm = new PrecisionModel(scaleFactor);
  //   OverlayNG ov = new OverlayNG(a, b, pm, INTERSECTION);
  //   ov.setOptimized(false);
  //   return ov.getResult();
  // }

};

typedef test_group<test_overlayng_data> group;
typedef group::object object;

group test_overlayng_group("geos::overation::overlayng::OverlayNG");

//
// Test Cases
//

//  Square overlapping square
template<>
template<>
void object::test<1> ()
{
    std::string a = "POLYGON((1000 1000, 2000 1000, 2000 2000, 1000 2000, 1000 1000))";
    std::string b = "POLYGON((1500 1500, 2500 1500, 2500 2500, 1500 2500, 1500 1500))";
    std::string exp = "POLYGON((1500 2000,2000 2000,2000 1500,1500 1500,1500 2000))";
    testOverlay(a, b, exp, OverlayNG::INTERSECTION, 1);
}





} // namespace tut
