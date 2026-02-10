#include <utility.h>
#include <tut/tut.hpp>
#include <tut/tut_macros.hpp>

#include <geos/algorithm/CurveToLineParams.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateFilter.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/CompoundCurve.h>
#include <geos/geom/Point.h>
#include <geos/geom/CircularString.h>
#include <geos/geom/IntersectionMatrix.h>
#include <geos/io/WKTReader.h>
#include <geos/operation/valid/RepeatedPointTester.h>
#include <geos/util.h>

#include "geos/algorithm/LineToCurveParams.h"


using geos::algorithm::CurveToLineParams;
using geos::geom::CompoundCurve;
using geos::geom::Curve;
using geos::geom::CoordinateXY;
using geos::geom::CoordinateSequence;
using geos::geom::SimpleCurve;

namespace tut {
// Common data used by tests
struct test_compoundcurve_data {

    geos::geom::GeometryFactory::Ptr factory_ = geos::geom::GeometryFactory::create();
    geos::io::WKTReader wktreader_;

    std::unique_ptr<CompoundCurve> cc_;

    test_compoundcurve_data()
    {
        std::vector<std::unique_ptr<SimpleCurve>> curves;

        curves.emplace_back(factory_->createCircularString({
            CoordinateXY(0, 0),
            CoordinateXY(1, 1),
            CoordinateXY(2, 0)
        }));

        curves.emplace_back(factory_->createLineString({
            CoordinateXY(2, 0),
            CoordinateXY(2, 2)
        }));

        cc_ = factory_->createCompoundCurve(std::move(curves));
    }

