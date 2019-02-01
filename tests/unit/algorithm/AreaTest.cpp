//
// Test Suite for geos::algorithm::Area
// Ported from JTS junit/algorithm/AreaTest.java

#include <tut/tut.hpp>
// geos
#include <geos/algorithm/Area.h>
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
        pm_(1),
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
        std::unique_ptr<Geometry> lineGeom(reader_.read(wkt));
        std::unique_ptr<LineString> line(dynamic_cast<LineString*>(lineGeom.release()));
        ensure(nullptr != line.get());
        const CoordinateSequence* ringSeq = line->getCoordinatesRO();

        std::vector<Coordinate> ringCoords;
        ringSeq->toVector(ringCoords);

        double actual1 = algorithm::Area::ofRing(ringCoords);
        double actual2 = algorithm::Area::ofRing(ringSeq);

        ensure_equals(actual1, expectedArea);
        ensure_equals(actual2, expectedArea);
    }

    void
    checkAreaOfRingSigned(std::string wkt, double expectedArea)
    {
        std::unique_ptr<Geometry> lineGeom(reader_.read(wkt));
        std::unique_ptr<LineString> line(dynamic_cast<LineString*>(lineGeom.release()));
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


} // namespace tut

