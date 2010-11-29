/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2010      Sandro Santilli <strk@keybit.net>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: original work
 *
 **********************************************************************/

#ifndef GEOS_OPERATION_SHAREDPATHSOP_H
#define GEOS_OPERATION_SHAREDPATHSOP_H


#include <vector> 

// Forward declarations
namespace geos {
	namespace geom {
		//class LineString;
		class Geometry;
	}
}


namespace geos {
namespace operation { // geos.operation
namespace sharedpaths { // geos.operation.sharedpaths

/** \brief
 * Find shared paths among two linear Geometry objects
 *
 * For each shared path report if it direction is the same
 * or opposite.
 *
 */
class GEOS_DLL SharedPathsOp
{
public:

	/// Find paths shared between two linear geometries
	//
	/// @param g1
	///	First geometry. Must be linear.
	///
	/// @param g2
	///	Second geometry. Must be linear.
	///
	/// @param tol
	///	Tolerance by which very close paths are considered shared.
	///	TODO: specify more about the semantic, check SnapOp 
	///
	/// @param sameDir
	///	Shared edges having the same direction are pushed
	///     onto this vector. They'll be of type LineString.
	///	Ownership of the edges is tranferred.
	///
	/// @param oppositeDir
	///	Shared edges having the opposite direction are pushed
	///     onto this vector. They'll be of type LineString.
	///	Ownership of the edges is tranferred.
	///
	static void SharedPathsOp(const Geometry& g1, const Geometry& g2,
		double tol,
		std::vector<Geometry*>& sameDirection,
		std::vector<Geometry*>& oppositeDirection);

};

} // namespace geos.operation.sharedpaths
} // namespace geos.operation
} // namespace geos

#endif

