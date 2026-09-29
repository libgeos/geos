// tut
#include <tut/tut.hpp>
#include <tut/tut_macros.hpp>
// geos
#include <geos/io/WKTReader.h>
#include <geos/geom/CircularString.h>
#include <geos/geom/util/CurveBuilder.h>
#include <geos/util.h>

#include <utility.h>
// std
#include <vector>

using namespace geos::geom;
using geos::geom::util::CurveBuilder;

namespace tut {
//
// Test Group
//

// Common data used by tests
struct test_curvebuilder_data {
    geos::io::WKTReader reader_;

    GeometryFactory::Ptr factory_;
    CurveBuilder builder_;

    test_curvebuilder_data() :
        factory_(GeometryFactory::create()),
        builder_(*factory_, false, false)
    {}

    void add(const std::string& wkt) {
        auto g = reader_.read<Curve>(wkt);
        builder_.add(*g);
    }

};

typedef test_group<test_curvebuilder_data> group;
typedef group::object object;

group test_curvebuilder_group("geos::geom::util::CurveBuilder");

template<>
template<>
void object::test<1>()
{
    set_test_name("empty result");

    CurveBuilder builder(*factory_, false, true);
    auto result = builder.getGeometry();

    ensure(result->isEmpty());
    ensure_equals(result->getGeometryTypeId(), GEOS_LINESTRING);
    ensure(!result->hasZ());
    ensure(result->hasM());
}

template<>
template<>
void object::test<2>()
{
    set_test_name("CompoundCurve result");

    add("LINESTRING (-6 0, -5 0)");
    add("CIRCULARSTRING (-5 0, 0 5, 5 0)");
    add("LINESTRING (5 0, 6 0)");
    add("CIRCULARSTRING (6 0, 0 6, -6 0)");

    auto result = builder_.getGeometry();

    auto expected = reader_.read("COMPOUNDCURVE ((-6 0, -5 0), CIRCULARSTRING (-5 0, 0 5,  5 0), (5 0, 6 0), CIRCULARSTRING (6 0, 0 6, -6 0))");

    ensure_equals_geometry(static_cast<const Geometry*>(result.get()), expected.get());
}

template<>
template<>
void object::test<3>()
{
    set_test_name("LinearRing result");

    add("LINESTRING (0 0, 1 0, 1 1)");
    add("LINESTRING (1 1, 0 1, 0 0)");

    auto result = builder_.getGeometry();

    auto expected = reader_.read("LINEARRING (0 0, 1 0, 1 1, 0 1, 0 0)");

    ensure_equals_geometry(static_cast<const Geometry*>(result.get()), expected.get());
}

template<>
template<>
void object::test<4>()
{
    set_test_name("closed LineString result");

    builder_.setOutputLinearRing(false);

    add("LINESTRING (0 0, 1 0, 1 1)");
    add("LINESTRING (1 1, 0 1, 0 0)");

    auto result = builder_.getGeometry();

    auto expected = reader_.read("LINESTRING (0 0, 1 0, 1 1, 0 1, 0 0)");

    ensure_equals_geometry(static_cast<const Geometry*>(result.get()), expected.get());
}

template<>
template<>
void object::test<5>()
{
    set_test_name("empty component");

    add("LINESTRING (0 0, 1 1)");
    add("LINESTRING EMPTY");
    add("LINESTRING (1 1, 2 2)");

    auto result = builder_.getGeometry();

    auto expected = reader_.read("LINESTRING (0 0, 1 1, 2 2)");

    ensure_equals_geometry(static_cast<const Geometry*>(result.get()), expected.get());
}

template<>
template<>
void object::test<6>()
{
    set_test_name("disjoint curves");

    add("LINESTRING (-100 0, -50 0)");
    add("CIRCULARSTRING (0 0, 1 1, 2 0)");

    ensure_THROW(builder_.getGeometry(), geos::util::IllegalArgumentException);
}

template<>
template<>
void object::test<7>()
{
    set_test_name("closeRing after arc");

    auto cs = reader_.read<CircularString>("CIRCULARSTRING (0 0, 1 1, 2 0)");
    builder_.add(*cs);
    builder_.closeRing();

    auto result = builder_.getGeometry();

    auto expected = reader_.read("COMPOUNDCURVE(CIRCULARSTRING (0 0, 1 1, 2 0), (2 0, 0 0))");

    ensure_equals_geometry(static_cast<const Geometry*>(result.get()), expected.get());

}

template<>
template<>
void object::test<8>()
{
    set_test_name("closeRing after line");

    auto cs = reader_.read<LineString>("LINESTRING (0 0, 1 1, 2 0)");
    builder_.add(*cs);
    builder_.closeRing();

    auto result = builder_.getGeometry();

    auto expected = reader_.read("LINEARRING (0 0, 1 1, 2 0, 0 0)");

    ensure_equals_geometry(static_cast<const Geometry*>(result.get()), expected.get());
}

template<>
template<>
void object::test<9>()
{
    set_test_name("Z/M matched when combining mixed-dimension sequences");

    CurveBuilder builder(*factory_, true, true);
    builder.add(*reader_.read<LineString>("LINESTRING (0 0, 1 1)"));
    // Patch Z Linear <- Linear
    builder.add(*reader_.read<LineString>("LINESTRING Z (1 1 5, 2 2 4)"));
    // Patch Z Linear -> Linear
    builder.add(*reader_.read<LineString>("LINESTRING (2 2, 5 0)"));
    // Patch M Linear <- Curve
    builder.add(*reader_.read<CircularString>("CIRCULARSTRING M (5 0 8, 6 1 7, 7 0 8)"));
    // Patch M Curve -> Curve
    builder.add(*reader_.read<CircularString>("CIRCULARSTRING (7 0, 8 -1, 9 0)"));
    // Patch Z Curve <- Linear
    builder.add(*reader_.read<LineString>("LINESTRING (9 0 11, 10 20 12)"));

    auto result = builder.getGeometry();

    auto expected = reader_.read("COMPOUNDCURVE ZM ((0 0 NaN NaN, 1 1 5 NaN, 2 2 4 NaN, 5 0 NaN 8), CIRCULARSTRING ZM (5 0 NaN 8, 6 1 NaN 7, 7 0 NaN 8, 8 -1 NaN NaN, 9 0 11 NaN), (9 0 11 NaN, 10 20 12 NaN))");

    ensure_equals_geometry_xyzm(static_cast<const Geometry*>(result.get()), expected.get());
}

template<>
template<>
void object::test<10>()
{
    set_test_name("Z/M matched when calling closeRing on open CircularString");

    CurveBuilder builder(*factory_, true, true);
    builder.add(*reader_.read<CircularString>("CIRCULARSTRING ZM (0 0 100 200, 1 1 200 400, 2 0 300 600)"));
    builder.closeRing();

    auto result = builder.getGeometry();

    auto expected = reader_.read("COMPOUNDCURVE ZM (CIRCULARSTRING(0 0 100 200, 1 1 200 400, 2 0 300 600), (2 0 300 600, 0 0 100 200))");

    ensure_equals_geometry_xyzm(static_cast<const Geometry*>(result.get()), expected.get());
}

template<>
template<>
void object::test<11>()
{
    set_test_name("Z/M matched when calling closeRing on closed CircularString ZM -> LineString");

    CurveBuilder builder(*factory_, true, true);
    builder.add(*reader_.read<CircularString>("CIRCULARSTRING ZM (0 0 100 200, 1 1 200 400, 2 0 300 600)"));
    builder.add(*reader_.read<LineString>("LINESTRING (2 0, 0 0)"));

    builder.closeRing();

    auto result = builder.getGeometry();

    auto expected = reader_.read("COMPOUNDCURVE ZM (CIRCULARSTRING(0 0 100 200, 1 1 200 400, 2 0 300 600), (2 0 300 600, 0 0 100 200))");

    ensure_equals_geometry_xyzm(static_cast<const Geometry*>(result.get()), expected.get());
}

template<>
template<>
void object::test<12>()
{
    set_test_name("Z/M matched when calling closeRing on closed CircularString -> LineString ZM");

    CurveBuilder builder(*factory_, true, true);
    builder.add(*reader_.read<CircularString>("CIRCULARSTRING (0 0, 1 1, 2 0)"));
    builder.add(*reader_.read<LineString>("LINESTRING ZM (2 0 300 600, 0 0 100 200)"));

    builder.closeRing();

    auto result = builder.getGeometry();

    auto expected = reader_.read("COMPOUNDCURVE ZM (CIRCULARSTRING(0 0 100 200, 1 1 NaN NaN, 2 0 300 600), (2 0 300 600, 0 0 100 200))");

    ensure_equals_geometry_xyzm(static_cast<const Geometry*>(result.get()), expected.get());
}


} // namespace tut
