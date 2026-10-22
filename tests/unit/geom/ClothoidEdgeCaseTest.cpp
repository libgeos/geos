#include <tut/tut.hpp>
#include <tut/tut_macros.hpp>

#include <geos/algorithm/Area.h>
#include <geos/algorithm/CurveToLineParams.h>
#include <geos/algorithm/LineToCurveParams.h>
#include <geos/algorithm/PointLocator.h>
#include <geos/constants.h>
#include <geos/geom/Clothoid.h>
#include <geos/geom/CompoundCurve.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateFilter.h>
#include <geos/geom/CoordinateSequenceFilter.h>
#include <geos/geom/CurvePolygon.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/LineString.h>
#include <geos/geom/MultiCurve.h>
#include <geos/geom/util/CurveBuilder.h>
#include <geos/geom/util/GeometryTransformer.h>
#include <geos/io/ParseException.h>
#include <geos/io/WKBReader.h>
#include <geos/io/WKBWriter.h>
#include <geos/io/WKTReader.h>
#include <geos/io/WKTWriter.h>
#include <geos/noding/GeometryNoder.h>
#include <geos/operation/distance/DistanceOp.h>
#include <geos/operation/linemerge/LineMerger.h>
#include <geos/operation/overlayng/OverlayNG.h>
#include <geos/operation/polygonize/Polygonizer.h>
#include <geos/util/IllegalArgumentException.h>
#include <geos/util/UnsupportedOperationException.h>

#include <cmath>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

using geos::geom::Clothoid;
using geos::geom::ClothoidReferenceLocation;
using geos::geom::CompoundCurve;
using geos::geom::Coordinate;
using geos::geom::CoordinateXY;
using geos::geom::SimpleCurve;
using geos::io::ParseException;
using geos::util::IllegalArgumentException;
using geos::util::UnsupportedOperationException;

namespace tut {

struct test_clothoidedgecase_data {
    geos::geom::GeometryFactory::Ptr factory_ = geos::geom::GeometryFactory::create();
    geos::io::WKTReader reader_{*factory_};

    static ClothoidReferenceLocation at(double x, double y,
                                        double ux = 1.0, double uy = 0.0,
                                        double vx = 0.0, double vy = 1.0)
    {
        return ClothoidReferenceLocation{
            Coordinate(x, y, 0.0), Coordinate(ux, uy, 0.0), Coordinate(vx, vy, 0.0)};
    }

    std::unique_ptr<Clothoid> iso(const ClothoidReferenceLocation& loc,
                                  double scale, double t0, double t1) const
    {
        return factory_->createClothoid(loc, scale, t0, t1, false, false,
                                        geos::DoubleNotANumber, geos::DoubleNotANumber);
    }

    std::unique_ptr<Clothoid> spiral() const
    {
        return iso(at(0.0, 0.0), 2.0, 0.0, 2.0);
    }

    // The spiral closed by a chord back to its start.
    std::unique_ptr<CompoundCurve> closedSpiral() const
    {
        auto curve = spiral();
        const CoordinateXY start = curve->getStartCoordinate();
        const CoordinateXY end = curve->getEndCoordinate();
        std::vector<std::unique_ptr<SimpleCurve>> sections;
        sections.push_back(std::move(curve));
        sections.push_back(factory_->createLineString({end, start}));
        return factory_->createCompoundCurve(std::move(sections));
    }

    static std::string hexOf(const geos::geom::Geometry& g)
    {
        geos::io::WKBWriter writer;
        writer.setOutputDimension(4);
        writer.setByteOrder(1);
        std::stringstream hex;
        writer.writeHEX(g, hex);
        return hex.str();
    }

