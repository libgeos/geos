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

#include <geos/operation/overlayng/Edge.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/operation/overlayng/EdgeSourceInfo.h>
#include <geos/util/GEOSException.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/Dimension.h>
#include <geos/geom/Location.h>

namespace geos {
namespace operation {
namespace overlayng {

/*public*/
INLINE bool
Edge::direction() const
{
    if (pts->size() < 2) {
        throw util::GEOSException("Edge must have >= 2 points");
    }

    const geom::Coordinate& p0 = pts->getAt(0);
    const geom::Coordinate& p1 = pts->getAt(1);
    const geom::Coordinate& pn0 = pts->getAt(pts->size() - 1);
    const geom::Coordinate& pn1 = pts->getAt(pts->size() - 2);

    int cmp = 0;
    int cmp0 = p0.compareTo(pn0);
    if (cmp0 != 0) cmp = cmp0;

    if (cmp == 0) {
        int cmp1 = p1.compareTo(pn1);
        if (cmp1 != 0) cmp = cmp1;
    }

    if (cmp == 0) {
        throw util::GEOSException("Edge direction cannot be determined because endpoints are equal");
    }

    return cmp == -1;
}


/*public*/
INLINE const geom::CoordinateSequence*
Edge::getCoordinatesRO() const
{
    return pts.get();
}

/*public*/
INLINE std::unique_ptr<geom::CoordinateSequence>
Edge::getCoordinates()
{
    return pts->clone();
}

/*public*/
INLINE geom::CoordinateSequence*
Edge::releaseCoordinates()
{
    geom::CoordinateSequence* cs = pts.release();
    pts.reset(nullptr);
    return cs;
}


/*public*/
INLINE const geom::Coordinate&
Edge::getCoordinate(std::size_t index)  const
{
    return pts->getAt(index);
}

/*public*/
INLINE std::size_t
Edge::size() const
{
    return pts->size();
}

/**
* Compares two coincident edges to determine
* whether they have the same or opposite direction.
*/
/*public*/
INLINE bool
Edge::relativeDirection(const Edge* edge2) const
{
    // assert: the edges match (have the same coordinates up to direction)
    if (!getCoordinate(0).equals2D(edge2->getCoordinate(0))) {
        return false;
    }
    if (!getCoordinate(1).equals2D(edge2->getCoordinate(1))) {
        return false;
    }
    return true;
}

/*public*/
INLINE int
Edge::dimension(int geomIndex) const
{
    if (geomIndex == 0) return aDim;
    return bDim;
}


/*private*/
INLINE int
Edge::labelDim(int dim, int depthDelta)
{
    if (dim == geom::Dimension::False)
        return OverlayLabel::DIM_NOT_PART;

    if (dim == geom::Dimension::L)
        return OverlayLabel::DIM_LINE;

    // assert: dim is A
    bool isCollapse = (depthDelta == 0);
    if (isCollapse)
        return OverlayLabel::DIM_COLLAPSE;

    return OverlayLabel::DIM_BOUNDARY;
}

/*private*/
INLINE bool
Edge::isHole(int index) const
{
    if (index == 0)
        return aIsHole;
    return bIsHole;
}


/*private*/
INLINE bool
Edge::isBoundary(int geomIndex) const
{
    if (geomIndex == 0)
        return aDim == OverlayLabel::DIM_BOUNDARY;
    return bDim == OverlayLabel::DIM_BOUNDARY;
}

/**
* Tests whether the edge is part of a shell in the given geometry.
* This is only the case if the edge is a boundary.
*/
/*private*/
INLINE bool
Edge::isShell(int geomIndex) const
{
    if (geomIndex == 0) {
        return (aDim == OverlayLabel::DIM_BOUNDARY && ! aIsHole);
    }
    return (bDim == OverlayLabel::DIM_BOUNDARY && ! bIsHole);
}

/*private*/
INLINE Location
Edge::locationRight(int depthDelta)
{
    int sgn = delSign(depthDelta);
    switch (sgn) {
        case 0: return Location::NONE;
        case 1: return Location::INTERIOR;
        case -1: return Location::EXTERIOR;
    }
    return Location::NONE;
}

/*private*/
INLINE Location
Edge::locationLeft(int depthDelta)
{
    // TODO: is it always safe to ignore larger depth deltas?
    int sgn = delSign(depthDelta);
    switch (sgn) {
        case 0: return Location::NONE;
        case 1: return Location::EXTERIOR;
        case -1: return Location::INTERIOR;
    }
    return Location::NONE;
}

/*private*/
INLINE int
Edge::delSign(int depthDel)
{
    if (depthDel > 0) return 1;
    if (depthDel < 0) return -1;
    return 0;
}

/*private*/
INLINE void
Edge::copyInfo(const EdgeSourceInfo* info)
{
    if (info->getIndex() == 0) {
        aDim = info->getDimension();
        aIsHole = info->isHole();
        aDepthDelta = info->getDepthDelta();
    }
    else {
        bDim = info->getDimension();
        bIsHole = info->isHole();
        bDepthDelta = info->getDepthDelta();
    }
}

/**
* Merges an edge into this edge,
* updating the topology info accordingly.
*/
/*public*/
INLINE void
Edge::merge(const Edge* edge)
{
    /**
     * Marks this
     * as a shell edge if any contributing edge is a shell.
     * Update hole status first, since it depends on edge dim
     */
    aIsHole = isHoleMerged(0, this, edge);
    bIsHole = isHoleMerged(1, this, edge);

    if (edge->aDim > aDim) aDim = edge->aDim;
    if (edge->bDim > bDim) bDim = edge->bDim;

    bool relDir = relativeDirection(edge);
    int flipFactor = relDir ? 1 : -1;
    aDepthDelta += flipFactor * edge->aDepthDelta;
    bDepthDelta += flipFactor * edge->bDepthDelta;
}

/*private*/
INLINE bool
Edge::isHoleMerged(int geomIndex, const Edge* edge1, const Edge* edge2)
{
    // TOD: this might be clearer with tri-state logic for isHole?
    bool isShell1 = edge1->isShell(geomIndex);
    bool isShell2 = edge2->isShell(geomIndex);
    bool isShellMerged = isShell1 || isShell2;
    // flip since isHole is stored
    return !isShellMerged;
}

/*public*/
INLINE void
Edge::populateLabel(OverlayLabel &lbl) const
{
    initLabel(lbl, 0, aDim, aDepthDelta, aIsHole);
    initLabel(lbl, 1, bDim, bDepthDelta, bIsHole);
}

}
}
}
