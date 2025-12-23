#include <tut/tut.hpp>
#include <tut/tut_macros.hpp>

#include <geos/geom/CircularString.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/IntersectionMatrix.h>
#include <geos/io/WKTReader.h>
#include <geos/util/UnsupportedOperationException.h>

#include "utility.h"
#include "geos/geom/CircularArc.h"

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

    void checkLinearizeMaxDeviation(const std::string& wkt_in, const std::string& wkt_expected, double maxDeviation, double tol=1e-12) {
        cs_ = wktreader_.read<CircularString>(wkt_in);
        auto seq = cs_->getCoordinatesRO();
        geos::geom::CircularArc arc(*seq, 0);
        double radius = arc.getRadius();

        double stepSizeDegrees = getDegreesFromSagitta(radius, maxDeviation);
        checkLinearize(wkt_in, wkt_expected, stepSizeDegrees, tol);
    }

    void checkLinearize(const std::string& wkt_in, const std::string& wkt_expected, double stepSizeDegrees, double tol=1e-12) {
        cs_ = wktreader_.read<CircularString>(wkt_in);

        auto ls = cs_->getLinearized(stepSizeDegrees);

        auto expected = wktreader_.read(wkt_expected);

        ensure_equals_exact_geometry_xyzm(ls.get(), expected.get(), tol);

        auto csRev = cs_->reverse();
        auto lsRev = csRev->getLinearized(stepSizeDegrees);
        auto lsRevRev = lsRev->reverse();

        ensure_equals_exact_geometry_xyzm(lsRevRev.get(), expected.get(), tol);
    }

    static double getDegreesFromSagitta(double radius, double sagitta) {
        return std::acos(1 - sagitta / radius) * 360 / geos::MATH_PI;
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

template<>
template<>
void object::test<6>()
{
    set_test_name("half-circle, evenly divisible by requested step size");

    // expected result from GDAL 3.12
    checkLinearize("CIRCULARSTRING (0 0, 1 1, 2 0)",
    "LINESTRING (0 0, 0.002435949740176 0.069756473744125, 0.009731931258429 0.139173100960061, 0.021852399266194 0.20791169081776, 0.038738304061681 0.275637355817011, 0.060307379214091 0.342020143325669, 0.086454542357401 0.406736643075803, 0.117052407141074 0.469471562785898, 0.151951903843575 0.529919264233229, 0.190983005625057 0.587785252292491, 0.233955556881021 0.642787609686564, 0.280660199661355 0.694658370459024, 0.330869393641151 0.743144825477401, 0.384338524674348 0.788010753606727, 0.440807096529255 0.829037572555052, 0.5 0.866025403784448, 0.561628853210948 0.898794046299173, 0.625393406584095 0.927183854566806, 0.690983005625071 0.951056516295154, 0.75807810440034 0.970295726276021, 0.826351822333095 0.984807753012234, 0.895471536732373 0.99452189536828, 0.965100503297521 0.999390827019113, 1.03489949670251 0.999390827019113, 1.10452846326768 0.99452189536828, 1.17364817766696 0.984807753012234, 1.24192189559972 0.970295726276021, 1.30901699437499 0.951056516295182, 1.37460659341593 0.927183854566806, 1.43837114678911 0.898794046299173, 1.50000000000006 0.866025403784448, 1.55919290347077 0.829037572555052, 1.61566147532568 0.788010753606727, 1.66913060635886 0.743144825477401, 1.71933980033867 0.694658370459024, 1.76604444311903 0.642787609686564, 1.80901699437499 0.587785252292491, 1.84804809615645 0.529919264233229, 1.88294759285895 0.469471562785898, 1.91354545764261 0.406736643075803, 1.93969262078593 0.342020143325669, 1.96126169593833 0.275637355817011, 1.97814760073385 0.20791169081776, 1.99026806874161 0.139173100960068, 1.99756405025983 0.069756473744128, 2 0)",
    4);
}

template<>
template<>
void object::test<7>()
{
    set_test_name("half-circle, not divisible by requested step size");

    // expected result from PostGIS 3.6
    // SELECT ST_AsText(ST_CurveToLine('CIRCULARSTRING (0 0, 1 1, 2 0)', radians(23), 2, 1))
    checkLinearize("CIRCULARSTRING (0 0, 1 1, 2 0)",
    "LINESTRING(0 0,0.076120467488713 0.38268343236509,0.292893218813453 0.707106781186548,0.61731656763491 0.923879532511287,1 1,1.38268343236509 0.923879532511287,1.707106781186548 0.707106781186548,1.923879532511287 0.38268343236509,2 0)",
    23);
}

template<>
template<>
void object::test<8>()
{
    set_test_name("liblwgeom: 2 segments per quadrant");
    // SELECT ST_AsText(ST_CurveToLine('CIRCULARSTRING (0 0, 100 100, 200 0)', radians(45), 2, 1), 4);
    checkLinearize("CIRCULARSTRING(0 0,100 100,200 0)",
        "LINESTRING(0 0,29.2893 70.7107,100 100,170.7107 70.7107,200 0)", 90.0 / 2, 1e-4);
}

template<>
template<>
void object::test<9>()
{
    set_test_name("liblwgeom: 3 segments per quadrant");
    // SELECT ST_AsText(ST_CurveToLine('CIRCULARSTRING (0 0, 100 100, 200 0)', radians(90.0 / 3), 2, 1), 4);
    checkLinearize("CIRCULARSTRING(0 0,100 100,200 0)",
        "LINESTRING(0 0,13.3975 50,50 86.6025,100 100,150 86.6025,186.6025 50,200 0)", 90.0 / 3, 1e-4);
}

template<>
template<>
void object::test<10>()
{
    set_test_name("liblwgeom: 2 segments per quadrant");
    // SELECT ST_AsText(ST_CurveToLine('CIRCULARSTRING(29.2893218813453 70.7106781186548,100 100,200 0)', radians(90.0 / 2), 2, 1), 4);
    checkLinearize("CIRCULARSTRING(29.2893218813453 70.7106781186548,100 100,200 0)",
                "LINESTRING(29.2893 70.7107,100 100,170.7107 70.7107,200 0)", 90.0 / 2, 1e-4);
}

template<>
template<>
void object::test<11>()
{
    set_test_name("liblwgeom: 3 segments per quadrant - symmetric");
    // SELECT ST_AsText(ST_CurveToLine('CIRCULARSTRING(29.2893218813453 70.7106781186548,100 100,200 0)', radians(90.0 / 3), 2, 1), 4);
    checkLinearize("CIRCULARSTRING(29.2893218813453 70.7106781186548,100 100,200 0)",
        "LINESTRING(29.2893 70.7107,69.0983 95.1057,115.6434 98.7688,158.7785 80.9017,189.1007 45.399,200 0)", 90.0 / 3, 1e-4);
}

template<>
template<>
void object::test<12>() {
    set_test_name("liblwgeom: 10 segments per quadrant - circular");
    auto cs = wktreader_.read<CircularString>("CIRCULARSTRING (0 0, 1 0, 0 0)");
    auto ls = cs->getLinearized(90.0 / 10);
    ensure_equals(ls->getNumPoints(), 41u); // PostGIS test has 40, but this is incorrect
    const auto* seq = ls->getCoordinatesRO();
    seq->forEachSegment([](const auto& p0, const auto& p1) {
        ensure_equals("segment has expected length", p0.distance(p1), geos::MATH_PI/40, 0.01);
    });
}

template<>
template<>
void object::test<13>() {
    set_test_name("liblwgeom: maximum 10 units difference, symmetric");
    checkLinearizeMaxDeviation("CIRCULARSTRING(0 0,100 100,200 0)",
        "LINESTRING(0 0,30 70,100 100,170 70,200 0)", 10, 1.4);
}

template<>
template<>
void object::test<14>() {
    set_test_name("liblwgeom: maximum 20 units difference, symmetric");
    checkLinearizeMaxDeviation("CIRCULARSTRING(0 0,100 100,200 0)",
        "LINESTRING(0 0,50 86,150 86,200 0)", 20, 1.4);
}

template<>
template<>
void object::test<15>() {
    set_test_name("liblwgeom: ticket #3772 (1)");
    checkLinearizeMaxDeviation("CIRCULARSTRING(71.96 -65.64,22.2 -18.52,20 50)",
    "LINESTRING(72 -66,34 -38,16 4,20 50)", 4, 1.4);
}

template<>
template<>
void object::test<16>() {
    set_test_name("liblwgeom: ticket #3772 (2)");
    checkLinearizeMaxDeviation("CIRCULARSTRING(20 50,22.2 -18.52,71.96 -65.64)",
    "LINESTRING(20 50,16 4,34 -38,72 -66)", 4, 1.4);
}

template<>
template<>
void object::test<17>() {
    set_test_name("liblwgeom: ticket #4031, max deviation > 2*radius");
    checkLinearizeMaxDeviation("CIRCULARSTRING(20 50,22.2 -18.52,71.96 -65.64)",
    "LINESTRING(20 50,22 -18,72 -66)", 500, 1.4);
}

template<>
template<>
void object::test<18>() {
    //set_test_name("liblwgeom: ticket #4058, big radius, small tolerance");
    //checkLinearizeMaxDeviation("CIRCULARSTRING(2696000.553 1125699.831999999936670, 2695950.552000000141561 1125749.833000000100583, 2695865.195999999996275 1125835.189000)",
    //"LINESTRING(2696000 1125700,2695932 1125768,2695866 1125836)", 0.0001);
}

template<>
template<>
void object::test<19>() {
    set_test_name("liblwgeom: direction neutrality");
    auto cs = wktreader_.read("CIRCULARSTRING(71.96 -65.64,22.2 -18.52,20 50)");
    auto csRev = cs->reverse();

    auto ls1 = cs->getLinearized(90.0 / 4);
    auto ls2 = cs->reverse()->getLinearized(90.0 / 4)->reverse();

    ensure("Linearization of reversed CIRCULARSTRING is not direction neutral", ls1->equalsExact(ls2.get(), 0.0));
}

template<>
template<>
void object::test<20>() {
    set_test_name("getLinearized() called on linear arc");

    checkLinearize("CIRCULARSTRING(0 0, 2 1, 4 2)",
        "LINESTRING (0 0, 2 1, 4 2)", 90.0 / 4);
}

template<>
template<>
void object::test<21>() {
    set_test_name("getLinearized() called on multi-section CircularString");

    checkLinearize("CIRCULARSTRING (0 0, 1 1, 2 0, 3 -1, 4 0)",
    "LINESTRING (0 0, 0.0761 0.3827, 0.2929 0.7071, 0.6173 0.9239, 1 1, 1.3827 0.9239, 1.7071 0.7071, 1.9239 0.3827, 2 0, 2.0761 -0.3827, 2.2929 -0.7071, 2.6173 -0.9239, 3 -1, 3.3827 -0.9239, 3.7071 -0.7071, 3.9239 -0.3827, 4 0)",
    90.0 / 4, 1e-4);
}

template<>
template<>
void object::test<22>()
{
    set_test_name("getLinearized() on various CircularString base classes");

    auto cs = wktreader_.read<CircularString>("CIRCULARSTRING(0 0, 1 1, 2 0)");

    // check that we return LineString* rather than Curve* or Geometry*
    std::unique_ptr<LineString> linearized = cs->getLinearized(45);

    ensure_equals("CircularString::getLinearized", linearized.get()->getGeometryTypeId(), geos::geom::GEOS_LINESTRING);
    ensure_equals("SimpleCurve::getLinearized", static_cast<geos::geom::SimpleCurve*>(cs.get())->getLinearized(45)->getGeometryTypeId(), geos::geom::GEOS_LINESTRING);
    ensure_equals("Curve::getLinearized", static_cast<geos::geom::Curve*>(cs.get())->getLinearized(45)->getGeometryTypeId(), geos::geom::GEOS_LINESTRING);
    ensure_equals("Geometry::getLinearized", static_cast<Geometry*>(cs.get())->getLinearized(45)->getGeometryTypeId(), geos::geom::GEOS_LINESTRING);
}

template<>
template<>
void object::test<23>()
{
    set_test_name("getCurved()");

    auto cs = wktreader_.read<CircularString>("CIRCULARSTRING(0 0, 1 1, 2 0)");

    // Check that we return Curve* rather than Geometry*
    std::unique_ptr<geos::geom::Curve> curved = cs->getCurved(1000);

    ensure_equals_exact_geometry_xyzm(curved.get(), cs.get(), 0);
}

// TODO: Test Z, M interpolation in getLinearized()

}
