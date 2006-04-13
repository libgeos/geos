// $Id$
// 
// Test Suite for geos::io::WKBReader 
// Uses geos::io::WKTReader to check correctness 
// Uses geos::io::WKBWriter to check correctness 
// Currently only tests 2D geoms of all (7) types.
// Currently only tests NDR WKB 

// TUT
#include <tut.h>
// GEOS
#include <geos/io/WKBReader.h>
#include <geos/io/WKBConstants.h>
#include <geos/io/WKBWriter.h>
#include <geos/io/WKTReader.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Geometry.h>
#include <sstream>
#include <memory>

namespace tut
{
	//
	// Test Group
	//

	// dummy data, not used
	struct test_wkbreader_data
	{
		geos::geom::PrecisionModel pm;
		geos::geom::GeometryFactory gf;
		geos::io::WKBReader wkbreader;
		geos::io::WKBWriter xdrwkbwriter;
		geos::io::WKBWriter ndrwkbwriter;
		geos::io::WKTReader wktreader;

		typedef std::auto_ptr<geos::geom::Geometry> GeomPtr;

		test_wkbreader_data()
			:
			pm(1.0),
			gf(&pm),
			wkbreader(gf),
			// 2D only, XDR (big endian)
			xdrwkbwriter(2, geos::io::WKBConstants::wkbXDR),
			// 2D only, NDR (little endian)
			ndrwkbwriter(2, geos::io::WKBConstants::wkbNDR),
			wktreader(&gf)
		{}

	};

	typedef test_group<test_wkbreader_data> group;
	typedef group::object object;

	group test_wkbreader_group("geos::io::WKBReader");


	//
	// Test Cases
	//

	// 1 - Read a point
	template<>
	template<>
	void object::test<1>()
	{         
		// NDR
		std::stringstream sin("010100000000000000000000000000000000000000");

		GeomPtr gWKB(wkbreader.readHEX(sin));
		GeomPtr gWKT(wktreader.read("POINT(0 0)"));

		ensure( gWKB->equalsExact(gWKT.get()) );

		std::stringstream sout;

		ndrwkbwriter.writeHEX(*gWKB, sout);

		ensure_equals(sout.str(), sin.str());

	}

	// 2 - Read a linestring
	template<>
	template<>
	void object::test<2>()
	{         
		// NDR
		std::stringstream sin("010200000002000000000000000000F03F000000000000004000000000000008400000000000001040");

		GeomPtr gWKB (wkbreader.readHEX(sin));
		GeomPtr gWKT (wktreader.read("LINESTRING(1 2, 3 4)"));

		ensure( gWKB->equalsExact(gWKT.get()) );

		std::stringstream sout;
		ndrwkbwriter.writeHEX(*gWKB, sout);

		ensure_equals(sout.str(), sin.str());

	}

	// 3 - Read a polygon
	template<>
	template<>
	void object::test<3>()
	{         
		// NDR
		std::stringstream sin("0103000000020000000500000000000000000000000000000000000000000000000000244000000000000000000000000000002440000000000000244000000000000000000000000000002440000000000000000000000000000000000400000000000000000000400000000000000040000000000000004000000000000018400000000000001840000000000000104000000000000000400000000000000040");

		GeomPtr gWKB (wkbreader.readHEX(sin));
		GeomPtr gWKT (wktreader.read("POLYGON((0 0, 10 0, 10 10, 0 10, 0 0),(2 2, 2 6, 6 4, 2 2))"));

		ensure( gWKB->equalsExact(gWKT.get()) );

		std::stringstream sout;
		ndrwkbwriter.writeHEX(*gWKB, sout);

		ensure_equals(sout.str(), sin.str());
	}

	// 4 - Read a multipoint
	template<>
	template<>
	void object::test<4>()
	{         
		// NDR
		std::stringstream sin("010400000005000000010100000000000000000000000000000000000000010100000000000000000024400000000000000000010100000000000000000024400000000000002440010100000000000000000000000000000000002440010100000000000000000000000000000000000000");

		GeomPtr gWKB (wkbreader.readHEX(sin));
		GeomPtr gWKT (wktreader.read("MULTIPOINT(0 0, 10 0, 10 10, 0 10, 0 0)"));

		ensure( gWKB->equalsExact(gWKT.get()) );

		std::stringstream sout;
		ndrwkbwriter.writeHEX(*gWKB, sout);

		ensure_equals(sout.str(), sin.str());
	}

