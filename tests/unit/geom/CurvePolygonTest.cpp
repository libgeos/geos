#include <tut/tut.hpp>
#include <tut/tut_macros.hpp>

#include <geos/geom/GeometryFactory.h>
#include <geos/geom/CircularString.h>
#include <geos/geom/CompoundCurve.h>
#include <geos/geom/CurvePolygon.h>
#include <geos/geom/IntersectionMatrix.h>
#include <geos/io/WKTReader.h>

#include "utility.h"

using geos::geom::CoordinateXY;
using geos::geom::CurvePolygon;

namespace tut {

// Common data used by tests
struct test_curvepolygon_data {

    geos::geom::GeometryFactory::Ptr factory_ = geos::geom::GeometryFactory::create();
    geos::io::WKTReader wktreader_;

    std::unique_ptr<geos::geom::CurvePolygon> cp_;

    test_curvepolygon_data() {
        std::vector<std::unique_ptr<geos::geom::Curve>> holes;

        std::vector<std::unique_ptr<geos::geom::SimpleCurve>> shell_sections;
        shell_sections.emplace_back(
             factory_->createCircularString({
                 CoordinateXY(0, 0),
                 CoordinateXY(2, 0),
                 CoordinateXY(2, 1),
                 CoordinateXY(2, 3),
                 CoordinateXY(4, 3)
        }));
        shell_sections.emplace_back(
             factory_->createLineString({
                 CoordinateXY(4, 3),
                 CoordinateXY(4, 5),
                 CoordinateXY(1, 4),
                 CoordinateXY(0, 0)
        }));

        auto shell = factory_->createCompoundCurve(std::move(shell_sections));

        holes.emplace_back(factory_->createCircularString({
            CoordinateXY(1.7, 1),
            CoordinateXY(1.4, 0.4),
            CoordinateXY(1.6, 0.4),
            CoordinateXY(1.6, 0.5),
            CoordinateXY(1.7, 1)
        }));

        cp_ = factory_->createCurvePolygon(std::move(shell), std::move(holes));
    }
};

typedef test_group<test_curvepolygon_data> group;
typedef group::object object;

group test_curvepolygon_group("geos::geom::CurvePolygon");

template<>
template<>
void object::test<1>()
{
    auto cp = factory_->createCurvePolygon(false, false);

    ensure("isEmpty", cp->isEmpty());
    ensure_equals("getNumPoints", cp->getNumPoints(), 0u);
    ensure("hasZ", !cp->hasZ());
    ensure("hasM", !cp->hasM());
    ensure_equals("getCoordinateDimension", cp->getCoordinateDimension(), 2u);

    ensure("getCoordinates", cp->getCoordinates()->isEmpty());
    ensure("getCoordinate", cp->getCoordinate() == nullptr);

    ensure_equals("getArea", cp->getArea(), 0.0);
    ensure_equals("getLength", cp->getLength(), 0.0);
}

// Basic Geometry API
template<>
template<>
void object::test<2>()
{
    // Geometry type functions
    ensure_equals("getGeometryType", cp_->getGeometryType(), "CurvePolygon");
    ensure_equals("getGeometryTypdId", cp_->getGeometryTypeId(), geos::geom::GEOS_CURVEPOLYGON);
    ensure("isCollection", !cp_->isCollection());

    // Geometry size functions
    ensure("isEmpty", !cp_->isEmpty());
    ensure_equals("getArea", cp_->getArea(), 9.0526564962674, 1e-8); // expected value from PostGIS with ST_CurveToLine(geom, 1e-13, 1)
    ensure_equals("getLength", cp_->getLength(), 19.236489581872586, 1e-8);
    ensure_equals("getNumGeometries", cp_->getNumGeometries(), 1u);
    ensure_equals("getNumPoints", cp_->getNumPoints(), 14u);
    ensure_equals("getNumInteriorRing", cp_->getNumInteriorRing(), 1u);
    {
        geos::geom::Envelope expected(0, 4, -0.618033988749895, 5);
        const geos::geom::Envelope& actual = *cp_->getEnvelopeInternal();

        ensure_equals("getEnvelopeInternal MinX", actual.getMinX(), expected.getMinX());
        ensure_equals("getEnvelopeInternal MinY", actual.getMinY(), expected.getMinY());
        ensure_equals("getEnvelopeInternal MaxX", actual.getMaxX(), expected.getMaxX());
        ensure_equals("getEnvelopeInternal MaxY", actual.getMaxY(), expected.getMaxY());
    }

    // Geometry dimension functions
    ensure_equals("getDimension", cp_->getDimension(), geos::geom::Dimension::A);
    ensure("isLineal", !cp_->isLineal());
    ensure("isPuntal", !cp_->isPuntal());
    ensure("isPolygonal", cp_->isPolygonal());
    ensure("hasDimension(L)", !cp_->hasDimension(geos::geom::Dimension::L));
    ensure("hasDimension(P)", !cp_->hasDimension(geos::geom::Dimension::P));
    ensure("hasDimension(A)", cp_->hasDimension(geos::geom::Dimension::A));
    ensure("isDimensionStrict", cp_->isDimensionStrict(geos::geom::Dimension::A));
    ensure("isMixedDimension", !cp_->isMixedDimension());
    ensure_equals("getBoundaryDimension", cp_->getBoundaryDimension(), geos::geom::Dimension::L);

    // Coordinate dimension functions
    ensure("hasZ", !cp_->hasZ());
    ensure("hasM", !cp_->hasM());
    ensure_equals("getCoordinateDimension", cp_->getCoordinateDimension(), 2u);

    // Coordinate access functions
    ensure("getCoordinates", cp_->getCoordinates()->getSize() == 14u);
    ensure_equals("getCoordinate", *cp_->getCoordinate(), CoordinateXY(0, 0));
}

// Operations
template<>
template<>
void object::test<3>()
{
    // Predicates
    ensure_THROW(cp_->contains(cp_.get()), geos::util::UnsupportedOperationException);
    ensure_THROW(cp_->coveredBy(cp_.get()), geos::util::UnsupportedOperationException);
    ensure_THROW(cp_->covers(cp_.get()), geos::util::UnsupportedOperationException);
    ensure_THROW(cp_->crosses(cp_.get()), geos::util::UnsupportedOperationException);
    ensure_THROW(cp_->disjoint(cp_.get()), geos::util::UnsupportedOperationException);
    ensure_THROW(cp_->equals(cp_.get()), geos::util::UnsupportedOperationException);
    ensure_THROW(cp_->intersects(cp_.get()), geos::util::UnsupportedOperationException);
    ensure_THROW(cp_->overlaps(cp_.get()), geos::util::UnsupportedOperationException);
    ensure_THROW(cp_->relate(cp_.get()), geos::util::UnsupportedOperationException);
    ensure_THROW(cp_->touches(cp_.get()), geos::util::UnsupportedOperationException);
    ensure_THROW(cp_->within(cp_.get()), geos::util::UnsupportedOperationException);

    auto cp2 = cp_->clone();

    ensure("equalsExact", cp_->equalsExact(cp2.get()));
    ensure("equalsIdentical", cp_->equalsIdentical(cp2.get()));

    // Overlay
    ensure_THROW(cp_->Union(), geos::util::UnsupportedOperationException);
    ensure_THROW(cp_->Union(cp_.get()), geos::util::UnsupportedOperationException);
    ensure_THROW(cp_->difference(cp_.get()), geos::util::UnsupportedOperationException);
    ensure_THROW(cp_->intersection(cp_.get()), geos::util::UnsupportedOperationException);
    ensure_THROW(cp_->symDifference(cp_.get()), geos::util::UnsupportedOperationException);

    // Distance
    ensure_THROW(cp_->distance(cp_.get()), geos::util::UnsupportedOperationException);
    ensure_THROW(cp_->isWithinDistance(cp_.get(), 1), geos::util::UnsupportedOperationException);

    // Valid / Simple
    ensure_THROW(cp_->isSimple(), geos::util::UnsupportedOperationException);
    ensure_THROW(cp_->isValid(), geos::util::UnsupportedOperationException);

    // Operations
    ensure_THROW(cp_->convexHull(), geos::util::UnsupportedOperationException);
    ensure_THROW(cp_->buffer(1), geos::util::UnsupportedOperationException);
    ensure_THROW(cp_->getCentroid(), geos::util::UnsupportedOperationException);
    ensure_THROW(cp_->getBoundary(), geos::util::UnsupportedOperationException);

    ensure("clone", cp_->equalsIdentical(cp_->clone().get()));

    // each element is reversed but the order of the elements remains the same
    // this behavior is the same as for MultiLineString
    ensure("reverse", cp_->reverse()->equalsIdentical(wktreader_.read(
            "CURVEPOLYGON ("
            "COMPOUNDCURVE ((0 0, 1 4, 4 5, 4 3), CIRCULARSTRING (4 3, 2 3, 2 1, 2 0, 0 0)), "
            "CIRCULARSTRING (1.7 1, 1.6 0.5, 1.6 0.4, 1.4 0.4, 1.7 1))").get()));
    auto cc3 = cp_->reverse();
    ensure_THROW(cc3->normalize(), geos::util::UnsupportedOperationException);
}

template<>
template<>
void object::test<4>()
{
    set_test_name("getArea");

    // SELECT ST_Area(ST_CurveToLine('CURVEPOLYGON (CIRCULARSTRING(0 0,0 2,1 2,1 1,2 1,3 0,0 0))', 36000));
    auto cp = wktreader_.read<CurvePolygon>("CURVEPOLYGON (CIRCULARSTRING(0 0,0 2,1 2,1 1,2 1,3 0,0 0))");
    ensure_equals("cp->getArea()", cp->getArea(), 9.8185835, 1e-6);

    auto cpRev = cp->reverse();
    ensure_equals("cpRev->getArea()", cpRev->getArea(), 9.8185835, 1e-6);
}

template<>
template<>
void object::test<5>()
{
    set_test_name("getLinearized");

    auto cp = wktreader_.read<CurvePolygon>("CURVEPOLYGON (COMPOUNDCURVE (CIRCULARSTRING (0 0, 2 0, 2 1, 2 3, 4 3), (4 3, 4 5, 1 4, 0 0)), CIRCULARSTRING (1.7 1, 1.4 0.4, 1.6 0.4, 1.6 0.5, 1.7 1))");

    // check that we return Polygon* rather than Geometry*
    std::unique_ptr<Polygon> poly = cp->getLinearized(90.0 / 4);

    auto expected = wktreader_.read("POLYGON ((0 0, 0.2675 -0.3446, 0.6464 -0.5607, 1.0793 -0.6152, 1.5 -0.5, 1.8446 -0.2325, 2.0607 0.1464, 2.1152 0.5793, 2 1, 1.6934 1.4588, 1.5858 2, 1.6934 2.5412, 2 3, 2.4588 3.3066, 3 3.4142, 3.5412 3.3066, 4 3, 4 5, 1 4, 0 0), (1.7 1, 1.5871 1.0537, 1.4623 1.0629, 1.3427 1.0265, 1.2444 0.9492, 1.1806 0.8416, 1.16 0.7183, 1.1855 0.5958, 1.2534 0.4908, 1.3548 0.4175, 1.4757 0.3858, 1.6 0.4, 1.6203 0.705, 1.7 1))");

    ensure_equals_exact_geometry_xyzm(poly.get(), expected.get(), 1e-4);
}

template<>
template<>
void object::test<6>()
{
    set_test_name("getCurved");

    std::unique_ptr<CurvePolygon> curved = cp_->getCurved(1000);

    ensure_equals_exact_geometry_xyzm(curved.get(), cp_.get(), 0);
}

}
