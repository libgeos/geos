// $Id: WKBReaderTest.cpp 2344 2009-04-09 21:46:30Z mloskot $
// 
// Test Suite for geos::io::WKTReader 

// tut
#include <tut.hpp>
// geos
#include <geos/io/WKTReader.h>
#include <geos/io/WKTWriter.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Geometry.h>
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
	struct test_wktwriter_data
	{
		geos::geom::PrecisionModel pm;
		geos::geom::GeometryFactory gf;
		geos::io::WKTReader wktreader;
		geos::io::WKTWriter wktwriter;

		test_wktwriter_data()
			:
			pm(1000.0),
			gf(&pm),
			wktreader(&gf)
		{}

	};

	typedef test_group<test_wktwriter_data> group;
	typedef group::object object;

	group test_wktwriter_group("geos::io::WKTWriter");


	//
	// Test Cases
	//

	// 1 - Test the trim capability.
	template<>
	template<>
	void object::test<1>()
	{         
            geos::geom::Geometry *geom = wktreader.read("POINT(-117 33)");
            std::string  result;

            wktwriter.setTrim(false);
            result = wktwriter.write( geom );

            ensure( result == "POINT (-117.0000 33.0000)" );

            wktwriter.setTrim(true);
            result = wktwriter.write( geom );

            ensure( result == "POINT (-117 33)" );

            delete geom;
        }

	// 2 - Test the output precision capability
	template<>
	template<>
	void object::test<2>()
	{         
            geos::geom::Geometry *geom = wktreader.read("POINT(-117.1234567 33.1234567)");
            std::string  result;

            wktwriter.setTrim(false);
            result = wktwriter.write( geom );

            ensure( result == "POINT (-117.1230 33.1230)" );

            wktwriter.setRoundingPrecision(2);
            result = wktwriter.write( geom );

            ensure( result == "POINT (-117.12 33.12)" );

            delete geom;
        }

} // namespace tut

