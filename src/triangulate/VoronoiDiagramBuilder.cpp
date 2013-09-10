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
 * Last port: triangulate/VoronoiDiagramBuilder.java rev. r524
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
	if(diagramEnv)
	   	delete diagramEnv;
}

void 
VoronoiDiagramBuilder::setSites(const geom::Geometry& geom)
{
	siteCoords = DelaunayTriangulationBuilder::extractUniqueCoordinates(geom);
}

void 
VoronoiDiagramBuilder::setSites(const geom::CoordinateSequence& coords)
{
	siteCoords = coords.clone();
	DelaunayTriangulationBuilder::unique(*siteCoords);
}

void 
VoronoiDiagramBuilder::setClipEnvelope(const geom::Envelope& clipEnv)
{
	*(this->clipEnv) = clipEnv;
}

void 
VoronoiDiagramBuilder::setTolerance(const double tolerance)
{
	this->tolerance = tolerance;
}

void 
VoronoiDiagramBuilder::create()
{
	if(subdiv!=NULL)
		return;
	geom::Envelope siteEnv = DelaunayTriangulationBuilder::envelope(*siteCoords);
	diagramEnv = new Envelope();
	*diagramEnv = siteEnv;
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
}

quadedge::QuadEdgeSubdivision* 
VoronoiDiagramBuilder::getSubdivision()
{
	create();
	return subdiv;
}

std::auto_ptr<geom::GeometryCollection>
VoronoiDiagramBuilder::getDiagram(const geom::GeometryFactory& geomFact)
{
	create();
	std::auto_ptr<geom::GeometryCollection> polys = subdiv->getVoronoiDiagram(geomFact);
	return clipGeometryCollection(*polys,*diagramEnv);
}

std::auto_ptr<geom::GeometryCollection> 
VoronoiDiagramBuilder::clipGeometryCollection(const geom::GeometryCollection& geom, const geom::Envelope& clipEnv)
{
	geom::Geometry* clipPoly = geom.getFactory()->toGeometry(&clipEnv);
	std::auto_ptr< std::vector<Geometry*> >clipped(new std::vector<Geometry*>);
	for(std::size_t i=0 ; i < geom.getNumGeometries() ; i++)
	{
		Geometry* g = (Geometry*)geom.getGeometryN(i);
		Geometry* result=NULL;
		// don't clip unless necessary
		if(clipEnv.contains(g->getEnvelopeInternal()))
		{
			result = g->clone();
		}
		else if(clipEnv.intersects(g->getEnvelopeInternal()))
		{
			result = clipPoly->intersection(g);
			result->setUserData(g->getUserData());
		}

		if(result!=NULL && !result->isEmpty() )
		{
			clipped->push_back(result);
		}
	}
	delete clipPoly;
	return std::auto_ptr<GeometryCollection>(geom.getFactory()->createGeometryCollection(clipped.release()));
}

} //namespace geos.triangulate
} //namespace geos
