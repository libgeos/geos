//
// Test Suite for geos::operation::distance::DistanceOp class.

// tut
#include <tut/tut.hpp>
// geos
#include <geos/algorithm/PointLocator.h>
#include <geos/io/WKTReader.h>
#include <geos/io/WKBReader.h>
#include <geos/constants.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateArraySequence.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/LineSegment.h>
#include <geos/geom/LineString.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/operation/distance/DistanceOp.h>
#include <geos/operation/distance/IndexedFacetDistance.h>
// std
#include <memory>
#include <string>
#include <vector>

namespace tut {
//
// Test Group
//

// Common data used by tests
struct test_facetdistanceop_data {
    geos::io::WKTReader wktreader;

    typedef std::unique_ptr<geos::geom::Geometry> GeomPtr;
    typedef std::unique_ptr<geos::geom::CoordinateSequence> CSPtr;

    test_facetdistanceop_data()
        : wktreader()
    {}

    void
    checkDistanceNearestPoints(std::string wkt1, std::string wkt2, double distance,
                               geos::geom::Coordinate& p1, geos::geom::Coordinate& p2)
    {
        using geos::operation::distance::IndexedFacetDistance;
        GeomPtr g1(wktreader.read(wkt1));
        GeomPtr g2(wktreader.read(wkt2));
        std::vector<geos::geom::Coordinate> pts;
        pts = IndexedFacetDistance::nearestPoints(g1.get(), g2.get());
        ensure(fabs(pts[0].distance(pts[1])-distance) < 1e08);
        ensure(fabs(pts[0].x - p1.x) < 1e08);
        ensure(fabs(pts[0].y - p1.y) < 1e08);
        ensure(fabs(pts[1].x - p2.x) < 1e08);
        ensure(fabs(pts[1].y - p2.y) < 1e08);
        return;
    }

};

typedef test_group<test_facetdistanceop_data> group;
typedef group::object object;

group test_facetdistanceop_group("geos::operation::distance::IndexedFacetDistance");



//
// Test Cases
//
template<>
template<>
void object::test<1>
()
{
    using geos::operation::distance::IndexedFacetDistance;
    using geos::geom::Coordinate;

    std::string wkt0("POINT(0 0)");
    std::string wkt1("POINT(10 0)");
    GeomPtr g0(wktreader.read(wkt0));
    GeomPtr g1(wktreader.read(wkt1));
    double d = IndexedFacetDistance::distance(g0.get(), g1.get());
    ensure_equals(d, 10);
}

template<>
template<>
void object::test<2>
()
{
    using geos::operation::distance::DistanceOp;
    using geos::geom::Coordinate;

    std::string wkt0("POLYGON ((200 180, 60 140, 60 260, 200 180))");
    std::string wkt1("POINT (140 280)");
    double dist = 57.05597791103589;
    Coordinate p1(111.6923076923077, 230.46153846153845);
    Coordinate p2(140, 280);

    checkDistanceNearestPoints(wkt0, wkt1, dist, p1, p2);
}

template<>
template<>
void object::test<3>
()
{
    using geos::operation::distance::DistanceOp;
    using geos::geom::Coordinate;

    std::string wkt0("POLYGON ((200 180, 60 140, 60 260, 200 180))");
    std::string wkt1("MULTIPOINT ((140 280), (140 320))");
    double dist = 57.05597791103589;
    Coordinate p1(111.6923076923077, 230.46153846153845);
    Coordinate p2(140, 280);

    checkDistanceNearestPoints(wkt0, wkt1, dist, p1, p2);
}

template<>
template<>
void object::test<4>
()
{
    using geos::operation::distance::DistanceOp;
    using geos::geom::Coordinate;

    std::string wkt0("LINESTRING (100 100, 200 100, 200 200, 100 200, 100 100)");
    std::string wkt1("POINT (10 10)");
    double dist = 127.27922061357856;
    Coordinate p1(100, 100);
    Coordinate p2(10, 10);

    checkDistanceNearestPoints(wkt0, wkt1, dist, p1, p2);
}

template<>
template<>
void object::test<5>
()
{
    using geos::operation::distance::DistanceOp;
    using geos::geom::Coordinate;

    std::string wkt0("POLYGON ((76 185, 125 283, 331 276, 324 122, 177 70, 184 155, 69 123, 76 185), (267 237, 148 248, 135 185, 223 189, 251 151, 286 183, 267 237))");
    std::string wkt1("LINESTRING (153 204, 185 224, 209 207, 238 222, 254 186)");
    double dist = 13.788860460124573;
    Coordinate p1(139.4956500724988, 206.78661188980183);
    Coordinate p2(153, 204);

    checkDistanceNearestPoints(wkt0, wkt1, dist, p1, p2);
}

template<>
template<>
void object::test<6>
()
{
    using geos::operation::distance::DistanceOp;
    using geos::geom::Coordinate;

    std::string wkt0("POLYGON ((76 185, 125 283, 331 276, 324 122, 177 70, 184 155, 69 123, 76 185), (267 237, 148 248, 135 185, 223 189, 251 151, 286 183, 267 237))");
    std::string wkt1("LINESTRING (120 215, 185 224, 209 207, 238 222, 254 186)");
    double dist = 0.0;
    Coordinate p1(120, 215);
    Coordinate p2(120, 215);

    checkDistanceNearestPoints(wkt0, wkt1, dist, p1, p2);
}

// TODO: finish the tests by adding:
// 	LINESTRING - *all*
// 	MULTILINESTRING - *all*
// 	POLYGON - *all*
// 	MULTIPOLYGON - *all*
// 	COLLECTION - *all*

} // namespace tut

