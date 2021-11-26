// $Id$
//
// Test Suite for geos::precision::GeometryPrecisionReducer class.

// tut
#include <tut/tut.hpp>
#include <utility.h>
// geos
#include <geos/precision/GeometryPrecisionReducer.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/io/WKTReader.h>
// std
#include <string>
#include <vector>
#include <iostream>

using namespace geos::geom;
using namespace geos::precision;

namespace tut {
//
// Test Group
//

// Common data used by tests
struct test_gpr_data {

    PrecisionModel pm_float_;
    PrecisionModel pm_fixed_;
    GeometryFactory::Ptr factory_;
    GeometryFactory::Ptr factory_fixed_;
    geos::io::WKTReader reader_;
    GeometryPrecisionReducer reducer_;
    GeometryPrecisionReducer reducerKeepCollapse_; // keep collapse
    GeometryPrecisionReducer reducerChangePM_; // change precision model

    test_gpr_data() :
        pm_float_(),
        pm_fixed_(1),
        factory_(GeometryFactory::create(&pm_float_, 0)),
        factory_fixed_(GeometryFactory::create(&pm_fixed_, 0)),
        reader_(factory_.get()),
        reducer_(pm_fixed_),
        reducerKeepCollapse_(pm_fixed_),
        reducerChangePM_(*factory_fixed_)
    {
        reducerKeepCollapse_.setRemoveCollapsedComponents(false);
        reducerChangePM_.setChangePrecisionModel(true);
        reducer_.setRemoveCollapsedComponents(true);
        reducer_.setChangePrecisionModel(false);
    }

    void checkReducePointwise(
        const char* wkt,
        const char* wktExpected)
    {
        std::unique_ptr<Geometry> g = reader_.read(wkt);
        std::unique_ptr<Geometry> expected = reader_.read(wktExpected);
        std::unique_ptr<Geometry> actual = GeometryPrecisionReducer::reducePointwise(*g, pm_fixed_);
        ensure_equals_geometry(expected.get(), actual.get());
        ensure("Factories are not the same", expected->getFactory() == actual->getFactory());
    }

    void checkReduceKeepCollapse(
        const char* wkt,
        const char* wktExpected)
    {
        checkReduceAny(wkt, wktExpected, reducerKeepCollapse_);
    }

    void checkReduceNewPM(
        const char* wkt,
        const char* wktExpected)
    {
        checkReduceAny(wkt, wktExpected, reducerChangePM_, false);
    }

    void checkReduce(
        double scaleFactor,
        const char* wkt,
        const char* wktExpected)
    {
        PrecisionModel pm(scaleFactor);
        GeometryPrecisionReducer reducer(pm);
        reducer.setRemoveCollapsedComponents(true);
        reducer.setChangePrecisionModel(false);
        checkReduceAny(wkt, wktExpected, reducer);
    }

    void checkReduce(
        const char* wkt,
        const char* wktExpected)
    {
        checkReduceAny(wkt, wktExpected, reducer_);
    }

