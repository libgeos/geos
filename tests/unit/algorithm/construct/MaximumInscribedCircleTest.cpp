//
// Test Suite for geos::algorithm::construct::MaximumInscribedCircle

#include <tut/tut.hpp>
// geos
#include <geos/operation/distance/IndexedFacetDistance.h>
#include <geos/algorithm/construct/MaximumInscribedCircle.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateArraySequence.h>
#include <geos/geom/Dimension.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/LineString.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/io/WKTReader.h>
// std
#include <sstream>
#include <string>
#include <memory>



using namespace geos;
using namespace geos::geom;

using geos::algorithm::construct::MaximumInscribedCircle;

namespace tut {
//
// Test Group
//

// dummy data, not used
struct test_mic_data {
    geos::geom::Geometry* geom_;
    geos::geom::PrecisionModel pm_;
    geos::geom::GeometryFactory::Ptr factory_;
    geos::io::WKTReader reader_;

    test_mic_data():
        geom_(nullptr),
        pm_(1),
        factory_(GeometryFactory::create(&pm_, 0)),
        reader_(factory_.get())
    {}

    ~test_mic_data()
    {
        factory_->destroyGeometry(geom_);
        geom_ = nullptr;
    }

    void
    ensure_equals_coordinate(const geos::geom::Coordinate &lhs,
                             const geos::geom::Coordinate &rhs, double tolerance)
    {
        ensure_equals("x coordinate does not match", lhs.x, rhs.y, tolerance);
        ensure_equals("y coordinate does not match", lhs.y, rhs.y, tolerance);
    }

    void
    checkCircle(const Geometry *geom, double tolerance, double x, double y, double expectedRadius)
    {
        MaximumInscribedCircle mic(geom, tolerance);
        std::unique_ptr<Point> centerPoint = mic.getCenter();
        const Coordinate* centerPt = centerPoint->getCoordinate();
        Coordinate expectedCenter(x, y);
        ensure_equals_coordinate(*centerPt, expectedCenter, tolerance);
        std::unique_ptr<LineString> radiusLine = mic.getRadiusLine();
        double actualRadius = radiusLine->getLength();
        ensure_equals("radius", actualRadius, expectedRadius, tolerance);
        const Coordinate linePt0 = radiusLine->getCoordinateN(0);
        const Coordinate linePt1 = radiusLine->getCoordinateN(1);

        ensure_equals_coordinate(*centerPt, linePt0, tolerance);
        const Coordinate* radiusPt = mic.getRadiusPoint()->getCoordinate();
        ensure_equals_coordinate(*radiusPt, linePt1, tolerance);
    }

    void
    checkCircle(std::string wkt, double tolerance, double x, double y, double expectedRadius)
    {
        std::unique_ptr<Geometry> geom(reader_.read(wkt));
        checkCircle(geom.get(), tolerance, x, y, expectedRadius);
    }

};

typedef test_group<test_mic_data> group;
typedef group::object object;

group test_mic_group("geos::algorithm::construct::MaximumInscribedCircle");


//
// Square
//
template<>
template<>
void object::test<1>
()
{
    checkCircle("POLYGON ((100 200, 200 200, 200 100, 100 100, 100 200))",
                0.001, 150, 150, 50 );
}

// template<>
// template<>
// void object::test<2>
// ()
// {
//     checkAreaOfRingSigned("LINESTRING (100 200, 200 200, 200 100, 100 100, 100 200)", 10000.0);
// }

// template<>
// template<>
// void object::test<3>
// ()
// {
//     checkAreaOfRingSigned("LINESTRING (100 200, 100 100, 200 100, 200 200, 100 200)", -10000.0);
// }


} // namespace tut

