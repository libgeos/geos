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

#ifndef GEOS_PRECISION_COMMONBITSOP_H
#define GEOS_PRECISION_COMMONBITSOP_H

#include <vector>

namespace geos {
	namespace geom {
		class Geometry;
	}
	namespace precision {
		class CommonBitsRemover;
	}
}

namespace geos {
namespace precision { // geos.precision

/** \brief
 * Provides versions of Geometry spatial functions which use
 * common bit removal to reduce the likelihood of robustness problems.
 * 
 * In the current implementation no rounding is performed on the
 * reshifted result geometry, which means that it is possible
 * that the returned Geometry is invalid.
 * Client classes should check the validity of the returned result themselves.
 */
class CommonBitsOp {

private:

	bool returnToOriginalPrecision;

	CommonBitsRemover *cbr;

	/**
	 * Computes a copy of the input Geometry with the calculated
	 * common bits removed from each coordinate.
	 * @param geom0 the Geometry to remove common bits from
	 * @return a copy of the input Geometry with common bits removed
	 */
	geom::Geometry* removeCommonBits(geom::Geometry *geom0);

	/**
	 * Computes a copy of each input Geometry with the calculated
	 * common bits
	 * removed from each coordinate.
	 * @param geom0 a Geometry to remove common bits from
	 * @param geom1 a Geometry to remove common bits from
	 * @return an array containing copies
	 * of the input Geometry's with common bits removed
	 */
	std::vector<geom::Geometry*>* removeCommonBits(
			geom::Geometry *geom0,
			geom::Geometry *geom1);

public:

	/**
	 * Creates a new instance of class, which reshifts result Geometry
	 */
	CommonBitsOp();

	/**
	 * Creates a new instance of class, specifying whether
	 * the result {@link Geometry}s should be reshifted.
	 *
	 * @param returnToOriginalPrecision
	 */
	CommonBitsOp(bool nReturnToOriginalPrecision);

	/**
	 * Computes the set-theoretic intersection of two Geometry,
	 * using enhanced precision.
	 * @param geom0 the first Geometry
	 * @param geom1 the second Geometry
	 * @return the Geometry representing the set-theoretic
	 *  intersection of the input Geometries.
	 */
	geom::Geometry* intersection(geom::Geometry *geom0,
			geom::Geometry *geom1);

	/**
	 * Computes the set-theoretic union of two Geometry,
	 * using enhanced precision.
	 * @param geom0 the first Geometry
	 * @param geom1 the second Geometry
	 * @return the Geometry representing the set-theoretic union
	 * of the input Geometries.
	 */
	geom::Geometry* Union(geom::Geometry *geom0, geom::Geometry *geom1);

	/**
	 * Computes the set-theoretic difference of two Geometry,
	 * using enhanced precision.
	 * @param geom0 the first Geometry
	 * @param geom1 the second Geometry, to be subtracted from the first
	 * @return the Geometry representing the set-theoretic difference
	 * of the input Geometries.
	 */
	geom::Geometry* difference(geom::Geometry *geom0,
			geom::Geometry *geom1);

	/**
	 * Computes the set-theoretic symmetric difference of two geometries,
	 * using enhanced precision.
	 * @param geom0 the first Geometry
	 * @param geom1 the second Geometry
	 * @return the Geometry representing the set-theoretic symmetric
	 * difference of the input Geometries.
	 */
	geom::Geometry* symDifference(geom::Geometry *geom0,
			geom::Geometry *geom1);

	/**
	 * Computes the buffer a geometry,
	 * using enhanced precision.
	 * @param geom0 the Geometry to buffer
	 * @param distance the buffer distance
	 * @return the Geometry representing the buffer of the input Geometry.
	 */
	geom::Geometry* buffer(geom::Geometry *geom0, double distance);

	/**
	 * If required, returning the result to the orginal precision
	 * if required.
	 * 
	 * In this current implementation, no rounding is performed on the
	 * reshifted result geometry, which means that it is possible
	 * that the returned Geometry is invalid.
	 *
	 * @param result the result Geometry to modify
	 * @return the result Geometry with the required precision
	 */
	geom::Geometry* computeResultPrecision(geom::Geometry *result);
};


} // namespace geos.precision
} // namespace geos

#endif // GEOS_PRECISION_COMMONBITSOP_H

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/03/23 09:17:19  strk
 * precision.h header split, minor optimizations
 *
 **********************************************************************/
