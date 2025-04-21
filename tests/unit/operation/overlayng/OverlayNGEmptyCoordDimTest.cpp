//
// Test Suite for geos::operation::overlayng::OverlayNG coordinate dimension handling
// for EMPTY geometries

#include <tut/tut.hpp>
#include <utility.h>

// geos
#include <geos/operation/overlayng/OverlayNG.h>

// std
#include <memory>

using namespace geos::geom;
using namespace geos::operation::overlayng;
using geos::io::WKTReader;
using geos::io::WKTWriter;

namespace tut {
//
// Test Group
//

// Common data used by all tests
struct test_overlayngcemptyoorddim_data {

    WKTReader r;
    WKTWriter w;

    void
    testOverlay(const std::string& a, const std::string& b, int opCode, const std::string& expected)
    {
        std::unique_ptr<Geometry> geom_a = r.read(a);
        std::unique_ptr<Geometry> geom_b = r.read(b);
        std::unique_ptr<Geometry> geom_expected = r.read(expected);
        std::unique_ptr<Geometry> geom_result = OverlayNG::overlay(geom_a.get(), geom_b.get(), opCode);
        // std::string wkt_result = w.write(geom_result.get());
        // std::cout << std::endl << wkt_result << std::endl;
        ensure_equals_geometry(geom_expected.get(), geom_result.get());
        ensure_equals(  "Coordinate dimension: ", 
                        (int) geom_result.get()->getCoordinateDimension(),
                        (int) geom_expected.get()->getCoordinateDimension()
                    );
    }

};

typedef test_group<test_overlayngcemptyoorddim_data> group;
typedef group::object object;

group test_overlayngcoorddim_group("geos::operation::overlayng::OverlayNGEmptyCoordDim");

//
// Test Cases
//

//--------- POINT / POINT

// test ZM dim for empty POINT union
template<>
template<>
void object::test<1> ()
{
    testOverlay("POINT ZM EMPTY", "POINT ZM EMPTY", 
                OverlayNG::UNION, "POINT ZM EMPTY");
}

// test ZM dim for empty POINT intersection
template<>
template<>
void object::test<2> ()
{
    testOverlay("POINT ZM EMPTY", "POINT ZM EMPTY", 
        OverlayNG::INTERSECTION, "POINT ZM EMPTY");
}

// test mixed ZM and XY dim for empty POINT union
template<>
template<>
void object::test<3> ()
{
    testOverlay("POINT ZM EMPTY", "POINT EMPTY", 
        OverlayNG::UNION, "POINT EMPTY");
}

// test mixed ZM and Z dim for empty POINT union
template<>
template<>
void object::test<4> ()
{
    testOverlay("POINT ZM EMPTY", "POINT Z EMPTY", 
        OverlayNG::UNION, "POINT Z EMPTY");
}

//--------- LINESTRING / POINT

template<>
template<>
void object::test<5> ()
{
    testOverlay("POINT ZM EMPTY", "LINESTRING ZM EMPTY", 
        OverlayNG::UNION, "LINESTRING ZM EMPTY");    
}

template<>
template<>
void object::test<6> ()
{
    testOverlay("POINT ZM EMPTY", "LINESTRING Z EMPTY", 
        OverlayNG::UNION, "LINESTRING Z EMPTY");
}

template<>
template<>
void object::test<7> ()
{
    testOverlay("POINT ZM EMPTY", "LINESTRING EMPTY", 
        OverlayNG::UNION, "LINESTRING EMPTY");
}

//-- ensure coord dim is lowest of either operand
template<>
template<>
void object::test<8> ()
{
    testOverlay("POINT EMPTY", "LINESTRING ZM EMPTY", 
        OverlayNG::UNION, "LINESTRING EMPTY");
}

//--------- LINESTRING / LINESTRING

// test ZM dim for empty LINESTRING union
template<>
template<>
void object::test<9> ()
{
    testOverlay("LINESTRING ZM EMPTY", "LINESTRING ZM EMPTY", 
        OverlayNG::UNION, "LINESTRING ZM EMPTY");
}

// test mixed ZM and XY dim for empty LINESTRING union
template<>
template<>
void object::test<10> ()
{
    testOverlay("LINESTRING ZM EMPTY", "LINESTRING Z EMPTY", 
        OverlayNG::UNION, "LINESTRING Z EMPTY");
}

// test mixed ZM and Z dim for empty LINESTRING union
template<>
template<>
void object::test<11> ()
{
    testOverlay("LINESTRING ZM EMPTY", "LINESTRING EMPTY", 
        OverlayNG::UNION, "LINESTRING EMPTY");
}

//--------- GEOMETRYCOLLECTION

//-- coord dim of GC (ZM) EMPTY is always 2
template<>
template<>
void object::test<12> ()
{
    testOverlay("GEOMETRYCOLLECTION ZM EMPTY", 
                "POINT ZM EMPTY", 
                OverlayNG::UNION, "POINT EMPTY");
}

//-- coord dim of GC containing EMPTYs is lowest coord dim of elements
template<>
template<>
void object::test<13> ()
{
    testOverlay("GEOMETRYCOLLECTION (POINT ZM EMPTY)", 
        "GEOMETRYCOLLECTION (POINT ZM EMPTY, LINESTRING ZM EMPTY)", 
        OverlayNG::UNION, "LINESTRING ZM EMPTY");
}

template<>
template<>
void object::test<14> ()
{
    testOverlay("GEOMETRYCOLLECTION (POINT Z EMPTY)", 
        "GEOMETRYCOLLECTION (POINT ZM EMPTY, LINESTRING ZM EMPTY)", 
        OverlayNG::UNION, "LINESTRING Z EMPTY");
}

template<>
template<>
void object::test<15> ()
{
    testOverlay("GEOMETRYCOLLECTION (POINT EMPTY)", 
        "GEOMETRYCOLLECTION (POINT ZM EMPTY, LINESTRING ZM EMPTY)", 
        OverlayNG::UNION, "LINESTRING EMPTY");
}

} // namespace tut
