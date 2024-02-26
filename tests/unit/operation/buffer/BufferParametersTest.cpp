//
// Test Suite for geos::operation::buffer::BufferParameters class.

// tut
#include <tut/tut.hpp>
#include <utility.h>
// geos
#include <geos/operation/buffer/BufferParameters.h>
#include <geos/operation/buffer/BufferOp.h>
#include <geos/geom/Geometry.h>
#include <geos/io/WKTReader.h>
#include <geos/constants.h>
// std
#include <memory>

namespace tut {
//
// Test Group
//

using geos::operation::buffer::BufferParameters;
using geos::operation::buffer::BufferOp;
using geos::geom::Geometry;

// Common data used by tests
struct test_bufferparameters_data {

    geos::io::WKTReader _reader;

    test_bufferparameters_data() { }

    void
    checkBuffer(const std::string& wkt, double dist, int quadSegs, const std::string& wktExpected)
    {
        checkBuffer(wkt, dist, quadSegs, BufferParameters::JOIN_ROUND, wktExpected);
    }

    void
    checkBuffer(const std::string& wkt, double dist, int quadSegs, BufferParameters::JoinStyle joinStyle, const std::string& wktExpected)
    {
        BufferParameters param;
        param.setQuadrantSegments(quadSegs);
        param.setJoinStyle(joinStyle);
        checkBuffer(wkt, dist, param, wktExpected);
    }

    void
    checkBuffer(const std::string& wkt, double dist, BufferParameters param, const std::string& wktExpected)
    {
        std::unique_ptr<Geometry> geom = _reader.read(wkt);
        std::unique_ptr<Geometry> result = BufferOp::bufferOp(geom.get(), dist, param);
        std::unique_ptr<Geometry> expected = _reader.read(wktExpected);
        ensure_equals_geometry(expected.get(), result.get(), 0.00001);
    }

