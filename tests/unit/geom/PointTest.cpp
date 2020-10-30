//
// Test Suite for geos::geom::Point class.

#include <tut/tut.hpp>
// geos
#include <geos/geom/Point.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateArraySequence.h>
#include <geos/geom/Dimension.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/io/WKTReader.h>
#include <geos/util/IllegalArgumentException.h>
// std
#include <memory>
#include <string>


namespace tut {
//
// Test Group
//

// Common data used by tests
struct test_point_data {
    typedef geos::geom::Coordinate* CoordinatePtr;
    typedef geos::geom::Coordinate const* CoordinateCPtr;

    typedef geos::geom::Geometry* GeometryPtr;
    typedef std::unique_ptr<geos::geom::Geometry> GeometryAutoPtr;
    typedef geos::geom::Geometry const* GeometryCPtr;

    typedef geos::geom::Point* PointPtr;
    typedef std::unique_ptr<geos::geom::Point> PointAutoPtr;
    typedef geos::geom::Point const* PointCPtr;
    typedef geos::geom::GeometryFactory GeometryFactory;

    geos::geom::PrecisionModel pm_;
    GeometryFactory::Ptr factory_;
    geos::io::WKTReader reader_;
    PointAutoPtr empty_point_;
    PointPtr point_;

    test_point_data()
        : pm_(1000), factory_(GeometryFactory::create(&pm_, 0))
        , reader_(factory_.get()), empty_point_(factory_->createPoint())
    {
        // Create non-empty Point
        auto geo = reader_.read("POINT(1.234 5.678)");
        point_ = dynamic_cast<PointPtr>(geo.release());
    }

