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
#include <cmath>

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

	// 4 - Test that SRID is output only once
  // See http://trac.osgeo.org/geos/ticket/583
	template<>
	template<>
	void object::test<4>()
	{         
    typedef geos::geom::Geometry Geom;
    typedef std::vector<Geom *> GeomVect;
    GeomVect *geoms = new GeomVect;
    geoms->push_back( wktreader.read("POLYGON((0 0,1 0,1 1,0 1,0 0))") );
    geoms->back()->setSRID(4326);
    Geom *geom = gf.createGeometryCollection(geoms);
    std::stringstream result_stream;

    wkbwriter.setOutputDimension( 2 );
    wkbwriter.setByteOrder( 1 );
    wkbwriter.setIncludeSRID( 1 );
    wkbwriter.writeHEX( *geom, result_stream );
    delete geom;

    std::string actual = result_stream.str();
    ensure_equals( actual, "0107000000010000000103000000010000000500000000000000000000000000000000000000000000000000F03F0000000000000000000000000000F03F000000000000F03F0000000000000000000000000000F03F00000000000000000000000000000000");

  }


} // namespace tut

