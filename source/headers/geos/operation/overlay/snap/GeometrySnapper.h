/**********************************************************************
 * $Id: GeometrySnapper.h 1948 2006-12-18 18:44:20Z strk $
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2009  Sandro Santilli <strk@keybit.net>
 * Copyright (C) 2006 Refractions Research Inc.
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

#include <geos/geom/Coordinate.h>

#include <memory>
#include <vector>

// Forward declarations
namespace geos {
	namespace geom { 
		//class PrecisionModel;
		class Geometry;
		class CoordinateSequence;
	}
}

namespace geos {
namespace operation { // geos::operation
namespace overlay { // geos::operation::overlay
namespace snap { // geos::operation::overlay::snap

/** \brief
 * Snaps the vertices and segments of a geometry to another's vertices.
 * Should improve robustness for overlay operations.
 */
class GEOS_DLL GeometrySnapper {

public:

	typedef std::auto_ptr<geom::Geometry> GeomPtr;
	typedef std::pair<GeomPtr, GeomPtr> GeomPtrPair;

	/**
	 * Snaps two geometries together with a given tolerance.
	 *
	 * @param g0 a geometry to snap
	 * @param g1 a geometry to snap
	 * @param snapTolerance the tolerance to use
	 * @param ret the snapped geometries as a pair of auto_ptrs
	 *            (output parameter)
	 */
	static void snap(const geom::Geometry& g0,
	                        const geom::Geometry& g1,
	                        double snapTolerance, GeomPtrPair& ret);

	/**
	 * Creates a new snapper acting on the given geometry
	 *
	 * @param g the geometry to snap
	 */
	GeometrySnapper(const geom::Geometry& g)
		:
		srcGeom(g)
	{
	}

	/** \brief
	 * Snaps the vertices in the component {@link LineString}s
	 * of the source geometry to the vertices of the given snap geometry
	 * with a given snap tolerance
	 *
	 * @param g a geometry to snap the source to
	 * @param snapTolerance
	 * @return a new snapped Geometry
	 */
	std::auto_ptr<geom::Geometry> snapTo(const geom::Geometry& g,
	                                     double snapTolerance);

	/** \brief
	 * Estimates the snap tolerance for a Geometry, taking into account
	 * its precision model.
	 *
	 * @param g a Geometry
	 * @return the estimated snap tolerance
	 */
	static double computeOverlaySnapTolerance(const geom::Geometry& g);

	static double computeSizeBasedSnapTolerance(const geom::Geometry& g);

	/** \brief
	 * Computes the snap tolerance based on input geometries;
	 */
	static double computeOverlaySnapTolerance(const geom::Geometry& g1,
			const geom::Geometry& g2);


private:

	// eventually this will be determined from the geometry topology
	//static const double snapTol; //  = 0.000001;

	static const double snapPrecisionFactor; //  = 10e-10

	const geom::Geometry& srcGeom;

	/// Extract target (unique) coordinates
	std::auto_ptr<geom::Coordinate::ConstVect> extractTargetCoordinates(
			const geom::Geometry& g);
};


} // namespace geos::operation::overlay::snap
} // namespace geos::operation::overlay
} // namespace geos::operation
} // namespace geos

#endif // GEOS_OP_OVERLAY_SNAP_GEOMETRYSNAPPER_H