    ~test_point_data()
    {
        factory_->destroyGeometry(point_);
    }
};

typedef test_group<test_point_data> group;
typedef group::object object;

group test_point_group("geos::geom::Point");

//
// Test Cases
//

// Test of user's constructor to build empty Point
template<>
template<>
void object::test<1>
()
{
    PointAutoPtr point(factory_->createPoint());
    ensure(point->isEmpty());
}

// Test of user's constructor to build non-empty Point
template<>
template<>
void object::test<2>
()
{
    using geos::geom::Coordinate;
    using geos::geom::CoordinateArraySequence;

    CoordinateArraySequence* coords = new CoordinateArraySequence();
    ensure(coords != nullptr);
    coords->add(Coordinate(1.234, 5.678));

    PointAutoPtr point(factory_->createPoint(coords));
    ensure(!point->isEmpty());

    // currently the empty CoordinateArraySequence constructor
    // produces a dimension 0 sequence. The dimension is then
    // autodetected when the first point is inserted.
    ensure(point->getCoordinateDimension() == 2);
}

// Test of user's constructor throwing IllegalArgumentException
template<>
template<>
void object::test<3>
()
{
    using geos::geom::Coordinate;
    using geos::geom::CoordinateArraySequence;

    // TODO - mloskot - temporary solution of Bug #89
    CoordinateArraySequence* coords = nullptr;
    try {
        coords = new CoordinateArraySequence();
        ensure(coords != nullptr);
        coords->add(Coordinate(1.234, 5.678));
        coords->add(Coordinate(4.321, 8.765));

        PointAutoPtr point(factory_->createPoint(coords));

        fail("IllegalArgumentException expected.");
    }
    catch(geos::util::IllegalArgumentException const& e) {
        // TODO - mloskot - Bug #89: Possible memory leaks caused by Point constructor
        //delete coords;

        const char* msg = e.what(); // ok
        ensure(msg != nullptr);
    }
}

// Test of copy constructor
template<>
template<>
void object::test<4>
()
{
    GeometryAutoPtr copy(empty_point_->clone());
    ensure(copy->isEmpty());
}

// Test of isEmpty() for empty Point
template<>
template<>
void object::test<5>
()
{
    ensure(empty_point_->isEmpty());
}

// Test of isSimple() for empty Point
template<>
template<>
void object::test<6>
()
{
    ensure(empty_point_->isSimple());
}

// Test of isValid() for empty Point
template<>
template<>
void object::test<7>
()
{
    ensure(empty_point_->isValid());
}

// Test of getEnvelope() for empty Point
template<>
template<>
void object::test<8>
()
{
    auto envelope = empty_point_->getEnvelope();
    ensure(envelope != nullptr);
    ensure(envelope->isEmpty());
}

// Test of getBoundary() for empty Point
template<>
template<>
void object::test<9>
()
{
    auto boundary = empty_point_->getBoundary();
    ensure(boundary != nullptr);
    ensure(boundary->isEmpty());
}

// Test of convexHull() for empty Point
template<>
template<>
void object::test<10>
()
{
    auto hull = empty_point_->convexHull();
    ensure(hull != nullptr);
    ensure(hull->isEmpty());
}

// Test of getGeometryTypeId() for empty Point
template<>
template<>
void object::test<11>
()
{
    ensure_equals(empty_point_->getGeometryTypeId(), geos::geom::GEOS_POINT);
}

// Test of getGeometryType() for empty Polygon
template<>
template<>
void object::test<12>
()
{
    const std::string type("Point");
    ensure_equals(empty_point_->getGeometryType(), type);
}

// Test of getDimension() for empty Point
template<>
template<>
void object::test<13>
()
{
    ensure_equals(empty_point_->getDimension(), geos::geom::Dimension::P);
}

// Test of getBoundaryDimension() for empty Point
template<>
template<>
void object::test<14>
()
{
    ensure_equals(empty_point_->getBoundaryDimension(), geos::geom::Dimension::False);
}

// Test of getNumPoints() for empty Point
template<>
template<>
void object::test<15>
()
{
    ensure_equals(empty_point_->getNumPoints(), (size_t)0);
}

// Test of getLength() for empty Point
template<>
template<>
void object::test<16>
()
{
    ensure_equals(empty_point_->getLength(), 0);
}

// Test of getArea() for empty Point
template<>
template<>
void object::test<17>
()
{
    ensure_equals(empty_point_->getArea(), 0);
}

// Test of isEmpty() for non-empty Point
template<>
template<>
void object::test<18>
()
{
    ensure(!point_->isEmpty());
}

// Test of isSimple() for non-empty Point
template<>
template<>
void object::test<19>
()
{
    ensure(point_->isSimple());
}

// Test of isValid() for non-empty Point
template<>
template<>
void object::test<20>
()
{
    ensure(point_->isValid());
}

// Test of getEnvelope() for non-empty Point
template<>
template<>
void object::test<21>
()
{
    auto envelope = point_->getEnvelope();
    ensure(envelope != nullptr);
    ensure(!envelope->isEmpty());
}

// Test of getBoundary() for non-empty Point
template<>
template<>
void object::test<22>
()
{
    auto boundary = point_->getBoundary();
    ensure(boundary != nullptr);
    ensure(boundary->isEmpty());
}

// Test of convexHull() for non-empty Point
template<>
template<>
void object::test<23>
()
{
    auto hull = point_->convexHull();
    ensure(hull != nullptr);
    ensure(!hull->isEmpty());
}

// Test of getGeometryTypeId() for non-empty Point
template<>
template<>
void object::test<24>
()
{
    ensure_equals(point_->getGeometryTypeId(), geos::geom::GEOS_POINT);
}

// Test of getGeometryType() for non-empty Polygon
template<>
template<>
void object::test<25>
()
{
    const std::string type("Point");
    ensure_equals(point_->getGeometryType(), type);
}

// Test of getDimension() for non-empty Point
template<>
template<>
void object::test<26>
()
{
    ensure_equals(point_->getDimension(), geos::geom::Dimension::P);
}

// Test of getBoundaryDimension() for non-empty Point
template<>
template<>
void object::test<27>
()
{
    ensure_equals(empty_point_->getBoundaryDimension(), geos::geom::Dimension::False);
}

// Test of getNumPoints() for non-empty Point
template<>
template<>
void object::test<28>
()
{
    ensure_equals(point_->getNumPoints(), (size_t)1);
}

// Test of getLength() for non-empty Point
template<>
template<>
void object::test<29>
()
{
    ensure_equals(point_->getLength(), 0);
}

// Test of getArea() for non-empty Point
template<>
template<>
void object::test<30>
()
{
    ensure_equals(point_->getArea(), 0);
}

// Test of equals() for empty Point
template<>
template<>
void object::test<31>
()
{
    GeometryAutoPtr geo(empty_point_->clone());

    ensure(empty_point_->equals(geo.get()));
}

// Test of equals() for non-empty Point (1.234,5.678)
template<>
template<>
void object::test<32>
()
{
    auto p1 = reader_.read("POINT(1.234 5.678)");
    auto p2 = reader_.read("POINT(1.234 5.678)");

    ensure(p1->equals(p2.get()));
}

// Test of equals() for non-empty Point (1.23 5.67)
template<>
template<>
void object::test<33>
()
{
    auto p1 = reader_.read("POINT(1.23 5.67)");
    auto p2 = reader_.read("POINT(1.23 5.67)");

    ensure(p1->equals(p2.get()));
}

// Test of equals() for non-empty Points (1.235 5.678) and (1.234 5.678)
template<>
template<>
void object::test<34>
()
{
    auto p1 = reader_.read("POINT(1.235 5.678)");
    auto p2 = reader_.read("POINT(1.234 5.678)");

    ensure(!p1->equals(p2.get()));
}

// Test of equals() for non-empty Points (1.2334 5.678) and (1.2333 5.678)
template<>
template<>
void object::test<35>
()
{
    auto p1 = reader_.read("POINT(1.2334 5.678)");
    auto p2 = reader_.read("POINT(1.2333 5.678)");

    ensure(p1->equals(p2.get()));
}

// Test of equals() for non-empty Points (1.2334 5.678) and (1.2335 5.678)
template<>
template<>
void object::test<36>
()
{
    auto p1 = reader_.read("POINT(1.2334 5.678)");
    auto p2 = reader_.read("POINT(1.2335 5.678)");

    ensure(!p1->equals(p2.get()));
}

// Test of equals() for non-empty Points (1.2324 5.678) and (1.2325 5.678)
template<>
template<>
void object::test<37>
()
{
    auto p1 = reader_.read("POINT(1.2324 5.678)");
    auto p2 = reader_.read("POINT(1.2325 5.678)");

    ensure(!p1->equals(p2.get()));
}

// Test of equals() for non-empty Points (1.2324 5.678) and (EMPTY)
template<>
template<>
void object::test<38>
()
{
    auto p1 = reader_.read("POINT(1.2324 5.678)");
    auto p2 = reader_.read("POINT EMPTY");

    ensure(!p1->equals(p2.get()));
}

// Test of equals() for non-empty Points with negative coordiantes
template<>
template<>
void object::test<39>
()
{
    auto pLo = reader_.read("POINT(-1.233 5.678)");
    auto pHi = reader_.read("POINT(-1.232 5.678)");

    auto p1 = reader_.read("POINT(-1.2326 5.678)");
    auto p2 = reader_.read("POINT(-1.2325 5.678)");
    auto p3 = reader_.read("POINT(-1.2324 5.678)");

    ensure(!p1->equals(p2.get()));
    ensure(p3->equals(p2.get()));

    ensure(p1->equals(pLo.get()));
    ensure(p2->equals(pHi.get()));
    ensure(p3->equals(pHi.get()));
}

// Test of getCoordinateDimension() for 2d/3d.
template<>
template<>
void object::test<40>
()
{
    auto p = reader_.read("POINT(-1.233 5.678 1.0)");

    ensure(p->getCoordinateDimension() == 3);

    p = reader_.read("POINT(-1.233 5.678)");

    ensure(p->getCoordinateDimension() == 2);
}

template<>
template<>
void object::test<41>
()
{
    // getCoordinate() returns nullptr for empty geometry
    auto gf = geos::geom::GeometryFactory::create();
    std::unique_ptr<geos::geom::Geometry> g(gf->createPoint());

    ensure(g->getCoordinate() == nullptr);
}

// test isDimensionStrict for empty Point
template<>
template<>
void object::test<42>
()
{
    ensure(empty_point_->isDimensionStrict(geos::geom::Dimension::P));
    ensure(!empty_point_->isDimensionStrict(geos::geom::Dimension::A));
}

// test isDimensionStrict for non-empty Point
template<>
template<>
void object::test<43>
()
{
    ensure(point_->isDimensionStrict(geos::geom::Dimension::P));
    ensure(!point_->isDimensionStrict(geos::geom::Dimension::A));
}

// empty point has size-0 coordinate sequence
template<>
template<>
void object::test<44>
()
{
    ensure_equals(empty_point_->getCoordinates()->getSize(), 0u);
    ensure_equals(empty_point_->getCoordinatesRO()->getSize(), 0u);
}

// check dimensionality of empty points
template<>
template<>
void object::test<45>
()
{
    auto empty3d = factory_->createPoint();
    ensure_equals(empty3d->getCoordinateDimension(), 2);

    geos::geom::FixedSizeCoordinateSequence<0> seq2(2);
    ensure_equals(seq2.getDimension(), 2u);
    std::unique_ptr<geos::geom::Point> pt2(factory_->createPoint(seq2));
    ensure_equals(pt2->getCoordinateDimension(), 2);

    geos::geom::FixedSizeCoordinateSequence<0> seq3(3);
    ensure_equals(seq3.getDimension(), 3u);
    std::unique_ptr<geos::geom::Point> pt3(factory_->createPoint(seq3));
    ensure_equals(pt3->getCoordinateDimension(), 3);
}

} // namespace tut

