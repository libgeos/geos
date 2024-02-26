//
// Test Suite for geos::operation::buffer::BufferParameters class.

// tut
#include <tut/tut.hpp>
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
    typedef geos::operation::buffer::BufferParameters BufferParameters;

    geos::io::WKTReader _reader;

    test_bufferparameters_data() { }

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
void object::test<1>
()
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
void object::test<2>
()
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
void object::test<3>
()
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
void object::test<4>
()
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
void object::test<5>
()
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
void object::test<6>
()
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
void object::test<7>
()
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
void object::test<8>
()
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
void object::test<9>
()
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
void object::test<10>
()
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

