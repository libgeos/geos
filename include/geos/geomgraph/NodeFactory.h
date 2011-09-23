/**********************************************************************
 * $Id: NodeFactory.h 2557 2009-06-08 09:30:55Z strk $
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
 **********************************************************************
 *
 * Last port: geomgraph/NodeFactory.java rev. 1.3 (JTS-1.10)
 *
 **********************************************************************/


#ifndef GEOS_GEOMGRAPH_NODEFACTORY_H
#define GEOS_GEOMGRAPH_NODEFACTORY_H

#include <geos/export.h>
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

class GEOS_DLL NodeFactory {
public:
	virtual Node* createNode(const geom::Coordinate &coord) const;
	static const NodeFactory &instance();
	virtual ~NodeFactory() {}
protected:
	NodeFactory() {};
};


} // namespace geos.geomgraph
} // namespace geos

//#ifdef GEOS_INLINE
//# include "geos/geomgraph/NodeFactory.inl"
//#endif

#endif // ifndef GEOS_GEOMGRAPH_NODEFACTORY_H

/**********************************************************************
 * $Log$
 * Revision 1.3  2006/06/08 11:20:24  strk
 * Added missing virtual destructor to abstract classes.
 *
 * Revision 1.2  2006/03/24 09:52:41  strk
 * USE_INLINE => GEOS_INLINE
 *
 * Revision 1.1  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/

