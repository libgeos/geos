// $Id: ValidClosedRingTest.cpp 2809 2009-12-06 01:05:24Z mloskot $
// 
// Test Suite for geos::operation::valid::IsValidOp class
// Ported from JTS junit/operation/valid/ValidClosedRingTest.java rev. 1.1

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

    struct test_validclosedring_data
    {
	typedef std::auto_ptr<Geometry> GeomPtr;

        geos::geom::PrecisionModel pm_;
        geos::geom::GeometryFactory factory_;
        geos::io::WKTReader rdr;

        test_validclosedring_data()
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

        void updateNonClosedRing(LinearRing& ring)
	{
		CoordinateSequence& pts = *(const_cast<CoordinateSequence*>(
			ring.getCoordinatesRO()
					    ));
		Coordinate c = pts[0];
		c.x += 0.0001;
		pts.setAt(c, 0);
	}

	void checkIsValid(Geometry& geom, bool expected)
	{
		IsValidOp validator(&geom);
		bool isValid = validator.isValid();
		ensure_equals(isValid, expected);
	}

    };

    typedef test_group<test_validclosedring_data> group;
    typedef group::object object;

    group test_validclosedring_group("geos::operation::valid::ValidClosedRing");

    //
    // Test Cases
    //

    // 1 - testBadLinearRing
    template<>
    template<>
    void object::test<1>()
    {
	GeomPtr geom = fromWKT("LINEARRING (0 0, 0 10, 10 10, 10 0, 0 0)");
	LinearRing* ring_chk = dynamic_cast<LinearRing*>(geom.get());
	ensure(0 != ring_chk);
	LinearRing& ring = *ring_chk;
	//cout << ring.toString() << endl;
	updateNonClosedRing(ring);
	//cout << ring.toString() << endl;
	checkIsValid(*geom, false);
    }

    // 2 - testGoodLinearRing
    template<>
    template<>
    void object::test<2>()
    {
	GeomPtr geom = fromWKT("LINEARRING (0 0, 0 10, 10 10, 10 0, 0 0)");
	checkIsValid(*geom, true);
    }

    // 3 - testBadPolygonShell
    template<>
    template<>
    void object::test<3>()
    {
	GeomPtr geom = fromWKT("POLYGON ((0 0, 0 10, 10 10, 10 0, 0 0), (1 1, 2 1, 2 2, 1 2, 1 1) ))");
	Polygon* poly = dynamic_cast<Polygon*>(geom.get());
	ensure(0 != poly);
	const LineString* ring = poly->getExteriorRing();

	const LinearRing* lr = dynamic_cast<const LinearRing*>(ring);
	ensure(0 != lr);

	LinearRing* nclr = const_cast<LinearRing*>(lr);

	updateNonClosedRing(*nclr);
	checkIsValid(*geom, false);
    }

    // 4 - testBadPolygonHole
    template<>
    template<>
    void object::test<4>()
    {
	GeomPtr geom = fromWKT("POLYGON ((0 0, 0 10, 10 10, 10 0, 0 0), (1 1, 2 1, 2 2, 1 2, 1 1) ))");
	Polygon* poly = dynamic_cast<Polygon*>(geom.get());
	ensure(0 != poly);
	const LineString* ring = poly->getInteriorRingN(0);

	const LinearRing* lr = dynamic_cast<const LinearRing*>(ring);
	ensure(0 != lr);

	LinearRing* nclr = const_cast<LinearRing*>(lr);

	updateNonClosedRing(*nclr);
	checkIsValid(*geom, false);
    }

    // 5 - testGoodPolygon
    template<>
    template<>
    void object::test<5>()
    {
	GeomPtr geom = fromWKT("POLYGON ((0 0, 0 10, 10 10, 10 0, 0 0))");
	checkIsValid(*geom, true);
    }


} // namespace tut
