/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 * Copyright (C) 2005 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/operation/buffer/BufferNodeFactory.h>
#include <geos/geomgraph/Node.h>
#include <geos/geomgraph/DirectedEdgeStar.h>

using namespace geos::geomgraph;

namespace geos {
namespace operation { // geos.operation
namespace buffer { // geos.operation.buffer

Node*
BufferNodeFactory::createNode(const geom::Coordinate& coord) const
{
    return new Node(coord, new DirectedEdgeStar());
}

const NodeFactory&
BufferNodeFactory::instance()
{
    static BufferNodeFactory onf;
    return onf;
}

} // namespace geos.operation.buffer
} // namespace geos.operation
} // namespace geos

