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
#include <geos/geom/Curve.h>

namespace geos {
namespace operation { // geos::operation
namespace spanning { // geos::operation::spanning

SpanningTreeEdge::SpanningTreeEdge(const geom::Curve* newCurve, std::size_t newIndex)
    :
    planargraph::Edge(),
    curve(newCurve),
    index(newIndex)
{
}

const geom::Curve*
SpanningTreeEdge::getCurve() const
{
    return curve;
}

double
SpanningTreeEdge::getLength() const
{
    return curve->getLength();
}

std::size_t
SpanningTreeEdge::getIndex() const
{
    return index;
}

} // namespace geos::operation::spanning
} // namespace geos::operation
} // namespace geos
