/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 ***********************************************************************
 *
 * Last port: operation/overlay/snap/GeometrySnapper.java rev. 1.6
 * (we should move in GEOS too, probably)
 *
 **********************************************************************/

#ifndef GEOS_PRECISION_GEOMETRYSNAPPER_H
#define GEOS_PRECISION_GEOMETRYSNAPPER_H

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
namespace precision { // geos.precision

/** \brief
 * Snaps the vertices and segments of a geometry to another's vertices.
 * Should improve robustness for overlay operations.
 */
class GeometrySnapper {

public:

	GeometrySnapper(const geom::Geometry& g)
		:
		srcGeom(g)
	{
	}

	/** \brief
	 * Snaps the vertices in the component {@link LineString}s
	 * of the source geometry to the vertices of the given geometry
	 * with a given snap tolerance
	 *
	 * @param g
	 * @param snapTolerance
	 * @return a new snapped Geometry
	 */
	std::auto_ptr<geom::Geometry> snapTo(const geom::Geometry& g, double snapTolerance);

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


} // namespace geos.precision
} // namespace geos

#endif // GEOS_PRECISION_GEOMETRYSNAPPER_H

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/07/21 17:09:14  strk
 * Added new precision::LineStringSnapper class + test
 * and precision::GeometrySnapper (w/out test)
 *
 **********************************************************************/
