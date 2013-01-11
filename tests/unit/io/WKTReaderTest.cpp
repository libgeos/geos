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
#include <geos/geom/CoordinateSequence.h>
#include <geos/util/IllegalArgumentException.h>
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
		geos::io::WKTWriter wktwriter;

		typedef std::auto_ptr<geos::geom::Geometry> GeomPtr;

		test_wktreader_data()
			:
			pm(1.0),
			gf(&pm),
			wktreader(&gf)
		{
            wktwriter.setOutputDimension( 3 );
        }

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

	// 4 - Ensure we can read ZM geometries, just discarding the M.
	template<>
	template<>
	void object::test<4>()
	{         
            GeomPtr geom(wktreader.read("LINESTRING ZM (-117 33 2 3, -116 34 4 5)"));
            geos::geom::CoordinateSequence *coords = geom->getCoordinates();

            ensure( coords->getDimension() == 3 );

            ensure_equals( wktwriter.write(geom.get()), 
                           std::string("LINESTRING Z (-117 33 2, -116 34 4)") );

            delete coords;
    }

	// 5 - Check support for mixed case keywords (and old style 3D)
	template<>
	template<>
	void object::test<5>()
	{         
            GeomPtr geom(wktreader.read("LineString (-117 33 2, -116 34 4)"));
            ensure_equals( wktwriter.write(geom.get()), 
                           std::string("LINESTRING Z (-117 33 2, -116 34 4)") );
    }

    // 6 - invalid WKT (see http://trac.osgeo.org/geos/ticket/361)
    template<>
    template<>
    void object::test<6>()
    {         
        GeomPtr geom;

        try {
            geom.reset(wktreader.read("POLYGON( EMPTY, (1 1,2 2,1 2,1 1))"));
            ensure( !"Didn't get expected exception" );
        } catch (const geos::util::IllegalArgumentException& ex) {
            ensure( "Did get expected exception" );
            ex.what();
        } catch (...) {
            ensure( !"Got unexpected exception" );
        }
    }

    // POINT(0 0) http://trac.osgeo.org/geos/ticket/610
    template<>
    template<>
    void object::test<7>()
    {         
        GeomPtr geom;

        try
        {
            // use FLOATING model
            namespace ggm = geos::geom;
            namespace gio = geos::io;
            ggm::PrecisionModel pm(ggm::PrecisionModel::FLOATING);
            ggm::GeometryFactory gf(&pm);
            gio::WKTReader wktReader(&gf);
            const std::string str = " POINT (0 0) ";
            geom.reset(wktReader.read(str)); //HERE IT FAILS

            geos::geom::CoordinateSequence *coords = geom->getCoordinates();
            ensure_equals(coords->getDimension(), 2U);
            ensure_distance(coords->getX(0), 0.0, 1e-12);
            ensure_distance( coords->getY(0), 0.0, 1e-12);
            delete coords;
        }
        catch (const geos::util::IllegalArgumentException& ex)
        {
            ensure( "Did get expected exception" );
            ex.what();
        }
        catch (...)
        {
            ensure( !"Got unexpected exception" );
        }
    }
} // namespace tut


