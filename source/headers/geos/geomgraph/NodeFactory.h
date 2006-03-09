/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2005-2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/


#ifndef GEOS_GEOMGRAPH_NODEFACTORY_H
#define GEOS_GEOMGRAPH_NODEFACTORY_H

#include <geos/inline.h>

// Forward declarations
namespace geos {
	namespace geom {
		class Coordinate;
	}
	namespace geomgraph {
		class Node;
	}
}

namespace geos {
namespace geomgraph { // geos.geomgraph

class NodeFactory {
public:
	virtual Node* createNode(const geom::Coordinate &coord) const;
	static const NodeFactory &instance();
protected:
	NodeFactory() {};
};


} // namespace geos.geomgraph
} // namespace geos

//#ifdef USE_INLINE
//# include "geos/geomgraph/NodeFactory.inl"
//#endif

#endif // ifndef GEOS_GEOMGRAPH_NODEFACTORY_H

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/

