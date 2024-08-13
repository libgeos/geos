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

#include <geos/operation/relateng/IMPatternMatcher.h>
#include <geos/geom/Envelope.h>
#include <geos/geom/Location.h>

#include <sstream>


using geos::geom::Envelope;
using geos::geom::Location;


namespace geos {      // geos
namespace operation { // geos.operation
namespace relateng {  // geos.operation.relateng


/* public */
std::string
IMPatternMatcher::name() const
{
    return "IMPattern";
}


/* public */
void
IMPatternMatcher::init(const Envelope& envA, const Envelope& envB)
{
    IMPredicate::init(dimA, dimB);
    //-- if pattern specifies any non-E/non-E interaction, envelopes must not be disjoint
    bool requiresInteraction = requireInteraction(patternMatrix);
    bool isDisjoint = envA.disjoint(&envB);
    setValueIf(false, requiresInteraction && isDisjoint);
}


/* public */
bool
IMPatternMatcher::requireInteraction() const
{
    return requireInteraction(patternMatrix);
}


/* private static */
bool
IMPatternMatcher::requireInteraction(const IntersectionMatrix& im)
{
    bool requiresInteraction =
        isInteraction(im.get(Location::INTERIOR, Location::INTERIOR)) ||
        isInteraction(im.get(Location::INTERIOR, Location::BOUNDARY)) ||
        isInteraction(im.get(Location::BOUNDARY, Location::INTERIOR)) ||
        isInteraction(im.get(Location::BOUNDARY, Location::BOUNDARY));
    return requiresInteraction;
}


/* private static */
bool
IMPatternMatcher::isInteraction(int imDim)
{
    return imDim == Dimension::True || imDim >= Dimension::P;
}


/* public */
bool
IMPatternMatcher::isDetermined() const
{
    /**
     * Matrix entries only increase in dimension as topology is computed.
     * The predicate can be short-circuited (as false) if
     * any computed entry is greater than the mask value.
     */
    std::array<Location,3> locs = {
        Location::INTERIOR, Location::BOUNDARY, Location::EXTERIOR};

    for (Location i : locs) {
        for (Location j : locs) {
            int patternEntry = patternMatrix.get(i, j);

            if (patternEntry == Dimension::DONTCARE)
                continue;

            int matrixVal = getDimension(i, j);

            //-- mask entry TRUE requires a known matrix entry
            if (patternEntry == Dimension::True) {
                if (matrixVal < 0)
                    return false;
            }
            //-- result is known (false) if matrix entry has exceeded mask
            else if (matrixVal > patternEntry)
                return true;
        }
    }
    return false;
}


/* public */
bool
IMPatternMatcher::valueIM()
{
    bool val = intMatrix.matches(imPattern);
    return val;
}


/* public */
std::string
IMPatternMatcher::toString() const
{
    return name() + "(" + imPattern + ")";
}


/* public friend */
std::ostream&
operator<<(std::ostream& os, const IMPatternMatcher& imp)
{
    os << imp.toString();
    return os;
}



} // namespace geos.operation.overlayng
} // namespace geos.operation
} // namespace geos




