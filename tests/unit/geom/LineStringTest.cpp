//
// Test Suite for geos::geom::LineString class.

// tut
#include <tut/tut.hpp>
#include <utility.h>
// geos
#include <geos/geom/LineString.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateFilter.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/Dimension.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/io/WKTReader.h>
#include <geos/util/GEOSException.h>
#include <geos/util/IllegalArgumentException.h>
#include <geos/constants.h>
#include <geos/util.h>
// std
#include <string>
#include <cmath>
#include <cassert>

using geos::geom::CoordinateXY;

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
    geos::io::WKTWriter writer_;

    std::unique_ptr<geos::geom::LineString> empty_line_;
    std::unique_ptr<geos::geom::LineString> line_;

    test_linestring_data()
        : pm_(1000)
        , factory_(geos::geom::GeometryFactory::create(&pm_, 0))
        , reader_(factory_.get())
        , empty_line_(factory_->createLineString())
    {
        assert(nullptr != empty_line_);

        auto cs = geos::detail::make_unique<geos::geom::CoordinateSequence>(2u, false, false);
        cs->setAt(geos::geom::Coordinate{0, 0}, 0);
        cs->setAt(geos::geom::Coordinate{1, 1}, 1);
        line_ = factory_->createLineString(std::move(cs));
    }

    ~test_linestring_data()
    {
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
    auto pseq = geos::detail::make_unique<geos::geom::CoordinateSequence>();
    ensure("sequence is null pointer.", pseq != nullptr);

    // Create empty linstring instance
    auto ls = factory_->createLineString(std::move(pseq));

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
    const std::size_t size3 = 3;

    auto pseq = geos::detail::make_unique<geos::geom::CoordinateSequence>();
    ensure("sequence is null pointer.", pseq != nullptr);

    pseq->add(Coordinate(0, 0, 0));
    pseq->add(Coordinate(5, 5, 5));
    pseq->add(Coordinate(10, 10, 10));

    ensure_equals(pseq->size(), size3);

    // Create non-empty LineString instance
    auto ls = factory_->createLineString(std::move(pseq));

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
    try {
        auto pseq = geos::detail::make_unique<CoordinateSequence>();
        ensure("sequence is null pointer.", pseq != nullptr);
        pseq->add(geos::geom::Coordinate(0, 0, 0));
        ensure_equals(pseq->size(), 1u);

        // Create incomplete LineString
        auto ls= factory_->createLineString(std::move(pseq));
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
    const std::size_t size = 3;

    auto pseq = geos::detail::make_unique<CoordinateSequence>();
    ensure("sequence is null pointer.", pseq != nullptr);

    pseq->add(Coordinate(0, 0, 0));
    pseq->add(Coordinate(5, 5, 5));
    pseq->add(Coordinate(10, 10, 10));

    ensure_equals(pseq->size(), size);

    // Create example of LineString instance
    auto examplar = factory_->createLineString(std::move(pseq));

    // Create copy
    auto copy = examplar->clone();

    ensure(nullptr != copy);

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
    const std::size_t size = 4;
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
    std::unique_ptr<geos::geom::Geometry> geo(reader_.read("LINESTRING (0 0, 10 10, 20 0)"));

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

// getSharedCoordinates
template<>
template<>
void object::test<30>()
{
    auto ls = reader_.read<geos::geom::LineString>("LINESTRING (0 0, 10 10)");
    auto env = ls->getEnvelopeInternal();
    ensure_equals(*env, geos::geom::Envelope(0,10, 0, 10));

    auto cs = ls->getSharedCoordinates();
    ensure_equals(cs->getSize(), 2u);

    struct AddFive : public geos::geom::CoordinateFilter {
        void filter_rw(CoordinateXY* pt) const override {
            pt->x += 5;
            pt->y += 5;
        }
    };

    AddFive filter;
    ls->apply_rw(&filter);

    ensure_equals(cs->getAt<CoordinateXY>(0), CoordinateXY(0, 0));
    ensure_equals(cs->getAt<CoordinateXY>(1), CoordinateXY(10, 10));
}

// Test of LinearRing constructor with a NaN coordinate
template<>
template<>
void object::test<31>
()
{
    using geos::geom::Coordinate;

    // Non-empty sequence of coordinates
    const std::size_t size3 = 3;

    auto pseq = geos::detail::make_unique<CoordinateSequence>();
    ensure("sequence is null pointer.", pseq != nullptr);

    pseq->add(Coordinate(0, geos::DoubleNotANumber));
    pseq->add(Coordinate(5, 5));
    pseq->add(Coordinate(0, geos::DoubleNotANumber));
    ensure_equals(pseq->size(), size3);

    try {
        // Create non-empty LineString instance
        auto lr(factory_->createLinearRing(std::move(pseq)));
        ensure(!lr->isEmpty());
        fail("IllegalArgumentException expected.");
    }
    catch(geos::util::IllegalArgumentException const& e) {
        const char* msg = e.what(); // OK
        ensure(msg != nullptr);
    }

}

// Test of hasDimension()
template<>
template<>
void object::test<32>
()
{
    ensure(!line_->hasDimension(geos::geom::Dimension::P));
    ensure(line_->hasDimension(geos::geom::Dimension::L));
    ensure(!line_->hasDimension(geos::geom::Dimension::A));
}

// https://github.com/libgeos/geos/issues/1191
// line->getPoint(n) loses M dimension
template<>
template<>
void object::test<33>
()
{
    auto line = reader_.read<LineString>("LINESTRING M (0 1 2, 10 11 12, 20 21 22)");
    ensure_equals(line->getCoordinateDimension(), 3);
    auto pt = line->getPointN(2);
    auto out = writer_.write(*pt);
    ensure_equals(out, "POINT M (20 21 22)");
}

template<>
template<>
void object::test<34>
()
{
    auto line = reader_.read<LineString>("LINESTRING Z (0 1 2, 10 11 12, 20 21 22)");
    ensure_equals(line->getCoordinateDimension(), 3);
    auto pt = line->getPointN(2);
    auto out = writer_.write(*pt);
    ensure_equals(out, "POINT Z (20 21 22)");
}

template<>
template<>
void object::test<35>
()
{
    auto line = reader_.read<LineString>("LINESTRING ZM (0 1 2 3, 10 11 12 13, 20 21 22 23)");
    ensure_equals(line->getCoordinateDimension(), 4);
    auto pt = line->getPointN(2);
    auto out = writer_.write(*pt);
    ensure_equals(out, "POINT ZM (20 21 22 23)");
}

template<>
template<>
void object::test<36>()
{
    set_test_name("getLinearized");

    // check that we return LineString* rather than Curve* or Geometry*
    std::unique_ptr<LineString> linearized = line_->getLinearized(1e-3);

    ensure_equals_exact_geometry_xyzm(linearized.get(), line_.get(), 0);
}

template<>
template<>
void object::test<37>()
{
    set_test_name("getCurved");

    WKTReader reader;

    auto input = reader.read<LineString>("LINESTRING (2 2, 2.292893 2.707107, 3 3, 3.707107 2.707107, 4 2, 2 2)");

    // check that we return Curve* instead of Geometry*
    std::unique_ptr<geos::geom::Curve> curved = input->getCurved(1e-3);

    ensure_equals(curved->getGeometryTypeId(), geos::geom::GEOS_COMPOUNDCURVE);

    auto expected = reader.read("COMPOUNDCURVE (CIRCULARSTRING (2 2, 3 3, 4 2), (4 2, 2 2))");

    ensure_equals_exact_geometry_xyzm(curved.get(), expected.get(), 1e-6);
}


} // namespace tut

