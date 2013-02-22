// 
// Test Suite for geos::io::WKTWriter 

// tut
#include <tut.hpp>
// geos
#include <geos/io/WKTReader.h>
#include <geos/io/WKTWriter.h>
#include <geos/io/Writer.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Geometry.h>
// std
#include <sstream>
#include <string>
#include <memory>
#include <iostream>

namespace tut
{
	//
	// Test Group
	//

	// dummy data, not used
	struct test_writer_data
	{
		typedef geos::geom::PrecisionModel PrecisionModel;
		typedef geos::geom::GeometryFactory GeometryFactory;
		typedef geos::io::WKTReader WKTReader;
		typedef geos::io::WKTWriter WKTWriter;
		typedef geos::io::Writer Writer;
		typedef std::auto_ptr<geos::geom::Geometry> GeomPtr;

		PrecisionModel pm;
		GeometryFactory gf;
		WKTReader wktreader;
		WKTWriter wktwriter;
		Writer writer;

		test_writer_data()
                :
                pm(1000.0),
                gf(&pm),
                wktreader(&gf),
		writer(20U)
            {}

	};

	typedef test_group<test_writer_data> group;
	typedef group::object object;

	group test_writer_group("geos::io::Writer");


	//
	// Test Cases
	//

	// 1 - Test allocating a too large buffer.
	template<>
	template<>
	void object::test<1>()
	{         
        GeomPtr geom ( wktreader.read("POINT(-117 33)") );

        wktwriter.setTrim(true);
	
	std::string check_alloc = writer.toString();
	std::size_t allocated_size = check_alloc.capacity();

        wktwriter.write( geom.get(), &writer );
	std::string result = writer.toString();

        ensure_equals( result, "POINT (-117 33)" );
	
	ensure_equals( "Allocated size from the Writer string is not correct",  result.capacity(), allocated_size );

	ensure_equals( "Actual size from the Writer string is not correct", result.size(), 15U );
    }

	// 2 - Test allocating a too small buffer.
	template<>
	template<>
	void object::test<2>()
	{         
        GeomPtr geom ( wktreader.read("LINESTRING(-117 33, 100 20)") );

        wktwriter.setTrim(false);

        wktwriter.write( geom.get(), &writer );
	std::string result = writer.toString();

        ensure_equals( result, "LINESTRING (-117.000 33.000, 100.000 20.000)" );
	
	ensure(result.capacity() > 20U);
	
	ensure_equals( "Actual size from the Writer string is not correct", result.size(), 44U );
    }
    
} // namespace tut

