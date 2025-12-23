#include <tut/tut.hpp>
#include <tut/tut_macros.hpp>

#include <geos/geom/GeometryFactory.h>
#include <geos/geom/CompoundCurve.h>
#include <geos/geom/CircularString.h>
#include <geos/geom/IntersectionMatrix.h>
#include <geos/geom/MultiCurve.h>
#include <geos/io/WKTReader.h>

#include "utility.h"

using geos::geom::CoordinateXY;

namespace tut {

// Common data used by tests
struct test_multicurve_data {

    geos::geom::GeometryFactory::Ptr factory_ = geos::geom::GeometryFactory::create();
    geos::io::WKTReader wktreader_;

    std::unique_ptr<geos::geom::MultiCurve> mc_;

    test_multicurve_data() {
        std::vector<std::unique_ptr<geos::geom::Curve>> curves;

        // Add a CompoundCurve
        std::vector<std::unique_ptr<geos::geom::SimpleCurve>> cc_sections;
        cc_sections.emplace_back(
             factory_->createCircularString({
                 CoordinateXY(0, 0),
                 CoordinateXY(2, 0),
                 CoordinateXY(2, 1),
                 CoordinateXY(2, 3),
                 CoordinateXY(4, 3)
        }));
        cc_sections.emplace_back(
             factory_->createLineString({
                 CoordinateXY(4, 3),
                 CoordinateXY(4, 5),
                 CoordinateXY(1, 4),
                 CoordinateXY(0, 0)
        }));

        curves.emplace_back(factory_->createCompoundCurve(std::move(cc_sections)));

        // Add a LineString
        curves.emplace_back(factory_->createLineString({CoordinateXY(8, 9), CoordinateXY(10, 11)}));

        // Add a CircularString
        curves.emplace_back(factory_->createCircularString({
            CoordinateXY(1.7, 1),
            CoordinateXY(1.4, 0.4),
            CoordinateXY(1.6, 0.4),
            CoordinateXY(1.6, 0.5),
            CoordinateXY(1.7, 1)
        }));

        mc_ = factory_->createMultiCurve(std::move(curves));
    }

};

typedef test_group<test_multicurve_data> group;
typedef group::object object;

group test_multicurve_group("geos::geom::MultiCurve");

template<>
template<>
void object::test<1>()
{
    auto mc = factory_->createMultiCurve();

    ensure("isEmpty", mc->isEmpty());
    ensure_equals("getNumPoints", mc->getNumPoints(), 0u);
    ensure("hasZ", !mc->hasZ());
    ensure("hasM", !mc->hasM());
    ensure_equals("getCoordinateDimension", mc->getCoordinateDimension(), 2u);

    ensure("getCoordinates", mc->getCoordinates()->isEmpty());
    ensure("getCoordinate", mc->getCoordinate() == nullptr);

    ensure_equals("getArea", mc->getArea(), 0);
    ensure_equals("getLength", mc->getLength(), 0);
}

// Basic Geometry API
template<>
template<>
void object::test<2>()
{
    // Geometry type functions
    ensure_equals("getGeometryType", mc_->getGeometryType(), "MultiCurve");
    ensure_equals("getGeometryTypdId", mc_->getGeometryTypeId(), geos::geom::GEOS_MULTICURVE);
    ensure("isCollection", mc_->isCollection());

    // Geometry size functions
    ensure("isEmpty", !mc_->isEmpty());
    ensure_equals("getArea", mc_->getArea(), 0);
    ensure_equals("getLength", mc_->getLength(), 22.064916706618778, 1e-8);
    ensure_equals("getNumGeometries", mc_->getNumGeometries(), 3u);
    ensure_equals("getNumPoints", mc_->getNumPoints(), 16u);
    ensure(!mc_->getEnvelopeInternal()->isNull());

    // Geometry dimension functions
    ensure_equals("getDimension", mc_->getDimension(), geos::geom::Dimension::L);
    ensure("isLineal", mc_->isLineal());
    ensure("isPuntal", !mc_->isPuntal());
    ensure("isPolygonal", !mc_->isPolygonal());
    ensure("hasDimension(L)", mc_->hasDimension(geos::geom::Dimension::L));
    ensure("hasDimension(P)", !mc_->hasDimension(geos::geom::Dimension::P));
    ensure("hasDimension(A)", !mc_->hasDimension(geos::geom::Dimension::A));
    ensure("isDimensionStrict", mc_->isDimensionStrict(geos::geom::Dimension::L));
    ensure("isMixedDimension", !mc_->isMixedDimension());
    ensure_equals("getBoundaryDimension", mc_->getBoundaryDimension(), geos::geom::Dimension::P);

    // Coordinate dimension functions
    ensure("hasZ", !mc_->hasZ());
    ensure("hasM", !mc_->hasM());
    ensure_equals("getCoordinateDimension", mc_->getCoordinateDimension(), 2u);

    // Coordinate access functions
    ensure("getCoordinates", mc_->getCoordinates()->getSize() == 16u);
    ensure_equals("getCoordinate", *mc_->getCoordinate(), CoordinateXY(0, 0));
}

// Operations
template<>
template<>
void object::test<3>()
{
    // Predicates
    ensure_THROW(mc_->contains(mc_.get()), geos::util::UnsupportedOperationException);
    ensure_THROW(mc_->coveredBy(mc_.get()), geos::util::UnsupportedOperationException);
    ensure_THROW(mc_->covers(mc_.get()), geos::util::UnsupportedOperationException);
    ensure_THROW(mc_->crosses(mc_.get()), geos::util::UnsupportedOperationException);
    ensure_THROW(mc_->disjoint(mc_.get()), geos::util::UnsupportedOperationException);
    ensure_THROW(mc_->equals(mc_.get()), geos::util::UnsupportedOperationException);
    ensure_THROW(mc_->intersects(mc_.get()), geos::util::UnsupportedOperationException);
    ensure_THROW(mc_->overlaps(mc_.get()), geos::util::UnsupportedOperationException);
    ensure_THROW(mc_->relate(mc_.get()), geos::util::UnsupportedOperationException);
    ensure_THROW(mc_->touches(mc_.get()), geos::util::UnsupportedOperationException);
    ensure_THROW(mc_->within(mc_.get()), geos::util::UnsupportedOperationException);

    auto cc2 = mc_->clone();

    ensure("equalsExact", mc_->equalsExact(cc2.get()));
    ensure("equalsIdentical", mc_->equalsIdentical(cc2.get()));

    // Overlay
    ensure_THROW(mc_->Union(), geos::util::UnsupportedOperationException);
    ensure_THROW(mc_->Union(mc_.get()), geos::util::UnsupportedOperationException);
    ensure_THROW(mc_->difference(mc_.get()), geos::util::UnsupportedOperationException);
    ensure_THROW(mc_->intersection(mc_.get()), geos::util::UnsupportedOperationException);
    ensure_THROW(mc_->symDifference(mc_.get()), geos::util::UnsupportedOperationException);

    // Distance
    ensure_THROW(mc_->distance(mc_.get()), geos::util::UnsupportedOperationException);
    ensure_THROW(mc_->isWithinDistance(mc_.get(), 1), geos::util::UnsupportedOperationException);

    // Valid / Simple
    ensure_THROW(mc_->isSimple(), geos::util::UnsupportedOperationException);
    ensure_THROW(mc_->isValid(), geos::util::UnsupportedOperationException);

    // Operations
    ensure_THROW(mc_->convexHull(), geos::util::UnsupportedOperationException);
    ensure_THROW(mc_->buffer(1), geos::util::UnsupportedOperationException);
    ensure_THROW(mc_->getCentroid(), geos::util::UnsupportedOperationException);
    ensure_THROW(mc_->getBoundary(), geos::util::UnsupportedOperationException);

    ensure("clone", mc_->equalsIdentical(mc_->clone().get()));

    // each element is reversed but the order of the elements remains the same
    // this behavior is the same as for MultiLineString
    ensure("reverse", mc_->reverse()->equalsIdentical(wktreader_.read(""
            "MULTICURVE ("
            "  COMPOUNDCURVE ((0 0, 1 4, 4 5, 4 3), CIRCULARSTRING (4 3, 2 3, 2 1, 2 0, 0 0)), "
            "  (10 11, 8 9),"
            "  CIRCULARSTRING (1.7 1, 1.6 0.5, 1.6 0.4, 1.4 0.4, 1.7 1))").get()));
    auto cc3 = mc_->reverse();
    ensure_THROW(cc3->normalize(), geos::util::UnsupportedOperationException);
}

// isClosed
template<>
template<>
void object::test<4>()
{
    // union of elements is closed, but individual elements are not => MultiCurve is not closed
    ensure(!wktreader_.read<geos::geom::MultiCurve>("MULTICURVE ((0 0, 1 0), (1 0, 1 1, 0 0))")->isClosed());

    // all elements are closed => MulitCurve is closed
    ensure(wktreader_.read<geos::geom::MultiCurve>("MULTICURVE ((0 0, 1 0, 1 1, 0 0), CIRCULARSTRING (3 3, 5 5, 3 3))")->isClosed());

    // some elements are closed => MultiCurve is not closed
    ensure(!wktreader_.read<geos::geom::MultiCurve>("MULTICURVE ((0 0, 1 0, 1 1, 0 0), CIRCULARSTRING (3 3, 4 4, 5 3))")->isClosed());
}

template<>
template<>
void object::test<5>()
{
    set_test_name("getLinearized()");

    // check that we return MultiLineString*, not Geometry*
    std::unique_ptr<geos::geom::MultiLineString> mls = mc_->getLinearized(2);

    ensure_equals(mls->getGeometryTypeId(), geos::geom::GEOS_MULTILINESTRING);
    ensure_equals("getLength()", mls->getLength(), mc_->getLength(), 1e-3);
}

template<>
template<>
void object::test<6>()
{
    set_test_name("getCurved()");

    std::unique_ptr<geos::geom::GeometryCollection> curved = mc_->getCurved(100);
    ensure_equals_exact_geometry_xyzm(mc_.get(), curved.get(), 0);
}

}