    BufferParameters
    bufParamFlatMitre(double mitreLimit)
    {
        BufferParameters param;
        param.setJoinStyle(BufferParameters::JOIN_MITRE);
        param.setMitreLimit(mitreLimit);
        param.setEndCapStyle(BufferParameters::CAP_FLAT);
        return param;
    }

private:
    // noncopyable
    test_bufferparameters_data(test_bufferparameters_data const& other) = delete;
    test_bufferparameters_data& operator=(test_bufferparameters_data const& rhs) = delete;
};

typedef test_group<test_bufferparameters_data> group;
typedef group::object object;

group test_bufferparameters_group("geos::operation::buffer::BufferParameters");

//
// Test Cases
//

// Default constructor
template<>
template<>
void object::test<1>()
{
    BufferParameters bp;

    ensure_equals(bp.getEndCapStyle(), BufferParameters::CAP_ROUND);
    ensure_equals(bp.getJoinStyle(), BufferParameters::JOIN_ROUND);
    ensure_equals(bp.getMitreLimit(), 5.0);
    ensure_equals(bp.getQuadrantSegments(), int(8));
    ensure(! bp.isSingleSided());
}

// Constructor with single integer argument
template<>
template<>
void object::test<2>()
{
    BufferParameters bp(16);

    ensure_equals(bp.getEndCapStyle(), BufferParameters::CAP_ROUND);
    ensure_equals(bp.getJoinStyle(), BufferParameters::JOIN_ROUND);
    ensure_equals(bp.getMitreLimit(), 5.0);
    ensure_equals(bp.getQuadrantSegments(), int(16));
    ensure(! bp.isSingleSided());
}

// Constructor with quadrantSegments and EndCapStyle
template<>
template<>
void object::test<3>()
{
    BufferParameters bp(16, BufferParameters::CAP_FLAT);

    ensure_equals(bp.getEndCapStyle(), BufferParameters::CAP_FLAT);
    ensure_equals(bp.getJoinStyle(), BufferParameters::JOIN_ROUND);
    ensure_equals(bp.getMitreLimit(), 5.0);
    ensure_equals(bp.getQuadrantSegments(), int(16));
    ensure(! bp.isSingleSided());
}

// Constructor with quadrantSegments and EndCapStyle (2)
template<>
template<>
void object::test<4>()
{
    BufferParameters bp(16, BufferParameters::CAP_ROUND);

    ensure_equals(bp.getEndCapStyle(), BufferParameters::CAP_ROUND);
    ensure_equals(bp.getJoinStyle(), BufferParameters::JOIN_ROUND);
    ensure_equals(bp.getMitreLimit(), 5.0);
    ensure_equals(bp.getQuadrantSegments(), int(16));
    ensure(! bp.isSingleSided());
}

// Constructor with quadrantSegments, EndCapStyle, JoinStyle and mitreLimit
template<>
template<>
void object::test<5>()
{
    BufferParameters bp(31, BufferParameters::CAP_SQUARE,
                        BufferParameters::JOIN_MITRE,
                        2.0);

    ensure_equals(bp.getEndCapStyle(), BufferParameters::CAP_SQUARE);
    ensure_equals(bp.getJoinStyle(), BufferParameters::JOIN_MITRE);
    ensure_equals(bp.getMitreLimit(), 2.0);
    ensure_equals(bp.getQuadrantSegments(), int(31));
    ensure(! bp.isSingleSided());
}

// setQuadrantSegments and getQuadrantSegments
template<>
template<>
void object::test<6>()
{
    BufferParameters bp;
    ensure_equals(bp.getQuadrantSegments(), int(8));
    bp.setQuadrantSegments(16);
    ensure_equals(bp.getQuadrantSegments(), int(16));
    bp.setQuadrantSegments(3);
    ensure_equals(bp.getQuadrantSegments(), int(3));

    ensure_equals(bp.getEndCapStyle(), BufferParameters::CAP_ROUND);
    ensure_equals(bp.getJoinStyle(), BufferParameters::JOIN_ROUND);
    ensure_equals(bp.getMitreLimit(), 5.0);
    ensure(! bp.isSingleSided());
}

// setEndCapStyle and getEndCapStyle
template<>
template<>
void object::test<7>()
{
    BufferParameters bp;
    ensure_equals(bp.getEndCapStyle(), BufferParameters::CAP_ROUND);
    bp.setEndCapStyle(BufferParameters::CAP_FLAT);
    ensure_equals(bp.getEndCapStyle(), BufferParameters::CAP_FLAT);
    bp.setEndCapStyle(BufferParameters::CAP_SQUARE);
    ensure_equals(bp.getEndCapStyle(), BufferParameters::CAP_SQUARE);

    ensure_equals(bp.getJoinStyle(), BufferParameters::JOIN_ROUND);
    ensure_equals(bp.getMitreLimit(), 5.0);
    ensure_equals(bp.getQuadrantSegments(), int(8));
    ensure(! bp.isSingleSided());
}

// setJoinStyle and getJoinStyle
template<>
template<>
void object::test<8>()
{
    BufferParameters bp;
    ensure_equals(bp.getJoinStyle(), BufferParameters::JOIN_ROUND);
    bp.setJoinStyle(BufferParameters::JOIN_MITRE);
    ensure_equals(bp.getJoinStyle(), BufferParameters::JOIN_MITRE);
    bp.setJoinStyle(BufferParameters::JOIN_BEVEL);
    ensure_equals(bp.getJoinStyle(), BufferParameters::JOIN_BEVEL);

    ensure_equals(bp.getEndCapStyle(), BufferParameters::CAP_ROUND);
    ensure_equals(bp.getMitreLimit(), 5.0);
    ensure_equals(bp.getQuadrantSegments(), int(8));
    ensure(! bp.isSingleSided());
}

// setMitreLimit and getMitreLimit
template<>
template<>
void object::test<9>()
{
    BufferParameters bp;
    ensure_equals(bp.getMitreLimit(), 5.0);
    bp.setMitreLimit(2.0);
    ensure_equals(bp.getMitreLimit(), 2.0);
    bp.setMitreLimit(10.12);
    ensure_equals(bp.getMitreLimit(), 10.12);

    ensure_equals(bp.getEndCapStyle(), BufferParameters::CAP_ROUND);
    ensure_equals(bp.getJoinStyle(), BufferParameters::JOIN_ROUND);
    ensure_equals(bp.getQuadrantSegments(), int(8));
    ensure(! bp.isSingleSided());
}

// setSingleSided and getSingleSided
template<>
template<>
void object::test<10>()
{
    BufferParameters bp;
    ensure(! bp.isSingleSided());
    bp.setSingleSided(true);
    ensure(bp.isSingleSided());
    bp.setSingleSided(false);
    ensure(! bp.isSingleSided());

    ensure_equals(bp.getEndCapStyle(), BufferParameters::CAP_ROUND);
    ensure_equals(bp.getJoinStyle(), BufferParameters::JOIN_ROUND);
    ensure_equals(bp.getQuadrantSegments(), int(8));
    ensure_equals(bp.getMitreLimit(), 5.0);
}




//----------------------------------------------------

// testMitreRight0
template<>
template<>
void object::test<11>()
{
    checkBuffer("LINESTRING (20 20, 20 80, 80 80)",
        10.0, bufParamFlatMitre(0),
        "POLYGON ((10 80, 20 90, 80 90, 80 70, 30 70, 30 20, 10 20, 10 80))");
}

// testMitreRight1
template<>
template<>
void object::test<12>()
{
    checkBuffer("LINESTRING (20 20, 20 80, 80 80)",
        10.0, bufParamFlatMitre(1),
        "POLYGON ((10 20, 10 84.14213562373095, 15.857864376269049 90, 80 90, 80 70, 30 70, 30 20, 10 20))");
}

// testMitreRight2
template<>
template<>
void object::test<13>()
{
    checkBuffer("LINESTRING (20 20, 20 80, 80 80)",
        10.0, bufParamFlatMitre(2),
        "POLYGON ((10 20, 10 90, 80 90, 80 70, 30 70, 30 20, 10 20))");
}

// testMitreNarrow0
template<>
template<>
void object::test<14>()
{
    checkBuffer("LINESTRING (10 20, 20 80, 30 20)",
        10.0, bufParamFlatMitre(0),
        "POLYGON ((10.136060761678563 81.64398987305357, 29.863939238321436 81.64398987305357, 39.863939238321436 21.643989873053574, 20.136060761678564 18.356010126946426, 20 19.172374697017812, 19.863939238321436 18.356010126946426, 0.1360607616785625 21.643989873053574, 10.136060761678563 81.64398987305357))");
}

// testMitreNarrow1
template<>
template<>
void object::test<15>()
{
    checkBuffer("LINESTRING (10 20, 20 80, 30 20)",
        10.0, bufParamFlatMitre(1),
        "POLYGON ((11.528729116169634 90, 28.47127088383036 90, 39.863939238321436 21.643989873053574, 20.136060761678564 18.356010126946426, 20 19.172374697017812, 19.863939238321436 18.356010126946426, 0.1360607616785625 21.643989873053574, 11.528729116169634 90))");
}

// testMitreNarrow5
template<>
template<>
void object::test<16>()
{
    checkBuffer("LINESTRING (10 20, 20 80, 30 20)",
        10.0, bufParamFlatMitre(5),
        "POLYGON ((18.1953957828363 130, 21.804604217163696 130, 39.863939238321436 21.643989873053574, 20.136060761678564 18.356010126946426, 20 19.172374697017812, 19.863939238321436 18.356010126946426, 0.1360607616785625 21.643989873053574, 18.1953957828363 130))");
}

// testMitreNarrow10
template<>
template<>
void object::test<17>()
{
    checkBuffer("LINESTRING (10 20, 20 80, 30 20)",
        10.0, bufParamFlatMitre(10),
        "POLYGON ((20 140.82762530298217, 39.863939238321436 21.643989873053574, 20.136060761678564 18.356010126946426, 20 19.172374697017812, 19.863939238321436 18.356010126946426, 0.1360607616785625 21.643989873053574, 20 140.82762530298217))");
}

// testMitreObtuse0
template<>
template<>
void object::test<18>()
{
    checkBuffer("LINESTRING (10 10, 50 20, 90 10)",
        1.0, bufParamFlatMitre(0),
        "POLYGON ((49.75746437496367 20.970142500145332, 50.24253562503633 20.970142500145332, 90.24253562503634 10.970142500145332, 89.75746437496366 9.029857499854668, 50 18.969223593595583, 10.242535625036332 9.029857499854668, 9.757464374963668 10.970142500145332, 49.75746437496367 20.970142500145332))");
}

// testMitreObtuse1
template<>
template<>
void object::test<19>()
{
    checkBuffer("LINESTRING (10 10, 50 20, 90 10)",
        1.0, bufParamFlatMitre(1),
        "POLYGON ((9.757464374963668 10.970142500145332, 49.876894374382324 21, 50.12310562561766 20.999999999999996, 90.24253562503634 10.970142500145332, 89.75746437496366 9.029857499854668, 50 18.969223593595583, 10.242535625036332 9.029857499854668, 9.757464374963668 10.970142500145332))");
}

// testMitreObtuse2
template<>
template<>
void object::test<20>()
{
    checkBuffer("LINESTRING (10 10, 50 20, 90 10)",
        1.0, bufParamFlatMitre(2),
        "POLYGON ((50 21.030776406404417, 90.24253562503634 10.970142500145332, 89.75746437496366 9.029857499854668, 50 18.969223593595583, 10.242535625036332 9.029857499854668, 9.757464374963668 10.970142500145332, 50 21.030776406404417))");
}

