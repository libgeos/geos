//
// Test Suite for geos::geom::LinearRing class.

// tut
#include <tut/tut.hpp>
// geos
#include <geos/geom/LinearRing.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateArraySequence.h>
#include <geos/geom/Dimension.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/io/WKTReader.h>
#include <geos/util/IllegalArgumentException.h>
// std
#include <cmath>
#include <memory>
#include <string>

namespace tut {
//
// Test Group
//

// Common data used by tests
struct test_linearring_data {
    // Typedefs used as short names by test cases
    typedef geos::geom::Geometry* GeometryPtr;
    typedef geos::geom::Geometry const* GeometryCPtr;

    typedef geos::geom::Coordinate* CoordinatePtr;
    typedef geos::geom::Coordinate const* CoordinateCPtr;

    typedef geos::geom::CoordinateArraySequence* CoordArrayPtr;
    typedef geos::geom::CoordinateArraySequence const* CoordArrayCPtr;

    typedef geos::geom::LinearRing* LinearRingPtr;
    typedef geos::geom::LinearRing const* LinearRingCPtr;

    geos::geom::PrecisionModel pm_;
    geos::geom::GeometryFactory::Ptr factory_;
    geos::io::WKTReader reader_;

    geos::geom::LinearRing empty_ring_;
    LinearRingPtr ring_;
    const size_t ring_size_;

    test_linearring_data()
        : pm_(1000), factory_(geos::geom::GeometryFactory::create(&pm_, 0))
        , reader_(factory_.get())
        , empty_ring_(new geos::geom::CoordinateArraySequence(), factory_.get()),
          ring_size_(7)
    {
        // Create non-empty LinearRing
        GeometryPtr geo = nullptr;
        geo = reader_.read("LINEARRING(0 10, 5 5, 10 5, 15 10, 10 15, 5 15, 0 10)").release();
        ring_ = dynamic_cast<LinearRingPtr>(geo);
    }

