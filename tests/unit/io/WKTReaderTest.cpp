// $Id: WKBReaderTest.cpp 2344 2009-04-09 21:46:30Z mloskot $
// 
// Test Suite for geos::io::WKTReader 

// tut
#include <tut.hpp>
// geos
#include <geos/io/WKTReader.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/CoordinateSequence.h>
// std
#include <sstream>
#include <string>
#include <memory>

namespace tut
{
	//
	// Test Group
	//

	// dummy data, not used
	struct test_wktreader_data
	{
		geos::geom::PrecisionModel pm;
		geos::geom::GeometryFactory gf;
		geos::io::WKTReader wktreader;

		typedef std::auto_ptr<geos::geom::Geometry> GeomPtr;

		test_wktreader_data()
			:
			pm(1.0),
			gf(&pm),
			wktreader(&gf)
		{}

	};

	typedef test_group<test_wktreader_data> group;
	typedef group::object object;

	group test_wktreader_group("geos::io::WKTReader");


	//
	// Test Cases
	//

	// 1 - Read a point, confirm 2D.
	template<>
	template<>
	void object::test<1>()
	{         
            GeomPtr geom(wktreader.read("POINT(-117 33)"));
            geos::geom::CoordinateSequence *coords = geom->getCoordinates();

            ensure( coords->getDimension() == 2 );
            ensure( coords->getX(0) == -117 );
            ensure( coords->getY(0) == 33 );
            delete coords;
        }

	// 2 - Read a point, confirm 3D.
	template<>
	template<>
	void object::test<2>()
	{         
            GeomPtr geom(wktreader.read("POINT(-117 33 10)"));
            geos::geom::CoordinateSequence *coords = geom->getCoordinates();

            ensure( coords->getDimension() == 3 );
            ensure( coords->getOrdinate(0,geos::geom::CoordinateSequence::Z) == 10.0 );
            delete coords;
        }

	// 3 - Linestring dimension preserved.
	template<>
	template<>
	void object::test<3>()
	{         
            GeomPtr geom(wktreader.read("LINESTRING(-117 33, -116 34)"));
            geos::geom::CoordinateSequence *coords = geom->getCoordinates();

            ensure( coords->getDimension() == 2 );

            delete coords;
        }

} // namespace tut

