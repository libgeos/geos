/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2021 Paul Ramsey <pramsey@cleverelephant.ca>
 * Copyright (C) 2021 Martin Davis
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/algorithm/PointLocation.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/Envelope.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/Polygon.h>
#include <geos/index/strtree/TemplateSTRtree.h>
#include <geos/operation/valid/IndexedNestedHoleTester.h>
#include <geos/operation/valid/PolygonTopologyAnalyzer.h>

namespace geos {      // geos
namespace operation { // geos.operation
namespace valid {     // geos.operation.valid

using namespace geos::geom;

/* private */
void
IndexedNestedHoleTester::loadIndex()
{
    for (std::size_t i = 0; i < polygon->getNumInteriorRing(); i++) {
        const LinearRing* hole = static_cast<const LinearRing*>(polygon->getInteriorRingN(i));
        const Envelope* env = hole->getEnvelopeInternal();
        index.insert(*env, hole);
    }
}

/* public */
bool
IndexedNestedHoleTester::isNested()
{
    for (std::size_t i = 0; i < polygon->getNumInteriorRing(); i++) {
        const LinearRing* hole = static_cast<const LinearRing*>(polygon->getInteriorRingN(i));

        std::vector<const LinearRing*> results;
        index.query(*(hole->getEnvelopeInternal()), results);

        for (const LinearRing* testHole: results) {
            if (hole == testHole)
                continue;

            /**
             * Hole is not fully covered by test hole, so cannot be nested
             */
            if (! testHole->getEnvelopeInternal()->covers(hole->getEnvelopeInternal()))
                continue;

            /**
             * Checks nesting via a point-in-polygon test,
             * or if the point lies on the boundary via
             * the topology of the incident edges.
             */
            if (PolygonTopologyAnalyzer::isRingNested(hole, testHole)) {
                nestedPt = hole->getCoordinatesRO()->getAt<CoordinateXY>(0);
                return true;
            }
        }
    }
    return false;
}



} // namespace geos.operation.valid
} // namespace geos.operation
} // namespace geos
