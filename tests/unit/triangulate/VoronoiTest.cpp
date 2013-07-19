
// 
// Test Suite for geos::triangulate::quadedge::QuadEdge
//
// tut
#include <tut.hpp>
// geos
#include <geos/triangulate/quadedge/QuadEdge.h>
#include <geos/triangulate/quadedge/QuadEdgeSubdivision.h>
#include <geos/triangulate/IncrementalDelaunayTriangulator.h>
#include <geos/triangulate/VoronoiDiagramBuilder.h>

#include <geos/io/WKTWriter.h>
#include <geos/io/WKTReader.h>
#include <geos/geom/GeometryCollection.h>
#include <geos/geom/GeometryFactory.h>

//#include <stdio.h>

using namespace geos::triangulate;
using namespace geos::triangulate::quadedge;
using namespace geos::geom;
using namespace geos::io;

namespace tut
{
	//
	// Test Group
	//

	// dummy data, not used
	struct test_voronoidiag_data
	{
		test_voronoidiag_data()
		{
		}
	};

	typedef test_group<test_voronoidiag_data> group;
	typedef group::object object;

	group test_voronoidiag_group("geos::triangulate::Voronoi");

	//helper function for funning triangulation
	void runVoronoi(const char *sitesWkt, const char *expectedWkt)
	{
		WKTReader reader;
		WKTWriter writer;
		std::auto_ptr<GeometryCollection> results;
		Geometry *sites = reader.read(sitesWkt);
		Geometry *expected = reader.read(expectedWkt);
		VoronoiDiagramBuilder builder;
		GeometryFactory geomFact;

		builder.setSites(*sites);
		results=builder.getDiagram(geomFact);

		std::string out = writer.write(results.get());

	//	std::cout << out << endl;
		results->normalize();
		expected->normalize();
			
		ensure(results->equalsExact(expected, 1e-7));
		ensure(results->getCoordinateDimension() == expected->getCoordinateDimension());

		delete sites;
		delete expected;
	}

	// Test Cases
	// 1 - test
	template<>
	template<>
	void object::test<1>()
	{
		const char *wkt = "MULTIPOINT ((150 200), (180 270), (275 163))";

		const char *expected = "GEOMETRYCOLLECTION (POLYGON ((25 38, 25 295, 221.20588235294116 210.91176470588235, 170.024 38, 25 38)), POLYGON ((400 369.6542056074766, 400 38, 170.024 38, 221.20588235294116 210.91176470588235, 400 369.6542056074766)), POLYGON ((25 295, 25 395, 400 395, 400 369.6542056074766, 221.20588235294116 210.91176470588235, 25 295)))";


		runVoronoi(wkt,expected);
				
	}
} // namespace tut

