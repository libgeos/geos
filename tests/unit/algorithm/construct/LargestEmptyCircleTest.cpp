//
// Test Suite for geos::algorithm::construct::LargestEmptyCircle


#include <tut/tut.hpp>
// geos
#include <geos/operation/distance/IndexedFacetDistance.h>
#include <geos/algorithm/construct/LargestEmptyCircle.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateArraySequence.h>
#include <geos/geom/Dimension.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/LineString.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/io/WKTReader.h>
#include <geos/io/WKTWriter.h>
// std
#include <sstream>
#include <string>
#include <memory>



using namespace geos;
using namespace geos::geom;

using geos::algorithm::construct::LargestEmptyCircle;

namespace tut {
//
// Test Group
//

// dummy data, not used
struct test_lec_data {
    geos::geom::Geometry* geom_;
    geos::geom::PrecisionModel pm_;
    geos::geom::GeometryFactory::Ptr factory_;
    geos::io::WKTReader reader_;
    geos::io::WKTWriter writer_;

    test_lec_data():
        geom_(nullptr),
        pm_(geos::geom::PrecisionModel::FLOATING),
        factory_(GeometryFactory::create(&pm_, 0)),
        reader_(factory_.get())
    {}

    ~test_lec_data()
    {
        factory_->destroyGeometry(geom_);
        geom_ = nullptr;
    }

    void
    ensure_equals_coordinate(const geos::geom::Coordinate &lhs,
                             const geos::geom::Coordinate &rhs, double tolerance)
    {
        ensure_equals("x coordinate does not match", lhs.x, rhs.x, tolerance);
        ensure_equals("y coordinate does not match", lhs.y, rhs.y, tolerance);
    }

    void
    checkCircle(const Geometry *geom, double build_tolerance, double x, double y, double expectedRadius)
    {
        double tolerance = 2*build_tolerance;
        LargestEmptyCircle lec(geom, tolerance);
        std::unique_ptr<Point> centerPoint = lec.getCenter();
        const Coordinate* centerPt = centerPoint->getCoordinate();
        Coordinate expectedCenter(x, y);
        ensure_equals_coordinate(*centerPt, expectedCenter, tolerance);
        std::unique_ptr<LineString> radiusLine = lec.getRadiusLine();
        double actualRadius = radiusLine->getLength();
        ensure_equals("radius", actualRadius, expectedRadius, tolerance);
        const Coordinate& linePt0 = radiusLine->getCoordinateN(0);
        const Coordinate& linePt1 = radiusLine->getCoordinateN(1);

        // std::cout << std::endl;
        // std::cout << writer_.write(geom) << std::endl;
        // std::cout << writer_.write(radiusLine.get()) << std::endl;

        ensure_equals_coordinate(*centerPt, linePt0, tolerance);
        const Coordinate radiusPt = *lec.getRadiusPoint()->getCoordinate();
        ensure_equals_coordinate(radiusPt, linePt1, tolerance);
    }

    void
    checkCircleZeroRadius(const Geometry *geom, double tolerance)
    {
        LargestEmptyCircle lec(geom, tolerance);
        std::unique_ptr<Point> centerPoint = lec.getCenter();
        const Coordinate* centerPt = centerPoint->getCoordinate();
        std::unique_ptr<LineString> radiusLine = lec.getRadiusLine();
        double actualRadius = radiusLine->getLength();
        ensure_equals("radius", actualRadius, 0.0, tolerance);

        const Coordinate& linePt0 = radiusLine->getCoordinateN(0);
        const Coordinate& linePt1 = radiusLine->getCoordinateN(1);

        ensure_equals_coordinate(*centerPt, linePt0, tolerance);
        const Coordinate radiusPt = *lec.getRadiusPoint()->getCoordinate();
        ensure_equals_coordinate(radiusPt, linePt1, tolerance);
    }

    void
    checkCircleZeroRadius(std::string wkt, double tolerance)
    {
        std::unique_ptr<Geometry> geom(reader_.read(wkt));
        checkCircleZeroRadius(geom.get(), tolerance);
    }

    void
    checkCircle(std::string wkt, double tolerance, double x, double y, double expectedRadius)
    {
        std::unique_ptr<Geometry> geom(reader_.read(wkt));
        checkCircle(geom.get(), tolerance, x, y, expectedRadius);
    }

};

typedef test_group<test_lec_data> group;
typedef group::object object;

group test_lec_group("geos::algorithm::construct::LargestEmptyCircle");

//
// testPointsSquare
//
template<>
template<>
void object::test<1>
()
{

    checkCircle("MULTIPOINT ((100 100), (100 200), (200 200), (200 100))",
       0.01, 150, 150, 70.71 );}

//
// testPointsTriangleOnHull
//
template<>
template<>
void object::test<2>
()
{
    checkCircle("MULTIPOINT ((100 100), (300 100), (150 50))",
       0.01, 216.66, 99.99, 83.33 );
}

//
// testPointsTriangleInterior
//
template<>
template<>
void object::test<3>
()
{
 checkCircle("MULTIPOINT ((100 100), (300 100), (200 250))",
       0.01, 200.00, 141.66, 108.33 );
}

//
// testLinesOpenDiamond
//
template<>
template<>
void object::test<4>
()
{

    checkCircle("MULTILINESTRING ((50 100, 150 50), (250 50, 350 100), (350 150, 250 200), (50 150, 150 200))",
       0.01, 200, 125, 90.13 );}

//
//    testLinesCrossed
//
template<>
template<>
void object::test<5>
()
{
  checkCircle("MULTILINESTRING ((100 100, 300 300), (100 200, 300 0))",
       0.01, 299.99, 150.00, 106.05 );
}


//
// testLinesZigzag
//
template<>
template<>
void object::test<6>
()
{

    checkCircle("MULTILINESTRING ((100 100, 200 150, 100 200, 250 250, 100 300, 300 350, 100 400), (50 400, 0 350, 50 300, 0 250, 50 200, 0 150, 50 100))",
       0.01, 77.52, 349.99, 54.81 );
}

//
// testPointsLinesTriangle
//
template<>
template<>
void object::test<7>
()
{
checkCircle("GEOMETRYCOLLECTION (LINESTRING (100 100, 300 100), POINT (250 200))",
       0.01, 196.49, 164.31, 64.31 );
}


//
// testPointsLinesTriangle
//
template<>
template<>
void object::test<8>
()
{
checkCircleZeroRadius("POINT (100 100)",
       0.01 );
}

//
// testLineFlat
//
template<>
template<>
void object::test<9>
()
{
 checkCircleZeroRadius("LINESTRING (0 0, 50 50)",
       0.01 );
}


} // namespace tut


