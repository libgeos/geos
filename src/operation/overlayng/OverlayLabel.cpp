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


namespace geos {      // geos
namespace operation { // geos.operation
namespace overlayng { // geos.operation.overlayng

/*public*/
void
OverlayLabel::initBoundary(uint8_t index, Location locLeft, Location locRight, bool p_isHole)
{
    if (index == 0) {
        aDim = DIM_BOUNDARY;
        aIsHole = p_isHole;
        aLocLeft = locLeft;
        aLocRight = locRight;
        aLocLine = Location::INTERIOR;
    }
    else {
        bDim = DIM_BOUNDARY;
        bIsHole = p_isHole;
        bLocLeft = locLeft;
        bLocRight = locRight;
        bLocLine = Location::INTERIOR;
    }
}

/*public*/
void
OverlayLabel::initCollapse(uint8_t index, bool p_isHole)
{
    if (index == 0) {
        aDim = DIM_COLLAPSE;
        aIsHole = p_isHole;
    }
    else {
        bDim = DIM_COLLAPSE;
        bIsHole = p_isHole;
    }
}

/*public*/
void
OverlayLabel::initLine(uint8_t index)
{
    if (index == 0) {
        aDim = DIM_LINE;
        aLocLine = LOC_UNKNOWN;
    }
    else {
        bDim = DIM_LINE;
        bLocLine = LOC_UNKNOWN;
    }
}

/*public*/
void
OverlayLabel::initNotPart(uint8_t index)
{
    // this assumes locations are initialized to UNKNOWN
    if (index == 0) {
        aDim = DIM_NOT_PART;
    }
    else {
        bDim = DIM_NOT_PART;
    }
}

/*public*/
void
OverlayLabel::setLocationLine(uint8_t index, Location loc)
{
    if (index == 0) {
        aLocLine = loc;
    }
    else {
        bLocLine = loc;
    }
}

/*public*/
void
OverlayLabel::setLocationAll(uint8_t index, Location loc)
{
    if (index == 0) {
        aLocLine = loc;
        aLocLeft = loc;
        aLocRight = loc;
    }
    else {
        bLocLine = loc;
        bLocLeft = loc;
        bLocRight = loc;
    }
}

/*public*/
void
OverlayLabel::setLocationCollapse(uint8_t index)
{
    Location loc = isHole(index) ? Location::INTERIOR : Location::EXTERIOR;
    if (index == 0) {
        aLocLine = loc;
    }
    else {
        bLocLine = loc;
    }
}


/*public*/
bool
OverlayLabel::isCollapseAndNotPartInterior() const
{
    if (aDim == DIM_COLLAPSE &&
        bDim == DIM_NOT_PART &&
        bLocLine == Location::INTERIOR)
        return true;

    if (bDim == DIM_COLLAPSE &&
        aDim == DIM_NOT_PART &&
        aLocLine == Location::INTERIOR)
        return true;

    return false;
}

/*public*/
Location
OverlayLabel::getLocation(uint8_t index, int position, bool isForward) const
{
    if (index == 0) {
        switch (position) {
        case Position::LEFT:
            return isForward ? aLocLeft : aLocRight;
        case Position::RIGHT:
            return isForward ? aLocRight : aLocLeft;
        case Position::ON:
            return aLocLine;
        }
    }

    switch (position) {
    case Position::LEFT:
        return isForward ? bLocLeft : bLocRight;
    case Position::RIGHT:
        return isForward ? bLocRight : bLocLeft;
    case Position::ON:
        return bLocLine;
    }

    return LOC_UNKNOWN;
}

/*private*/
std::string
OverlayLabel::dimensionSymbol(int dim) const
{
    switch (dim) {
    case DIM_LINE: return std::string("L");
    case DIM_COLLAPSE: return std::string("C");
    case DIM_BOUNDARY: return std::string("B");
    }
    return std::string("U");
}

/*public static*/
std::ostream&
operator<<(std::ostream& os, const OverlayLabel& ol)
{
    ol.toString(true, os);
    return os;
}

/*public*/
void
OverlayLabel::toString(bool isForward, std::ostream& os) const
{
    os << "A:";
    locationString(0, isForward, os);
    os << "/B:";
    locationString(1, isForward, os);
}

/*private*/
void
OverlayLabel::locationString(uint8_t index, bool isForward, std::ostream& os) const
{
    if (isBoundary(index)) {
        os << getLocation(index, Position::LEFT, isForward);
        os << getLocation(index, Position::RIGHT, isForward);
    }
    else {
        os << (index == 0 ? aLocLine : bLocLine);
    }
    if (isKnown(index))
    {
      os << dimensionSymbol(index == 0 ? aDim : bDim);
    }
    if (isCollapse(index)) {
        bool p_isHole = (index == 0 ? aIsHole : bIsHole);
        if (p_isHole)
            os << "h";
        else
            os << "s";
    }
}



} // namespace geos.operation.overlayng
} // namespace geos.operation
} // namespace geos