    void checkReduceAny(
        const char* wkt,
        const char* wktExpected,
        GeometryPrecisionReducer& reducerAny,
        bool samePM = true)
    {
        std::unique_ptr<Geometry> g = reader_.read(wkt);
        std::unique_ptr<Geometry> expected = reader_.read(wktExpected);
        std::unique_ptr<Geometry> actual = reducerAny.reduce(*g);
        ensure_equals_geometry(expected.get(), actual.get());
        if (samePM)
            ensure("Factories are not the same", actual->getFactory() == g->getFactory());
        else
            ensure("Factories are the same", actual->getFactory() != g->getFactory());
    }

};

typedef test_group<test_gpr_data> group;
typedef group::object object;

group test_gpr_group("geos::precision::GeometryPrecisionReducer");

//
// Test Cases
//

// Test square
template<>
template<>
void object::test<1> ()
{
    checkReduce(
        "POLYGON (( 0 0, 0 1.4, 1.4 1.4, 1.4 0, 0 0 ))",
        "POLYGON (( 0 0, 0 1, 1 1, 1 0, 0 0 ))"
        );
}

// Test tiny square collapse
template<>
template<>
void object::test<2> ()
{
    checkReduce(
        "POLYGON (( 0 0, 0 .4, .4 .4, .4 0, 0 0 ))",
        "POLYGON EMPTY"
        );
}

// Test square collapse
template<>
template<>
void object::test<3> ()
{
    checkReduce(
        "POLYGON (( 0 0, 0 1.4, .4 .4, .4 0, 0 0 ))",
        "POLYGON EMPTY"
        );
}

// Test square keep collapse
template<>
template<>
void object::test<4> ()
{
    checkReduceKeepCollapse(
        "POLYGON (( 0 0, 0 1.4, .4 .4, .4 0, 0 0 ))",
        "POLYGON EMPTY"
        );
}

// Test line
template<>
template<>
void object::test<5> ()
{
    checkReduce(
        "LINESTRING ( 0 0, 0 1.4 )",
        "LINESTRING (0 0, 0 1)"
        );
}

// testLineNotNoded
template<>
template<>
void object::test<6> ()
{
    checkReduce(
        "LINESTRING(1 1, 3 3, 9 9, 5.1 5, 2.1 2)",
        "LINESTRING(1 1, 3 3, 9 9, 5 5, 2 2)"
        );
}


// Test line remove collapse
template<>
template<>
void object::test<7> ()
{
    checkReduce(
        "LINESTRING ( 0 0, 0 .4 )",
        "LINESTRING EMPTY"
        );
}

// /* Test line keep collapse */
template<>
template<>
void object::test<8>
()
{
    checkReduceKeepCollapse(
        "LINESTRING (0 0, 0 .4)",
        "LINESTRING (0 0, 0 0)"
        );
}

// Test square with changed PM
template<>
template<>
void object::test<9> ()
{
    checkReduceNewPM(
        "POLYGON (( 0 0, 0 1.4, 1.4 1.4, 1.4 0, 0 0 ))",
        "POLYGON (( 0 0, 0 1, 1 1, 1 0, 0 0 ))"
        );
}

// Test points with changed PM
template<>
template<>
void object::test<10> ()
{
    checkReduceNewPM(
        "MULTIPOINT ((0 0), (0 1.4), (1.4 1.4), (1.4 0), (0.9 0) ))",
        "MULTIPOINT ((0 0), (0 1), (1 1), (1 0), (1 0) ))"
    );
}

// testPoint
template<>
template<>
void object::test<11> ()
{
    checkReduce(
        "POINT(1.1 4.9)",
        "POINT(1 5)");
}

// testMultiPoint
template<>
template<>
void object::test<12> ()
{
    checkReduce(
        "MULTIPOINT( (1.1 4.9),(1.2 4.8), (3.3 6.6))",
        "MULTIPOINT((1 5), (1 5), (3 7))");
}

// testPolgonWithCollapsedLine
template<>
template<>
void object::test<13> ()
{
    checkReduce(
        "POLYGON ((10 10, 100 100, 200 10.1, 300 10, 10 10))",
        "POLYGON ((10 10, 100 100, 200 10, 10 10))");
}

// testPolgonWithCollapsedPoint
template<>
template<>
void object::test<14> ()
{
    checkReduce(
        "POLYGON ((10 10, 100 100, 200 10.1, 300 100, 400 10, 10 10))",
        "MULTIPOLYGON (((10 10, 100 100, 200 10, 10 10)), ((200 10, 300 100, 400 10, 200 10)))"
        );
}

// testMultiPolgonCollapse
template<>
template<>
void object::test<15> ()
{
    checkReduce(
        "MULTIPOLYGON (((1 9, 5 9, 5 1, 1 1, 1 9)), ((5.2 8.7, 9 8.7, 9 1, 5.2 1, 5.2 8.7)))",
        "POLYGON ((1 1, 1 9, 5 9, 9 9, 9 1, 5 1, 1 1))");
}

// testGC
template<>
template<>
void object::test<16> ()
{
    checkReduce(
        "GEOMETRYCOLLECTION (POINT (1.1 2.2), MULTIPOINT ((1.1 2), (3.1 3.9)), LINESTRING (1 2.1, 3 3.9), MULTILINESTRING ((1 2, 3 4), (5 6, 7 8)), POLYGON ((2 2, -2 2, -2 -2, 2 -2, 2 2), (1 1, 1 -1, -1 -1, -1 1, 1 1)), MULTIPOLYGON (((2 2, -2 2, -2 -2, 2 -2, 2 2), (1 1, 1 -1, -1 -1, -1 1, 1 1)), ((7 2, 3 2, 3 -2, 7 -2, 7 2))))",
        "GEOMETRYCOLLECTION (POINT (1 2),     MULTIPOINT ((1 2), (3 4)),       LINESTRING (1 2, 3 4),     MULTILINESTRING ((1 2, 3 4), (5 6, 7 8)), POLYGON ((2 2, -2 2, -2 -2, 2 -2, 2 2), (1 1, 1 -1, -1 -1, -1 1, 1 1)), MULTIPOLYGON (((2 2, -2 2, -2 -2, 2 -2, 2 2), (1 1, 1 -1, -1 -1, -1 1, 1 1)), ((7 2, 3 2, 3 -2, 7 -2, 7 2))))"
    );
}

// testGCPolygonCollapse
template<>
template<>
void object::test<17> ()
{
    checkReduce(
        "GEOMETRYCOLLECTION (POINT (1.1 2.2), POLYGON ((10 10, 100 100, 200 10.1, 300 100, 400 10, 10 10)) )",
        "GEOMETRYCOLLECTION (POINT (1 2),     MULTIPOLYGON (((10 10, 100 100, 200 10, 10 10)), ((200 10, 300 100, 400 10, 200 10))) )"
    );
}

// testGCNested
template<>
template<>
void object::test<18> ()
{
    checkReduce(
        "GEOMETRYCOLLECTION (POINT (1.1 2.2), GEOMETRYCOLLECTION( POINT (1.1 2.2), LINESTRING (1 2.1, 3 3.9) ) )",
        "GEOMETRYCOLLECTION (POINT (1 2),     GEOMETRYCOLLECTION( POINT (1 2),     LINESTRING (1 2, 3 4) ) )"
    );
}

// testPolgonWithCollapsedLinePointwise
template<>
template<>
void object::test<19> ()
{
    checkReducePointwise(
        "POLYGON ((10 10, 100 100, 200 10.1, 300 10, 10 10))",
        "POLYGON ((10 10, 100 100, 200 10,   300 10, 10 10))"
        );
}

// testPolgonWithCollapsedPointPointwise
template<>
template<>
void object::test<20> ()
{
    checkReducePointwise(
        "POLYGON ((10 10, 100 100, 200 10.1, 300 100, 400 10, 10 10))",
        "POLYGON ((10 10, 100 100, 200 10,   300 100, 400 10, 10 10))"
        );
}

// testGridsize
template<>
template<>
void object::test<21> ()
{
    checkReduce(-100,
        "POLYGON ((100 120, 190 400, 485 398, 250 380, 400 100, 100 120))",
        "POLYGON ((200 400, 300 400, 400 100, 100 100, 200 400))");
}

/**
* Test issue showing bug in SnapRoundingNoder not passing tolerance to MCIndexNoder.
*
* See https://trac.osgeo.org/geos/ticket/1127
*/
template<>
template<>
void object::test<22> ()
{
    checkReduce(-612,
        "POLYGON((3670939.6336634574 3396937.3777869204, 3670995.4715200397 3396926.0316904164, 3671077.280213823 3396905.4302639295, 3671203.8838707027 3396908.120176068, 3671334.962571111 3396904.8310892633, 3670037.299066126 3396904.8310892633, 3670037.299066126 3398075.9808747065, 3670939.6336634574 3396937.3777869204))",
        "POLYGON ((3670164 3396600, 3670164 3397824, 3670776 3397212, 3670776 3396600, 3670164 3396600))");
}



} // namespace tut

