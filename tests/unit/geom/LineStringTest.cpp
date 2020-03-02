//
// Test Suite for geos::geom::LineString class.

// tut
#include <tut/tut.hpp>
#include <utility.h>
// geos
#include <geos/geom/LineString.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateArraySequence.h>
#include <geos/geom/Dimension.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/io/WKTReader.h>
#include <geos/util/GEOSException.h>
#include <geos/util/IllegalArgumentException.h>
// std
#include <string>
#include <cmath>
#include <cassert>

namespace tut {
//
// Test Group
//

// Common data used by tests
struct test_linestring_data {
    // Typedefs used as short names by test cases
    typedef std::unique_ptr<geos::geom::LineString> LineStringAutoPtr;

    geos::geom::PrecisionModel pm_;
    geos::geom::GeometryFactory::Ptr factory_;
    geos::io::WKTReader reader_;

    LineStringPtr empty_line_;

    test_linestring_data()
        : pm_(1000)
        , factory_(geos::geom::GeometryFactory::create(&pm_, 0))
        , reader_(factory_.get())
        , empty_line_(factory_->createLineString(new geos::geom::CoordinateArraySequence()))
    {
        assert(nullptr != empty_line_);
    }

    ~test_linestring_data()
    {
        factory_->destroyGeometry(empty_line_);
        empty_line_ = nullptr;
    }
};

typedef test_group<test_linestring_data> group;
typedef group::object object;

group test_linestring_group("geos::geom::LineString");

//
// Test Cases
//

// Test of empty LineString created with user's constructor
template<>
template<>
void object::test<1>
()
{
    using geos::geom::Coordinate;

    // Empty sequence of coordinates
    CoordArrayPtr pseq = new geos::geom::CoordinateArraySequence();
    ensure("sequence is null pointer.", pseq != nullptr);

    // Create empty linstring instance
    LineStringAutoPtr ls(factory_->createLineString(pseq));

    ensure(ls->isEmpty());
    ensure(ls->isSimple());
    ensure(ls->isValid());
}

// Test of non-empty LineString created with user's constructor
template<>
template<>
void object::test<2>
()
{
    using geos::geom::Coordinate;

    // Non-empty sequence of coordinates
    const size_t size3 = 3;

    CoordArrayPtr pseq = new geos::geom::CoordinateArraySequence();
    ensure("sequence is null pointer.", pseq != nullptr);

    pseq->add(Coordinate(0, 0, 0));
    pseq->add(Coordinate(5, 5, 5));
    pseq->add(Coordinate(10, 10, 10));

    ensure_equals(pseq->size(), size3);

    // Create non-empty LineString instance
    LineStringAutoPtr ls(factory_->createLineString(pseq));

    ensure(!ls->isEmpty());
    ensure(ls->isSimple());
    ensure(ls->isValid());

    ensure(!ls->isClosed());
    ensure(!ls->isRing());

    std::unique_ptr<geos::geom::Geometry> geo;
    geo = ls->getEnvelope();
    ensure(geo != nullptr);
    ensure(!geo->isEmpty());

    geo = ls->getBoundary();
    ensure(geo != nullptr);
    ensure(!geo->isEmpty());

    geo = ls->convexHull();
    ensure(geo != nullptr);
    ensure(!geo->isEmpty());

    ensure_equals(ls->getGeometryTypeId(), geos::geom::GEOS_LINESTRING);
    ensure_equals(ls->getDimension(), geos::geom::Dimension::L);
    ensure_equals(ls->getBoundaryDimension(), geos::geom::Dimension::P);
    ensure_equals(ls->getCoordinateDimension(), 3);
    ensure_equals(ls->getNumPoints(), size3);
    ensure_equals(ls->getArea(), 0.0);
    ensure(ls->getLength() != 0.0);
}

// Test of incomplete LineString user's constructor throwing the IllegalArgumentException
template<>
template<>
void object::test<3>
()
{
    // Single-element sequence of coordinates
    CoordArrayPtr pseq = nullptr;
    try {
        pseq = new geos::geom::CoordinateArraySequence();
        ensure("sequence is null pointer.", pseq != nullptr);
        pseq->add(geos::geom::Coordinate(0, 0, 0));
        ensure_equals(pseq->size(), 1u);

        // Create incomplete LineString
        LineStringAutoPtr ls(factory_->createLineString(pseq));
        fail("IllegalArgumentException expected.");
    }
    catch(geos::util::IllegalArgumentException const& e) {
        // TODO - mloskot - is waiting for "exception-safety" and bugs resolution
        // If removed, memory 3 leaks occur
        //delete pseq;

        const char* msg = e.what(); // OK
        ensure(msg != nullptr);
    }
}

// Test of copy constructor
template<>
template<>
void object::test<4>
()
{
    using geos::geom::Coordinate;

    // Non-empty sequence of coordinates
    const size_t size = 3;

    CoordArrayPtr pseq = new geos::geom::CoordinateArraySequence();
    ensure("sequence is null pointer.", pseq != nullptr);

    pseq->add(Coordinate(0, 0, 0));
    pseq->add(Coordinate(5, 5, 5));
    pseq->add(Coordinate(10, 10, 10));

    ensure_equals(pseq->size(), size);

    // Create example of LineString instance
    LineStringAutoPtr examplar(factory_->createLineString(pseq));

    // Create copy
    LineStringAutoPtr copy(dynamic_cast<geos::geom::LineString*>(examplar->clone().release()));

    ensure(nullptr != copy.get());

    ensure(!copy->isEmpty());
    ensure(copy->isSimple());
    ensure(copy->isValid());

    ensure(!copy->isClosed());
    ensure(!copy->isRing());

    std::unique_ptr<geos::geom::Geometry> geo;
    geo = copy->getEnvelope();
    ensure(geo != nullptr);
    ensure(!geo->isEmpty());

    geo = copy->getBoundary();
    ensure(geo != nullptr);
    ensure(!geo->isEmpty());

    geo = copy->convexHull();
    ensure(geo != nullptr);
    ensure(!geo->isEmpty());

    ensure_equals(copy->getGeometryTypeId(), geos::geom::GEOS_LINESTRING);
    ensure_equals(copy->getDimension(), geos::geom::Dimension::L);
    ensure_equals(copy->getBoundaryDimension(), geos::geom::Dimension::P);
    ensure_equals(copy->getNumPoints(), size);
    ensure_equals(copy->getArea(), 0.0);
    ensure(copy->getLength() != 0.0);
}

// Test of isClosed() and isRing() for empty linestring
template<>
template<>
void object::test<5>
()
{
    ensure(!empty_line_->isClosed());
    ensure(!empty_line_->isRing());
}

// Test of getEnvelope() for empty linestring
template<>
template<>
void object::test<6>
()
{
    auto geo = empty_line_->getEnvelope();
    ensure(geo != nullptr);
    ensure(geo->isEmpty());
}

// Test of getBoundary() for empty linestring
template<>
template<>
void object::test<7>
()
{
    auto geo = empty_line_->getBoundary();
    ensure(geo != nullptr);
    ensure(geo->isEmpty());
}

// Test of convexHull() for empty linestring
template<>
template<>
void object::test<8>
()
{
    auto geo = empty_line_->convexHull();
    ensure(geo != nullptr);
    ensure(geo->isEmpty());
}

// Test of getGeometryTypeId() for empty linestring
template<>
template<>
void object::test<9>
()
{
    ensure_equals(empty_line_->getGeometryTypeId(), geos::geom::GEOS_LINESTRING);
}

// Test of getDimension() for empty linestring
template<>
template<>
void object::test<10>
()
{
    ensure_equals(empty_line_->getDimension(), geos::geom::Dimension::L);
}

// Test of getBoundaryDimension() for empty linestring
template<>
template<>
void object::test<11>
()
{
    ensure_equals(empty_line_->getBoundaryDimension(), geos::geom::Dimension::P);
}

// Test of getNumPoints() for empty linestring
template<>
template<>
void object::test<12>
()
{
    ensure_equals(empty_line_->getNumPoints(), 0u);
}

// Test of getLength() for empty linestring
template<>
template<>
void object::test<13>
()
{
    ensure_equals(empty_line_->getLength(), 0.0);
}

// Test of getArea() for empty linestring
template<>
template<>
void object::test<14>
()
{
    ensure_equals(empty_line_->getArea(), 0.0);
}

// Test of isClosed() and isRing() for non-empty linestring
template<>
template<>
void object::test<15>
()
{
    auto geo = reader_.read("LINESTRING (0 0, 5 5, 10 5, 10 10)");
    ensure(geo != nullptr);

    LineStringPtr line = dynamic_cast<LineStringPtr>(geo.get());
    ensure(line != nullptr);

    ensure(!line->isEmpty());
    ensure(!line->isClosed());
    ensure(!line->isRing());
    ensure(line->getCoordinateDimension() == 2);
}

// Test of getEnvelope() for non-empty linestring
template<>
template<>
void object::test<16>
()
{
    auto geo = reader_.read("LINESTRING (0 0, 5 5, 10 5, 10 10)");
    ensure(geo != nullptr);

    LineStringPtr line = dynamic_cast<LineStringPtr>(geo.get());
    ensure(line != nullptr);

    auto envelope = line->getEnvelope();
    ensure(envelope != nullptr);
    ensure(!envelope->isEmpty());
    ensure_equals(envelope->getDimension(), geos::geom::Dimension::A);
}

// Test of getBoundary() for non-empty linestring
template<>
template<>
void object::test<17>
()
{
    auto geo = reader_.read("LINESTRING (0 0, 5 5, 10 5, 10 10)");
    ensure(geo != nullptr);

    LineStringPtr line = dynamic_cast<LineStringPtr>(geo.get());
    ensure(line != nullptr);

    auto boundary = line->getBoundary();
    ensure(boundary != nullptr);
    ensure(!boundary->isEmpty());
    ensure_equals(boundary->getGeometryTypeId(), geos::geom::GEOS_MULTIPOINT);
    ensure_equals(boundary->getDimension(), geos::geom::Dimension::P);
}

// Test of convexHull() for non-empty linestring
template<>
template<>
void object::test<18>
()
{
    auto geo = reader_.read("LINESTRING (0 0, 5 5, 10 5, 10 10)");
    ensure(geo != nullptr);

    LineStringPtr line = dynamic_cast<LineStringPtr>(geo.get());
    ensure(line != nullptr);

    auto hull = line->convexHull();
    ensure(hull != nullptr);
    ensure(!hull->isEmpty());
    ensure_equals(hull->getGeometryTypeId(), geos::geom::GEOS_POLYGON);
    ensure_equals(hull->getDimension(), geos::geom::Dimension::A);
}

// Test of getGeometryTypeId() for non-empty linestring
template<>
template<>
void object::test<19>
()
{
    auto geo = reader_.read("LINESTRING (0 0, 5 5, 10 5, 10 10)");
    ensure(geo != nullptr);

    ensure_equals(geo->getGeometryTypeId(), geos::geom::GEOS_LINESTRING);
}

// Test of getDimension() for non-empty linestring
template<>
template<>
void object::test<20>
()
{
    auto geo = reader_.read("LINESTRING (0 0, 5 5, 10 5, 10 10)");
    ensure(geo != nullptr);

    ensure_equals(geo->getDimension(), geos::geom::Dimension::L);
}

// Test of getBoundaryDimension() for non-empty linestring
template<>
template<>
void object::test<21>
()
{
    auto geo = reader_.read("LINESTRING (0 0, 5 5, 10 5, 10 10)");
    ensure(geo != nullptr);

    ensure_equals(geo->getBoundaryDimension(), geos::geom::Dimension::P);
}

// Test of getNumPoints() for non-empty linestring
template<>
template<>
void object::test<22>
()
{
    const size_t size = 4;
    auto geo = reader_.read("LINESTRING (0 0, 5 5, 10 5, 10 10)");
    ensure(geo != nullptr);

    ensure_equals(geo->getNumPoints(), size);
}

// Test of getLength() for non-empty linestring
template<>
template<>
void object::test<23>
()
{
    const double tolerance = 0.0001;
    const double expected = 2 * 14.142135600000000;
    auto geo = reader_.read("LINESTRING (0 0, 10 10, 20 0)");
    ensure(geo != nullptr);

    ensure(geo->getLength() != 0.0);

    const double diff = std::fabs(geo->getLength() - expected);
    ensure(diff <= tolerance);
}

// Test of getArea() for non-empty linestring
template<>
template<>
void object::test<24>
()
{
    auto geo = reader_.read("LINESTRING (0 0, 10 10, 20 0)");
    ensure(geo != nullptr);

    ensure_equals(geo->getArea(), 0.0);
}

// Test of getGeometryType() for non-empty LineString
template<>
template<>
void object::test<25>
()
{
    auto geo = reader_.read("LINESTRING (0 0, 10 10, 20 0)");
    ensure(geo != nullptr);

    const std::string type("LineString");
    ensure_equals(geo->getGeometryType(), type);
}

template<>
template<>
void object::test<26>
()
{
    // getCoordinate() returns nullptr for empty geometry
    auto gf = geos::geom::GeometryFactory::create();
    std::unique_ptr<geos::geom::Geometry> g(gf->createLineString());

    ensure(g->getCoordinate() == nullptr);
}

// test isDimensionStrict for empty LineString
template<>
template<>
void object::test<27>
()
{
    ensure(empty_line_->isDimensionStrict(geos::geom::Dimension::L));
    ensure(!empty_line_->isDimensionStrict(geos::geom::Dimension::A));
}

// test isDimensionStrict for non-empty LineString
template<>
template<>
void object::test<28>
()
{
    std::unique_ptr<geos::geom::Geometry> geo(reader_.read("LINESTRING (0 0, 10 10, 20 0))"));

    ensure(geo->isDimensionStrict(geos::geom::Dimension::L));
    ensure(!geo->isDimensionStrict(geos::geom::Dimension::A));
}

// test dynamic_cast for LineString (shows that vtable is created)
// https://github.com/libgeos/geos/issues/285
template<>
template<>
void object::test<29>
()
{
    std::unique_ptr<geos::geom::LineString> a = geos::geom::GeometryFactory::getDefaultInstance()->createLineString();
    geos::geom::Geometry *b = a.get(); // ok
    geos::geom::LineString *c = dynamic_cast<geos::geom::LineString *>(b);

    ensure(c != nullptr);
}

} // namespace tut

