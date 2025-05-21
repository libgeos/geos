/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (c) 2025 Martin Davis
 * Copyright (C) 2025 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/dissolve/DissolveHalfEdge.h>

namespace geos {      // geos
namespace dissolve {  // geos.dissolve

/**
 * Tests whether this edge is the starting segment
 * in a LineString being dissolved.
 *
 * @return true if this edge is a start segment
 */
bool
DissolveHalfEdge::isStart()
{
    return m_isStart;
}

/**
 * Sets this edge to be the start segment of an input LineString.
 */
void
DissolveHalfEdge::setStart()
{
    m_isStart = true;
}


} // namespace geos.dissolve
} // namespace geos