  //----------------------------------------------------

// testMitreSquareCCW1
template<>
template<>
void object::test<21>()
{
    checkBuffer("POLYGON((0 0, 100 0, 100 100, 0 100, 0 0))",
        10.0, bufParamFlatMitre(1),
        "POLYGON ((-10 -4.142135623730949, -10 104.14213562373095, -4.142135623730949 110, 104.14213562373095 110, 110 104.14213562373095, 110 -4.142135623730949, 104.14213562373095 -10, -4.142135623730949 -10, -10 -4.142135623730949))");
}

// testMitreSquare1
template<>
template<>
void object::test<22>()
{
    checkBuffer("POLYGON ((0 0, 0 100, 100 100, 100 0, 0 0))",
        10.0, bufParamFlatMitre(1),
        "POLYGON ((-4.14213562373095 -10, -10 -4.14213562373095, -10 104.14213562373095, -4.14213562373095 110, 104.14213562373095 110, 110 104.14213562373095, 110 -4.142135623730951, 104.14213562373095 -10, -4.14213562373095 -10))");
}

// Buffer produces invalid output
// https://github.com/libgeos/geos/issues/856
template<>
template<>
void object::test<23>()
{
    auto geom = _reader.read("POLYGON ((-23.989123360549296 73.1287474328027, -22.537997105552297 94.06368412079055,-18.796973600895146 93.80437130274495,-17.80121237894408 108.16990157009043,-21.542235883601226 108.42921438813606,-20.967403753721864 116.7221345967023,-4.728530705460814 116.7221568196225,-7.82790182044367 72.00851605865441,-23.989123360549296 73.1287474328027))");

    BufferParameters bp;
    bp.setJoinStyle(BufferParameters::JOIN_MITRE);
    BufferOp op(geom.get(), bp);

    auto result = op.getResultGeometry(10);
    ensure(result->isValid());
}


} // namespace tut

