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

#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/Dimension.h>
#include <geos/operation/overlayng/OverlayEdge.h>
#include <geos/operation/overlayng/OverlayLabel.h>
#include <geos/operation/overlayng/EdgeSourceInfo.h>
#include <geos/util/GEOSException.h>
#include <geos/export.h>


#include <memory>


namespace geos {      // geos.
namespace operation { // geos.operation
namespace overlayng { // geos.operation.overlayng

/**
 * Represents the underlying linework for edges in a topology graph,
 * and carries the topology information
 * derived from the two parent geometries.
 * The edge may be the result of the merging of
 * two or more edges which have the same underlying linework
 * (although possibly different orientations).
 * In this case the topology information is
 * derived from the merging of the information in the
 * source edges.
 * Merged edges can occur in the following situations
 *
 *  - Due to topology collapse caused by snapping or rounding
 *    of polygonal geometries.
 *  - Due to coincident linework in a linear input
 *
 * The source edges may have the same parent geometry,
 * or different ones, or a mix of the two.
 *
 * @author mdavis
 */
class GEOS_DLL Edge {

private:

    // Members
    int aDim = OverlayLabel::DIM_UNKNOWN;
    int aDepthDelta = 0;
    bool aIsHole = false;
    int bDim = OverlayLabel::DIM_UNKNOWN;
    int bDepthDelta = 0;
    bool bIsHole = false;
    std::unique_ptr<geom::CoordinateSequence> pts;

    // Methods

    /**
    * Populates the label for an edge resulting from an input geometry.
    *  - If the edge is not part of the input, the label is left as NOT_PART
    *  - If input is an Area and the edge is on the boundary
    * (which may include some collapses),
    * edge is marked as an AREA edge and side locations are assigned
    *  - If input is an Area and the edge is collapsed
    * (depth delta = 0),
    * the label is set to COLLAPSE.
    * The location will be determined later
    * by evaluating the final graph topology.
    *  - If input is a Line edge is set to a LINE edge.
    * For line edges the line location is not significant
    * (since there is no parent area for which to determine location).
    */
    static void initLabel(OverlayLabel& lbl, uint8_t geomIndex, int dim, int depthDelta, bool p_isHole);

    static int labelDim(int dim, int depthDelta)
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
    };

    bool isHole(int index) const
    {
        if (index == 0)
            return aIsHole;
        return bIsHole;
    };

    bool isBoundary(int geomIndex) const
    {
        if (geomIndex == 0)
            return aDim == OverlayLabel::DIM_BOUNDARY;
        return bDim == OverlayLabel::DIM_BOUNDARY;
    };

    /**
    * Tests whether the edge is part of a shell in the given geometry.
    * This is only the case if the edge is a boundary.
    */
    bool isShell(int geomIndex) const
    {
        if (geomIndex == 0) {
            return (aDim == OverlayLabel::DIM_BOUNDARY && ! aIsHole);
        }
        return (bDim == OverlayLabel::DIM_BOUNDARY && ! bIsHole);
    };

    static Location locationRight(int depthDelta)
    {
        int sgn = delSign(depthDelta);
        switch (sgn) {
            case 0: return Location::NONE;
            case 1: return Location::INTERIOR;
            case -1: return Location::EXTERIOR;
        }
        return Location::NONE;
    };

    static Location locationLeft(int depthDelta)
    {
        // TODO: is it always safe to ignore larger depth deltas?
        int sgn = delSign(depthDelta);
        switch (sgn) {
            case 0: return Location::NONE;
            case 1: return Location::EXTERIOR;
            case -1: return Location::INTERIOR;
        }
        return Location::NONE;
    };

    static int delSign(int depthDel)
    {
        if (depthDel > 0) return 1;
        if (depthDel < 0) return -1;
        return 0;
    };

    void copyInfo(const EdgeSourceInfo* info)
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
    };

    static bool isHoleMerged(int geomIndex, const Edge* edge1, const Edge* edge2)
    {
        // TOD: this might be clearer with tri-state logic for isHole?
        bool isShell1 = edge1->isShell(geomIndex);
        bool isShell2 = edge2->isShell(geomIndex);
        bool isShellMerged = isShell1 || isShell2;
        // flip since isHole is stored
        return !isShellMerged;
    };


public:

    Edge()
        : aDim(OverlayLabel::DIM_UNKNOWN)
        , aDepthDelta(0)
        , aIsHole(false)
        , bDim(OverlayLabel::DIM_UNKNOWN)
        , bDepthDelta(0)
        , bIsHole(false)
        , pts(nullptr)
        {};

    friend std::ostream& operator<<(std::ostream& os, const Edge& e);

    static bool isCollapsed(const geom::CoordinateSequence* pts);

    // takes ownership of pts from caller
    Edge(std::unique_ptr<geom::CoordinateSequence>&& p_pts, const EdgeSourceInfo* info);

    // return a clone of the underlying points
    std::unique_ptr<geom::CoordinateSequence> getCoordinates()
    {
        return pts->clone();
    };

    // return a read-only pointer to the underlying points
    const geom::CoordinateSequence* getCoordinatesRO() const
    {
        return pts.get();
    };

    // release the underlying points to the caller
    geom::CoordinateSequence* releaseCoordinates()
    {
        geom::CoordinateSequence* cs = pts.release();
        pts.reset(nullptr);
        return cs;
    };

    const geom::Coordinate& getCoordinate(std::size_t index)  const
    {
        return pts->getAt(index);
    };

    std::size_t size() const
    {
        return pts->size();
    };

    bool direction() const
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
    };

    /**
    * Compares two coincident edges to determine
    * whether they have the same or opposite direction.
    */
    bool relativeDirection(const Edge* edge2) const
    {
        // assert: the edges match (have the same coordinates up to direction)
        if (!getCoordinate(0).equals2D(edge2->getCoordinate(0))) {
            return false;
        }
        if (!getCoordinate(1).equals2D(edge2->getCoordinate(1))) {
            return false;
        }
        return true;
    };

    int dimension(int geomIndex) const
    {
        if (geomIndex == 0) return aDim;
        return bDim;
    };

    /**
    * Merges an edge into this edge,
    * updating the topology info accordingly.
    */
    void merge(const Edge* edge)
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
    };

    void populateLabel(OverlayLabel &lbl) const
    {
        initLabel(lbl, 0, aDim, aDepthDelta, aIsHole);
        initLabel(lbl, 1, bDim, bDepthDelta, bIsHole);
    };

    bool compareTo(const Edge& e) const
    {
        const geom::Coordinate& ca = getCoordinate(0);
        const geom::Coordinate& cb = e.getCoordinate(0);
        if(ca.compareTo(cb) < 0) {
            return true;
        }
        else if (ca.compareTo(cb) > 0) {
            return false;
        }
        else {
            const geom::Coordinate& cca = getCoordinate(1);
            const geom::Coordinate& ccb = e.getCoordinate(1);
            if(cca.compareTo(ccb) < 0) {
                return true;
            }
            else if (cca.compareTo(ccb) > 0) {
                return false;
            }
            else {
                return false;
            }
        }
    }

};

bool EdgeComparator(const Edge* a, const Edge* b);



} // namespace geos.operation.overlayng
} // namespace geos.operation
} // namespace geos

