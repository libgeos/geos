/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2009  Sandro Santilli <strk@keybit.net>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 ***********************************************************************
 *
 * Last port: operation/overlay/snap/GeometrySnapper.java rev 1.8 (JTS-1.10)
 *
 **********************************************************************/

#ifndef GEOS_OP_OVERLAY_SNAP_GEOMETRYSNAPPER_H
#define GEOS_OP_OVERLAY_SNAP_GEOMETRYSNAPPER_H

#include <geos/geom/LineSegment.h> // for composition
#include <geos/geom/LineString.h> // for inlined ctor

//#include <vector>
#include <memory> // for auto_ptr

// Forward declarations
namespace geos {
	namespace geom {
		class CoordinateSequence;
	}
}

namespace geos {
namespace operation { // geos::operation
namespace overlay { // geos::operation::overlay
namespace snap { // geos::operation::overlay::snap

/** \brief
 * Snaps the vertices and segments of a geom::Geometry to another
 * Geometry's vertices.
 * 
 * Improves robustness for overlay operations, by eliminating
 * nearly parallel edges (which cause problems during noding and
 * intersection calculation).
 *
 */
class GeometrySnapper
{

public:

	/** \brief
	 * Estimates the snap tolerance for a Geometry, taking into account
	 * its precision model.
	 *
	 * @param g a Geometry
	 * @return the estimated snap tolerance
	 */
	static double computeOverlaySnapTolerance(const geom::Geometry& g);

	static double computeSizeBasedSnapTolerance(const geom::Geometry& g);

	static double computeOverlaySnapTolerance(const geom::Geometry& g0,
	                                          const geom::Geometry& g1);


	typedef std::auto_ptr<geom::Geometry> GeomPtr;
	typedef std::pair<GeomPtr, GeomPtr> GeomPtrPair;

	/**
	 * Snaps two geometries together with a given tolerance.
	 *
	 * @param g0 a geometry to snap
	 * @param g1 a geometry to snap
	 * @param snapTolerance the tolerance to use
	 * @return the snapped geometries as a pair of auto_ptrs
	 */
	static GeomPtrPair snap(const geom::Geometry& g0,
	                        const geom::Geometry& g1,
	                        double snapTolerance);

	/**
	 * Creates a new snapper acting on the given geometry
	 *
	 * @param nSrcGeom the geometry to snap
	 */
	GeometrySnapper(const geom::Geometry& nSrcGeom)
		:
		srcGeom(nSrcGeom)
	{}

	/**
	 * Snaps the vertices in the component {@link LineString}s
	 * of the source geometry
	 * to the vertices of the given snap geometry.
	 *
	 * @param snapGeom a geometry to snap the source to
	 * @param snapTolerance
	 *
	 * @return a new snapped Geometry
	 */
	GeomPtr snapTo(const geom::Geometry& snapGeom, double snapTolerance);

	// why is this public ??
	void extractTargetCoordinates(const geom::Geometry& g,
	                    std::vector<const geom::Coordinate*>& target);
 

private:

	static const double SNAP_PRECISION_FACTOR; // = 10e-10;

	const geom::Geometry& srcGeom;
};

} // namespace geos::operation::overlay::snap
} // namespace geos::operation::overlay
} // namespace geos::operation
} // namespace geos

#endif // ndef GEOS_OP_OVERLAY_SNAP_GEOMETRYSNAPPER_H

/**********************************************************************
 * $Log$
 **********************************************************************/

