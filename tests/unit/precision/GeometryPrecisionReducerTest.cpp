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

namespace tut {
//
// Test Group
//

// Common data used by tests
struct test_gpr_data {
    typedef std::unique_ptr<geos::geom::Geometry> GeometryPtr;
    typedef geos::geom::GeometryFactory GeometryFactory;

    geos::geom::PrecisionModel pm_float_;
    geos::geom::PrecisionModel pm_fixed_;
    GeometryFactory::Ptr factory_;
    GeometryFactory::Ptr factory_fixed_;
    geos::io::WKTReader reader_;
    geos::precision::GeometryPrecisionReducer reducer_;
    geos::precision::GeometryPrecisionReducer reducerKeepCollapse_; // keep collapse
    geos::precision::GeometryPrecisionReducer reducerChangePM_; // change precision model

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
void object::test<1>
()
{
    GeometryPtr g1(reader_.read("POLYGON (( 0 0, 0 1.4, 1.4 1.4, 1.4 0, 0 0 ))"));
    GeometryPtr g2(reader_.read("POLYGON (( 0 0, 0 1, 1 1, 1 0, 0 0 ))"));

    GeometryPtr result(reducer_.reduce(*g1));

    ensure_equals_geometry(g2.get(), result.get());
    ensure(result->getFactory() == g2->getFactory());
}

// Test tiny square collapse
template<>
template<>
void object::test<2>
()
{
    GeometryPtr g1(reader_.read("POLYGON (( 0 0, 0 .4, .4 .4, .4 0, 0 0 ))"));
    GeometryPtr g2(reader_.read("POLYGON EMPTY"));

    GeometryPtr result(reducer_.reduce(*g1));

    ensure_equals_geometry(g2.get(), result.get());
    ensure(result->getFactory() == g2->getFactory());
}

// Test square collapse
template<>
template<>
void object::test<3>
()
{
    GeometryPtr g1(reader_.read("POLYGON (( 0 0, 0 1.4, .4 .4, .4 0, 0 0 ))"));
    GeometryPtr g2(reader_.read("POLYGON EMPTY"));

    GeometryPtr result(reducer_.reduce(*g1));

    ensure_equals_geometry(g2.get(), result.get());
    ensure(result->getFactory() == g2->getFactory());
}

// Test square keep collapse
template<>
template<>
void object::test<4>
()
{
    GeometryPtr g1(reader_.read("POLYGON (( 0 0, 0 1.4, .4 .4, .4 0, 0 0 ))"));
    /*
     * For polygonal geometries, collapses are always removed, in order
     * to produce correct topology
     */
    GeometryPtr g2(reader_.read("POLYGON EMPTY"));

    GeometryPtr result(reducerKeepCollapse_.reduce(*g1));

    ensure_equals_geometry(g2.get(), result.get());
    ensure(result->getFactory() == g2->getFactory());
}

// Test line
template<>
template<>
void object::test<5>
()
{
    GeometryPtr g1(reader_.read("LINESTRING ( 0 0, 0 1.4 )"));
    GeometryPtr g2(reader_.read("LINESTRING (0 0, 0 1)"));

    GeometryPtr result(reducer_.reduce(*g1));

    ensure_equals_geometry(g2.get(), result.get());
    ensure(result->getFactory() == g2->getFactory());
}

// Test line remove collapse
template<>
template<>
void object::test<6>
()
{
    GeometryPtr g1(reader_.read("LINESTRING ( 0 0, 0 .4 )"));
    GeometryPtr g2(reader_.read("LINESTRING EMPTY"));

    GeometryPtr result(reducer_.reduce(*g1));

    ensure_equals_geometry(g2.get(), result.get());
    ensure(result->getFactory() == g2->getFactory());
}

// Test line keep collapse
template<>
template<>
void object::test<7>
()
{
    GeometryPtr g1(reader_.read("LINESTRING ( 0 0, 0 .4 )"));
    GeometryPtr g2(reader_.read("LINESTRING ( 0 0, 0 0 )"));

    GeometryPtr result(reducerKeepCollapse_.reduce(*g1));

    ensure_equals_geometry(g2.get(), result.get());
    ensure(result->getFactory() == g2->getFactory());
}

// Test square with changed PM
template<>
template<>
void object::test<8>
()
{
    GeometryPtr g1(reader_.read("POLYGON (( 0 0, 0 1.4, 1.4 1.4, 1.4 0, 0 0 ))"));
    GeometryPtr g2(reader_.read("POLYGON (( 0 0, 0 1, 1 1, 1 0, 0 0 ))"));

    GeometryPtr result(reducerChangePM_.reduce(*g1));

    ensure_equals_geometry(g2.get(), result.get());
    ensure(result->getFactory() != g2->getFactory());
}

// Test points with changed PM
template<>
template<>
void object::test<9>
()
{
    GeometryPtr g1(reader_.read("MULTIPOINT ((0 0), (0 1.4), (1.4 1.4), (1.4 0), (0.9 0) ))"));
    GeometryPtr g2(reader_.read("MULTIPOINT ((0 0), (0 1), (1 1), (1 0), (1 0) ))"));

    GeometryPtr result(reducerChangePM_.reduce(*g1));

    ensure_equals_geometry(g2.get(), result.get());
    ensure(result->getFactory() != g2->getFactory());
}


} // namespace tut

