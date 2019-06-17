//
// Test Suite for geos::operation::overlay::validate::OffsetPointGenerator class.

// tut
#include <tut/tut.hpp>
// geos
#include <geos/operation/overlay/validate/OffsetPointGenerator.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Point.h>
#include <geos/geom/Geometry.h>
#include <geos/algorithm/PointLocator.h>
#include <geos/io/WKTReader.h>
#include <geos/geom/Coordinate.h>
// std
#include <memory>
#include <vector>

namespace tut {
using namespace std;
using namespace geos::geom;

//
// Test Group
//

// Common data used by tests
struct test_offsetpointgenerator_data {
    const geos::geom::GeometryFactory& gf;
    geos::io::WKTReader wktreader;
    geos::algorithm::PointLocator locator;

    typedef geos::geom::Geometry::Ptr GeomPtr;

    GeomPtr g;

    test_offsetpointgenerator_data()
        :
        gf(*GeometryFactory::getDefaultInstance()),
        wktreader(&gf)
    {
    }

    bool
    pointsWithinDistance(vector<Coordinate>& coords, double dist)
    {
        // we expect some numerical instability here
        // OffsetPointGenerator could produce points
        // at *slightly* farther locations then
        // requested
        //
        dist *= 1.0000001;

        for(size_t i = 0, n = coords.size(); i < n; ++i) {
            const Coordinate& c = coords[i];
            unique_ptr<Geometry> pg(gf.createPoint(c));
            double rdist =  g->distance(pg.get());
            if(rdist > dist) {
                return false;
            }
        }
        return true;
    }

};


typedef test_group<test_offsetpointgenerator_data> group;
typedef group::object object;

group test_offsetpointgenerator_group("geos::operation::overlay::validate::OffsetPointGenerator");

//
// Test Cases
//

template<>
template<>
void object::test<1>
()
{
    using geos::operation::overlay::validate::OffsetPointGenerator;
    using geos::geom::Coordinate;
    using geos::algorithm::PointLocator;
    using std::unique_ptr;
    using std::vector;

    std::string wkt("POLYGON((0 0, 10 0, 10 10, 0 10, 0 0))");
    g = wktreader.read(wkt);

    double dist = 10;
    OffsetPointGenerator gen(*g, dist);

    unique_ptr< vector<Coordinate> > coords(gen.getPoints());

    ensure_equals(coords->size(), (g->getNumPoints() - 1) * 2);

    ensure(pointsWithinDistance(*coords, dist));

}

template<>
template<>
void object::test<2>
()
{
    using geos::operation::overlay::validate::OffsetPointGenerator;
    using geos::geom::Location;
    using geos::geom::Coordinate;
    using std::unique_ptr;
    using std::vector;

    std::string wkt("POLYGON((0 0, 10 0, 10 5, 10 10, 0 10, 0 0))");
    g = wktreader.read(wkt);

    double dist = 0.0003;

    OffsetPointGenerator gen(*g, dist);

    unique_ptr< vector<Coordinate> > coords(gen.getPoints());

    ensure_equals(coords->size(), 10u);

    ensure(pointsWithinDistance(*coords, dist));
}

template<>
template<>
void object::test<3>
()
{
    using geos::operation::overlay::validate::OffsetPointGenerator;
    using geos::geom::Location;
    using geos::geom::Coordinate;
    using std::unique_ptr;
    using std::vector;

    std::string wkt("POINT(10 -10)");
    g = wktreader.read(wkt);

    double dist = 0.0003;

    OffsetPointGenerator gen(*g, dist);

    unique_ptr< vector<Coordinate> > coords(gen.getPoints());

    ensure_equals(coords->size(), 0u);

    //ensure(pointsWithinDistance(*coords, dist));
}

template<>
template<>
void object::test<4>
()
{
    using geos::operation::overlay::validate::OffsetPointGenerator;
    using geos::geom::Location;
    using geos::geom::Coordinate;
    using std::unique_ptr;
    using std::vector;

    std::string wkt("LINESTRING(10 -10, 5 600)");
    g = wktreader.read(wkt);

    double dist = 0.03;

    OffsetPointGenerator gen(*g, dist);

    unique_ptr< vector<Coordinate> > coords(gen.getPoints());

    ensure_equals(coords->size(), 2u);

    ensure(pointsWithinDistance(*coords, dist));
}

template<>
template<>
void object::test<5>
()
{
    using geos::operation::overlay::validate::OffsetPointGenerator;
    using geos::geom::Location;
    using geos::geom::Coordinate;
    using std::unique_ptr;
    using std::vector;

    std::string wkt("MULTILINESTRING((10 -10, 5 600), (1045 -12, 0 0, -435 34))");
    g = wktreader.read(wkt);

    double dist = 0.2;

    OffsetPointGenerator gen(*g, dist);

    unique_ptr< vector<Coordinate> > coords(gen.getPoints());

    ensure_equals(coords->size(), 6u);

    ensure(pointsWithinDistance(*coords, dist));
}



} // namespace tut

