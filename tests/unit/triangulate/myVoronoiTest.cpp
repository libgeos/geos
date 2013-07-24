// geos
#include <geos/triangulate/quadedge/QuadEdge.h>
#include <geos/triangulate/quadedge/QuadEdgeSubdivision.h>
#include <geos/triangulate/VoronoiDiagramBuilder.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateArraySequence.h>

#include <geos/io/WKTWriter.h>
#include <geos/io/WKTReader.h>
#include <geos/geom/GeometryCollection.h>
#include <geos/geom/GeometryFactory.h>
#include <vector>
#include <stdio.h>

using namespace geos::triangulate;
using namespace geos::triangulate::quadedge;
using namespace geos::geom;
using namespace geos::io;
using geos::geom::CoordinateSequence;
using namespace std;

int main()
{
	WKTReader reader;
	WKTWriter writer;
	geos::triangulate::VoronoiDiagramBuilder builder;
//	Geometry* sites = reader.read("MULTIPOINT ((150 240), (180 300), (300 290), (230 330), (244 284), (230 250), (150 240) , (230 250))");
//	Geometry* sites = reader.read("MULTIPOINT ((71 85) , (130 150) , (130 160) )");
//	Geometry* sites = reader.read("MULTIPOINT ((150 200), (180 270), (275 163))");
//	Geometry* sites = reader.read("MULTIPOINT ((170 210), (190 240), (230 280), (296 225), (245 183), (200 230), (200 140), (330 1    80), (150 290), (210 330))");
	Geometry* sites = reader.read("MULTIPOINT ((150 200), (180 270), (275 163))");

	std::auto_ptr<GeometryCollection> results;
	
	Coordinate a(180,300);
	Coordinate b(300,290);
	Coordinate c(230,330);
	Coordinate d(244,284);
	Coordinate e(230,250);
	Coordinate f(150,240);
	Coordinate g(250,240);
	Coordinate h(230,330);
	Coordinate i(200,300);
	std::vector<Coordinate>* v = new std::vector<Coordinate>();
	v->push_back(a);
	v->push_back(b);
	v->push_back(c);
	v->push_back(d);
	v->push_back(e);
	v->push_back(f);
	v->push_back(g);
	v->push_back(h);
	v->push_back(i);

	geos::geom::CoordinateArraySequence *seq = new CoordinateArraySequence(v);
	builder.setSites(*sites);

	//getting the subdiv()
	QuadEdgeSubdivision* subdiv = builder.getSubdivision();

//	cout << "Tolerance::" <<subdiv->getTolerance() << endl;
//	cout << "Envelope" << subdiv->getEnvelope().toString() << endl;
	GeometryFactory geomFact;

//	results = subdiv->getVoronoiDiagram(geomFact);
	std::vector<Geometry*> vorCells = subdiv->getVoronoiCellPolygons( geomFact );

	for(std::vector<Geometry*>::iterator it=vorCells.begin() ; it!=vorCells.end() ; ++it)
	{
		cout << (*it)->toString() << endl;
	}
//	results = builder.getDiagram(geomFact);

//	string out = writer.write(results.get());
//	cout << out << endl;

	delete seq;
	delete sites;
	return 0;
}
