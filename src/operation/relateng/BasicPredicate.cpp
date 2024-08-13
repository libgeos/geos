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

#include <geos/operation/relateng/BasicPredicate.h>
#include <geos/geom/Envelope.h>
#include <geos/geom/Location.h>
#include <geos/constants.h>

#include <sstream>


using geos::geom::Envelope;
using geos::geom::Location;


namespace geos {      // geos
namespace operation { // geos.operation
namespace relateng {  // geos.operation.relateng



/* private static */
bool
BasicPredicate::isKnown(int val)
{
     return val > UNKNOWN;
}

/* private static */
bool
BasicPredicate::toBoolean(int val)
{
    return val == TRUE;
}

/* private static */
int
BasicPredicate::toValue(bool val)
{
    return val ? TRUE : FALSE;
}


/* public static */
bool
BasicPredicate::isIntersection(Location locA, Location locB)
{
    //-- i.e. some location on both geometries intersects
    return locA != Location::EXTERIOR && locB != Location::EXTERIOR;
}


// /* public */
// bool isSelfNodingRequired() {
//     return false;
//   }


/* public override */
bool
BasicPredicate::isKnown() const
{
    return isKnown(m_value);
}

/* public override */
bool
BasicPredicate::value() const
{
    return toBoolean(m_value);
}


/* protected */
void
BasicPredicate::setValue(bool val)
{
    //-- don't change already-known value
    if (isKnown())
        return;
    m_value = toValue(val);
}

/* protected */
void
BasicPredicate::setValue(int val)
{
    //-- don't change already-known value
    if (isKnown())
        return;
    m_value = val;
}


/* protected */
void
BasicPredicate::setValueIf(bool val, bool cond)
{
    if (cond)
        setValue(val);
}

/* protected */
void
BasicPredicate::require(bool cond)
{
    if (! cond)
        setValue(false);
}

/* protected */
void
BasicPredicate::requireCovers(const Envelope& a, const Envelope& b)
{
    require(a.covers(b));
}


} // namespace geos.operation.overlayng
} // namespace geos.operation
} // namespace geos