    ~test_linearring_data()
    {
        factory_->destroyGeometry(ring_);
    }

private:
    // Declare type as noncopyable
    test_linearring_data(const test_linearring_data& other) = delete;
    test_linearring_data& operator=(const test_linearring_data& rhs) = delete;
};

typedef test_group<test_linearring_data> group;
typedef group::object object;

group test_linearring_group("geos::geom::LinearRing");

//
// Test Cases
//

// Test of user's constructor to create non-empty LinearRing
template<>
template<>
void object::test<1>
()
{
    using geos::geom::Coordinate;

    // Non-empty sequence of coordiantes
    const size_t size7 = 7;
    CoordArrayPtr coords = new geos::geom::CoordinateArraySequence();
    ensure("sequence is null pointer.", coords != nullptr);

    coords->add(Coordinate(0, 10));
    coords->add(Coordinate(5, 5));
    coords->add(Coordinate(10, 5));
    coords->add(Coordinate(15, 10));
    coords->add(Coordinate(10, 15));
    coords->add(Coordinate(5, 15));
    coords->add(Coordinate(0, 10));

    ensure_equals(coords->size(), size7);

    try {
        // Create non-empty linearring instance
        geos::geom::LinearRing ring(coords, factory_.get());
        ensure(!ring.isEmpty());
        ensure(ring.isClosed());
        ensure(ring.isRing());
        ensure(ring.isSimple());
        ensure(ring.isValid());
        //ensure_equals( rint.getNumPoints(), 7u );
    }
    catch(geos::util::IllegalArgumentException const& e) {
        fail(e.what());
    }
}

// Test of copy contructor
template<>
template<>
void object::test<2>
()
{
    geos::geom::LinearRing copy(empty_ring_);

    ensure(copy.isEmpty());
}

// Test of isEmpty() for empty LinearRing
template<>
template<>
void object::test<3>
()
{
    ensure(empty_ring_.isEmpty());
}

// Test of isClosed() for empty LinearRing
template<>
template<>
void object::test<4>
()
{
    ensure(empty_ring_.isClosed());
}

// Test of isRing() for empty LinearRing
template<>
template<>
void object::test<5>
()
{
    ensure(empty_ring_.isRing());
}

// Test of isSimple() for empty LinearRing
template<>
template<>
void object::test<6>
()
{
    ensure(empty_ring_.isSimple());
}

// Test of isValid() for empty LinearRing
template<>
template<>
void object::test<7>
()
{
    ensure(empty_ring_.isValid());
}


// Test of getEnvelope() for empty LinearRing
template<>
template<>
void object::test<8>
()
{
    auto envelope = empty_ring_.getEnvelope();
    ensure(envelope != nullptr);
    ensure(envelope->isEmpty());
}

// Test of getBoundary() for empty LinearRing
template<>
template<>
void object::test<9>
()
{
    auto boundary = empty_ring_.getBoundary();
    ensure(boundary != nullptr);
    ensure(boundary->isEmpty());
}

// Test of convexHull() for empty LinearRing
template<>
template<>
void object::test<10>
()
{
    auto hull = empty_ring_.convexHull();
    ensure(hull != nullptr);
    ensure(hull->isEmpty());
}

// Test of getGeometryTypeId() for empty LinearRing
template<>
template<>
void object::test<11>
()
{
    ensure_equals(empty_ring_.getGeometryTypeId(), geos::geom::GEOS_LINEARRING);
}

// Test of getDimension() for empty LinearRing
template<>
template<>
void object::test<12>
()
{
    ensure_equals(empty_ring_.getDimension(), geos::geom::Dimension::L);
}

// Test of getBoundaryDimension() for empty LinearRing
template<>
template<>
void object::test<13>
()
{
    ensure_equals(empty_ring_.getBoundaryDimension(), geos::geom::Dimension::False);
}

// Test of getNumPoints() for empty LinearRing
template<>
template<>
void object::test<14>
()
{
    ensure_equals(empty_ring_.getNumPoints(), 0u);
}

// Test of getLength() for empty LinearRing
template<>
template<>
void object::test<15>
()
{
    ensure_equals(empty_ring_.getLength(), 0.0);
}

// Test of getArea() for empty LinearRing
template<>
template<>
void object::test<16>
()
{
    ensure_equals(empty_ring_.getArea(), 0.0);
}

// Test of isEmpty() for non-empty LinearRing
template<>
template<>
void object::test<17>
()
{
    ensure(ring_ != nullptr);
    ensure(!ring_->isEmpty());
}

// Test of isClosed() and isRing() for non-empty LinearRing
template<>
template<>
void object::test<18>
()
{
    ensure(ring_ != nullptr);
    ensure(ring_->isClosed());
    ensure(ring_->isRing());
}

// Test of getEnvelope() for non-empty LinearRing
template<>
template<>
void object::test<19>
()
{
    ensure(ring_ != nullptr);

    auto envelope = ring_->getEnvelope();
    ensure(envelope != nullptr);
    ensure(!envelope->isEmpty());
    ensure_equals(envelope->getDimension(), geos::geom::Dimension::A);
}

// Test of getBoundary() for non-empty LinearRing
template<>
template<>
void object::test<20>
()
{
    ensure(ring_ != nullptr);

    auto boundary = ring_->getBoundary();
    ensure(boundary != nullptr);

    // OGC 05-126, Version: 1.1.0, Chapter 6.1.6 Curve
    ensure("[OGC] The boundary of a closed Curve must be empty.", boundary->isEmpty());
}

// Test of convexHull() for non-empty LinearRing
template<>
template<>
void object::test<21>
()
{
    ensure(ring_ != nullptr);

    auto hull = ring_->convexHull();
    ensure(hull != nullptr);
    ensure(!hull->isEmpty());
    ensure_equals(hull->getGeometryTypeId(), geos::geom::GEOS_POLYGON);
    ensure_equals(hull->getDimension(), geos::geom::Dimension::A);
}

// Test of getGeometryTypeId() for non-empty LinearRing
template<>
template<>
void object::test<22>
()
{
    ensure(ring_ != nullptr);
    ensure_equals(ring_->getGeometryTypeId(), geos::geom::GEOS_LINEARRING);
}

// Test of getDimension() for non-empty LinearRing
template<>
template<>
void object::test<23>
()
{
    ensure(ring_ != nullptr);
    ensure_equals(ring_->getDimension(), geos::geom::Dimension::L);
}

// Test of getBoundaryDimension() for non-empty LinearRing
template<>
template<>
void object::test<24>
()
{
    ensure(ring_ != nullptr);
    ensure_equals(ring_->getBoundaryDimension(), geos::geom::Dimension::False);
}

// Test of getNumPoints() for non-empty LinearRing
template<>
template<>
void object::test<25>
()
{
    ensure(ring_ != nullptr);
    ensure_equals(ring_->getNumPoints(), ring_size_);
}

// Test of getLength() for non-empty LinearRing
template<>
template<>
void object::test<26>
()
{
    ensure(ring_ != nullptr);
    ensure(ring_->getLength() != 0.0);

    const double tolerance = 0.0001;
    const double expected = 38.284271247461902;
    const double diff = std::fabs(ring_->getLength() - expected);
    ensure(diff <= tolerance);
}

// Test of getArea() for non-empty LinearRing
template<>
template<>
void object::test<27>
()
{
    ensure(ring_ != nullptr);
    ensure_equals(ring_->getArea(), 0.0);
}

// Test of exception thrown when constructing non-empty and non-closed LinearRing
template<>
template<>
void object::test<28>
()
{
    try {
        auto geo = reader_.read("LINEARRING(0 0, 5 5, 10 10)");
        ensure(geo != nullptr);
        fail("IllegalArgumentException expected.");
    }
    catch(geos::util::IllegalArgumentException const& e) {
        const char* msg = e.what(); // ok
        ensure(msg != nullptr);
    }
}

// Test of exception thrown when constructing a self-intersecting LinearRing
template<>
template<>
void object::test<29>
()
{
    try {
        // Construct LinearRing self-intersecting in point (5,5)
        auto geo = reader_.read("LINEARRING(0 0, 5 5, 10 10, 15 5, 5 5, 0 10)");
        ensure(geo != nullptr);

        LinearRingPtr ring = dynamic_cast<LinearRingPtr>(geo.get());
        ensure(ring != nullptr);

        ensure(!ring->isValid());

        fail("IllegalArgumentException expected.");
    }
    catch(geos::util::IllegalArgumentException const& e) {
        const char* msg = e.what(); // ok
        ensure(msg != nullptr);
    }
}

// Test of getGeometryType() for non-empty LinearRing
template<>
template<>
void object::test<30>
()
{
    ensure(ring_ != nullptr);

    const std::string type("LinearRing");
    ensure_equals(ring_->getGeometryType(), type);
}

template<>
template<>
void object::test<31>
()
{
    // getCoordinate() returns nullptr for empty geometry
    auto gf = geos::geom::GeometryFactory::create();
    std::unique_ptr<geos::geom::Geometry> g(gf->createLinearRing());

    ensure(g->getCoordinate() == nullptr);
}

// test isDimensionStrict for empty LinearRing
template<>
template<>
void object::test<32>
()
{
    ensure(empty_ring_.isDimensionStrict(geos::geom::Dimension::L));
    ensure(!empty_ring_.isDimensionStrict(geos::geom::Dimension::A));
}

// test isDimensionStrict for non-empty LinearRing
template<>
template<>
void object::test<33>
()
{
    ensure(ring_->isDimensionStrict(geos::geom::Dimension::L));
    ensure(!ring_->isDimensionStrict(geos::geom::Dimension::A));
}
} // namespace tut
