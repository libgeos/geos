/**********************************************************************
 * $Id: RelateOp.cpp 2549 2009-06-05 17:50:34Z strk $
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 * Copyright (C) 2005 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: operation/relate/RelateOp.java rev. 1.19 (JTS-1.10)
 *
 **********************************************************************/


#include <geos/operation/relate/RelateComputer.h>
#include <geos/operation/relate/RelateOp.h>

// Forward declarations
namespace geos {
	namespace geom {
		class IntersectionMatrix;
		class Geometry;
	}
}

using namespace geos::geom;

namespace geos {
namespace operation { // geos.operation
namespace relate { // geos.operation.relate

IntersectionMatrix*
RelateOp::relate(const Geometry *a, const Geometry *b)
{
	RelateOp relOp(a,b);
	return relOp.getIntersectionMatrix();
}

IntersectionMatrix*
RelateOp::relate(const Geometry *a, const Geometry *b,
		const algorithm::BoundaryNodeRule& boundaryNodeRule)
{
	RelateOp relOp(a, b, boundaryNodeRule);
	return relOp.getIntersectionMatrix();
}

RelateOp::RelateOp(const Geometry *g0, const Geometry *g1):
	GeometryGraphOperation(g0, g1),
	relateComp(&arg)
{
}

RelateOp::RelateOp(const Geometry *g0, const Geometry *g1,
		const algorithm::BoundaryNodeRule& boundaryNodeRule)
	:
	GeometryGraphOperation(g0, g1, boundaryNodeRule),
	relateComp(&arg)
{
}

RelateOp::~RelateOp()
{
}

IntersectionMatrix*
RelateOp::getIntersectionMatrix()
{
	return relateComp.computeIM();
}

} // namespace geos.operation.relate
} // namespace geos.operation
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.19  2006/03/21 13:11:29  strk
 * opRelate.h header split
 *
 * Revision 1.18  2006/03/17 16:48:55  strk
 * LineIntersector and PointLocator made complete components of RelateComputer
 * (were statics const pointers before). Reduced inclusions from opRelate.h
 * and opValid.h, updated .cpp files to allow build.
 *
 * Revision 1.17  2006/02/19 19:46:50  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 **********************************************************************/

