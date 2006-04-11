/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: simplify/DouglasPeuckerSimplifier.java rev. 1.5 (JTS-1.7)
 *
 **********************************************************************/

#ifndef _GEOS_SIMPLIFY_DOUBGLASPEUCKERSIMPLIFIER_H_
#define _GEOS_SIMPLIFY_DOUBGLASPEUCKERSIMPLIFIER_H_ 

#include <geos/geom/Geometry.h> // for dtor in auto_ptr

#include <memory> // for auto_ptr

// Forward declarations
namespace geos {
	namespace geom {
		//class Coordinate;
		//class CoordinateSequence;
	}
}

namespace geos {
namespace simplify { // geos::simplify


/** \brief
 * Simplifies a Geometry using the standard Douglas-Peucker algorithm.
 *
 * Ensures that any polygonal geometries returned are valid.
 * Simple lines are not guaranteed to remain simple after simplification.
 * 
 * Note that in general D-P does not preserve topology -
 * e.g. polygons can be split, collapse to lines or disappear
 * holes can be created or disappear,
 * and lines can cross.
 * To simplify geometry while preserving topology use TopologyPreservingSimplifier.
 * (However, using D-P is significantly faster).
 *
 */
class DouglasPeuckerSimplifier {

public:

	static geom::Geometry::AutoPtr simplify(
			const geom::Geometry* geom,
			double tolerance);

	DouglasPeuckerSimplifier(const geom::Geometry* geom);

	/** \brief
	 * Sets the distance tolerance for the simplification.
	 *
	 * All vertices in the simplified geometry will be within this
	 * distance of the original geometry.
	 * The tolerance value must be non-negative.  A tolerance value
	 * of zero is effectively a no-op.
	 *
	 * @param distanceTolerance the approximation tolerance to use
	 */
	void setDistanceTolerance(double tolerance);

	geom::Geometry::AutoPtr getResultGeometry();


private:

	const geom::Geometry* inputGeom;

	double distanceTolerance;
};


} // namespace geos::simplify
} // namespace geos

#endif // _GEOS_SIMPLIFY_DOUBGLASPEUCKERSIMPLIFIER_H_ 

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/04/11 16:04:34  strk
 * geos::simplify::DouglasPeukerSimplifier class + unit test
 *
 **********************************************************************/
