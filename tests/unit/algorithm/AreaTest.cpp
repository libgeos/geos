//
// Test Suite for geos::algorithm::Area
// Ported from JTS junit/algorithm/AreaTest.java

#include <tut/tut.hpp>
// geos
#include <geos/algorithm/Area.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/Dimension.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/LineString.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/io/WKTReader.h>
// std
#include <string>
#include <memory>

using namespace geos;
using namespace geos::geom;
using geos::algorithm::Area;

namespace tut {
//
// Test Group
//

// dummy data, not used
struct test_area_data {
    geos::geom::Geometry* geom_;
    geos::geom::PrecisionModel pm_;
    geos::geom::GeometryFactory::Ptr factory_;
    geos::io::WKTReader reader_;
    test_area_data():
        geom_(nullptr),
        pm_(),
        factory_(GeometryFactory::create(&pm_, 0)), reader_(factory_.get())
    {
        assert(nullptr == geom_);
    }

    ~test_area_data()
    {
        factory_->destroyGeometry(geom_);
        geom_ = nullptr;
    }

    void
    checkAreaOfRing(std::string wkt, double expectedArea)
    {
        auto ringGeom = reader_.read<Curve>(wkt);

        if (const LineString* line = dynamic_cast<const LineString*>(ringGeom.get())) {
            const CoordinateSequence* ringSeq = line->getCoordinatesRO();

            std::vector<Coordinate> ringCoords;
            ringSeq->toVector(ringCoords);

            double actual1 = algorithm::Area::ofRing(ringCoords);
            double actual2 = algorithm::Area::ofRing(ringSeq);

            ensure_equals(actual1, expectedArea);
            ensure_equals(actual2, expectedArea);
        }

        double actual3 = algorithm::Area::ofClosedCurve(*ringGeom);
        ensure_equals("Area::ofClosedCurve", actual3, expectedArea, 1e-6);
    }

    void
    checkAreaOfRingSigned(std::string wkt, double expectedArea)
    {
        auto line = reader_.read<LineString>(wkt);
        ensure(nullptr != line.get());
        const CoordinateSequence* ringSeq = line->getCoordinatesRO();

        std::vector<Coordinate> ringCoords;
        ringSeq->toVector(ringCoords);

        double actual1 = algorithm::Area::ofRingSigned(ringCoords);
        double actual2 = algorithm::Area::ofRingSigned(ringSeq);

        ensure_equals(actual1, expectedArea);
        ensure_equals(actual2, expectedArea);
    }
};

typedef test_group<test_area_data> group;
typedef group::object object;

group test_area_group("geos::algorithm::Area");


//
// Test Cases
//
template<>
template<>
void object::test<1>
()
{
    checkAreaOfRing("LINESTRING (100 200, 200 200, 200 100, 100 100, 100 200)", 10000.0);
}

template<>
template<>
void object::test<2>
()
{
    checkAreaOfRingSigned("LINESTRING (100 200, 200 200, 200 100, 100 100, 100 200)", 10000.0);
}

template<>
template<>
void object::test<3>
()
{
    checkAreaOfRingSigned("LINESTRING (100 200, 100 100, 200 100, 200 200, 100 200)", -10000.0);
}

template<>
template<>
void object::test<4>
()
{
    checkAreaOfRing("CIRCULARSTRING (0 0, 2 2, 4 0, 2 -2, 0 0)", 4*MATH_PI);
}

template<>
template<>
void object::test<5>
()
{
    checkAreaOfRing("COMPOUNDCURVE (CIRCULARSTRING (0 0, 2 2, 4 0), (4 0, 0 0))", 2*MATH_PI);
}

template<>
template<>
void object::test<6>
()
{
    // expected area from PostGIS after ST_CurveToLine(geom, 1e-13, 1)
    checkAreaOfRing("CIRCULARSTRING (0 0, 2 2, 4 0, 2 1, 0 0)", 3.48759);
}

template<>
template<>
void object::test<7>
()
{
    // expected area from PostGIS after ST_CurveToLine(geom, 1e-13, 1)
    checkAreaOfRing("COMPOUNDCURVE (CIRCULARSTRING (0 0, 2 0, 2 1, 2 3, 4 3, 4 5, 1 4, 0.5 0.8, 0 0))", 11.243342);
    checkAreaOfRing("COMPOUNDCURVE (CIRCULARSTRING (0 0, 2 0, 2 1, 2 3, 4 3), (4 3, 4 5, 1 4, 0 0))", 9.321903);
}


} // namespace tut

