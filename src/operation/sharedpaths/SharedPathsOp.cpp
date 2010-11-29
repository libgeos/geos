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

#include <geos/operation/sharedpaths/SharedPathsOp.h>
#include <geos/util/IllegalArgumentException.h>

namespace geos {
namespace operation { // geos.operation
namespace sharedpaths { // geos.operation.sharedpaths

using namespace geos::geom;

/* public static */
void
SharedPathsOp::getSharedPaths(const Geometry& g1, const Geometry& g2,
		double tol,
		std::vector<Geometry*>& sameDirection,
		std::vector<Geometry*>& oppositeDirection)
{
	throw geos::util::IllegalArgumentException("Not implemented yet");
}

} // namespace geos.operation.sharedpaths
} // namespace geos::operation
} // namespace geos

