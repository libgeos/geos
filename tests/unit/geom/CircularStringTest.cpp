#include <tut/tut.hpp>
#include <tut/tut_macros.hpp>

#include <geos/geom/CircularString.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/IntersectionMatrix.h>
#include <geos/io/WKTReader.h>
#include <geos/util/UnsupportedOperationException.h>

#include "utility.h"

using geos::geom::CoordinateSequence;
using geos::geom::CircularString;
using XY = geos::geom::CoordinateXY;

namespace tut {
// Common data used by tests
struct test_circularstring_data {

    geos::geom::GeometryFactory::Ptr factory_ = geos::geom::GeometryFactory::create();
    geos::io::WKTReader wktreader_;

    std::unique_ptr<CircularString> cs_;

    test_circularstring_data()
    {
        CoordinateSequence seq{
            XY(0, 0),
            XY(1, 1),
            XY(2, 0),
            XY(3, -1),
            XY(4, 0)
        };

        cs_ = factory_->createCircularString(seq);
    }
};

typedef test_group<test_circularstring_data> group;
typedef group::object object;

group test_circularstring_group("geos::geom::CircularString");

template<>
template<>
void object::test<1>()
{

    auto cs = factory_->createCircularString(false, false);

    ensure(cs->isEmpty());
    ensure_equals(cs->getNumPoints(), 0u);
    ensure(!cs->hasZ());
    ensure(!cs->hasM());
    ensure_equals(cs->getCoordinateDimension(), 2u);

    ensure(cs->getCoordinatesRO()->isEmpty());
    ensure(cs->getCoordinates()->isEmpty());
    ensure(cs->getCoordinate() == nullptr);

    ensure_equals(cs->getArea(), 0);
    ensure_equals(cs->getLength(), 0);
}

// Basic Geometry API
template<>
template<>
void object::test<2>()
{
    // Geometry type functions
    ensure_equals("getGeometryType", cs_->getGeometryType(), "CircularString");
    ensure_equals("getGeometryTypdId", cs_->getGeometryTypeId(), geos::geom::GEOS_CIRCULARSTRING);
    ensure("isCollection", !cs_->isCollection());

    // Geometry size functions
    ensure("isEmpty", !cs_->isEmpty());
    ensure_equals("getArea", cs_->getArea(), 0);
    ensure_equals("getLength", cs_->getLength(), 2*geos::MATH_PI);
    ensure_equals("getNumGeometries", cs_->getNumGeometries(), 1u);
    ensure_equals("getNumPoints", cs_->getNumPoints(), 5u);
    geos::geom::Envelope expected(0, 4, -1, 1);
    ensure("getEnvelopeInternal", cs_->getEnvelopeInternal()->equals(&expected));

    // Geometry dimension functions
    ensure_equals("getDimension", cs_->getDimension(), geos::geom::Dimension::L);
    ensure("isLineal", cs_->isLineal());
    ensure("isPuntal", !cs_->isPuntal());
    ensure("isPolygonal", !cs_->isPolygonal());
    ensure("hasDimension(L)", cs_->hasDimension(geos::geom::Dimension::L));
    ensure("hasDimension(P)", !cs_->hasDimension(geos::geom::Dimension::P));
    ensure("hasDimension(A)", !cs_->hasDimension(geos::geom::Dimension::A));
    ensure("isDimensionStrict", cs_->isDimensionStrict(geos::geom::Dimension::L));
    ensure("isMixedDimension", !cs_->isMixedDimension());
    ensure_equals("getBoundaryDimension", cs_->getBoundaryDimension(), geos::geom::Dimension::P);

    // Coordinate dimension functions
    ensure("hasZ", !cs_->hasZ());
    ensure("hasM", !cs_->hasM());
    ensure_equals("getCoordinateDimension", cs_->getCoordinateDimension(), 2u);

    // Coordinate access functions
    ensure("getCoordinates", cs_->getCoordinates()->getSize() == 5u);
    ensure_equals("getCoordinate", *cs_->getCoordinate(), XY(0, 0));
}

// Operations
template<>
template<>
void object::test<3>()
{
    // Predicates
    ensure_THROW(cs_->contains(cs_.get()), geos::util::UnsupportedOperationException);
    ensure_THROW(cs_->coveredBy(cs_.get()), geos::util::UnsupportedOperationException);
    ensure_THROW(cs_->covers(cs_.get()), geos::util::UnsupportedOperationException);
    ensure_THROW(cs_->crosses(cs_.get()), geos::util::UnsupportedOperationException);
    ensure_THROW(cs_->disjoint(cs_.get()), geos::util::UnsupportedOperationException);
    ensure_THROW(cs_->equals(cs_.get()), geos::util::UnsupportedOperationException);
    ensure_THROW(cs_->intersects(cs_.get()), geos::util::UnsupportedOperationException);
    ensure_THROW(cs_->overlaps(cs_.get()), geos::util::UnsupportedOperationException);
    ensure_THROW(cs_->relate(cs_.get()), geos::util::UnsupportedOperationException);
    ensure_THROW(cs_->touches(cs_.get()), geos::util::UnsupportedOperationException);
    ensure_THROW(cs_->within(cs_.get()), geos::util::UnsupportedOperationException);

    auto cs2 = cs_->clone();

    ensure("equalsExact", cs_->equalsExact(cs2.get()));
    ensure("equalsIdentical", cs_->equalsIdentical(cs2.get()));

    // Overlay
    ensure_THROW(cs_->Union(), geos::util::UnsupportedOperationException);
    ensure_THROW(cs_->Union(cs_.get()), geos::util::UnsupportedOperationException);
    ensure_THROW(cs_->difference(cs_.get()), geos::util::UnsupportedOperationException);
    ensure_THROW(cs_->intersection(cs_.get()), geos::util::UnsupportedOperationException);
    ensure_THROW(cs_->symDifference(cs_.get()), geos::util::UnsupportedOperationException);

    // Distance
    ensure_THROW(cs_->distance(cs_.get()), geos::util::UnsupportedOperationException);
    ensure_THROW(cs_->isWithinDistance(cs_.get(), 1), geos::util::UnsupportedOperationException);

    // Valid / Simple
    ensure_THROW(cs_->isSimple(), geos::util::UnsupportedOperationException);
    ensure_THROW(cs_->isValid(), geos::util::UnsupportedOperationException);

    // Operations
    ensure_THROW(cs_->convexHull(), geos::util::UnsupportedOperationException);
    ensure_THROW(cs_->buffer(1), geos::util::UnsupportedOperationException);

    ensure_THROW(cs_->getCentroid(), geos::util::UnsupportedOperationException);

    //auto expected_boundary = wktreader_.read("MULTIPOINT ((0 0), (1 1), (2 0), (3 -1), (4 0))");
    //ensure("getBoundary", cs_->getBoundary()->equalsIdentical(expected_boundary.get()));
    ensure_THROW(cs_->getBoundary(), geos::util::UnsupportedOperationException);

    ensure("clone", cs_->equalsIdentical(cs_->clone().get()));

    ensure("reverse", cs_->reverse()->equalsIdentical(wktreader_.read("CIRCULARSTRING (4 0, 3 -1, 2 0, 1 1, 0 0)").get()));

    auto cs3 = cs_->reverse();
    ensure_THROW(cs3->normalize(), geos::util::UnsupportedOperationException);
}

// SimpleCurve API
template<>
template<>
void object::test<4>()
{
    ensure("getCoordinateN", cs_->getCoordinateN(3).equals(XY(3, -1)));
    ensure("getPointN", cs_->getPointN(1)->equalsIdentical(wktreader_.read("POINT (1 1)").get()));

    ensure("getStartPoint", cs_->getStartPoint()->equalsIdentical(wktreader_.read("POINT (0 0)").get()));
    ensure("getEndPoint", cs_->getEndPoint()->equalsIdentical(wktreader_.read("POINT (4 0)").get()));

    ensure("getCoordinatesRO", cs_->getCoordinatesRO()->getSize() == 5u);
    ensure("isClosed", !cs_->isClosed());
    XY pt(4, 0);
    ensure("isCoordinate", cs_->isCoordinate(pt));
}

template<>
template<>
void object::test<5>()
{
    set_test_name("invalid number of points");

    auto pts = std::make_shared<CoordinateSequence>();
    ensure_NO_THROW(factory_->createCircularString(pts));

    pts->add(0.0, 0.0);
    ensure_THROW(factory_->createCircularString(pts), geos::util::GEOSException);

    pts->add(1.0, 1.0);
    ensure_THROW(factory_->createCircularString(pts), geos::util::GEOSException);

    pts->add(2.0, 0.0);
    ensure_NO_THROW(factory_->createCircularString(pts));

    pts->add(3.0, -1.0);
    ensure_THROW(factory_->createCircularString(pts), geos::util::GEOSException);
}

}