	// 5 - Read a multilinestring
	template<>
	template<>
	void object::test<5>()
	{         
		// NDR
		std::stringstream sin("010500000002000000010200000005000000000000000000000000000000000000000000000000002440000000000000000000000000000024400000000000002440000000000000000000000000000024400000000000002440000000000000344001020000000400000000000000000000400000000000000040000000000000004000000000000018400000000000001840000000000000104000000000000034400000000000000040");

		GeomPtr gWKB (wkbreader.readHEX(sin));
		GeomPtr gWKT (wktreader.read("MULTILINESTRING((0 0, 10 0, 10 10, 0 10, 10 20),(2 2, 2 6, 6 4, 20 2))"));

		ensure( gWKB->equalsExact(gWKT.get()) );

		std::stringstream sout;
		ndrwkbwriter.writeHEX(*gWKB, sout);

		ensure_equals(sout.str(), sin.str());
	}

	// 6 - Read a multipolygon
	template<>
	template<>
	void object::test<6>()
	{         
		// NDR
		std::stringstream sin("0106000000020000000103000000020000000500000000000000000000000000000000000000000000000000244000000000000000000000000000002440000000000000244000000000000000000000000000002440000000000000000000000000000000000400000000000000000000400000000000000040000000000000004000000000000018400000000000001840000000000000104000000000000000400000000000000040010300000001000000040000000000000000004E400000000000004E400000000000004E400000000000004940000000000080514000000000000044400000000000004E400000000000004E40");

		GeomPtr gWKB (wkbreader.readHEX(sin));
		GeomPtr gWKT (wktreader.read("MULTIPOLYGON(((0 0, 10 0, 10 10, 0 10, 0 0),(2 2, 2 6, 6 4, 2 2)),((60 60, 60 50, 70 40, 60 60)))"));

		ensure( gWKB->equalsExact(gWKT.get()) );

		std::stringstream sout;
		ndrwkbwriter.writeHEX(*gWKB, sout);

		ensure_equals(sout.str(), sin.str());
	}

	// 7 - Read a collection 
	template<>
	template<>
	void object::test<7>()
	{         
		// NDR
		std::stringstream sin("010700000006000000010100000000000000000000000000000000000000010200000002000000000000000000F03F00000000000000400000000000000840000000000000104001030000000200000005000000000000000000000000000000000000000000000000002440000000000000000000000000000024400000000000002440000000000000000000000000000024400000000000000000000000000000000004000000000000000000004000000000000000400000000000000040000000000000184000000000000018400000000000001040000000000000004000000000000000400104000000050000000101000000000000000000000000000000000000000101000000000000000000244000000000000000000101000000000000000000244000000000000024400101000000000000000000000000000000000024400101000000000000000000000000000000000000000105000000020000000102000000050000000000000000000000000000000000000000000000000024400000000000000000000000000000244000000000000024400000000000000000000000000000244000000000000024400000000000003440010200000004000000000000000000004000000000000000400000000000000040000000000000184000000000000018400000000000001040000000000000344000000000000000400106000000020000000103000000020000000500000000000000000000000000000000000000000000000000244000000000000000000000000000002440000000000000244000000000000000000000000000002440000000000000000000000000000000000400000000000000000000400000000000000040000000000000004000000000000018400000000000001840000000000000104000000000000000400000000000000040010300000001000000040000000000000000004E400000000000004E400000000000004E400000000000004940000000000080514000000000000044400000000000004E400000000000004E40");

		GeomPtr gWKB(wkbreader.readHEX(sin));
		GeomPtr gWKT(wktreader.read("GEOMETRYCOLLECTION(POINT(0 0),LINESTRING(1 2,3 4),POLYGON((0 0,10 0,10 10,0 10,0 0),(2 2,2 6,6 4,2 2)),MULTIPOINT(0 0,10 0,10 10,0 10,0 0),MULTILINESTRING((0 0,10 0,10 10,0 10,10 20),(2 2,2 6,6 4,20 2)),MULTIPOLYGON(((0 0,10 0,10 10,0 10,0 0),(2 2,2 6,6 4,2 2)),((60 60,60 50,70 40,60 60))))"));

		ensure( gWKB->equalsExact(gWKT.get()) );

		std::stringstream sout;
		ndrwkbwriter.writeHEX(*gWKB, sout);

		ensure_equals(sout.str(), sin.str());
	}


} // namespace tut

