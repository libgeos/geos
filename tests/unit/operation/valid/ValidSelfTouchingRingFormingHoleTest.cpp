// $Id: ValidSelfTouchingRingFormingHoleTest.cpp 2809 2009-12-06 01:05:24Z mloskot $
// 
// Test Suite for geos::operation::valid::IsValidOp class
// Ported from JTS junit/operation/valid/ValidSelfTouchingRingFormingHoleTest.java rev. 1.4

#include <tut.hpp>
// geos
#include <geos/operation/valid/IsValidOp.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateArraySequence.h>
#include <geos/geom/Dimension.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/LineString.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/util/GEOSException.h>
#include <geos/io/WKTReader.h>
#include <geos/operation/valid/TopologyValidationError.h>
#include <geos/platform.h> // for ISNAN
// std
#include <string>
#include <memory>
#include <iostream>

using namespace geos::geom;
//using namespace geos::operation;
using namespace geos::operation::valid;
using namespace geos::util;
using namespace std;

namespace tut
{
    //
    // Test Group
    //

    struct test_ValidSelfTouchingRingFormingHole_data
    {
	typedef std::auto_ptr<Geometry> GeomPtr;

        geos::geom::PrecisionModel pm_;
        geos::geom::GeometryFactory factory_;
        geos::io::WKTReader rdr;

        test_ValidSelfTouchingRingFormingHole_data()
			: pm_(1), factory_(&pm_, 0), rdr(&factory_)
        {}

	GeomPtr fromWKT(const std::string& wkt)
	{
		GeomPtr geom;
		try {
			geom.reset( rdr.read(wkt) );
		}
		catch (const GEOSException& ex) {
			cerr << ex.what() << endl;
		}
		return geom;
	}

	void checkIsValidDefault(const std::string& wkt, bool expected)
	{
		GeomPtr geom = fromWKT(wkt);
		IsValidOp validator(geom.get());
		bool isValid = validator.isValid();
		ensure(isValid == expected);
	}

	void checkIsValidSTR(const std::string& wkt, bool expected)
	{
		GeomPtr geom = fromWKT(wkt);
		IsValidOp validator(geom.get());
		validator.setSelfTouchingRingFormingHoleValid(true);
		bool isValid = validator.isValid();
		ensure(isValid == expected);
	}

    };

    typedef test_group<test_ValidSelfTouchingRingFormingHole_data> group;
    typedef group::object object;

    group test_ValidSelfTouchingRingFormingHole_group("geos::operation::valid::ValidSelfTouchingRingFormingHole");

    //
    // Test Cases
    //

    // 1 - testShellAndHoleSelfTouch
    //
    //  Tests a geometry with both a shell self-touch and a hole self=touch.
    //  This is valid if STR is allowed, but invalid in OGC
    //
    template<>
    template<>
    void object::test<1>()
    {
    string wkt = "POLYGON ((0 0, 0 340, 320 340, 320 0, 120 0, 180 100, 60 100, 120 0, 0 0),   (80 300, 80 180, 200 180, 200 240, 280 200, 280 280, 200 240, 200 300, 80 300))";
    checkIsValidSTR(wkt, true);
    checkIsValidDefault(wkt, false);
    }

  /*
   * 2 - testShellHoleAndHoleHoleTouch
   *
   * Tests a geometry representing the same area as in
   * {@link #testShellAndHoleSelfTouch}
   * but using a shell-hole touch and a hole-hole touch.
   * This is valid in OGC.
   */
    template<>
    template<>
    void object::test<2>()
    {
    string wkt = "POLYGON ((0 0, 0 340, 320 340, 320 0, 120 0, 0 0),   (120 0, 180 100, 60 100, 120 0),   (80 300, 80 180, 200 180, 200 240, 200 300, 80 300),  (200 240, 280 200, 280 280, 200 240))";
    checkIsValidSTR(wkt, true);
    checkIsValidDefault(wkt, true);
    }

  /*
   * 3 - testShellSelfTouchHoleOverlappingHole
   *
   * Tests an overlapping hole condition, where one of the holes is
   * created by a shell self-touch.
   * This is never valid.
   */
    template<>
    template<>
    void object::test<3>()
    {
    string wkt = "POLYGON ((0 0, 220 0, 220 200, 120 200, 140 100, 80 100, 120 200, 0 200, 0 0),   (200 80, 20 80, 120 200, 200 80))";
    checkIsValidSTR(wkt, false);
    checkIsValidDefault(wkt, false);
    }

  /*
   * 4 - testDisconnectedInteriorShellSelfTouchAtNonVertex
   *
   * Ensure that the Disconnected Interior condition is not validated
   */
    template<>
    template<>
    void object::test<4>()
    {
    string wkt = "POLYGON ((40 180, 40 60, 240 60, 240 180, 140 60, 40 180))";
    checkIsValidSTR(wkt, false);
    checkIsValidDefault(wkt, false);
    }

  /*
   * 5 - testDisconnectedInteriorShellSelfTouchAtVertex
   *
   * Ensure that the Disconnected Interior condition is not validated
   */
    template<>
    template<>
    void object::test<5>()
    {
    string wkt = "POLYGON ((20 20, 20 100, 140 100, 140 180, 260 180, 260 100, 140 100, 140 20, 20 20))";
    checkIsValidSTR(wkt, false);
    checkIsValidDefault(wkt, false);
    }

    /// 6 - testShellCross()
    template<>
    template<>
    void object::test<6>()
    {
    string wkt = "POLYGON ((20 20, 120 20, 120 220, 240 220, 240 120, 20 120, 20 20))";
    checkIsValidSTR(wkt, false);
    checkIsValidDefault(wkt, false);
    }

    /// 7 - testShellCrossAndSTR
    template<>
    template<>
    void object::test<7>()
    {
    string wkt = "POLYGON ((20 20, 120 20, 120 220, 180 220, 140 160, 200 160, 180 220, 240 220, 240 120, 20 120,  20 20))";
    checkIsValidSTR(wkt, false);
    checkIsValidDefault(wkt, false);
    }



} // namespace tut
