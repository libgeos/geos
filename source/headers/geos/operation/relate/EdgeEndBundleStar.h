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
 **********************************************************************/

#ifndef GEOS_OP_RELATE_EDGEENDBUNDLESTAR_H
#define GEOS_OP_RELATE_EDGEENDBUNDLESTAR_H

#include <geos/geomgraph/EdgeEndStar.h> // for EdgeEndBundleStar inheritance

// Forward declarations
namespace geos {
	namespace geom {
		class IntersectionMatrix;
	}
	namespace geomgraph {
		class EdgeEnd;
	}
}


namespace geos {
namespace operation { // geos::operation
namespace relate { // geos::operation::relate

/** \brief
 * An ordered list of EdgeEndBundle objects around a RelateNode.
 *
 * They are maintained in CCW order (starting with the positive x-axis)
 * around the node
 * for efficient lookup and topology building.
 */
class EdgeEndBundleStar: public geomgraph::EdgeEndStar {
public:

	EdgeEndBundleStar() {}
	
	virtual ~EdgeEndBundleStar();
	void insert(geomgraph::EdgeEnd *e);
	void updateIM(geom::IntersectionMatrix *im);
};


} // namespace geos:operation:relate
} // namespace geos:operation
} // namespace geos

#endif // GEOS_OP_RELATE_EDGEENDBUNDLESTAR_H

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/03/21 13:11:29  strk
 * opRelate.h header split
 *
 **********************************************************************/