    void checkNormalize(const std::string& wkt_in, const std::string& wkt_expected) const {
        auto cs = wktreader_.read(wkt_in);
        cs->normalize();

        auto expected = wktreader_.read(wkt_expected);

        ensure_equals_exact_geometry_xyzm(cs.get(), expected.get(), 0);
    }

};

typedef test_group<test_compoundcurve_data> group;
typedef group::object object;

group test_compoundcurve_group("geos::geom::CompoundCurve");

template<>
template<>
void object::test<1>()
{
    set_test_name("empty CompoundCurve");

    auto cc = factory_->createCompoundCurve();

    ensure("isEmpty", cc->isEmpty());
    ensure_equals("getNumPoints", cc->getNumPoints(), 0u);
    ensure_equals("getNumCurves", cc->getNumCurves(), 0u);
    ensure("hasZ", !cc->hasZ());
    ensure("hasM", !cc->hasM());
    ensure_equals("getCoordinateDimension", cc->getCoordinateDimension(), 2u);

    ensure("getCoordinates", cc->getCoordinates()->isEmpty());
    ensure("getCoordinate", cc->getCoordinate() == nullptr);

    ensure_equals("getArea", cc->getArea(), 0);
    ensure_equals("getLength", cc->getLength(), 0);

    ensure("getStartPoint", cc->getStartPoint() == nullptr);
    ensure("getEndPoint", cc->getEndPoint() == nullptr);
    ensure("getPointN", cc->getPointN(3) == nullptr);
}

// Basic Geometry API
template<>
template<>
void object::test<2>()
{
    // Geometry type functions
    ensure_equals("getGeometryType", cc_->getGeometryType(), "CompoundCurve");
    ensure_equals("getGeometryTypeId", cc_->getGeometryTypeId(), geos::geom::GEOS_COMPOUNDCURVE);
    ensure("isCollection", !cc_->isCollection());

    // Geometry size functions
    ensure("isEmpty", !cc_->isEmpty());
    ensure_equals("getArea", cc_->getArea(), 0);
    ensure_equals("getLength", cc_->getLength(), geos::MATH_PI + 2);
    ensure_equals("getNumGeometries", cc_->getNumGeometries(), 1u);
    ensure_equals("getNumCurves", cc_->getNumCurves(), 2u);
    ensure_equals("getNumPoints", cc_->getNumPoints(), 5u);
    geos::geom::Envelope expected(0, 2, 0, 2);
    ensure("getEnvelopeInternal", cc_->getEnvelopeInternal()->equals(&expected));

    // Geometry dimension functions
    ensure_equals("getDimension", cc_->getDimension(), geos::geom::Dimension::L);
    ensure("isLineal", cc_->isLineal());
    ensure("isPuntal", !cc_->isPuntal());
    ensure("isPolygonal", !cc_->isPolygonal());
    ensure("hasDimension(L)", cc_->hasDimension(geos::geom::Dimension::L));
    ensure("hasDimension(P)", !cc_->hasDimension(geos::geom::Dimension::P));
    ensure("hasDimension(A)", !cc_->hasDimension(geos::geom::Dimension::A));
    ensure("isDimensionStrict", cc_->isDimensionStrict(geos::geom::Dimension::L));
    ensure("isMixedDimension", !cc_->isMixedDimension());
    ensure_equals("getBoundaryDimension", cc_->getBoundaryDimension(), geos::geom::Dimension::P);

    // Coordinate dimension functions
    ensure("hasZ", !cc_->hasZ());
    ensure("hasM", !cc_->hasM());
    ensure_equals("getCoordinateDimension", cc_->getCoordinateDimension(), 2u);

    // Coordinate access functions
    ensure("getCoordinates", cc_->getCoordinates()->getSize() == 5u);
    ensure_equals("getCoordinate", *cc_->getCoordinate(), CoordinateXY(0, 0));
    ensure_equals_geometry(static_cast<Geometry*>(cc_->getStartPoint().get()), wktreader_.read("POINT (0 0)").get());
    ensure_equals_geometry(static_cast<Geometry*>(cc_->getEndPoint().get()), wktreader_.read("POINT (2 2)").get());

    ensure_equals_geometry(cc_->getStartPoint().get(), factory_->createPoint(CoordinateXY{0, 0}).get());
    ensure_equals_geometry(cc_->getEndPoint().get(), factory_->createPoint(CoordinateXY{2, 2}).get());
    ensure_equals_geometry(cc_->getPointN(4).get(), factory_->createPoint(CoordinateXY{2, 2}).get());
}

// Operations
template<>
template<>
void object::test<3>()
{
    set_test_name("operations");

    // Predicates
    ensure_THROW(cc_->contains(cc_.get()), geos::util::UnsupportedOperationException);
    ensure_THROW(cc_->coveredBy(cc_.get()), geos::util::UnsupportedOperationException);
    ensure_THROW(cc_->covers(cc_.get()), geos::util::UnsupportedOperationException);
    ensure_THROW(cc_->crosses(cc_.get()), geos::util::UnsupportedOperationException);
    ensure_THROW(cc_->disjoint(cc_.get()), geos::util::UnsupportedOperationException);
    ensure_THROW(cc_->equals(cc_.get()), geos::util::UnsupportedOperationException);
    ensure_THROW(cc_->intersects(cc_.get()), geos::util::UnsupportedOperationException);
    ensure_THROW(cc_->overlaps(cc_.get()), geos::util::UnsupportedOperationException);
    ensure_THROW(cc_->relate(cc_.get()), geos::util::UnsupportedOperationException);
    ensure_THROW(cc_->touches(cc_.get()), geos::util::UnsupportedOperationException);
    ensure_THROW(cc_->within(cc_.get()), geos::util::UnsupportedOperationException);

    auto cc2 = cc_->clone();

    ensure("equalsExact", cc_->equalsExact(cc2.get()));
    ensure("equalsIdentical", cc_->equalsIdentical(cc2.get()));

    // Overlay
    ensure_THROW(cc_->Union(), geos::util::UnsupportedOperationException);
    ensure_THROW(cc_->Union(cc_.get()), geos::util::UnsupportedOperationException);
    ensure_THROW(cc_->difference(cc_.get()), geos::util::UnsupportedOperationException);
    ensure_THROW(cc_->intersection(cc_.get()), geos::util::UnsupportedOperationException);
    ensure_THROW(cc_->symDifference(cc_.get()), geos::util::UnsupportedOperationException);

    // Distance
    ensure_THROW(cc_->distance(cc_.get()), geos::util::UnsupportedOperationException);
    ensure_THROW(cc_->isWithinDistance(cc_.get(), 1), geos::util::UnsupportedOperationException);

    // Valid / Simple
    ensure_THROW(cc_->isSimple(), geos::util::UnsupportedOperationException);
    ensure_THROW(cc_->isValid(), geos::util::UnsupportedOperationException);

    // Operations
    ensure_THROW(cc_->convexHull(), geos::util::UnsupportedOperationException);
    ensure_THROW(cc_->buffer(1), geos::util::UnsupportedOperationException);
    ensure_THROW(cc_->getCentroid(), geos::util::UnsupportedOperationException);

    {
        CoordinateSequence seq(2, false, false);
        seq.setAt(CoordinateXY{0, 0}, 0);
        seq.setAt(CoordinateXY{2, 2}, 1);
        std::unique_ptr<Geometry> mp = factory_->createMultiPoint(seq);

        ensure_equals_geometry(cc_->getBoundary().get(), mp.get());
    }

    ensure("clone", cc_->equalsIdentical(cc_->clone().get()));

    ensure("reverse", cc_->reverse()->equalsIdentical(wktreader_.read(""
            "COMPOUNDCURVE ((2 2, 2 0), CIRCULARSTRING (2 0, 1 1, 0 0))").get()));
    auto cc3 = cc_->reverse();
}

// GeometryFilter
template<>
template<>
void object::test<4>()
{
    struct TestGeometryFilter : public geos::geom::GeometryFilter {
        void filter_ro(const geos::geom::Geometry* g) override
        {
            calls++;
            last_arg = g;
        }

        std::size_t calls = 0;
        const Geometry* last_arg = nullptr;
    };

    TestGeometryFilter tgf;
    cc_->apply_ro(&tgf);

    ensure_equals(tgf.calls, 1u);
    ensure_equals(tgf.last_arg, cc_.get());
}

// GeometryComponentFilter RO
template<>
template<>
void object::test<5>()
{
    struct TestGeometryComponentFilter : public geos::geom::GeometryComponentFilter {
        void filter_ro(const geos::geom::Geometry* g) override
        {
            calls++;
            last_arg = g;
        }

        std::size_t calls = 0;
        const Geometry* last_arg = nullptr;
    };

    TestGeometryComponentFilter tgf;
    cc_->apply_ro(&tgf);

    ensure_equals(tgf.calls, 1u);
    ensure_equals(tgf.last_arg, cc_.get());
}

// CoordinateFilter RO
template<>
template<>
void object::test<6>()
{
    struct TestCoordinateFilter : public geos::geom::CoordinateFilter {
        void filter_ro(const geos::geom::Coordinate* x) override
        {
            coords.push_back(*x);
        }

        bool isDone() const override
        {
            return coords.size() >= 4;
        }

        std::vector<CoordinateXY> coords;
    };

    TestCoordinateFilter tcf;
    cc_->apply_ro(&tcf);

    ensure_equals(tcf.coords.size(), 4u);
    ensure_equals(tcf.coords[0], CoordinateXY(0, 0));
    ensure_equals(tcf.coords[1], CoordinateXY(1, 1));
    ensure_equals(tcf.coords[2], CoordinateXY(2, 0));
    ensure_equals(tcf.coords[3], CoordinateXY(2, 0));
}

// CoordinateFilter RW
template<>
template<>
void object::test<7>()
{
    struct TestCoordinateFilter : public geos::geom::CoordinateFilter {
        void filter_rw(geos::geom::Coordinate* c) const override
        {
            c->z = count;
            count += 1.0;
        }

        bool isDone() const override
        {
            return count >= 4;
        }

        mutable double count = 0.0;
    };

    TestCoordinateFilter tcf;
    cc_->apply_rw(&tcf);

    ensure_equals(tcf.count, 4.0);
    auto newCoords = cc_->getCoordinates();

    ensure_equals(newCoords->getOrdinate(0, CoordinateSequence::Z), 0.0);
    ensure_equals(newCoords->getOrdinate(1, CoordinateSequence::Z), 1.0);
    ensure_equals(newCoords->getOrdinate(2, CoordinateSequence::Z), 2.0);
    ensure_equals(newCoords->getOrdinate(3, CoordinateSequence::Z), 3.0);
    ensure_same(newCoords->getOrdinate(4, CoordinateSequence::Z), geos::DoubleNotANumber);
}

// CoordinateSequenceFilter RO
template<>
template<>
void object::test<8>()
{
    struct TestCoordinateSequenceFilter : public geos::geom::CoordinateSequenceFilter {
        void filter_ro(const CoordinateSequence& seq, std::size_t i) override
        {
            args.emplace_back(&seq, i);
        }

        bool isDone() const override
        {
            return args.size() >= 4;
        }

        bool isGeometryChanged() const override
        {
            return false;
        }

        std::vector<std::pair<const CoordinateSequence*, std::size_t>> args;
    };

    TestCoordinateSequenceFilter tcsf;
    cc_->apply_ro(tcsf);

    ensure_equals(tcsf.args.size(), 4u);

    ensure_equals(tcsf.args[0].first, cc_->getCurveN(0)->getCoordinatesRO());
    ensure_equals(tcsf.args[0].second, 0u);

    ensure_equals(tcsf.args[1].first, cc_->getCurveN(0)->getCoordinatesRO());
    ensure_equals(tcsf.args[1].second, 1u);

    ensure_equals(tcsf.args[2].first, cc_->getCurveN(0)->getCoordinatesRO());
    ensure_equals(tcsf.args[2].second, 2u);

    ensure_equals(tcsf.args[3].first, cc_->getCurveN(1)->getCoordinatesRO());
    ensure_equals(tcsf.args[3].second, 0u);
}

template<>
template<>
void object::test<9>()
{
    set_test_name("construction failure on non-contiguous curves");

    std::vector<std::unique_ptr<SimpleCurve>> curves;

    curves.push_back(wktreader_.read<SimpleCurve>("LINESTRING (0 0, 1 2)"));
    curves.push_back(wktreader_.read<SimpleCurve>("CIRCULARSTRING (2 1, 3 3, 4 1)"));
    ensure_THROW(factory_->createCompoundCurve(std::move(curves)), geos::util::IllegalArgumentException);

    curves.clear();
    curves.push_back(wktreader_.read<SimpleCurve>("LINESTRING (0 0, 1 2)"));
    curves.push_back(wktreader_.read<SimpleCurve>("CIRCULARSTRING EMPTY"));
    ensure_THROW(factory_->createCompoundCurve(std::move(curves)), geos::util::IllegalArgumentException);
}

template<>
template<>
void object::test<10>()
{
    set_test_name("normalize non-closed CompoundCurve");

    // already normalized
    checkNormalize("COMPOUNDCURVE((-2 2, -3 5, 0 10), CIRCULARSTRING (0 10, -5 5, 0 0))",
              "COMPOUNDCURVE((-2 2, -3 5, 0 10), CIRCULARSTRING (0 10, -5 5, 0 0))");

    checkNormalize("COMPOUNDCURVE((-2 2, -3 5, 0 10), CIRCULARSTRING (0 10, -5 5, 0 0))",
              "COMPOUNDCURVE((-2 2, -3 5, 0 10), CIRCULARSTRING (0 10, -5 5, 0 0))");

    // needs to be reversed
    checkNormalize("COMPOUNDCURVE(CIRCULARSTRING (0 0, -5 5, 0 10), (0 10, -3 5, -2 2))",
              "COMPOUNDCURVE((-2 2, -3 5, 0 10), CIRCULARSTRING (0 10, -5 5, 0 0))");
}

template<>
template<>
void object::test<11>()
{
    set_test_name("normalize closed CompoundCurve");
    // already normalized
    checkNormalize("COMPOUNDCURVE(CIRCULARSTRING(-5 0, 0 5, 5 0), (5 0, -5 0))",
              "COMPOUNDCURVE(CIRCULARSTRING(-5 0, 0 5, 5 0), (5 0, -5 0))");

    // needs to be reversed
    checkNormalize("COMPOUNDCURVE((-5 0, 5 0), CIRCULARSTRING(5 0, 0 5, -5 0))",
              "COMPOUNDCURVE(CIRCULARSTRING(-5 0, 0 5, 5 0), (5 0, -5 0))");

    // needs to be reversed and scrolled
    checkNormalize("COMPOUNDCURVE((5 0, -5 0), CIRCULARSTRING(-5 0, 0 5, 5 0))",
              "COMPOUNDCURVE(CIRCULARSTRING(-5 0, 0 5, 5 0), (5 0, -5 0))");

    // needs to be reversed and scrolled, but no geometries split
    // actual minimum point is a control point and cannot become the origin
    checkNormalize("COMPOUNDCURVE (CIRCULARSTRING (0 10, -5 5, 0 0), (0 0, 0 10))",
        "COMPOUNDCURVE(CIRCULARSTRING (0 0, -5 5, 0 10), (0 10, 0 0))");

    // needs to be reversed, scrolled, split
    checkNormalize("COMPOUNDCURVE (CIRCULARSTRING (10 0, 0 -10, -10 0, -5 5, 0 0), (0 0, 10 0))",
    "COMPOUNDCURVE (CIRCULARSTRING (-10 0, -5 5, 0 0), (0 0, 10 0), CIRCULARSTRING (10 0, 0 -10, -10 0))");
}

template<>
template<>
void object::test<12>()
{
    set_test_name("getLinearized()");

    auto cc = wktreader_.read("COMPOUNDCURVE (CIRCULARSTRING(-5 0, 0 5, 4 3, 5 0, 0 -5), (0 -5, -5 0))");
    auto ls = cc->getLinearized(CurveToLineParams::stepSizeDegrees(90.0 / 4));

    ensure_equals(ls->getGeometryTypeId(), geos::geom::GEOS_LINESTRING);
    ensure_equals(static_cast<geos::geom::Curve*>(cc.get())->getLinearized(CurveToLineParams::stepSizeDegrees(90.0 / 4))->getGeometryTypeId(), geos::geom::GEOS_LINESTRING);
    ensure_equals(static_cast<CompoundCurve*>(ls.get())->getLinearized(CurveToLineParams::stepSizeDegrees(90.0 / 4))->getGeometryTypeId(), geos::geom::GEOS_LINESTRING);

    geos::operation::valid::RepeatedPointTester rpt;
    ensure(!rpt.hasRepeatedPoint(ls.get()));

    auto expected = wktreader_.read("LINESTRING (-5 0, -4.685 1.7467, -3.7796 3.2733, -2.3979 4.3875, -0.7141 4.9487, 1.0597 4.8864, 2.6999 4.2084, 4 3, 4.8129 1.3551, 4.9776 -0.4723, 4.4721 -2.2361, 3.3644 -3.6987, 1.8036 -4.6634, 0 -5, -5 0)");
    ensure_equals_exact_geometry(static_cast<Geometry*>(ls.get()), expected.get(), 1e-4);

    auto ccRev = cc->reverse();
    auto lsRev = ccRev->getLinearized(CurveToLineParams::stepSizeDegrees(90.0 / 4));
    ensure_equals_exact_geometry(static_cast<Geometry*>(lsRev->reverse().get()), expected.get(), 1e-4);
}

template<>
template<>
void object::test<13>()
{
    set_test_name("getCurved()");

    // check that we return Curve* rather than Geometry*
    std::unique_ptr<Curve> curved = cc_->getCurved(geos::algorithm::LineToCurveParams::getDefault());

    ensure_equals_exact_geometry_xyzm(curved.get(), cc_.get(), 0);
}

}
