// $Id$
// 
// Test Suite for geos::io::WKBWriter

// tut
#include <tut.hpp>
// geos
#include <geos/io/WKBReader.h>
#include <geos/io/WKBWriter.h>
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
	struct test_wkbwriter_data
	{
		geos::geom::PrecisionModel pm;
		geos::geom::GeometryFactory gf;
		geos::io::WKTReader wktreader;
		geos::io::WKTWriter wktwriter;
		geos::io::WKBReader wkbreader;
		geos::io::WKBWriter wkbwriter;

		test_wkbwriter_data()
			:
			pm(1000.0),
			gf(&pm),
			wktreader(&gf),
            wkbreader(gf)
		{}

	};

	typedef test_group<test_wkbwriter_data> group;
	typedef group::object object;

	group test_wkbwriter_group("geos::io::WKBWriter");


	//
	// Test Cases
	//

	// 1 - Test writing a 2D geometry with the WKBWriter in 3D output dimension.
	template<>
	template<>
	void object::test<1>()
	{         
            geos::geom::Geometry *geom = wktreader.read("POINT(-117 33)");
			std::stringstream result_stream;

            wkbwriter.setOutputDimension( 3 );
            wkbwriter.write( *geom, result_stream );
            delete geom;
            geom = NULL;

            ensure( result_stream.str().length() == 21 );

            result_stream.seekg( 0 );
            geom = wkbreader.read( result_stream );

            ensure( geom != NULL );

            ensure( geom->getCoordinateDimension() == 2 );
            ensure( geom->getCoordinate()->x == -117.0 );
            ensure( geom->getCoordinate()->y == 33.0 );
            ensure( ISNAN(geom->getCoordinate()->z) != 0 );

            delete geom;
    }

	// 2 - Test writing a 3D geometry with the WKBWriter in 3D output dimension.
	template<>
	template<>
	void object::test<2>()
	{         
            geos::geom::Geometry *geom = wktreader.read("POINT(-117 33 11)");
			std::stringstream result_stream;

            wkbwriter.setOutputDimension( 3 );
            wkbwriter.write( *geom, result_stream );
            delete geom;
            geom = NULL;

            ensure( result_stream.str().length() == 29 );

            result_stream.seekg( 0 );
            geom = wkbreader.read( result_stream );

            ensure( geom->getCoordinateDimension() == 3 );
            ensure( geom->getCoordinate()->x == -117.0 );
            ensure( geom->getCoordinate()->y == 33.0 );
            ensure( geom->getCoordinate()->z == 11.0 );

            delete geom;
    }

	// 3 - Test writing a 3D geometry with the WKBWriter in 2D output dimension.
	template<>
	template<>
	void object::test<3>()
	{         
            geos::geom::Geometry *geom = wktreader.read("POINT(-117 33 11)");
			std::stringstream result_stream;

            wkbwriter.setOutputDimension( 2 );
            wkbwriter.write( *geom, result_stream );
            delete geom;
            geom = NULL;

            ensure( result_stream.str().length() == 21 );

            result_stream.seekg( 0 );
            geom = wkbreader.read( result_stream );

            ensure( geom->getCoordinateDimension() == 2 );
            ensure( geom->getCoordinate()->x == -117.0 );
            ensure( geom->getCoordinate()->y == 33.0 );
            ensure( ISNAN(geom->getCoordinate()->z) != 0);

            delete geom;
    }


} // namespace tut

