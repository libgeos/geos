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

#include <geos/algorithm/IndexedNestedHoleTester.h>
#include <geos/algorithm/PointLocation.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/Envelope.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/Location.h>
#include <geos/index/strtree/TemplateSTRtree.h>

namespace geos {      // geos
namespace operation { // geos.operation
namespace valid {     // geos.operation.valid

using namespace geos::geom;


/* private */
void
IndexedNestedHoleTester::loadIndex()
{
    for (std::size_t i = 0; i < polygon->getNumInteriorRing(); i++) {
        const LinearRing* hole = static_cast<LinearRing*>(polygon->getInteriorRingN(i));
        const Envelope* env = hole->getEnvelopeInternal();
        index.insert(env, hole);
    }
}


/* public */
bool
IndexedNestedHoleTester::isNested() const
{
    for (std::size_t i = 0; i < polygon->getNumInteriorRing(); i++) {
        const LinearRing* hole = static_cast<LinearRing*>(polygon->getInteriorRingN(i));

        std::vector<void*> results;
        index.query(hole->getEnvelopeInternal(), results);

        for (std::size_t j = 0; j < results.size(); j++) {
            const LinearRing* testHole = static_cast<LinearRing*>(results[j]);
            if (hole == testHole)
                continue;

            /**
            * Hole is not covered by in test hole,
            * so cannot be inside
            */
            if (! testHole->getEnvelopeInternal()->covers(hole->getEnvelopeInternal()))
                continue;

            if (isHoleInsideHole(hole, testHole))
                return true;
        }
    }
    return false;
}


/* private */
bool
IndexedNestedHoleTester::isHoleInsideHole(
    const LinearRing* hole, const LinearRing* testHole) const
{
    const CoordinateSequence* testPts = testHole->getCoordinatesRO();
    for (std::size_t i = 0; i < hole->getNumPoints(); i++) {
        const Coordinate& holePt = hole->getCoordinateN(i);
        Location loc = PointLocation::locateInRing(holePt, *testPts);
        switch (loc) {
            case Location::EXTERIOR:
                return false;
            case Location::INTERIOR:
                nestedPt = holePt;
                return true;
            default:
                // Location::BOUNDARY, so keep trying points
                continue;
        }
    }
    return false;
}


} // namespace geos.operation.valid
} // namespace geos.operation
} // namespace geos
