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
	tolerance(0.0), clipEnv(0)
{
}

VoronoiDiagramBuilder::~VoronoiDiagramBuilder()
{
}

void 
VoronoiDiagramBuilder::setSites(const geom::Geometry& geom)
{
	siteCoords.reset( DelaunayTriangulationBuilder::extractUniqueCoordinates(geom) );
}

void 
VoronoiDiagramBuilder::setSites(const geom::CoordinateSequence& coords)
{
	siteCoords.reset( coords.clone() );
	DelaunayTriangulationBuilder::unique(*siteCoords);
}

void 
VoronoiDiagramBuilder::setClipEnvelope(const geom::Envelope* nClipEnv)
{
	clipEnv = nClipEnv;
}

void 
VoronoiDiagramBuilder::setTolerance(double nTolerance)
{
	tolerance = nTolerance;
}

void 
VoronoiDiagramBuilder::create()
{
	if( subdiv.get() ) return;

	diagramEnv = DelaunayTriangulationBuilder::envelope(*siteCoords);
	//adding buffer around the final envelope
	double expandBy = std::max(diagramEnv.getWidth() , diagramEnv.getHeight());
	diagramEnv.expandBy(expandBy);
	if(clipEnv)
		diagramEnv.expandToInclude(clipEnv);

	std::auto_ptr<IncrementalDelaunayTriangulator::VertexList> vertices (
    DelaunayTriangulationBuilder::toVertices(*siteCoords)
  );

	subdiv.reset( new quadedge::QuadEdgeSubdivision(diagramEnv,tolerance) );
	IncrementalDelaunayTriangulator triangulator(subdiv.get());
	triangulator.insertSites(*vertices);
}

std::auto_ptr<quadedge::QuadEdgeSubdivision> 
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
	return clipGeometryCollection(*polys,diagramEnv);
}

std::auto_ptr<geom::Geometry>
VoronoiDiagramBuilder::getDiagramEdges(const geom::GeometryFactory& geomFact)
{
	create();
	std::auto_ptr<geom::MultiLineString> edges = subdiv->getVoronoiDiagramEdges(geomFact);
  if ( edges->isEmpty() ) return std::auto_ptr<Geometry>(edges.release());
  std::auto_ptr<geom::Geometry> clipPoly ( geomFact.toGeometry(&diagramEnv) );
  std::auto_ptr<Geometry> clipped( clipPoly->intersection(edges.get()) );
	return clipped;
}

std::auto_ptr<geom::GeometryCollection> 
VoronoiDiagramBuilder::clipGeometryCollection(const geom::GeometryCollection& geom, const geom::Envelope& clipEnv)
{
	std::auto_ptr<geom::Geometry> clipPoly ( geom.getFactory()->toGeometry(&clipEnv) );
	std::auto_ptr< std::vector<Geometry*> >clipped(new std::vector<Geometry*>);
	for(std::size_t i=0 ; i < geom.getNumGeometries() ; i++)
	{
		const Geometry* g = geom.getGeometryN(i);
		std::auto_ptr<Geometry> result;
		// don't clip unless necessary
		if(clipEnv.contains(g->getEnvelopeInternal()))
		{
			result.reset( g->clone() );
      // TODO: check if userData is correctly cloned here?
		}
		else if(clipEnv.intersects(g->getEnvelopeInternal()))
		{
			result.reset( clipPoly->intersection(g) );
			result->setUserData(((Geometry*)g)->getUserData()); // TODO: needed ?
		}

		if(result.get() && !result->isEmpty() )
		{
			clipped->push_back(result.release());
		}
	}
	return std::auto_ptr<GeometryCollection>(geom.getFactory()->createGeometryCollection(clipped.release()));
}

} //namespace geos.triangulate
} //namespace geos
