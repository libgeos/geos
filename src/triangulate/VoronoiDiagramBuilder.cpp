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
//	if(diagramEnv)
//	   	delete diagramEnv;
}


void VoronoiDiagramBuilder::setSites(const geom::Geometry& geom)
{
   siteCoords = DelaunayTriangulationBuilder::extractUniqueCoordinates(geom);
//   std::cout << "Size After "<< siteCoords->getSize() << std::endl;
}

void VoronoiDiagramBuilder::setSites(const geom::CoordinateSequence& coords)
{
//   std::cout << "Size before:: "<< coords.getSize() << std::endl;
   CoordinateSequence* coords_cpy = coords.clone();
   DelaunayTriangulationBuilder::unique(*coords_cpy);
   siteCoords = coords_cpy->clone();

//   std::string str = siteCoords->toString();		//remove
//   std::cout << str << std::endl;			//remove
//   std::cout << "Size after:: "<<siteCoords->getSize() << std::endl;
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
   diagramEnv = &siteEnv;
   std::cout << diagramEnv->toString() << endl;
   //adding buffer around the final envelope
   double expandBy = fmax(diagramEnv->getWidth() , diagramEnv->getHeight());
   diagramEnv->expandBy(expandBy);
   if(clipEnv!=NULL)
      diagramEnv->expandToInclude(clipEnv);

   IncrementalDelaunayTriangulator::VertexList* vertices = DelaunayTriangulationBuilder::toVertices(*siteCoords);

   subdiv = new quadedge::QuadEdgeSubdivision(siteEnv,tolerance);
   IncrementalDelaunayTriangulator triangulator(subdiv);
   triangulator.insertSites(*vertices);
   delete vertices;
}

quadedge::QuadEdgeSubdivision* VoronoiDiagramBuilder::getSubdivision()
{
   create();
   std::cout << "Done with create()\n";
   return subdiv;
}

std::auto_ptr<geom::GeometryCollection>
VoronoiDiagramBuilder::getDiagram(const geom::GeometryFactory& geomFact)
{
	create();
	std::auto_ptr<geom::GeometryCollection> polys = subdiv->getVoronoiDiagram(geomFact);

	geom::GeometryCollection* pol = polys.get();
	return clipGeometryCollection(*pol,*diagramEnv);
}

std::auto_ptr<geom::GeometryCollection> 
VoronoiDiagramBuilder::clipGeometryCollection(const geom::GeometryCollection& geom, const geom::Envelope& clipEnv)
{
   geom::Geometry* clipPoly = geom.getFactory()->toGeometry((Envelope*)&clipEnv);
   std::vector<Geometry*> clipped;
   for(std::size_t i=0 ; i < geom.getNumGeometries() ; i++)
   {
	   const Geometry* ge = geom.getGeometryN(i);
	   Geometry* g = ge->clone();
	   Geometry* result=NULL;

	   if(clipEnv.contains(g->getEnvelopeInternal()))
		   result = g;
	   else if(clipEnv.intersects(g->getEnvelopeInternal()))
	   {
		   result = clipPoly->intersection(ge);
		   result->setUserData(g->getUserData());
	   }

	   if(result!=NULL && !result->isEmpty() )
	   {
		   clipped.push_back(result);
	   }
	   delete g;
   }
   GeometryCollection* ret = geom.getFactory()->createGeometryCollection(clipped);
   return std::auto_ptr<GeometryCollection>(ret);
}

} //namespace geos.triangulate
} //namespace goes
