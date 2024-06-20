#include <tut/tut.hpp>
#include <tut/tut_macros.hpp>

#include <geos/geom/CircularString.h>
#include <geos/geom/CurvePolygon.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/MultiSurface.h>
#include <geos/geom/IntersectionMatrix.h>
#include <geos/io/WKTReader.h>
#include <geos/util/UnsupportedOperationException.h>

using geos::geom::CoordinateXY;

namespace tut {

// Common data used by tests
struct test_multisurface_data {

    geos::geom::GeometryFactory::Ptr factory_ = geos::geom::GeometryFactory::create();
    geos::io::WKTReader wktreader_;

    std::unique_ptr<geos::geom::MultiSurface> ms_;

    test_multisurface_data() {
        std::vector<std::unique_ptr<geos::geom::Surface>> surfaces;

        surfaces.emplace_back(
            factory_->createPolygon(
                factory_->createLinearRing({
                     CoordinateXY(0, 0),
                     CoordinateXY(1, 0),
                     CoordinateXY(1, 1),
                     CoordinateXY(0, 1),
                     CoordinateXY(0, 0)
        })));

        surfaces.emplace_back(
            factory_->createCurvePolygon(
                factory_->createCircularString({
                     CoordinateXY(10, 10),
                     CoordinateXY(11, 11),
                     CoordinateXY(12, 10),
                     CoordinateXY(11, 9),
                     CoordinateXY(10, 10)
        })));

        ms_ = factory_->createMultiSurface(std::move(surfaces));
    }

};

typedef test_group<test_multisurface_data> group;
typedef group::object object;

group test_multisurface_group("geos::geom::MultiSurface");

template<>
template<>
void object::test<1>()
{
    auto ms = factory_->createMultiSurface();

    ensure("isEmpty", ms->isEmpty());
    ensure_equals("getNumPoints", ms->getNumPoints(), 0u);
    ensure("hasZ", !ms->hasZ());
    ensure("hasM", !ms->hasM());
    ensure_equals("getCoordinateDimension", ms->getCoordinateDimension(), 2u);

    ensure("getCoordinates", ms->getCoordinates()->isEmpty());
    ensure("getCoordinate", ms->getCoordinate() == nullptr);

    ensure_equals("getArea", ms->getArea(), 0);
    ensure_equals("getLength", ms->getLength(), 0.0);
}

// Basic Geometry API
template<>
template<>
void object::test<2>()
{
    // Geometry type functions
    ensure_equals("getGeometryType", ms_->getGeometryType(), "MultiSurface");
    ensure_equals("getGeometryTypdId", ms_->getGeometryTypeId(), geos::geom::GEOS_MULTISURFACE);
    ensure("isCollection", !ms_->isCollection());

    // Geometry size functions
    ensure("isEmpty", !ms_->isEmpty());
    ensure_equals("getArea", ms_->getArea(), 4.141592653589132, 1e-6); // expected value from PostGIS with ST_CurveToLine(geom, 1e-13, 1)
    ensure_equals("getLength", ms_->getLength(), 10.283185307179586);
    ensure_equals("getNumGeometries", ms_->getNumGeometries(), 2u);
    ensure_equals("getNumPoints", ms_->getNumPoints(), 10u);
    ensure(!ms_->getEnvelopeInternal()->isNull());

    // Geometry dimension functions
    ensure_equals("getDimension", ms_->getDimension(), geos::geom::Dimension::A);
    ensure("isLineal", !ms_->isLineal());
    ensure("isPuntal", !ms_->isPuntal());
    ensure("isPolygonal", ms_->isPolygonal());
    ensure("hasDimension(L)", !ms_->hasDimension(geos::geom::Dimension::L));
    ensure("hasDimension(P)", !ms_->hasDimension(geos::geom::Dimension::P));
    ensure("hasDimension(A)", ms_->hasDimension(geos::geom::Dimension::A));
    ensure("isDimensionStrict", ms_->isDimensionStrict(geos::geom::Dimension::A));
    ensure("isMixedDimension", !ms_->isMixedDimension());
    ensure_equals("getBoundaryDimension", ms_->getBoundaryDimension(), geos::geom::Dimension::L);

    // Coordinate dimension functions
    ensure("hasZ", !ms_->hasZ());
    ensure("hasM", !ms_->hasM());
    ensure_equals("getCoordinateDimension", ms_->getCoordinateDimension(), 2u);

    // Coordinate access functions
    ensure("getCoordinates", ms_->getCoordinates()->getSize() == 10u);
    ensure_equals("getCoordinate", *ms_->getCoordinate(), CoordinateXY(0, 0));
}

// Operations
template<>
template<>
void object::test<3>()
{
    // Predicates
    ensure_THROW(ms_->contains(ms_.get()), geos::util::UnsupportedOperationException);
    ensure_THROW(ms_->coveredBy(ms_.get()), geos::util::UnsupportedOperationException);
    ensure_THROW(ms_->covers(ms_.get()), geos::util::UnsupportedOperationException);
    ensure_THROW(ms_->crosses(ms_.get()), geos::util::UnsupportedOperationException);
    ensure_THROW(ms_->disjoint(ms_.get()), geos::util::UnsupportedOperationException);
    ensure_THROW(ms_->equals(ms_.get()), geos::util::UnsupportedOperationException);
    ensure_THROW(ms_->intersects(ms_.get()), geos::util::UnsupportedOperationException);
    ensure_THROW(ms_->overlaps(ms_.get()), geos::util::UnsupportedOperationException);
    ensure_THROW(ms_->relate(ms_.get()), geos::util::UnsupportedOperationException);
    ensure_THROW(ms_->touches(ms_.get()), geos::util::UnsupportedOperationException);
    ensure_THROW(ms_->within(ms_.get()), geos::util::UnsupportedOperationException);

    auto cp2 = ms_->clone();

    ensure("equalsExact", ms_->equalsExact(cp2.get()));
    ensure("equalsIdentical", ms_->equalsIdentical(cp2.get()));

    // Overlay
    ensure_THROW(ms_->Union(), geos::util::UnsupportedOperationException);
    ensure_THROW(ms_->Union(ms_.get()), geos::util::UnsupportedOperationException);
    ensure_THROW(ms_->difference(ms_.get()), geos::util::UnsupportedOperationException);
    ensure_THROW(ms_->intersection(ms_.get()), geos::util::UnsupportedOperationException);
    ensure_THROW(ms_->symDifference(ms_.get()), geos::util::UnsupportedOperationException);

    // Distance
    ensure_THROW(ms_->distance(ms_.get()), geos::util::UnsupportedOperationException);
    ensure_THROW(ms_->isWithinDistance(ms_.get(), 1), geos::util::UnsupportedOperationException);

    // Valid / Simple
    ensure_THROW(ms_->isSimple(), geos::util::UnsupportedOperationException);
    ensure_THROW(ms_->isValid(), geos::util::UnsupportedOperationException);

    // Operations
    ensure_THROW(ms_->convexHull(), geos::util::UnsupportedOperationException);
    ensure_THROW(ms_->buffer(1), geos::util::UnsupportedOperationException);
    ensure_THROW(ms_->getCentroid(), geos::util::UnsupportedOperationException);
    ensure_THROW(ms_->getBoundary(), geos::util::UnsupportedOperationException);

    ensure("clone", ms_->equalsIdentical(ms_->clone().get()));

    // each element is reversed but the order of the elements remains the same
    // this behavior is the same as for MultiLineString
    ensure("reverse", ms_->reverse()->equalsIdentical(wktreader_.read(
    "MULTISURFACE (((0 0, 0 1, 1 1, 1 0, 0 0)), "
                  "CURVEPOLYGON (CIRCULARSTRING (10 10, 11 9, 12 10, 11 11, 10 10)))").get()));
    auto cc3 = ms_->reverse();
    ensure_THROW(cc3->normalize(), geos::util::UnsupportedOperationException);
}

}
