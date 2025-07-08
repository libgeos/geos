#include <utility.h>
#include <tut/tut.hpp>
#include <tut/tut_macros.hpp>

#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateFilter.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/CompoundCurve.h>
#include <geos/geom/CircularString.h>
#include <geos/geom/IntersectionMatrix.h>
#include <geos/io/WKTReader.h>
#include <geos/util.h>

using geos::geom::CompoundCurve;
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

};

typedef test_group<test_compoundcurve_data> group;
typedef group::object object;

group test_compoundcurve_group("geos::geom::CompoundCurve");

template<>
template<>
void object::test<1>()
{
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
}

// Basic Geometry API
template<>
template<>
void object::test<2>()
{
    // Geometry type functions
    ensure_equals("getGeometryType", cc_->getGeometryType(), "CompoundCurve");
    ensure_equals("getGeometryTypdId", cc_->getGeometryTypeId(), geos::geom::GEOS_COMPOUNDCURVE);
    ensure("isCollection", !cc_->isCollection());

    // Geometry size functions
    ensure("isEmpty", !cc_->isEmpty());
    ensure_equals("getArea", cc_->getArea(), 0);
    ensure_equals("getLength", cc_->getLength(), geos::MATH_PI + 2);
    ensure_equals("getNumGeometries", cc_->getNumGeometries(), 1u);
    ensure_equals("getNumCurves", cc_->getNumCurves(), 2u);
    ensure_equals("getNumPoints", cc_->getNumPoints(), 5u); // FIXME should this be 5 or 4?
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
}

// Operations
template<>
template<>
void object::test<3>()
{
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
    ensure_THROW(cc_->getBoundary(), geos::util::UnsupportedOperationException);

    ensure("clone", cc_->equalsIdentical(cc_->clone().get()));

    ensure("reverse", cc_->reverse()->equalsIdentical(wktreader_.read(""
            "COMPOUNDCURVE ((2 2, 2 0), CIRCULARSTRING (2 0, 1 1, 0 0))").get()));
    auto cc3 = cc_->reverse();
    ensure_THROW(cc3->normalize(), geos::util::UnsupportedOperationException);
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
    std::vector<std::unique_ptr<SimpleCurve>> curves;

    curves.push_back(wktreader_.read<SimpleCurve>("LINESTRING (0 0, 1 2)"));
    curves.push_back(wktreader_.read<SimpleCurve>("CIRCULARSTRING (2 1, 3 3, 4 1)"));
    ensure_THROW(factory_->createCompoundCurve(std::move(curves)), geos::util::IllegalArgumentException);

    curves.clear();
    curves.push_back(wktreader_.read<SimpleCurve>("LINESTRING (0 0, 1 2)"));
    curves.push_back(wktreader_.read<SimpleCurve>("CIRCULARSTRING EMPTY"));
    ensure_THROW(factory_->createCompoundCurve(std::move(curves)), geos::util::IllegalArgumentException);
}

}
