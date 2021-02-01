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

#pragma once

#include <geos/operation/overlayng/OverlayEdge.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/operation/overlayng/OverlayLabel.h>

namespace geos {
namespace operation {
namespace overlayng {

/*public*/
INLINE bool
OverlayEdge::isForward() const
{
    return direction;
}

/*public*/
INLINE const Coordinate&
OverlayEdge::directionPt() const
{
    return dirPt;
}

/*public*/
INLINE OverlayLabel*
OverlayEdge::getLabel() const
{
    return label;
}

/*public*/
INLINE Location
OverlayEdge::getLocation(uint8_t index, int position) const
{
    return label->getLocation(index, position, direction);
}

/*public*/
INLINE const Coordinate&
OverlayEdge::getCoordinate() const
{
    return orig();
}

/*public*/
INLINE const CoordinateSequence*
OverlayEdge::getCoordinatesRO() const
{
    return pts;
}

/*public*/
INLINE std::unique_ptr<CoordinateSequence>
OverlayEdge::OverlayEdge::getCoordinates()
{
    // return a copy of pts
    return pts->clone();
}

/*public*/
INLINE OverlayEdge*
OverlayEdge::symOE() const
{
    return static_cast<OverlayEdge*>(sym());
}

/*public*/
INLINE OverlayEdge*
OverlayEdge::oNextOE() const
{
    return static_cast<OverlayEdge*>(oNext());
}

/*public*/
INLINE bool
OverlayEdge::isInResultArea() const
{
    return m_isInResultArea;
}

/*public*/
INLINE bool
OverlayEdge::isInResultAreaBoth() const
{
    return m_isInResultArea && symOE()->m_isInResultArea;
}

/*public*/
INLINE bool
OverlayEdge::isInResultEither() const
{
    return isInResult() || symOE()->isInResult();
}

/*public*/
INLINE void
OverlayEdge::unmarkFromResultAreaBoth()
{
    m_isInResultArea = false;
    symOE()->m_isInResultArea = false;
}

/*public*/
INLINE void
OverlayEdge::markInResultArea()
{
    m_isInResultArea  = true;
}

/*public*/
INLINE void
OverlayEdge::markInResultAreaBoth()
{
    m_isInResultArea  = true;
    symOE()->m_isInResultArea = true;
}

/*public*/
INLINE bool
OverlayEdge::isInResultLine() const
{
    return m_isInResultLine;
}

/*public*/
INLINE void
OverlayEdge::markInResultLine()
{
    m_isInResultLine  = true;
    symOE()->m_isInResultLine = true;
}

/*public*/
INLINE bool
OverlayEdge::isInResult() const
{
    return m_isInResultArea || m_isInResultLine;
}

INLINE void
OverlayEdge::setNextResult(OverlayEdge* e)
{
    // Assert: e.orig() == this.dest();
    nextResultEdge = e;
}

/*public*/
INLINE OverlayEdge*
OverlayEdge::nextResult() const
{
    return nextResultEdge;
}

/*public*/
INLINE bool
OverlayEdge::isResultLinked() const
{
    return nextResultEdge != nullptr;
}

INLINE void
OverlayEdge::setNextResultMax(OverlayEdge* e)
{
    // Assert: e.orig() == this.dest();
    nextResultMaxEdge = e;
}

/*public*/
INLINE OverlayEdge*
OverlayEdge::nextResultMax() const
{
    return nextResultMaxEdge;
}

/*public*/
INLINE bool
OverlayEdge::isResultMaxLinked() const
{
    return nextResultMaxEdge != nullptr;
}

/*public*/
INLINE bool
OverlayEdge::isVisited() const
{
    return m_isVisited;
}

/*private*/
INLINE void
OverlayEdge::markVisited()
{
    m_isVisited = true;
}

/*public*/
INLINE void
OverlayEdge::markVisitedBoth()
{
    markVisited();
    symOE()->markVisited();
}

/*public*/
INLINE void
OverlayEdge::setEdgeRing(const OverlayEdgeRing* p_edgeRing)
{
    edgeRing = p_edgeRing;
}

/*public*/
INLINE const OverlayEdgeRing*
OverlayEdge::getEdgeRing() const
{
    return edgeRing;
}

/*public*/
INLINE const MaximalEdgeRing*
OverlayEdge::getEdgeRingMax() const
{
    return maxEdgeRing;
}

/*public*/
INLINE void
OverlayEdge::setEdgeRingMax(const MaximalEdgeRing* p_maximalEdgeRing)
{
    maxEdgeRing = p_maximalEdgeRing;
}


}
}
}
