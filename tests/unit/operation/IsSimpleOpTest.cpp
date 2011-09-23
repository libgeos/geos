// $Id: IsSimpleOpTest.cpp 2344 2009-04-09 21:46:30Z mloskot $
// 
// Test Suite for geos::operation::IsSimpleOp class
// Ported from JTS junit/operation/IsSimpleTest.java

#include <tut.hpp>
// geos
#include <geos/operation/IsSimpleOp.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/Dimension.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/io/WKTReader.h>
// std
#include <string>
#include <memory>

using namespace geos::geom;
using namespace geos::operation;

namespace tut
{
    //
    // Test Group
    //

    struct test_issimpleop_data
    {
        geos::geom::PrecisionModel pm_;
        geos::geom::GeometryFactory factory_;
        geos::io::WKTReader reader_;
        double tolerance_;

        test_issimpleop_data()
			: pm_(1), factory_(&pm_, 0), reader_(&factory_), tolerance_(0.00005)
        {}
    };

    typedef test_group<test_issimpleop_data> group;
    typedef group::object object;

    group test_issimpleop_group("geos::operation::IsSimpleOp");

    //
    // Test Cases
    //

    // 1 - Test cross
    template<>
    template<>
    void object::test<1>()
    {
        const std::string wkt("MULTILINESTRING ((20 120, 120 20), (20 20, 120 120))");
        const Geometry::AutoPtr geom(reader_.read(wkt));

        // TODO - mloskot: What about support of new features of BoundaryNodeRule, in JTS

        IsSimpleOp op;
        bool simple = op.isSimpleLinearGeometry(geom.get());

        ensure( false == simple );

        // TODO - mloskot:
        // There are missing features not (re)implemented in IsSimpleOp, in GEOS.
        // So, all tests in this suite have been simplified in comparison to original JTS tests.
        //
        //Coordinate loc(70, 70);
        //Coordinate nonSimpleLoc = op.getNonSimpleLocation();
        //loc.distance(nonSimpleLoc) < TOLERANCE
    }

    // 2 - Test MultiLineString with ring touching at the end point
    template<>
    template<>
    void object::test<2>()
    {
        const std::string wkt("MULTILINESTRING ((100 100, 20 20, 200 20, 100 100), (100 200, 100 100))");
        const Geometry::AutoPtr geom(reader_.read(wkt));

        IsSimpleOp op;
        bool simple = op.isSimpleLinearGeometry(geom.get());

        ensure( false == simple );
    }

    // 3 - Test simple LineString
    template<>
    template<>
    void object::test<3>()
    {
        const std::string wkt("LINESTRING (100 100, 20 20, 200 20, 100 100)");
        const Geometry::AutoPtr geom(reader_.read(wkt));

        IsSimpleOp op;
        bool simple = op.isSimpleLinearGeometry(geom.get());

        ensure( true == simple );
    }

} // namespace tut
