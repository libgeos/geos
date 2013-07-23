
// 
// Test Suite for geos::triangulate::quadedge::QuadEdge
//
// tut
#include <tut.hpp>
// geos
#include <geos/triangulate/quadedge/Vertex.h>
#include <geos/triangulate/quadedge/QuadEdge.h>
#include <geos/triangulate/quadedge/QuadEdgeSubdivision.h>
#include <geos/geom/GeometryCollection.h>
#include <geos/geom/GeometryFactory.h>

#include <geos/triangulate/VoronoiDiagramBuilder.h>
//#include <geos/io/WKTWriter.h>
#include <geos/io/WKTReader.h>
#include <geos/geom/Envelope.h>
#include <geos/geom/Coordinate.h>
// std
#include <stdio.h>

using namespace geos::triangulate::quadedge;
using namespace geos::triangulate;
using namespace geos::geom;
using namespace geos::io;

namespace tut
{
	//
	// Test Group
	//

	// dummy data, not used
	struct test_quadedgesub_data
	{
		WKTReader reader;
		geos::triangulate::VoronoiDiagramBuilder builder;
		Geometry* sites;
		QuadEdgeSubdivision* subdiv;
		test_quadedgesub_data()
		{
			sites = reader.read("MULTIPOINT ((150 200), (180 270), (275 163))");
			builder.setSites(*sites);
			subdiv = builder.getSubdivision();
		}
	};

	typedef test_group<test_quadedgesub_data> group;
	typedef group::object object;

	group test_quadedgesub_group("geos::triangulate::quadedge::QuadEdgeSubdivision");


	//
	// Test Cases
	//

	// 1 - Basic function test
	template<>
	template<>
	void object::test<1>()
	{
		//create subdivision centered around (0,0)
		QuadEdgeSubdivision sub(Envelope(-100, 100, -100, 100), .00001);
		//stick a point in the middle
		QuadEdge& e = sub.insertSite(Vertex(0, 0));
		ensure(sub.isFrameEdge(e));
		ensure(sub.isOnEdge(e, e.orig().getCoordinate()));
		ensure(sub.isVertexOfEdge(e, e.orig()));

		ensure(!sub.isOnEdge(e, Coordinate(10, 10)));
		ensure(!sub.isVertexOfEdge(e, Vertex(10, 10)));

		GeometryFactory geomFact;
		std::auto_ptr<GeometryCollection> tris = sub.getTriangles(geomFact);
		tris.reset();
		//WKTWriter wkt;
		//printf("%s\n", wkt.writeFormatted(tris).c_str());
	}

	//Use the Voronoi subdiv to get the QuadEdgeSubdivision methods::
	//Testing getVoronoiDiagram::
	template<>
	template<>
	void object::test<2>()
	{
		GeometryFactory geomFact;
		std::auto_ptr<GeometryCollection> results;

		
		results = subdiv->getVoronoiDiagram(geomFact);

		const char *expected_str = "GEOMETRYCOLLECTION (POLYGON ((-5849.974929324658 2268.0517257497568, -4529.9920486948895 2247.139449440667, 221.20588235294116 210.91176470588235, -684.4227119984187 -2848.644297291955, -5849.974929324658 2268.0517257497568)), POLYGON ((212.5 -3774.5, -684.4227119984187 -2848.644297291955, 221.20588235294116 210.91176470588235, 2448.7167655626645 2188.608343256571, 6235.0370264064295 2248.0370264064295, 212.5 -3774.5)), POLYGON ((-4529.9920486948895 2247.139449440667, 2448.7167655626645 2188.608343256571, 221.20588235294116 210.91176470588235, -4529.9920486948895 2247.139449440667)))";

		Geometry *expected = reader.read(expected_str);
		results->normalize();
		expected->normalize();

		ensure(results->equalsExact(expected, 1e-7));
		ensure(results->getCoordinateDimension() == expected->getCoordinateDimension());
	}
} // namespace tut


