/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2011 Sandro Santilli <strk@kbt.io>
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: operation/buffer/BufferCurveSetBuilder.java 4c343e79f (JTS-1.19)
 *
 **********************************************************************/

#pragma once

#include <geos/export.h>
#include <geos/geom/Location.h>
#include <geos/operation/buffer/OffsetCurveBuilder.h>

#include <vector>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251) // warning C4251: needs to have dll-interface to be used by clients of class
#endif

// Forward declarations
namespace geos {
namespace geom {
class Geometry;
class CoordinateSequence;
class PrecisionModel;
class GeometryCollection;
class Point;
class LineString;
class LinearRing;
class Polygon;
}
namespace geomgraph {
class Label;
}
namespace noding {
class SegmentString;
}
namespace operation {
namespace buffer {
class BufferParameters;
}
}
}

namespace geos {
namespace operation { // geos.operation
namespace buffer { // geos.operation.buffer

/**
 * \class BufferCurveSetBuilder
 *
 * \brief
 * Creates all the raw offset curves for a buffer of a Geometry.
 *
 * Raw curves need to be noded together and polygonized to form the
 * final buffer area.
 *
 */
class GEOS_DLL BufferCurveSetBuilder {

private:

    static constexpr int MAX_INVERTED_RING_SIZE = 9;
    static constexpr int INVERTED_CURVE_VERTEX_FACTOR = 4;
    static constexpr double NEARNESS_FACTOR = 0.99;

    // To keep track of newly-created Labels.
    // Labels will be released by object dtor
    std::vector<geomgraph::Label*> newLabels;
    const geom::Geometry& inputGeom;
    double distance;
    OffsetCurveBuilder curveBuilder;

    /// The raw offset curves computed.
    /// This class holds ownership of std::vector elements.
    ///
    std::vector<noding::SegmentString*> curveList;
    bool isInvertOrientation = false;

    /**
     * Creates a noding::SegmentString for a coordinate list which is a raw
     * offset curve, and adds it to the list of buffer curves.
     * The noding::SegmentString is tagged with a geomgraph::Label
     * giving the topology of the curve.
     * The curve may be oriented in either direction.
     * If the curve is oriented CW, the locations will be:
     * - Left: Location.EXTERIOR
     * - Right: Location.INTERIOR
     *
     * @param coord is raw offset curve, ownership transferred here
     */
    void addCurve(geom::CoordinateSequence* coord, geom::Location leftLoc,
                  geom::Location rightLoc);

    void add(const geom::Geometry& g);

    void addCollection(const geom::GeometryCollection* gc);

    /**
     * Add a Point to the graph.
     */
    void addPoint(const geom::Point* p);

    void addLineString(const geom::LineString* line);

    void addPolygon(const geom::Polygon* p);

    void addRingBothSides(const geom::CoordinateSequence* coord, double p_distance);

    /**
     * Add an offset curve for a polygon ring.
     * The side and left and right topological location arguments
     * assume that the ring is oriented CW.
     * If the ring is in the opposite orientation,
     * the left and right locations must be interchanged and the side
     * flipped.
     *
     * @param coord the coordinates of the ring (must not contain
     * repeated points)
     * @param offsetDistance the distance at which to create the buffer
     * @param side the side of the ring on which to construct the buffer
     *             line
     * @param cwLeftLoc the location on the L side of the ring
     *                  (if it is CW)
     * @param cwRightLoc the location on the R side of the ring
     *                   (if it is CW)
     */
    void addRingSide(const geom::CoordinateSequence* coord,
                     double offsetDistance, int side, geom::Location cwLeftLoc,
                     geom::Location cwRightLoc);

    /**
     * Tests whether the offset curve for a ring is fully inverted.
     * An inverted ("inside-out") curve occurs in some specific situations
     * involving a buffer distance which should result in a fully-eroded (empty) buffer.
     * It can happen that the sides of a small, convex polygon
     * produce offset segments which all cross one another to form
     * a curve with inverted orientation.
     * This happens at buffer distances slightly greater than the distance at
     * which the buffer should disappear.
     * The inverted curve will produce an incorrect non-empty buffer (for a shell)
     * or an incorrect hole (for a hole).
     * It must be discarded from the set of offset curves used in the buffer.
     * Heuristics are used to reduce the number of cases which area checked,
     * for efficiency and correctness.
     * <p>
     * See https://github.com/locationtech/jts/issues/472
     *
     * @param inputPts the input ring
     * @param distance the buffer distance
     * @param curvePts the generated offset curve
     * @return true if the offset curve is inverted
     */
    static bool isRingCurveInverted(
        const geom::CoordinateSequence* inputPts, double dist,
        const geom::CoordinateSequence* curvePts);

