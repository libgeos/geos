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

#include <geos/edgegraph/HalfEdge.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/Location.h>
#include <geos/operation/overlayng/OverlayEdge.h>
#include <geos/operation/overlayng/OverlayLabel.h>
#include <geos/export.h>

#include <memory>

// Forward declarations
namespace geos {
namespace geom {
class Coordinate;
class CoordinateSequence;
}
namespace operation {
namespace overlayng {
class OverlayEdgeRing;
class MaximalEdgeRing;
}
}
}

using geos::geom::Coordinate;
using geos::geom::CoordinateXYZM;
using geos::geom::CoordinateSequence;
using geos::geom::Location;

namespace geos {      // geos.
namespace operation { // geos.operation
namespace overlayng { // geos.operation.overlayng

/**
* Creates a single OverlayEdge.
*/
class GEOS_DLL OverlayEdge : public edgegraph::HalfEdge {

private:

    // Members
    const CoordinateSequence* pts;
    /**
    * 'true' indicates direction is forward along segString
    * 'false' is reverse direction
    * The label must be interpreted accordingly.
    */
    bool direction;
    CoordinateXYZM dirPt;
    OverlayLabel* label;
    bool m_isInResultArea;
    bool m_isInResultLine;
    bool m_isVisited;
    OverlayEdge* nextResultEdge;
    const OverlayEdgeRing* edgeRing;
    const MaximalEdgeRing* maxEdgeRing;
    OverlayEdge* nextResultMaxEdge;

    void markVisited()
    {
        m_isVisited = true;
    };


public:

    OverlayEdge(const CoordinateXYZM& p_orig, const CoordinateXYZM& p_dirPt,
                bool p_direction, OverlayLabel* p_label,
                const CoordinateSequence* p_pts)
        : HalfEdge(p_orig)
        , pts(p_pts)
        , direction(p_direction)
        , dirPt(p_dirPt)
        , label(p_label)
        , m_isInResultArea(false)
        , m_isInResultLine(false)
        , m_isVisited(false)
        , nextResultEdge(nullptr)
        , edgeRing(nullptr)
        , maxEdgeRing(nullptr)
        , nextResultMaxEdge(nullptr)
     {}

    ~OverlayEdge() override {};

    bool isForward() const
    {
        return direction;
    };

    const CoordinateXYZM& directionPt() const override
    {
        return dirPt;
    };

    OverlayLabel* getLabel() const
    {
        return label;
    };

    Location getLocation(uint8_t index, int position) const
    {
        return label->getLocation(index, position, direction);
    };

    const CoordinateXYZM& getCoordinate() const
    {
        return orig();
    };

    const CoordinateSequence* getCoordinatesRO() const
    {
        return pts;
    };

    std::unique_ptr<CoordinateSequence> getCoordinates()
    {
        // return a copy of pts
        return pts->clone();
    };

    std::unique_ptr<CoordinateSequence> getCoordinatesOriented();

    /**
    * Adds the coordinates of this edge to the given list,
    * in the direction of the edge.
    * Duplicate coordinates are removed
    * (which means that this is safe to use for a path
    * of connected edges in the topology graph).
    *
    * @param coords the coordinate list to add to
    */
    void addCoordinates(CoordinateSequence* coords) const;

    OverlayEdge* symOE() const
    {
        return static_cast<OverlayEdge*>(sym());
    };

    OverlayEdge* oNextOE() const
    {
        return static_cast<OverlayEdge*>(oNext());
    };

    bool isInResultArea() const
    {
        return m_isInResultArea;
    };

    bool isInResultAreaBoth() const
    {
        return m_isInResultArea && symOE()->m_isInResultArea;
    };

    bool isInResultEither() const
    {
        return isInResult() || symOE()->isInResult();
    };

    void unmarkFromResultAreaBoth()
    {
        m_isInResultArea = false;
        symOE()->m_isInResultArea = false;
    };

    void markInResultArea()
    {
        m_isInResultArea  = true;
    };

    void markInResultAreaBoth()
    {
        m_isInResultArea  = true;
        symOE()->m_isInResultArea = true;
    };

    bool isInResultLine() const
    {
        return m_isInResultLine;
    };

    void markInResultLine()
    {
        m_isInResultLine  = true;
        symOE()->m_isInResultLine = true;
    };

    bool isInResult() const
    {
        return m_isInResultArea || m_isInResultLine;
    };

    void setNextResult(OverlayEdge* e)
    {
        // Assert: e.orig() == this.dest();
        nextResultEdge = e;
    };

    OverlayEdge* nextResult() const
    {
        return nextResultEdge;
    };

    bool isResultLinked() const
    {
        return nextResultEdge != nullptr;
    };

    void setNextResultMax(OverlayEdge* e)
    {
        // Assert: e.orig() == this.dest();
        nextResultMaxEdge = e;
    };

    OverlayEdge* nextResultMax() const
    {
        return nextResultMaxEdge;
    };

    bool isResultMaxLinked() const
    {
        return nextResultMaxEdge != nullptr;
    };

    bool isVisited() const
    {
        return m_isVisited;
    };

    void markVisitedBoth()
    {
        markVisited();
        symOE()->markVisited();
    };

    void setEdgeRing(const OverlayEdgeRing* p_edgeRing)
    {
        edgeRing = p_edgeRing;
    };

    const OverlayEdgeRing* getEdgeRing() const
    {
        return edgeRing;
    };

    const MaximalEdgeRing* getEdgeRingMax() const
    {
        return maxEdgeRing;
    };

    void setEdgeRingMax(const MaximalEdgeRing* p_maximalEdgeRing)
    {
        maxEdgeRing = p_maximalEdgeRing;
    };

    friend std::ostream& operator<<(std::ostream& os, const OverlayEdge& oe);
    std::string resultSymbol() const;

};


} // namespace geos.operation.overlayng
} // namespace geos.operation
} // namespace geos
