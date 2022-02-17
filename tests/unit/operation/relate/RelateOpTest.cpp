//
// Test Suite for geos::operation::relate::RelateOp class

#include <tut/tut.hpp>
// geos
#include <geos/constants.h> // for std::isnan
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateArraySequence.h>
#include <geos/geom/Dimension.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/LineString.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/io/WKTReader.h>
#include <geos/operation/relate/RelateOp.h>

// std
#include <cmath>
#include <string>
#include <memory>

using namespace geos::geom;
using namespace geos::operation::relate;

namespace tut {
//
// Test Group
//

struct test_relateop_data {
    geos::io::WKTReader wktreader;
    typedef std::unique_ptr<Geometry> GeomPtr;
    typedef geos::geom::GeometryFactory GeometryFactory;

    geos::geom::PrecisionModel pm_;
    GeometryFactory::Ptr factory_;

    test_relateop_data()
        : pm_(1), factory_(GeometryFactory::create(&pm_, 0))
    {}

    void checkRelate(const char* wkta, const char* wktb, const char* imExpected)
    {
        std::string wktstra(wkta);
        std::string wktstrb(wktb);
        auto ga = wktreader.read(wktstra);
        auto gb = wktreader.read(wktstrb);

        std::unique_ptr<IntersectionMatrix> im(ga->relate( gb.get() ));
        auto imActual = im->toString();
        ensure_equals(imExpected, imActual);
    }

};


typedef test_group<test_relateop_data> group;
typedef group::object object;

group test_relateop_group("geos::operation::relate::RelateOp");

//
// Test Cases
//

// 1 - testInvalidCoordinate
template<>
template<>
void object::test<1> ()
{
    checkRelate(
"LINESTRING (-57.2681216 49.4063466, -57.267725199999994 49.406617499999996, -57.26747895046037 49.406750916517765)",
"LINESTRING (-57.267475399999995 49.4067465, -57.2675701 49.406864299999995, -57.267989 49.407135399999994)",
"FF10F0102"
    );

}

} // namespace tut