    std::unique_ptr<geos::geom::Geometry> fromHex(const std::string& hex) const
    {
        geos::io::WKBReader reader(*factory_);
        return reader.readHEX(hex);
    }
};

typedef test_group<test_clothoidedgecase_data> group;
typedef group::object object;

group test_clothoidedgecase_group("geos::geom::ClothoidEdgeCase");

template<>
template<>
void object::test<1>()
{
    set_test_name("stored parameters");

    auto curve = factory_->createClothoid(at(1.0, 2.0), -3.0, -1.0, 4.0, false, true, 5.0, 9.0);
    ensure_equals(curve->getScaleFactor(), -3.0);
    ensure_equals(curve->getStartDistance(), -1.0);
    ensure_equals(curve->getEndDistance(), 4.0);
    ensure_equals(curve->getStartM(), 5.0);
    ensure_equals(curve->getEndM(), 9.0);
    ensure(curve->isCurved());
    ensure(curve->hasCurvedComponents());
    ensure_equals(curve->getCoordinatesRO()->getAt<geos::geom::CoordinateXYZM>(1).m, 9.0);

    auto copy = curve->clone();
    ensure(copy->equalsIdentical(curve.get()));
    auto curved = curve->getCurved(geos::algorithm::LineToCurveParams());
    ensure(curved->equalsIdentical(curve.get()));

    const auto before = *curve->getEnvelopeInternal();
    curve->geometryChanged();
    ensure(curve->getEnvelopeInternal()->equals(&before));
}

template<>
template<>
void object::test<2>()
{
    set_test_name("empty clothoid accessors");

    auto empty = factory_->createClothoid(true, true);
    ensure_equals(empty->getStartHeading(), 0.0);
    ensure_equals(empty->getEndHeading(), 0.0);
    ensure_equals(empty->getStartCurvature(), 0.0);
    ensure_equals(empty->getEndCurvature(), 0.0);
    ensure(!empty->hasCurvedComponents());
    ensure(empty->reverse()->isEmpty());
    auto line = empty->getLinearized(geos::algorithm::CurveToLineParams());
    ensure(line->isEmpty());
    ensure_equals(line->getGeometryTypeId(), geos::geom::GEOS_LINESTRING);
    empty->normalize();
    ensure(empty->isEmpty());

    auto fromType = factory_->createEmptyGeometry(geos::geom::GEOS_CLOTHOID, true, false);
    ensure_equals(fromType->getGeometryTypeId(), geos::geom::GEOS_CLOTHOID);
    ensure(fromType->hasZ());
    auto plain = factory_->createEmpty(geos::geom::GEOS_CLOTHOID);
    ensure_equals(plain->getGeometryTypeId(), geos::geom::GEOS_CLOTHOID);

    geos::io::WKTWriter writer;
    ensure_equals(writer.write(empty.get()), "CLOTHOID ZM EMPTY");
}

template<>
template<>
void object::test<3>()
{
    set_test_name("zero-length clothoid");

    auto point = iso(at(1.0, 1.0), 2.0, 1.0, 1.0);
    ensure_equals(point->getLength(), 0.0);
    ensure(!point->hasCurvedComponents());
    ensure(point->getStartCoordinate().equals2D(point->getEndCoordinate()));
    ensure_equals(point->getEnvelopeInternal()->getWidth(), 0.0);
    auto line = point->getLinearized(geos::algorithm::CurveToLineParams());
    ensure_equals(line->getNumPoints(), 2u);

    auto sheared = iso(at(1.0, 1.0, 2.0, 0.0, 0.5, 1.0), 2.0, 1.0, 1.0);
    ensure_equals(sheared->getLength(), 0.0);
}

template<>
template<>
void object::test<4>()
{
    set_test_name("linearization limits");

    // Curvature underflows to zero: one straight segment.
    auto straight = iso(at(0.0, 0.0), 1e100, 1e-130, 3e-130);
    auto line = straight->getLinearized(geos::algorithm::CurveToLineParams::maxDeviation(1e-3));
    ensure_equals(line->getNumPoints(), 2u);

    auto curve = spiral();
    ensure_THROW(curve->getLinearized(geos::algorithm::CurveToLineParams::maxDeviation(1e-300)),
                 IllegalArgumentException);
}

template<>
template<>
void object::test<5>()
{
    set_test_name("invalid construction");

    const auto loc = at(0.0, 0.0);
    const double inf = std::numeric_limits<double>::infinity();
    const double nan = geos::DoubleNotANumber;
    ensure_THROW(iso(loc, 0.0, 0.0, 1.0), IllegalArgumentException);
    ensure_THROW(iso(loc, 1e-101, 0.0, 1e-101), IllegalArgumentException);
    ensure_THROW(iso(loc, 1e101, 0.0, 1.0), IllegalArgumentException);
    ensure_THROW(iso(loc, nan, 0.0, 1.0), IllegalArgumentException);
    ensure_THROW(iso(loc, 1.0, 0.0, inf), IllegalArgumentException);
    ensure_THROW(iso(loc, 1.0, nan, 1.0), IllegalArgumentException);
    // 45.25^2 / 2 is 1023.8 rad, just inside the 1024 rad limit.
    ensure_NO_THROW(iso(loc, 1.0, 0.0, 45.25));
    ensure_THROW(iso(loc, 1.0, 0.0, 45.26), IllegalArgumentException);
    ensure_THROW(iso(loc, 1.0, -45.26, 0.0), IllegalArgumentException);
    // Past 1e15 A the heading cannot be reduced; only a point gets that far.
    ensure_NO_THROW(iso(loc, 1.0, 1e15, 1e15));
    ensure_THROW(iso(loc, 1.0, 1.01e15, 1.01e15), IllegalArgumentException);
    ensure_THROW(iso(loc, 1e-100, -1e-84, -1e-84), IllegalArgumentException);
    ensure_THROW(iso(at(nan, 0.0), 1.0, 0.0, 1.0), IllegalArgumentException);
    ensure_THROW(iso(at(0.0, 0.0, inf, 0.0), 1.0, 0.0, 1.0), IllegalArgumentException);
    ensure_THROW(factory_->createClothoid(loc, 1.0, 0.0, 1.0, false, true, nan, 1.0),
                 IllegalArgumentException);
    ensure_THROW(factory_->createClothoid(loc, 1.0, 0.0, 1.0, false, true, 0.0, inf),
                 IllegalArgumentException);
}

template<>
template<>
void object::test<6>()
{
    set_test_name("coordinate filters are rejected");

    struct NoOpFilter : public geos::geom::CoordinateFilter {
    } filter;
    struct NoOpSequenceFilter : public geos::geom::CoordinateSequenceFilter {
        bool isDone() const override { return true; }
        bool isGeometryChanged() const override { return false; }
    } sequenceFilter;

    auto curve = spiral();
    ensure_THROW(curve->apply_rw(&filter), UnsupportedOperationException);
    ensure_THROW(curve->apply_rw(sequenceFilter), UnsupportedOperationException);
}

template<>
template<>
void object::test<7>()
{
    set_test_name("compareTo and equality on parameters");

    // A and -A give the same points, so only the stored parameters differ.
    auto positive = iso(at(0.0, 0.0), 3.0, -2.0, 4.0);
    auto negative = iso(at(0.0, 0.0), -3.0, -2.0, 4.0);
    ensure(positive->getCoordinatesRO()->getAt<CoordinateXY>(1)
           == negative->getCoordinatesRO()->getAt<CoordinateXY>(1));
    ensure_equals(positive->compareTo(positive->clone().get()), 0);
    ensure_equals(negative->compareTo(positive.get()), -1);
    ensure_equals(positive->compareTo(negative.get()), 1);
    ensure(!positive->equalsExact(negative.get(), 1.0));
    ensure(positive->equalsExact(negative.get(), 6.0));
    ensure(!positive->equalsIdentical(negative.get()));
    ensure(!positive->equalsIdentical(spiral().get()));

    auto line = factory_->createLineString({CoordinateXY(0.0, 0.0), CoordinateXY(1.0, 0.0)});
    ensure(positive->compareTo(line.get()) != 0);
    ensure_equals(positive->compareTo(line.get()), -line->compareTo(positive.get()));
}

template<>
template<>
void object::test<8>()
{
    set_test_name("normalize");

    auto backward = iso(at(10.0, 0.0, -1.0, 0.0, 0.0, 1.0), 2.0, 0.0, 2.0);
    ensure(backward->getEndCoordinate().compareTo(backward->getStartCoordinate()) < 0);
    auto expected = backward->reverse();
    backward->normalize();
    ensure(backward->equalsIdentical(expected.get()));

    auto forward = spiral();
    auto unchanged = forward->clone();
    forward->normalize();
    ensure(forward->equalsIdentical(unchanged.get()));
}

template<>
template<>
void object::test<9>()
{
    set_test_name("degenerate placement");

    // Both directions are vertical, so there is no plan tangent.
    ClothoidReferenceLocation vertical{
        Coordinate(1.0, 2.0, 3.0), Coordinate(0.0, 0.0, 1.0), Coordinate(0.0, 0.0, 2.0)};
    auto curve = factory_->createClothoid(vertical, 1.0, 0.0, 1.0, true, false,
                                          geos::DoubleNotANumber, geos::DoubleNotANumber);
    ensure_equals(curve->getStartHeading(), 0.0);
    ensure_equals(curve->getEndHeading(), 0.0);
    ensure(curve->getStartCoordinate().equals2D(CoordinateXY(1.0, 2.0)));
    ensure(curve->getEndCoordinate().equals2D(CoordinateXY(1.0, 2.0)));
    ensure(!curve->hasCurvedComponents());
}

template<>
template<>
void object::test<10>()
{
    set_test_name("WKB with M");

    auto curve = factory_->createClothoid(at(1.0, 2.0), 2.0, -1.0, 3.0, false, true, 5.0, 9.0);
    auto back = fromHex(hexOf(*curve));
    ensure(curve->equalsIdentical(back.get()));

    auto zm = factory_->createClothoid(
                  ClothoidReferenceLocation{Coordinate(1.0, 2.0, 3.0), Coordinate(1.0, 0.0, 0.25),
                                            Coordinate(0.0, 1.0, 0.0)},
                  2.0, -1.0, 3.0, true, true, 5.0, 9.0);
    auto zmBack = fromHex(hexOf(*zm));
    ensure(zm->equalsIdentical(zmBack.get()));
}

template<>
template<>
void object::test<11>()
{
    set_test_name("WKB empty clothoid");

    auto empty = factory_->createClothoid(false, false);
    auto back = fromHex(hexOf(*empty));
    ensure(back->isEmpty());
    ensure_equals(back->getGeometryTypeId(), geos::geom::GEOS_CLOTHOID);

    std::vector<std::unique_ptr<geos::geom::Geometry>> parts;
    parts.push_back(factory_->createClothoid(false, false));
    parts.push_back(factory_->createPoint(CoordinateXY(1.0, 2.0)));
    auto collection = factory_->createGeometryCollection(std::move(parts));
    auto collectionBack = fromHex(hexOf(*collection));
    ensure_equals(collectionBack->getNumGeometries(), 2u);
    ensure(collectionBack->getGeometryN(0)->isEmpty());
    ensure_equals(collectionBack->getGeometryN(1)->getGeometryTypeId(), geos::geom::GEOS_POINT);
}

template<>
template<>
void object::test<12>()
{
    set_test_name("WKB errors");

    // Little endian 2D clothoid: order (1 byte), type (4), placement order (1),
    // placement type (4), location (16), direction count (4), directions (32), ...
    const std::string hex = hexOf(*spiral());
    ensure_equals(hex.substr(0, 10), "0116000000");
    ensure_equals(hex.substr(10, 10), "0166000000");
    ensure_equals(hex.substr(52, 8), "02000000");

    auto withAt = [&hex](std::size_t byte, const std::string& replacement) {
        std::string changed = hex;
        changed.replace(2 * byte, replacement.size(), replacement);
        return changed;
    };
    // Not a byte order.
    ensure_THROW(fromHex(withAt(5, "07")), ParseException);
    // Neither a placement nor a geometry type.
    ensure_THROW(fromHex(withAt(6, "e7030000")), ParseException);
    // A Z placement in a 2D clothoid.
    ensure_THROW(fromHex(withAt(6, "4e040000")), ParseException);
    // An M placement.
    ensure_THROW(fromHex(withAt(6, "36080000")), ParseException);
    // Three reference directions.
    ensure_THROW(fromHex(withAt(26, "03000000")), ParseException);
    // Truncated distances.
    ensure_THROW(fromHex(hex.substr(0, hex.size() - 16)), ParseException);
}

template<>
template<>
void object::test<13>()
{
    set_test_name("WKT errors");

    const std::string placement =
        "(LOCATION (0 0), REFERENCEDIRECTIONS (VECTOR (1 0), VECTOR (0 1)))";
    const std::string tail = ", SCALEFACTOR 1, STARTDISTANCE 0, ENDDISTANCE 1)";
    const std::vector<std::string> bad = {
        "CLOTHOID (AFFINEPLACEMENT " + placement + tail,
        "CLOTHOID (REFERENCELOCATION PLACEMENT " + placement + tail,
        "CLOTHOID (REFERENCELOCATION AFFINEPLACEMENT EMPTY" + tail,
        "CLOTHOID (REFERENCELOCATION AFFINEPLACEMENT LOCATION (0 0)" + tail,
        "CLOTHOID Z (REFERENCELOCATION AFFINEPLACEMENT " + placement + tail,
        "CLOTHOID (REFERENCELOCATION AFFINEPLACEMENT Z " + placement + tail,
        "CLOTHOID (REFERENCELOCATION AFFINEPLACEMENT (ORIGIN (0 0), "
        "REFERENCEDIRECTIONS (VECTOR (1 0), VECTOR (0 1)))" + tail,
        "CLOTHOID (REFERENCELOCATION AFFINEPLACEMENT (LOCATION Z (0 0 0), "
        "REFERENCEDIRECTIONS (VECTOR (1 0), VECTOR (0 1)))" + tail,
        "CLOTHOID Z (REFERENCELOCATION AFFINEPLACEMENT Z (LOCATION (0 0), "
        "REFERENCEDIRECTIONS (VECTOR Z (1 0 0), VECTOR Z (0 1 0)))" + tail,
        "CLOTHOID (REFERENCELOCATION AFFINEPLACEMENT (LOCATION 0 0, "
        "REFERENCEDIRECTIONS (VECTOR (1 0), VECTOR (0 1)))" + tail,
        "CLOTHOID (REFERENCELOCATION AFFINEPLACEMENT " + placement
        + ", SCALEFACTOR 1, STARTDISTANCE 0, ENDDISTANCE 1 STARTM 0)",
    };
    for (const auto& wkt : bad) {
        ensure_THROW(reader_.read(wkt), ParseException);
    }
    ensure_NO_THROW(reader_.read("CLOTHOID (REFERENCELOCATION AFFINEPLACEMENT " + placement + tail));
}

template<>
template<>
void object::test<14>()
{
    set_test_name("WKT inside a collection");

    std::vector<std::unique_ptr<geos::geom::Curve>> curves;
    curves.push_back(spiral());
    curves.push_back(factory_->createClothoid(false, false));
    auto multi = factory_->createMultiCurve(std::move(curves));
    geos::io::WKTWriter writer;
    writer.setRoundingPrecision(17);
    const std::string text = writer.write(multi.get());
    ensure(text.find("MULTICURVE (CLOTHOID (REFERENCELOCATION") == 0);
    ensure(text.find("CLOTHOID EMPTY") != std::string::npos);
    ensure(multi->equalsIdentical(reader_.read(text).get()));

    auto polygon = factory_->createCurvePolygon(closedSpiral());
    const std::string polygonText = writer.write(polygon.get());
    ensure(polygonText.find("CURVEPOLYGON (COMPOUNDCURVE (CLOTHOID") == 0);
}

template<>
template<>
void object::test<15>()
{
    set_test_name("operations without clothoid support");

    using geos::operation::overlayng::OverlayNG;
    auto curve = spiral();
    auto line = factory_->createLineString({CoordinateXY(0.0, -1.0), CoordinateXY(1.0, 1.0)});
    auto closed = closedSpiral();
    auto polygon = factory_->createCurvePolygon(closed->clone());
    auto square = reader_.read("POLYGON ((-1 -1, 3 -1, 3 3, -1 3, -1 -1))");

    ensure_THROW(geos::algorithm::Area::ofClosedCurve(*closed), UnsupportedOperationException);
    ensure_THROW(OverlayNG::overlay(curve.get(), line.get(), OverlayNG::INTERSECTION),
                 UnsupportedOperationException);
    ensure_THROW(OverlayNG::overlay(polygon.get(), square.get(), OverlayNG::INTERSECTION),
                 UnsupportedOperationException);
    ensure_THROW(geos::operation::distance::DistanceOp::distance(*curve, *line),
                 UnsupportedOperationException);
    ensure_THROW(geos::operation::distance::DistanceOp::distance(*line, *curve),
                 UnsupportedOperationException);

    geos::algorithm::PointLocator locator;
    ensure_THROW(locator.locate(CoordinateXY(0.5, 0.5), curve.get()), UnsupportedOperationException);

    geos::operation::polygonize::Polygonizer polygonizer;
    ensure_THROW(polygonizer.add(static_cast<const geos::geom::Geometry*>(curve.get())),
                 UnsupportedOperationException);

    geos::operation::linemerge::LineMerger merger;
    merger.add(static_cast<const geos::geom::Geometry*>(curve.get()));
    ensure_THROW(merger.getMergedCurves(), UnsupportedOperationException);

    ensure_THROW(geos::noding::GeometryNoder::node(*curve), UnsupportedOperationException);

    geos::geom::util::CurveBuilder builder(*factory_, false, false);
    ensure_THROW(builder.add(*curve), UnsupportedOperationException);

    geos::geom::util::GeometryTransformer transformer;
    ensure_THROW(transformer.transform(curve.get()), UnsupportedOperationException);
    ensure_THROW(transformer.transform(closed.get()), UnsupportedOperationException);
}

} // namespace tut
