// 
// Test Suite for geos::operation::OverlayOp class for UNION

#include <tut.hpp>
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
        typedef geos::geom::Geometry::AutoPtr GeometryPtr;
        typedef geos::geom::GeometryFactory GeometryFactory;
        typedef geos::geom::GeometryFactory::unique_ptr GeometryFactoryPtr;
    };

    typedef test_group<test_overlayopunion_data> group;
    typedef group::object object;

    group test_overlayopunion_group("geos::operation::OverlayOp::UNION");

    //
    // Test Cases
    //

    // 1 - Union four segments of a square
    template<>
    template<>
    void object::test<1>()
    {
        // Arrange
        geos::geom::PrecisionModel pm(1e+13);
        GeometryFactoryPtr factory = geos::geom::GeometryFactory::create(&pm);
        geos::io::WKTReader reader(*factory);
        GeometryPtr line1(reader.read("LINESTRING(0 0, 0 5)"));
        GeometryPtr line2(reader.read("LINESTRING(0 5, 5 5)"));
        GeometryPtr line3(reader.read("LINESTRING(5 5, 5 0)"));
        GeometryPtr line4(reader.read("LINESTRING(5 0, 0 0)"));

        // Act: union segments incrementally
        GeometryPtr lines12(line1->Union(line2.get()));
        GeometryPtr lines123(lines12->Union(line3.get())); 
        GeometryPtr lines1234(lines123->Union(line4.get()));

        // Assert
        ensure_equals(lines1234->getGeometryTypeId(), geos::geom::GEOS_MULTILINESTRING);
    }

} // namespace tut
