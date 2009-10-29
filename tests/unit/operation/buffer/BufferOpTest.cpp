// $Id$
// 
// Test Suite for geos::operation::buffer::BufferOp class.

// tut
#include <tut.hpp>
// geos
#include <geos/operation/buffer/BufferOp.h>
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
    struct test_bufferop_data
    {
        geos::geom::GeometryFactory gf;
        geos::io::WKTReader wktreader;

        typedef geos::geom::Geometry::AutoPtr GeomPtr;
        typedef std::auto_ptr<geos::geom::CoordinateSequence> CSPtr;

        test_bufferop_data()
            : gf(), wktreader(&gf)
        {}
    };

    typedef test_group<test_bufferop_data> group;
    typedef group::object object;

    group test_bufferop_group("geos::operation::buffer::BufferOp");

    //
    // Test Cases
    //
    template<>
    template<>
    void object::test<1>()
    {
        using geos::operation::buffer::BufferOp;

        std::string wkt0("POINT(0 0)");
        GeomPtr g0(wktreader.read(wkt0));

        double const distance = 0.0;
        BufferOp op(g0.get());
        GeomPtr gBuffer(op.getResultGeometry(distance));

        ensure(gBuffer->isEmpty());
        ensure(gBuffer->isValid());
        ensure_equals(gBuffer->getGeometryTypeId(), geos::geom::GEOS_POLYGON);
        ensure_equals(gBuffer->getNumPoints(), 0);
    }

    template<>
    template<>
    void object::test<2>()
    {
        using geos::operation::buffer::BufferOp;

        std::string wkt0("POINT(0 0)");
        GeomPtr g0(wktreader.read(wkt0));

        // Buffer point with default buffering parameters
        double const distance = 1.0;
        BufferOp op(g0.get());
        GeomPtr gBuffer(op.getResultGeometry(distance));

        ensure_not(gBuffer->isEmpty());
        ensure(gBuffer->isValid());
        ensure_equals(gBuffer->getGeometryTypeId(), geos::geom::GEOS_POLYGON);
        ensure(gBuffer->getNumPoints() > 32);
    }

    template<>
    template<>
    void object::test<3>()
    {
        using geos::operation::buffer::BufferOp;
        using geos::operation::buffer::BufferParameters;

        std::string wkt0("POINT(0 0)");
        GeomPtr g0(wktreader.read(wkt0));

        // Buffer point with custom parameters: 32 quadrant segments
        int const segments = 32;
        BufferParameters params(segments);
        
        BufferOp op(g0.get(), params);

        double const distance = 1.0;
        GeomPtr gBuffer(op.getResultGeometry(distance));

        ensure_not(gBuffer->isEmpty());
        ensure(gBuffer->isValid());
        ensure_equals(gBuffer->getGeometryTypeId(), geos::geom::GEOS_POLYGON);
        ensure(gBuffer->getNumPoints() > 129);
    }

    template<>
    template<>
    void object::test<4>()
    {
        using geos::operation::buffer::BufferOp;
        using geos::operation::buffer::BufferParameters;

        ensure_equals(BufferParameters::DEFAULT_QUADRANT_SEGMENTS, 8);

        std::string wkt0("MULTIPOLYGON(((708258.754920656 2402197.91172757,708257.029447455 2402206.56901508,708652.961095455 2402312.65463437,708657.068786251 2402304.6356364,708258.754920656 2402197.91172757)),((708653.498611049 2402311.54647056,708708.895756966 2402203.47250014,708280.326454234 2402089.6337791,708247.896591321 2402252.48269854,708367.379593851 2402324.00761653,708248.882609455 2402253.07294874,708249.523621829 2402244.3124463,708261.854734465 2402182.39086576,708262.818392579 2402183.35452387,708653.498611049 2402311.54647056)))");
        GeomPtr g0(wktreader.read(wkt0));

        // Buffer point with custom parameters: 24 quadrant segments
        {
            int const segments = BufferParameters::DEFAULT_QUADRANT_SEGMENTS * 3;
            BufferParameters params(segments);
            BufferOp op(g0.get(), params);
            double const distance = 0.0001;
            GeomPtr gBuffer(op.getResultGeometry(distance));
            ensure_not(gBuffer->isEmpty());
            ensure(gBuffer->isValid());
            ensure_equals(gBuffer->getGeometryTypeId(), geos::geom::GEOS_POLYGON);
            ensure(gBuffer->getNumPoints() >= 245);
        }

        // Buffer point with custom parameters: 32 quadrant segments
        {
            int const segments = BufferParameters::DEFAULT_QUADRANT_SEGMENTS * 4;
            BufferParameters params(segments);
            BufferOp op(g0.get(), params);
            double const distance = 0.0001;
            GeomPtr gBuffer(op.getResultGeometry(distance));
            ensure_not(gBuffer->isEmpty());
            ensure(gBuffer->isValid());
            ensure_equals(gBuffer->getGeometryTypeId(), geos::geom::GEOS_POLYGON);
            ensure(gBuffer->getNumPoints() >= 318);
        }
    }

} // namespace tut
