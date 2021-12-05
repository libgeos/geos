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

#include <geos/export.h>

#include <geos/geom/Envelope.h>
#include <geos/geom/CoordinateArraySequence.h>
#include <geos/geom/CoordinateSequence.h>

// Forward declarations
namespace geos {
namespace geom {
class Coordinate;
class CoordinateSequence;
class CoordinateArraySequence;
}
}

using namespace geos::geom;

namespace geos {      // geos.
namespace operation { // geos.operation
namespace overlayng { // geos.operation.overlayng

/**
 * Clips rings of points to a rectangle.
 * Uses a variant of Cohen-Sutherland clipping.
 *
 * In general the output is not topologically valid.
 * In particular, the output may contain coincident non-noded line segments
 * along the clip rectangle sides.
 * However, the output is sufficiently well-structured
 * that it can be used as input to the {@link OverlayNG} algorithm
 * (which is able to process coincident linework due
 * to the need to handle topology collapse under precision reduction).
 *
 * Because of the likelihood of creating
 * extraneous line segments along the clipping rectangle sides,
 * this class is not suitable for clipping linestrings.
 *
 * The clipping envelope should be generated using {@link RobustClipEnvelopeComputer},
 * to ensure that intersecting line segments are not perturbed
 * by clipping.
 * This is required to ensure that the overlay of the
 * clipped geometry is robust and correct (i.e. the same as
 * if clipping was not used).
 *
 * @see LineLimiter
 *
 * @author Martin Davis
 *
 */
class GEOS_DLL RingClipper {

private:

    // Constants
    static constexpr int BOX_LEFT = 3;
    static constexpr int BOX_TOP = 2;
    static constexpr int BOX_RIGHT = 1;
    static constexpr int BOX_BOTTOM = 0;

    // Members
    const Envelope clipEnv;

    // Methods

    /**
    * Clips line to the axis-parallel line defined by a single box edge.
    */
    std::vector<Coordinate> clipToBoxEdge(const std::vector<Coordinate> & pts, int edgeIndex, bool closeRing) const;

    /**
    * Computes the intersection point of a segment
    * with an edge of the clip box.
    * The segment must be known to intersect the edge.
    */
    void intersection(const Coordinate& a, const Coordinate& b, int edgeIndex, Coordinate& rsltPt) const {
        switch (edgeIndex) {
            case BOX_BOTTOM:
                rsltPt = Coordinate(intersectionLineY(a, b, clipEnv.getMinY()), clipEnv.getMinY());
                break;
            case BOX_RIGHT:
                rsltPt = Coordinate(clipEnv.getMaxX(), intersectionLineX(a, b, clipEnv.getMaxX()));
                break;
            case BOX_TOP:
                rsltPt = Coordinate(intersectionLineY(a, b, clipEnv.getMaxY()), clipEnv.getMaxY());
                break;
            case BOX_LEFT:
            default:
                rsltPt = Coordinate(clipEnv.getMinX(), intersectionLineX(a, b, clipEnv.getMinX()));
        }
    }

    static double intersectionLineY(const Coordinate& a, const Coordinate& b, double y) {
        double m = (b.x - a.x) / (b.y - a.y);
        double intercept = (y - a.y) * m;
        return a.x + intercept;
    }

    static double intersectionLineX(const Coordinate& a, const Coordinate& b, double x) {
        double m = (b.y - a.y) / (b.x - a.x);
        double intercept = (x - a.x) * m;
        return a.y + intercept;
    }

    bool isInsideEdge(const Coordinate& p, int edgeIndex) const {
        if (clipEnv.isNull()) {
            return false;
        }

        bool isInside = false;
        switch (edgeIndex) {
            case BOX_BOTTOM: // bottom
                isInside = p.y > clipEnv.getMinY();
                break;
            case BOX_RIGHT: // right
                isInside = p.x < clipEnv.getMaxX();
                break;
            case BOX_TOP: // top
                isInside = p.y < clipEnv.getMaxY();
                break;
            case BOX_LEFT:
            default: // left
                isInside = p.x > clipEnv.getMinX();
        }
        return isInside;
    }


public:

    RingClipper(const Envelope* env)
        : clipEnv(*env)
        {};

    /**
    * Clips a list of points to the clipping rectangle box.
    */
    std::unique_ptr<CoordinateArraySequence> clip(const CoordinateSequence* cs) const;

};


} // namespace geos.operation.overlayng
} // namespace geos.operation
} // namespace geos
