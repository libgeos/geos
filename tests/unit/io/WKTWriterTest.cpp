// $Id: WKBReaderTest.cpp 2344 2009-04-09 21:46:30Z mloskot $
// 
// Test Suite for geos::io::WKTWriter 

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
		typedef geos::geom::PrecisionModel PrecisionModel;
		typedef geos::geom::GeometryFactory GeometryFactory;
		typedef geos::io::WKTReader WKTReader;
		typedef geos::io::WKTWriter WKTWriter;
		typedef std::auto_ptr<geos::geom::Geometry> GeomPtr;

		PrecisionModel pm;
		GeometryFactory gf;
		WKTReader wktreader;
		WKTWriter wktwriter;

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
        GeomPtr geom ( wktreader.read("POINT(-117 33)") );
        std::string  result;

        wktwriter.setTrim(false);
        result = wktwriter.write( geom.get() );

        ensure_equals( result , "POINT (-117.000 33.000)" );

        wktwriter.setTrim(true);
        result = wktwriter.write( geom.get() );

        ensure_equals( result , "POINT (-117 33)" );
    }

	// 2 - Test the output precision capability
	template<>
	template<>
	void object::test<2>()
	{         
        GeomPtr geom ( wktreader.read("POINT(-117.1234567 33.1234567)") );
        std::string  result;

        wktwriter.setTrim(false);
        result = wktwriter.write( geom.get() );

        ensure_equals( result , "POINT (-117.123 33.123)" );

        wktwriter.setRoundingPrecision(2);
        result = wktwriter.write( geom.get() );

        ensure_equals( result , "POINT (-117.12 33.12)" );

    }
    
	// 3 - Test 3D generation from a 3D geometry.
	template<>
	template<>
	void object::test<3>()
	{         
        GeomPtr geom ( wktreader.read("POINT Z (-117 33 120)") );
        std::string  result;

        wktwriter.setOutputDimension(3);
        wktwriter.setTrim( true );
        wktwriter.setOld3D( false );

        result = wktwriter.write( geom.get() );

        ensure_equals( result, std::string("POINT Z (-117 33 120)") );

        wktwriter.setOld3D( true );
        result = wktwriter.write( geom.get() );

        ensure_equals( result, std::string("POINT (-117 33 120)") );

    }
    
	// 4 - Test 2D generation from a 3D geometry.
	template<>
	template<>
	void object::test<4>()
	{         
        GeomPtr geom ( wktreader.read("POINT(-117 33 120)") );
        std::string  result;

        wktwriter.setOutputDimension(2);
        wktwriter.setTrim( true );
        wktwriter.setOld3D( false );

        result = wktwriter.write( geom.get() );

        ensure_equals( result, std::string("POINT (-117 33)") );
    }

  // 5 - Test negative number of digits in precision model 
  template<>
  template<>
  void object::test<5>()
  {         
    PrecisionModel pm3(0.001);
    GeometryFactory gf3(&pm3);
    WKTReader wktreader3(&gf3);
    GeomPtr geom ( wktreader3.read("POINT(123456 654321)") );

    std::string  result = wktwriter.write( geom.get() );
    ensure_equals( result, std::string("POINT (123000 654000)") );
  }
    
} // namespace tut

