/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (c) 2024 Martin Davis
 * Copyright (C) 2024 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/algorithm/BoundaryNodeRule.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/LineString.h>
#include <geos/operation/relateng/LinearBoundary.h>
#include <geos/constants.h>

#include <map>
#include <set>

using geos::algorithm::BoundaryNodeRule;
using geos::geom::CoordinateXY;
using geos::geom::CoordinateSequence;
using geos::geom::LineString;


namespace geos {      // geos
namespace operation { // geos.operation
namespace relateng {  // geos.operation.relateng


LinearBoundary::LinearBoundary(std::vector<const LineString*>& lines, const BoundaryNodeRule& bnRule)
    : m_boundaryNodeRule(bnRule)
{
    //assert: dim(geom) == 1
    computeBoundaryPoints(lines, m_vertexDegree);
    m_hasBoundary = checkBoundary(m_vertexDegree);
}

/* private */
bool
LinearBoundary::checkBoundary(Coordinate::ConstIntMap& vertexDegree) const
{
    // Iterate over the map and test the values
    for (const auto& pair : vertexDegree) {
        int degree = pair.second;
        if (m_boundaryNodeRule.isInBoundary(degree)) {
            return true;
        }
    }
    return false;
}

/* public */
bool
LinearBoundary::hasBoundary() const
{
    return m_hasBoundary;
}

/* public */
bool
LinearBoundary::isBoundary(const CoordinateXY* pt) const
{
    auto it = m_vertexDegree.find(pt);
    if (it == m_vertexDegree.end())
        return false;

    int degree = it->second;
    return m_boundaryNodeRule.isInBoundary(degree);
}

/* private static */
void
LinearBoundary::computeBoundaryPoints(std::vector<const LineString*>& lines, Coordinate::ConstIntMap& vertexDegree)
{
    for (const LineString* line : lines) {
        if (line->isEmpty())
            continue;
        const CoordinateSequence* cs = line->getCoordinatesRO();
        const Coordinate& cs0 = cs->getAt(0);
        const Coordinate& csn = cs->getAt(line->getNumPoints() - 1);
        addEndpoint(&cs0, vertexDegree);
        addEndpoint(&csn, vertexDegree);
    }
}

/* private static */
void
LinearBoundary::addEndpoint(const CoordinateXY *p, Coordinate::ConstIntMap& vertexDegree)
{
    int dim = 0;
    auto it = vertexDegree.find(p);

    if (it != vertexDegree.end()) {
        dim = it->second;
    }
    dim++;
    std::pair<const CoordinateXY*, int> entry(p, dim);
    vertexDegree.insert(entry);
}



} // namespace geos.operation.overlayng
} // namespace geos.operation
} // namespace geos