    /**
     * Tests if there are points on the raw offset curve which may
     * lie on the final buffer curve
     * (i.e. they are (approximately) at the buffer distance from the input ring). 
     * For efficiency this only tests a limited set of points on the curve.
     * 
     * @param inputRing
     * @param distance
     * @param curveRing
     * @return true if the curve contains points lying at the required buffer distance
     */
    static bool hasPointOnBuffer(
        const CoordinateSequence* inputRing, double dist,
        const CoordinateSequence* curveRing);

    /**
     * The ringCoord is assumed to contain no repeated points.
     * It may be degenerate (i.e. contain only 1, 2, or 3 points).
     * In this case it has no area, and hence has a minimum diameter of 0.
     *
     * @param ringCoord
     * @param bufferDistance
     * @return
     */
    bool isErodedCompletely(const geom::LinearRing* ringCoord,
                            double bufferDistance);

    /**
     * Tests whether a triangular ring would be eroded completely by
     * the given buffer distance.
     * This is a precise test.  It uses the fact that the inner buffer
     * of a triangle converges on the inCentre of the triangle (the
     * point equidistant from all sides).  If the buffer distance is
     * greater than the distance of the inCentre from a side, the
     * triangle will be eroded completely.
     *
     * This test is important, since it removes a problematic case where
     * the buffer distance is slightly larger than the inCentre distance.
     * In this case the triangle buffer curve "inverts" with incorrect
     * topology, producing an incorrect hole in the buffer.
     *
     * @param triCoord
     * @param bufferDistance
     * @return
     */
    bool isTriangleErodedCompletely(const geom::CoordinateSequence* triCoords,
                                    double bufferDistance);

    // Declare type as noncopyable
    BufferCurveSetBuilder(const BufferCurveSetBuilder& other) = delete;
    BufferCurveSetBuilder& operator=(const BufferCurveSetBuilder& rhs) = delete;

    /**
    * Computes orientation of a ring using a signed-area orientation test.
    * For invalid (self-crossing) rings this ensures the largest enclosed area
    * is taken to be the interior of the ring.
    * This produces a more sensible result when
    * used for repairing polygonal geometry via buffer-by-zero.
    * For buffer, using the lower robustness of orientation-by-area
    * doesn't matter, since narrow or flat rings
    * produce an acceptable offset curve for either orientation.
    *
    * @param coord the ring coordinates
    * @return true if the ring is CCW
    */
    bool isRingCCW(const geom::CoordinateSequence* coords) const;

public:

    /// Constructor
    BufferCurveSetBuilder(
        const geom::Geometry& newInputGeom,
        double newDistance,
        const geom::PrecisionModel* newPm,
        const BufferParameters& newBufParams)
        : inputGeom(newInputGeom)
        , distance(newDistance)
        , curveBuilder(newPm, newBufParams)
        , curveList()
        , isInvertOrientation(false)
        {};

    /// Destructor
    ~BufferCurveSetBuilder();

    /** \brief
     * Computes the set of raw offset curves for the buffer.
     *
     * Each offset curve has an attached {@link geomgraph::Label} indicating
     * its left and right location.
     *
     * @return a Collection of SegmentStrings representing the raw buffer curves
     */
    std::vector<noding::SegmentString*>& getCurves();

    /// \brief Add raw curves for a set of CoordinateSequences.
    ///
    /// @param lineList is a list of CoordinateSequence, ownership
    ///                 of which is transferred here
    /// @param leftLoc left location
    /// @param rightLoc right location
    ///
    void addCurves(const std::vector<geom::CoordinateSequence*>& lineList,
                   geom::Location leftLoc, geom::Location rightLoc);

    /**
    * Sets whether the offset curve is generated
    * using the inverted orientation of input rings.
    * This allows generating a buffer(0) polygon from the smaller lobes
    * of self-crossing rings.
    *
    * @param p_isInvertOrientation true if input ring orientation should be inverted
    */
    void setInvertOrientation(bool p_isInvertOrientation) {
        isInvertOrientation = p_isInvertOrientation;
    }

};

} // namespace geos::operation::buffer
} // namespace geos::operation
} // namespace geos

#ifdef _MSC_VER
#pragma warning(pop)
#endif
