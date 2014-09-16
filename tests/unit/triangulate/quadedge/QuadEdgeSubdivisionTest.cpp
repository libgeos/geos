
// 
// Test Suite for geos::triangulate::quadedge::QuadEdge
//
// tut
#include <tut.hpp>
// geos
#include <geos/triangulate/quadedge/Vertex.h>
#include <geos/triangulate/quadedge/QuadEdge.h>
#include <geos/triangulate/quadedge/QuadEdgeSubdivision.h>
#include <geos/triangulate/DelaunayTriangulationBuilder.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/geom/LineString.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/GeometryCollection.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/io/WKTWriter.h>
#include <geos/io/WKTReader.h>
#include <geos/geom/Envelope.h>
#include <geos/geom/Coordinate.h>
// std
#include <stdio.h>
#include <iostream>
#include <algorithm>
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
		geos::geom::PrecisionModel pm;
		geos::geom::GeometryFactory gf;
		geos::io::WKTReader reader;
		geos::io::WKTWriter writer;
		test_quadedgesub_data()
      :
			pm(),
			gf(&pm),
			reader(gf),
			writer()
		{
			writer.setTrim(true);
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
	template<>
	template<>
	void object::test<2>()
	{
		Geometry* sites;
		QuadEdgeSubdivision* subdiv;
		sites = reader.read("MULTIPOINT ((150 200), (180 270), (275 163))");
		CoordinateSequence* siteCoords = DelaunayTriangulationBuilder::extractUniqueCoordinates(*sites);
		Envelope Env = DelaunayTriangulationBuilder::envelope(*siteCoords);

		double expandBy = std::max(Env.getWidth() , Env.getHeight());
		Env.expandBy(expandBy);

		IncrementalDelaunayTriangulator::VertexList* vertices = DelaunayTriangulationBuilder::toVertices(*siteCoords);
		subdiv = new quadedge::QuadEdgeSubdivision(Env,0);
		IncrementalDelaunayTriangulator triangulator(subdiv);
		triangulator.insertSites(*vertices);

		//Test for getVoronoiDiagram::
		GeometryFactory geomFact;
		std::auto_ptr<GeometryCollection> polys = subdiv->getVoronoiDiagram(geomFact);
		const char *expected_str = "GEOMETRYCOLLECTION (POLYGON ((-5849.974929324658 2268.0517257497568, -4529.9920486948895 2247.139449440667, 221.20588235294116 210.91176470588235, -684.4227119984187 -2848.644297291955, -5849.974929324658 2268.0517257497568)), POLYGON ((212.5 -3774.5, -684.4227119984187 -2848.644297291955, 221.20588235294116 210.91176470588235, 2448.7167655626645 2188.608343256571, 6235.0370264064295 2248.0370264064295, 212.5 -3774.5)), POLYGON ((-4529.9920486948895 2247.139449440667, 2448.7167655626645 2188.608343256571, 221.20588235294116 210.91176470588235, -4529.9920486948895 2247.139449440667)))";
//		std::cout << polys->toString() << std::endl;

		Geometry *expected = reader.read(expected_str);
		polys->normalize();
		expected->normalize();
		ensure(polys->equalsExact(expected, 1e-7));
		delete siteCoords;
		delete sites;
		delete subdiv;
		delete vertices;
		delete expected;
//		ensure(polys->getCoordinateDimension() == expected->getCoordinateDimension());
	}

  // Test that returned polygons do not have duplicated points
  // See http://trac.osgeo.org/geos/ticket/705
  template<> template<> void object::test<3>()
  {
    const char *wkt = 
      "MULTIPOINT ("
        " (170 270),"
        " (190 230),"
        " (230 250),"
        " (210 290)"
      ")";
    std::auto_ptr<Geometry> sites ( reader.read(wkt) );
    std::auto_ptr<CoordinateSequence> siteCoords (
      DelaunayTriangulationBuilder::extractUniqueCoordinates(*sites)
    );

    Envelope env = DelaunayTriangulationBuilder::envelope(*siteCoords);
    double expandBy = std::max(env.getWidth() , env.getHeight());
    env.expandBy(expandBy);
    std::auto_ptr<QuadEdgeSubdivision> subdiv(
      new quadedge::QuadEdgeSubdivision(env,10)
    );

    std::auto_ptr<IncrementalDelaunayTriangulator::VertexList> vertices (
      DelaunayTriangulationBuilder::toVertices(*siteCoords)
    );

    IncrementalDelaunayTriangulator triangulator(subdiv.get());
    triangulator.insertSites(*vertices);

    //Test for getVoronoiDiagram::
    GeometryFactory geomFact;
    std::auto_ptr<GeometryCollection> polys = subdiv->getVoronoiDiagram(geomFact);
    for (std::size_t i=0; i<polys->getNumGeometries(); ++i) {
      const Polygon* p = dynamic_cast<const Polygon*>(polys->getGeometryN(i));
      ensure(p);
      std::auto_ptr<CoordinateSequence> cs (
        p->getExteriorRing()->getCoordinates()
      );
      size_t from = cs->size();
      cs->removeRepeatedPoints();
      ensure_equals(from, cs->size());
    }
  }

} // namespace tut


