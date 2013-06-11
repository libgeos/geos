// 
// Test Suite for geos::operation::buffer::BufferBuilder class.

// tut
#include <tut.hpp>
// geos
#include <geos/operation/buffer/BufferBuilder.h>
#include <geos/operation/buffer/BufferParameters.h>
#include <geos/platform.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Geometry.h>
#include <geos/algorithm/PointLocator.h>
#include <geos/io/WKTReader.h>
#include <geos/geom/CoordinateSequence.h>
// std
#include <memory>
#include <string>
#include <vector>

namespace tut
{
    //
    // Test Group
    //

    // Common data used by tests
    struct test_bufferbuilder_data
    {
        geos::geom::GeometryFactory gf;
        geos::io::WKTReader wktreader;
        int const default_quadrant_segments;

        typedef geos::geom::Geometry::AutoPtr GeomPtr;
        typedef std::auto_ptr<geos::geom::CoordinateSequence> CSPtr;

        test_bufferbuilder_data()
            : gf(), wktreader(&gf), default_quadrant_segments(geos::operation::buffer::BufferParameters::DEFAULT_QUADRANT_SEGMENTS)
        {
            ensure_equals(default_quadrant_segments, int(8));
        }
    private:
        // noncopyable
        test_bufferbuilder_data(test_bufferbuilder_data const& other);
        test_bufferbuilder_data& operator=(test_bufferbuilder_data const& rhs);
    };

    typedef test_group<test_bufferbuilder_data> group;
    typedef group::object object;

    group test_bufferbuilder_group("geos::operation::buffer::BufferBuilder");

    //
    // Test Cases
    //

    // Left-side offset curve
    template<>
    template<>
    void object::test<1>()
    {
        using geos::operation::buffer::BufferBuilder;
        using geos::operation::buffer::BufferParameters;

        std::string wkt0("LINESTRING ("
            "665.7317504882812500 133.0762634277343700,"
            "1774.4752197265625000 19.9391822814941410,"
            "756.2413940429687500 466.8306579589843700,"
            "626.1337890625000000 1898.0147705078125000,"
            "433.8007202148437500 404.6052856445312500)");
        GeomPtr g0(wktreader.read(wkt0));
        double const distance = 57.164000837203;
        bool const leftSide = true;

        BufferParameters params;
        params.setJoinStyle(BufferParameters::JOIN_MITRE);
        params.setMitreLimit(5.57F); // somewhere between 5.5 and 5.6
        params.setSingleSided(true);
        BufferBuilder builder(params);
        GeomPtr gBuffer(builder.bufferLineSingleSided(g0.get(), distance, leftSide ));

        ensure_equals(gBuffer->getGeometryTypeId(), geos::geom::GEOS_LINESTRING);
        // Left-side offset curve expected with 5 vertices
        ensure_equals(gBuffer->getNumPoints(), std::size_t(5));
    }

    // Right-side offset curve
    template<>
    template<>
    void object::test<2>()
    {
        using geos::operation::buffer::BufferBuilder;
        using geos::operation::buffer::BufferParameters;

        std::string wkt0("LINESTRING ("
            "665.7317504882812500 133.0762634277343700,"
            "1774.4752197265625000 19.9391822814941410,"
            "756.2413940429687500 466.8306579589843700,"
            "626.1337890625000000 1898.0147705078125000,"
            "433.8007202148437500 404.6052856445312500)");
        GeomPtr g0(wktreader.read(wkt0));

        double const distance = 57.164000837203;
        bool const leftSide = false;

        BufferParameters params;
        params.setJoinStyle(BufferParameters::JOIN_MITRE);
        params.setMitreLimit(5.57F); // somewhere between 5.5 and 5.6
        params.setSingleSided(true);
        BufferBuilder builder(params);
        GeomPtr gBuffer(builder.bufferLineSingleSided(g0.get(), distance, leftSide));

        ensure_equals(gBuffer->getGeometryTypeId(), geos::geom::GEOS_LINESTRING);
        // Left-side offset curve expected with 5 vertices
        ensure_equals(gBuffer->getNumPoints(), std::size_t(5));
    }

} // namespace tut

