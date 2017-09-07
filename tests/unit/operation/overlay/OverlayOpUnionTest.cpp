//
// Test Suite for geos::operation::OverlayOp class for UNION

#include <tut/tut.hpp>
// geos
#include <geos/operation/overlay/OverlayOp.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/io/WKBReader.h>
#include <geos/io/WKTReader.h>
// std
#include <string>
#include <memory>

using namespace geos::geom;
using namespace geos::operation;

namespace tut
{
    //
    // Test Group
    //

    struct test_overlayopunion_data
    {
        typedef geos::geom::Geometry::Ptr GeometryPtr;
        typedef geos::geom::GeometryFactory GeometryFactory;
        typedef geos::geom::GeometryFactory::Ptr GeometryFactoryPtr;
    };

    typedef test_group<test_overlayopunion_data> group;
    typedef group::object object;

    group test_overlayopunion_group("geos::operation::OverlayOp::UNION");

    //
    // Test Cases
    //

    // 1 - Union four connected segments of a square
    template<>
    template<>
    void object::test<1>()
    {
        GeometryFactoryPtr factory = geos::geom::GeometryFactory::create();
        geos::io::WKTReader reader(*factory);
        GeometryPtr line1(reader.read("LINESTRING(0 0, 0 5)"));
        GeometryPtr line2(reader.read("LINESTRING(0 5, 5 5)"));
        GeometryPtr line3(reader.read("LINESTRING(5 5, 5 0)"));
        GeometryPtr line4(reader.read("LINESTRING(5 0, 0 0)"));

        // union segments incrementally
        GeometryPtr lines12(line1->Union(line2.get()));
        GeometryPtr lines123(lines12->Union(line3.get()));
        GeometryPtr lines1234(lines123->Union(line4.get()));

        ensure_equals(lines1234->getGeometryTypeId(), geos::geom::GEOS_MULTILINESTRING);

        // NOTE: Since the union operation makes no effort to simplify and
        // drop nodes of degree 2 from the built topology,
        // do not expect GEOS_LINESTRING.
        // See LineMergerTest where the test triangle is generated as a single LineString.
    }

} // namespace tut
