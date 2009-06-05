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
 **********************************************************************
 *
 * Last port: operation/relate/RelateOp.java rev. 1.17 (JTS-1.7)
 *
 * EXPOSED GEOS HEADER
 *
 **********************************************************************/

#ifndef GEOS_OP_RELATE_RELATEOP_H
#define GEOS_OP_RELATE_RELATEOP_H

#include <geos/operation/GeometryGraphOperation.h> // for inheritance
#include <geos/operation/relate/RelateComputer.h> // for composition

// Forward declarations
namespace geos {
	namespace geom {
		class IntersectionMatrix;
		class Geometry;
	}
}


namespace geos {
namespace operation { // geos::operation
namespace relate { // geos::operation::relate

/** \brief
 * Implements the relate() operation on Geometry.
 * 
 * WARNING: The current implementation of this class will compute a result for
 * GeometryCollections.  However, the semantics of this operation are
 * not well-defined and the value returned may not represent
 * an appropriate notion of relate.
 */
class RelateOp: public GeometryGraphOperation {

public:

	static geom::IntersectionMatrix* relate(
			const geom::Geometry *a,
			const geom::Geometry *b);

	RelateOp(const geom::Geometry *g0,
			const geom::Geometry *g1);

	virtual ~RelateOp();

	geom::IntersectionMatrix* getIntersectionMatrix();

private:

	RelateComputer relateComp;
};


} // namespace geos:operation:relate
} // namespace geos:operation
} // namespace geos

#endif // GEOS_OP_RELATE_RELATEOP_H

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/03/21 13:11:29  strk
 * opRelate.h header split
 *
 **********************************************************************/

