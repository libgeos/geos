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

#include <geos/operation/relateng/IMPredicate.h>
#include <geos/geom/Envelope.h>
#include <geos/geom/Location.h>
#include <geos/constants.h>

#include <sstream>


using geos::geom::Envelope;
using geos::geom::Location;


namespace geos {      // geos
namespace operation { // geos.operation
namespace relateng {  // geos.operation.relateng


/* public static */
bool
IMPredicate::isDimsCompatibleWithCovers(int dim0, int dim1)
{
    //- allow Points coveredBy zero-length Lines
    if (dim0 == Dimension::P && dim1 == Dimension::L)
        return true;
    return dim0 >= dim1;
}


/* public */
void
IMPredicate::init(int dA, int dB)
{
    dimA = dA;
    dimB = dB;
}


/* public */
void
IMPredicate::updateDimension(Location locA, Location locB, int dimension)
{
    //-- only record an increased dimension value
    if (isDimChanged(locA, locB, dimension)) {
        intMatrix.set(locA, locB, dimension);
        //-- set value if predicate value can be known
        if (isDetermined()) {
            setValue(valueIM());
        }
    }
}


/* public */
bool
IMPredicate::isDimChanged(Location locA, Location locB, int dimension) const
{
    return dimension > intMatrix.get(locA, locB);
}


/* protected */
bool
IMPredicate::intersectsExteriorOf(bool isA) const
{
    if (isA) {
        return isIntersects(Location::EXTERIOR, Location::INTERIOR)
            || isIntersects(Location::EXTERIOR, Location::BOUNDARY);
    }
    else {
        return isIntersects(Location::INTERIOR, Location::EXTERIOR)
            || isIntersects(Location::BOUNDARY, Location::EXTERIOR);
    }
}


/* protected */
bool
IMPredicate::isIntersects(Location locA, Location locB) const
{
    return intMatrix.get(locA, locB) >= Dimension::P;
}


/* public */
bool
IMPredicate::isKnown(Location locA, Location locB) const
{
    return intMatrix.get(locA, locB) != DIM_UNKNOWN;
}


/* public */
bool
IMPredicate::isDimension(Location locA, Location locB, int dimension) const
{
    return intMatrix.get(locA, locB) == dimension;
}


/* public */
int
IMPredicate::getDimension(Location locA, Location locB) const
{
    return intMatrix.get(locA, locB);
}


/* public */
void
IMPredicate::finish()
{
    setValue(valueIM());
}


/* public */
std::string
IMPredicate::toString() const
{
    return name() + ": " + intMatrix.toString();
}


/* public friend */
std::ostream&
operator<<(std::ostream& os, const IMPredicate& imp)
{
    os << imp.toString() << " " << imp.intMatrix;
    return os;
}



} // namespace geos.operation.overlayng
} // namespace geos.operation
} // namespace geos




