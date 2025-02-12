/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

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
namespace buffer { // geos::operation::buffer

/** \brief
 * Creates nodes for use in the geomgraph::PlanarGraph constructed during
 * buffer operations. NOTE: also used by operation::valid
 */
class GEOS_DLL BufferNodeFactory: public geomgraph::NodeFactory {
public:
    BufferNodeFactory(): geomgraph::NodeFactory() {}
    geomgraph::Node* createNode(const geom::Coordinate& coord) const override;
    static const geomgraph::NodeFactory& instance();
};


} // namespace geos::operation::buffer
} // namespace geos::operation
} // namespace geos

