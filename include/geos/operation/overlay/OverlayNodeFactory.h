/**********************************************************************
 * $Id: OverlayNodeFactory.h 3273 2011-03-26 14:02:03Z strk $
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
 * Last port: operation/overlay/OverlayNodeFactory.java rev. 1.11 (JTS-1.10)
 *
 **********************************************************************/

#ifndef GEOS_OP_OVERLAY_OVERLAYNODEFACTORY_H
#define GEOS_OP_OVERLAY_OVERLAYNODEFACTORY_H

#include <geos/export.h>

#include <vector>

#include <geos/geomgraph/NodeFactory.h> // for inheritance

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
namespace operation { // geos::operation
namespace overlay { // geos::operation::overlay

/** \brief
 * Creates nodes for use in the geomgraph::PlanarGraph constructed during
 * overlay operations. NOTE: also used by operation::valid
 */
class GEOS_DLL OverlayNodeFactory: public geomgraph::NodeFactory {
public:
	OverlayNodeFactory():geomgraph::NodeFactory() {}
	geomgraph::Node* createNode(const geom::Coordinate &coord) const;
	static const geomgraph::NodeFactory &instance();
};


} // namespace geos::operation::overlay
} // namespace geos::operation
} // namespace geos

#endif // ndef GEOS_OP_OVERLAY_OVERLAYNODEFACTORY_H

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/03/17 13:24:59  strk
 * opOverlay.h header splitted. Reduced header inclusions in operation/overlay implementation files. ElevationMatrixFilter code moved from own file to ElevationMatrix.cpp (ideally a class-private).
 *
 **********************************************************************/

