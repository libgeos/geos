/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2026 Paul Ramsey
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/operation/spanning/SpanningTreeEdge.h>
#include <geos/geom/LineString.h>

namespace geos {
namespace operation { // geos::operation
namespace spanning { // geos::operation::spanning

SpanningTreeEdge::SpanningTreeEdge(const geom::LineString* newLine, std::size_t newIndex)
    :
    planargraph::Edge(),
    line(newLine),
    index(newIndex)
{
}

const geom::LineString*
SpanningTreeEdge::getLineString() const
{
    return line;
}

double
SpanningTreeEdge::getLength() const
{
    return line->getLength();
}

std::size_t
SpanningTreeEdge::getIndex() const
{
    return index;
}

} // namespace geos::operation::spanning
} // namespace geos::operation
} // namespace geos
