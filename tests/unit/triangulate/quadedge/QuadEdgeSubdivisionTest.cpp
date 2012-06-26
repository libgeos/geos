
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
//#include <geos/io/WKTWriter.h>
#include <geos/geom/Envelope.h>
#include <geos/geom/Coordinate.h>
// std
#include <stdio.h>

using namespace geos::triangulate::quadedge;
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
		test_quadedgesub_data()
		{
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

} // namespace tut


