/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2005-2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#ifndef GEOS_PRECISION_ENHANCEDPRECISIONOP_H
#define GEOS_PRECISION_ENHANCEDPRECISIONOP_H

#include <geos/platform.h> // for int64

// Forward declarations
namespace geos {
	namespace geom {
		class Geometry;
	}
}

namespace geos {
namespace precision { // geos.precision

/** \brief
 * Provides versions of Geometry spatial functions which use
 * enhanced precision techniques to reduce the likelihood of robustness
 * problems.
 */
class EnhancedPrecisionOp {

public:

	/** \brief
	 * Computes the set-theoretic intersection of two
	 * Geometrys, using enhanced precision.
	 *
	 * @param geom0 the first Geometry
	 * @param geom1 the second Geometry
	 * @return the Geometry representing the set-theoretic
	 * intersection of the input Geometries.
	 */
	static geom::Geometry* intersection(geom::Geometry *geom0,
			geom::Geometry *geom1);

	/**
	 * Computes the set-theoretic union of two Geometrys,
	 * using enhanced precision.
	 * @param geom0 the first Geometry
	 * @param geom1 the second Geometry
	 * @return the Geometry representing the set-theoretic
	 * union of the input Geometries.
	 */
	static geom::Geometry* Union(geom::Geometry *geom0,
			geom::Geometry *geom1);

	/**
	 * Computes the set-theoretic difference of two Geometrys,
	 * using enhanced precision.
	 * @param geom0 the first Geometry
	 * @param geom1 the second Geometry
	 * @return the Geometry representing the set-theoretic
	 * difference of the input Geometries.
	 */
	static geom::Geometry* difference(geom::Geometry *geom0,
			geom::Geometry *geom1);

	/**
	 * Computes the set-theoretic symmetric difference of two
	 * Geometrys, using enhanced precision.
	 * @param geom0 the first Geometry
	 * @param geom1 the second Geometry
	 * @return the Geometry representing the set-theoretic symmetric
	 * difference of the input Geometries.
	 */
	static geom::Geometry* symDifference(geom::Geometry *geom0,
			geom::Geometry *geom1);

	/**
	 * Computes the buffer of a Geometry, using enhanced precision.
	 * This method should no longer be necessary, since the buffer
	 * algorithm now is highly robust.
	 *
	 * @param geom0 the first Geometry
	 * @param distance the buffer distance
	 * @return the Geometry representing the buffer of the input Geometry.
	 */
	static geom::Geometry* buffer(geom::Geometry *geom, double distance);
};


} // namespace geos.precision
} // namespace geos

#endif // GEOS_PRECISION_ENHANCEDPRECISIONOP_H

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/03/23 09:17:19  strk
 * precision.h header split, minor optimizations
 *
 **********************************************************************/
