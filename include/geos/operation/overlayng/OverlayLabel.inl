/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2020 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/operation/overlayng/OverlayLabel.h>

namespace geos {
namespace operation {
namespace overlayng {

/*public*/
INLINE bool
OverlayLabel::isLine() const
{
    return aDim == DIM_LINE || bDim == DIM_LINE;
}

/*public*/
INLINE bool
OverlayLabel::isLine(uint8_t index) const
{
    return index == 0 ? aDim == DIM_LINE : bDim == DIM_LINE;
}

/*public*/
INLINE bool
OverlayLabel::isLinear(uint8_t index) const
{
    if (index == 0) {
        return aDim == DIM_LINE || aDim == DIM_COLLAPSE;
    }
    return bDim == DIM_LINE || bDim == DIM_COLLAPSE;
}

/*public*/
INLINE bool
OverlayLabel::isKnown(uint8_t index) const
{
    if (index == 0) {
        return aDim != DIM_UNKNOWN;
    }
    return bDim != DIM_UNKNOWN;
}

/*public*/
INLINE bool
OverlayLabel::isNotPart(uint8_t index) const
{
    if (index == 0) {
        return aDim == DIM_NOT_PART;
    }
    return bDim == DIM_NOT_PART;
}

/*public*/
INLINE bool
OverlayLabel::isBoundaryEither() const
{
    return aDim == DIM_BOUNDARY || bDim == DIM_BOUNDARY;
}

/*public*/
INLINE bool
OverlayLabel::isBoundaryBoth() const
{
    return aDim == DIM_BOUNDARY && bDim == DIM_BOUNDARY;
}
/*public*/
INLINE bool
OverlayLabel::isBoundaryCollapse() const
{
    if (isLine()) return false;
    return ! isBoundaryBoth();
}

/*public*/
INLINE bool
OverlayLabel::isBoundary(uint8_t index) const
{
    if (index == 0) {
        return aDim == DIM_BOUNDARY;
    }
    return bDim == DIM_BOUNDARY;
}

/*public*/
INLINE bool
OverlayLabel::isBoundarySingleton() const
{
    if (aDim == DIM_BOUNDARY && bDim == DIM_NOT_PART) {
        return true;
    }

    if (bDim == DIM_BOUNDARY && aDim == DIM_NOT_PART) {
        return true;
    }

    return false;
}

/*public*/
INLINE bool
OverlayLabel::isLineLocationUnknown(int index) const
{
    if (index == 0) {
        return aLocLine == LOC_UNKNOWN;
    }
    else {
        return bLocLine == LOC_UNKNOWN;
    }
}


/*public*/
INLINE bool
OverlayLabel::isLineInArea(int8_t index) const
{
    if (index == 0) {
        return aLocLine == Location::INTERIOR;
    }
    return bLocLine == Location::INTERIOR;
}

/*public*/
INLINE bool
OverlayLabel::isHole(uint8_t index) const
{
    if (index == 0) {
        return aIsHole;
    }
    else {
        return bIsHole;
    }
}

/*public*/
INLINE bool
OverlayLabel::isCollapse(uint8_t index) const
{
    return dimension(index) == DIM_COLLAPSE;
}

/*public*/
INLINE bool
OverlayLabel::isInteriorCollapse() const
{
    if (aDim == DIM_COLLAPSE && aLocLine == Location::INTERIOR)
        return true;
    if (bDim == DIM_COLLAPSE && bLocLine == Location::INTERIOR)
        return true;

    return false;
}

/*public*/
INLINE bool
OverlayLabel::isBoundaryTouch() const
{
    return isBoundaryBoth() &&
           getLocation(0, Position::RIGHT, true) != getLocation(1, Position::RIGHT, true);
}

/*public*/
INLINE Location
OverlayLabel::getLineLocation(uint8_t index) const
{
    if (index == 0) {
        return aLocLine;
    }
    else {
        return bLocLine;
    }
}

/*public*/
INLINE bool
OverlayLabel::isLineInterior(uint8_t index) const
{
    if (index == 0) {
        return aLocLine == Location::INTERIOR;
    }
    return bLocLine == Location::INTERIOR;
}
/*public*/
INLINE Location
OverlayLabel::getLocationBoundaryOrLine(uint8_t index, int pos, bool isForward) const
{
    if (isBoundary(index)) {
        return getLocation(index, pos, isForward);
    }
    return getLineLocation(index);
}


/*public*/
INLINE Location
OverlayLabel::getLocation(uint8_t index) const {
    if (index == 0) {
        return aLocLine;
    }
    return bLocLine;
}

/*public*/
INLINE bool
OverlayLabel::hasSides(uint8_t index) const {
    if (index == 0) {
        return aLocLeft != LOC_UNKNOWN
               || aLocRight != LOC_UNKNOWN;
    }
    return bLocLeft != LOC_UNKNOWN
           || bLocRight != LOC_UNKNOWN;
}

/*public*/
INLINE OverlayLabel
OverlayLabel::copy() const
{
    OverlayLabel lbl = *this;
    return lbl;
}


}
}
}

