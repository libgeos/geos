/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2012 Excensus LLC.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: triangulate/DelaunayTriangulationBuilder.java rev. r524
 *
 **********************************************************************/

#include <geos/triangulate/VoronoiDiagramBuilder.h>

#include <algorithm>
#include <math.h>
#include <vector>
#include <iostream>

#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/Envelope.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/triangulate/IncrementalDelaunayTriangulator.h>
#include <geos/triangulate/DelaunayTriangulationBuilder.h>
#include <geos/triangulate/quadedge/QuadEdgeSubdivision.h>
// WKT writer
#include <geos/io/WKTWriter.h>

namespace geos {
namespace triangulate { //geos.triangulate

using namespace geos::geom;


VoronoiDiagramBuilder::VoronoiDiagramBuilder() :
	siteCoords(NULL), tolerance(0.0), subdiv(NULL) , clipEnv(NULL), diagramEnv(NULL)
{
}

VoronoiDiagramBuilder::~VoronoiDiagramBuilder()
{
	if(siteCoords)
		delete siteCoords;
	if(subdiv)
		delete subdiv;
	if(clipEnv)
	   	delete clipEnv;
	if(diagramEnv)
	   	delete diagramEnv;
}


void VoronoiDiagramBuilder::setSites(const geom::Geometry& geom)
{
   siteCoords = DelaunayTriangulationBuilder::extractUniqueCoordinates(geom);
//   std::cout << "Size After "<< siteCoords->getSize() << std::endl;
}

void VoronoiDiagramBuilder::setSites(const geom::CoordinateSequence& coords)
{
   CoordinateSequence* coords_cpy = coords.clone();
   DelaunayTriangulationBuilder::unique(*coords_cpy);
   siteCoords = coords_cpy->clone();

   delete coords_cpy;
}

void VoronoiDiagramBuilder::setClipEnvelope(const geom::Envelope& clpEnv)
{
   *clipEnv = clpEnv;
}

void VoronoiDiagramBuilder::setTolerance(const double toler)
{
   tolerance = toler;
}


void VoronoiDiagramBuilder::create()
{
   if(subdiv!=NULL)
      return;
   geom::Envelope siteEnv = DelaunayTriangulationBuilder::envelope(*siteCoords);
   std::cout << siteEnv.toString() << endl;
   std::cout << siteEnv.getWidth() << endl;
   std::cout << siteEnv.getHeight() << endl;
//   diagramEnv = &siteEnv;
   diagramEnv = new Envelope();
   *diagramEnv = siteEnv;
//   std::cout << "diagramEnv::"<< diagramEnv->toString() << endl;
//   std::cout << "siteEnv::"<< siteEnv.toString() << endl;
   //adding buffer around the final envelope
   double expandBy = fmax(diagramEnv->getWidth() , diagramEnv->getHeight());
   diagramEnv->expandBy(expandBy);
   if(clipEnv!=NULL)
      diagramEnv->expandToInclude(clipEnv);

   IncrementalDelaunayTriangulator::VertexList* vertices = DelaunayTriangulationBuilder::toVertices(*siteCoords);

   subdiv = new quadedge::QuadEdgeSubdivision(*diagramEnv,tolerance);
   IncrementalDelaunayTriangulator triangulator(subdiv);
   triangulator.insertSites(*vertices);
   delete vertices;
//   std::cout << "diagramEnv::"<< diagramEnv->toString() << endl;
//   std::cout << "siteEnv::"<< siteEnv.toString() << endl;
}

quadedge::QuadEdgeSubdivision* VoronoiDiagramBuilder::getSubdivision()
{
   create();
   return subdiv;
}

std::auto_ptr<geom::GeometryCollection>
VoronoiDiagramBuilder::getDiagram(const geom::GeometryFactory& geomFact)
{
	geos::io::WKTWriter writer;
	create();
	std::auto_ptr<geom::GeometryCollection> polys = subdiv->getVoronoiDiagram(geomFact);

//	std::cout << "This is before clip fun called::" << diagramEnv->toString() << endl;
//	cout << writer.write(polys.get()) << endl;
//	cout << "Out of the getVoronoiDiagram methods" << endl;
	return clipGeometryCollection(*polys,*diagramEnv);
}

std::auto_ptr<geom::GeometryCollection> 
VoronoiDiagramBuilder::clipGeometryCollection(const geom::GeometryCollection& geom, const geom::Envelope& clipEnv)
{
	geos::io::WKTWriter writer;
	geom::Geometry* clipPoly = geom.getFactory()->toGeometry(&clipEnv);
	std::vector<Geometry*> clipped;
//	cout << "Envelope details::" << endl;
//	cout << "Height:: " << clipEnv.getHeight() << " " << "Width::" << clipEnv.getWidth() << endl << endl;
	for(int i=0 ; i < geom.getNumGeometries() ; i++)
	{
		Geometry* g = (Geometry*)geom.getGeometryN(i);

//		cout << writer.write(g) << endl << endl;
		//	   Geometry* g = ge->clone();
		Geometry* result=NULL;

		// don't clip unless necessary
		if(clipEnv.contains(g->getEnvelopeInternal()))
		{
//			cout << "This g in contained in clipEnv::" << writer.write(g) << endl << endl;
			result = g;
		}
		else if(clipEnv.intersects(g->getEnvelopeInternal()))
		{
//			cout << "This g intersects in clipEnv::" << writer.write(g) << endl << endl;
			result = clipPoly->intersection(g);
			result->setUserData(g->getUserData());
		}

		if(result!=NULL && !result->isEmpty() )
		{
//			cout << "This g is pushed in clipped vector::" << writer.write(g) << endl << endl;
			clipped.push_back(result);
		}
//		delete g;
	}
	for(std::vector<Geometry*>::iterator it=clipped.begin() ; it!= clipped.end() ; ++it)
	{
		cout << (*it)->toString() << endl << endl;
	}
	GeometryCollection* ret = geom.getFactory()->createGeometryCollection(clipped);
	return std::auto_ptr<GeometryCollection>(ret);
}

} //namespace geos.triangulate
} //namespace goes
