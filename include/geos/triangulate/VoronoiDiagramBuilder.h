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
 * Last port: triangulate/VoronoiDiagramBuilder.java r524
 *
 **********************************************************************/

#ifndef GEOS_TRIANGULATE_VORONOIDIAGRAMBUILDER_H
#define GEOS_TRIANGULATE_VORONOIDIAGRAMBUILDER_H

#include <geos/triangulate/quadedge/QuadEdgeSubdivision.h>
#include <memory>
#include <iostream>

namespace geos {
  namespace geom{
	class Geometry;
	class CoordinateSequence;
	class GeometryCollection;
	class GeometryFactory;
	class Envelope;
}
namespace triangulate { //geos.triangulate

/**
 * A utility class which creates Voronoi Diagrams
 * from collections of points.
 * The diagram is returned as a {@link GeometryCollection} of {@link Polygon}s,
 * clipped to the larger of a supplied envelope or to an envelope determined
 * by the input sites.
 * 
 * @author Martin Davis
 *
 */
class GEOS_DLL VoronoiDiagramBuilder{
private:
	geom::CoordinateSequence* siteCoords;
	double tolerance;
	quadedge::QuadEdgeSubdivision* subdiv;
	geom::Envelope* clipEnv;
	geom::Envelope* diagramEnv;
	
public:
	/** 
	 * Creates a new Voronoi diagram builder.
	 *
	 */
	VoronoiDiagramBuilder();
	
	~VoronoiDiagramBuilder();
	
	/**
	 * Sets the sites (point or vertices) which will be diagrammed.
	 * All vertices of the given geometry will be used as sites.
	 * 
	 * @param geom the geometry from which the sites will be extracted.
	 */
	void setSites(const geom::Geometry& geom);
	
	/**
	 * Sets the sites (point or vertices) which will be diagrammed
	 * from a collection of {@link Coordinate}s.
	 * 
	 * @param coords a collection of Coordinates.
	 */
	void setSites(const geom::CoordinateSequence& coords);
	
	/**
	 * Sets the envelope to clip the diagram to.
	 * The diagram will be clipped to the larger
	 * of this envelope or an envelope surrounding the sites.
	 * 
	 * @param clipEnv the clip envelope.
	 */
	void setClipEnvelope(const geom::Envelope& clipEnv);
	
	/**
	 * Sets the snapping tolerance which will be used
	 * to improved the robustness of the triangulation computation.
	 * A tolerance of 0.0 specifies that no snapping will take place.
	 * 
	 * @param tolerance the tolerance distance to use
	 */
	void setTolerance(const double tolerance);
	
	/**
	 * Gets the {@link QuadEdgeSubdivision} which models the computed diagram.
	 * 
	 * @return the subdivision containing the triangulation
	 */
	quadedge::QuadEdgeSubdivision* getSubdivision();
	
	/**
	 * Gets the faces of the computed diagram as a {@link GeometryCollection} 
	 * of {@link Polygon}s, clipped as specified.
	 * 
	 * @param geomFact the geometry factory to use to create the output
	 * @return the faces of the diagram
	 */
	std::auto_ptr<geom::GeometryCollection> getDiagram(const geom::GeometryFactory& geomFact);

private:
	void create();
	
	static std::auto_ptr<geom::GeometryCollection> 
		clipGeometryCollection(const geom::GeometryCollection& geom, const geom::Envelope& clipEnv);

};

} //namespace geos.triangulate
} //namespace geos

#endif //GEOS_TRIANGULATE_VORONOIDIAGRAMBUILDER_H
