/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2005-2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: algorithm/RobustLineIntersector.java r785 (JTS-1.13+)
 *
 **********************************************************************/

#pragma once

#include <geos/export.h>
#include <geos/algorithm/Intersection.h>
#include <geos/algorithm/Interpolate.h>
#include <geos/algorithm/Orientation.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/Envelope.h>
#include <geos/geom/PrecisionModel.h>

#include <string>

// Forward declarations
namespace geos {
namespace geom {
class PrecisionModel;
}
}

namespace geos {
namespace algorithm { // geos::algorithm

/** \brief
 * A LineIntersector is an algorithm that can both test whether
 * two line segments intersect and compute the intersection point
 * if they do.
 *
 * The intersection point may be computed in a precise or non-precise manner.
 * Computing it precisely involves rounding it to an integer.  (This assumes
 * that the input coordinates have been made precise by scaling them to
 * an integer grid.)
 *
 */
class GEOS_DLL LineIntersector {
public:

    /// Computes the "edge distance" of an intersection point p in an edge.
    ///
    /// The edge distance is a metric of the point along the edge.
    /// The metric used is a robust and easy to compute metric function.
    /// It is <b>not</b> equivalent to the usual Euclidean metric.
    /// It relies on the fact that either the x or the y ordinates of the
    /// points in the edge are unique, depending on whether the edge is longer in
    /// the horizontal or vertical direction.
    ///
    /// NOTE: This function may produce incorrect distances
    ///  for inputs where p is not precisely on p1-p2
    /// (E.g. p = (139,9) p1 = (139,10), p2 = (280,1) produces distanct
    /// 0.0, which is incorrect.
    ///
    /// My hypothesis is that the function is safe to use for points which are the
    /// result of <b>rounding</b> points which lie on the line,
    /// but not safe to use for <b>truncated</b> points.
    ///
    static double computeEdgeDistance(const geom::CoordinateXY& p, const geom::CoordinateXY& p0, const geom::CoordinateXY& p1);

    static double nonRobustComputeEdgeDistance(const geom::Coordinate& p, const geom::Coordinate& p1,
            const geom::Coordinate& p2);

    explicit LineIntersector(const geom::PrecisionModel* initialPrecisionModel = nullptr)
        :
        precisionModel(initialPrecisionModel),
        result(0),
        inputLines(),
        isProperVar(false)
    {}

    ~LineIntersector() = default;

    /** \brief
     * Tests whether either intersection point is an interior point of
     * one of the input segments.
     *
     * @return <code>true</code> if either intersection point is in
     * the interior of one of the input segments
     */
    bool isInteriorIntersection()
    {
        if(isInteriorIntersection(0)) {
            return true;
        }
        if(isInteriorIntersection(1)) {
            return true;
        }
        return false;
    };

    /** \brief
     * Tests whether either intersection point is an interior point
     * of the specified input segment.
     *
     * @return <code>true</code> if either intersection point is in
     * the interior of the input segment
     */
    bool isInteriorIntersection(std::size_t inputLineIndex)
    {
        for(std::size_t i = 0; i < result; ++i) {
            if(!(intPt[i].equals2D(*inputLines[inputLineIndex][0])
                 || intPt[i].equals2D(*inputLines[inputLineIndex][1]))) {
                return true;
            }
        }
        return false;
    };

    /// Force computed intersection to be rounded to a given precision model.
    ///
    /// No getter is provided, because the precision model is not required
    /// to be specified.
    /// @param newPM the PrecisionModel to use for rounding
    ///
    void
    setPrecisionModel(const geom::PrecisionModel* newPM)
    {
        precisionModel = newPM;
    }

    /// Compute the intersection of a point p and the line p1-p2.
    ///
    /// This function computes the boolean value of the hasIntersection test.
    /// The actual value of the intersection (if there is one)
    /// is equal to the value of <code>p</code>.
    ///
    void computeIntersection(const geom::CoordinateXY& p, const geom::CoordinateXY& p1, const geom::CoordinateXY& p2);

    /// Same as above but doesn't compute intersection point. Faster.
    static bool hasIntersection(const geom::CoordinateXY& p, const geom::CoordinateXY& p1, const geom::CoordinateXY& p2);

    enum intersection_type : uint8_t {
        /// Indicates that line segments do not intersect
        NO_INTERSECTION = 0,

        /// Indicates that line segments intersect in a single point
        POINT_INTERSECTION = 1,

        /// Indicates that line segments intersect in a line segment
        COLLINEAR_INTERSECTION = 2
    };

    /// Computes the intersection of the lines p1-p2 and p3-p4
    template<typename C1, typename C2>
    void computeIntersection(const C1& p1, const C1& p2,
                             const C2& p3, const C2& p4)
    {
        inputLines[0][0] = &p1;
        inputLines[0][1] = &p2;
        inputLines[1][0] = &p3;
        inputLines[1][1] = &p4;
        result = computeIntersect(p1, p2, p3, p4);
    }

    /// Compute the intersection between two segments, given a sequence and starting index of each
    void computeIntersection(const geom::CoordinateSequence& p, std::size_t p0,
                             const geom::CoordinateSequence& q, std::size_t q0);

    std::string toString() const;

    /**
     * Tests whether the input geometries intersect.
     *
     * @return true if the input geometries intersect
     */
    bool
    hasIntersection() const
    {
        return result != NO_INTERSECTION;
    }


    /**
    * Gets an endpoint of an input segment.
    *
    * @param segmentIndex the index of the input segment (0 or 1)
    * @param ptIndex the index of the endpoint (0 or 1)
    * @return the specified endpoint
    */
    const geom::CoordinateXY*
    getEndpoint(std::size_t segmentIndex, std::size_t ptIndex) const
    {
        return inputLines[segmentIndex][ptIndex];
    }

    /// Returns the number of intersection points found.
    ///
    /// This will be either 0, 1 or 2.
    ///
    size_t
    getIntersectionNum() const
    {
        return result;
    }


    /// Returns the intIndex'th intersection point
    ///
    /// @param intIndex is 0 or 1
    ///
    /// @return the intIndex'th intersection point
    ///
    const geom::CoordinateXYZM&
    getIntersection(std::size_t intIndex) const
    {
        return intPt[intIndex];
    }

    /// Returns false if both numbers are zero.
    ///
    /// @return true if both numbers are positive or if both numbers are negative.
    ///
    static bool isSameSignAndNonZero(double a, double b);

    /** \brief
     * Test whether a point is a intersection point of two line segments.
     *
     * Note that if the intersection is a line segment, this method only tests for
     * equality with the endpoints of the intersection segment.
     * It does <b>not</b> return true if
     * the input point is internal to the intersection segment.
     *
     * @return true if the input point is one of the intersection points.
     */
    bool isIntersection(const geom::Coordinate& pt) const
    {
        for(std::size_t i = 0; i < result; ++i) {
            if(intPt[i].equals2D(pt)) {
                return true;
            }
        }
        return false;
    };

    /** \brief
     * Tests whether an intersection is proper.
     *
     * The intersection between two line segments is considered proper if
     * they intersect in a single point in the interior of both segments
     * (e.g. the intersection is a single point and is not equal to any of the
     * endpoints).
     *
     * The intersection between a point and a line segment is considered proper
     * if the point lies in the interior of the segment (e.g. is not equal to
     * either of the endpoints).
     *
     * @return true if the intersection is proper
     */
    bool
    isProper() const
    {
        return hasIntersection() && isProperVar;
    }

    /** \brief
     * Computes the intIndex'th intersection point in the direction of
     * a specified input line segment
     *
     * @param segmentIndex is 0 or 1
     * @param intIndex is 0 or 1
     *
     * @return the intIndex'th intersection point in the direction of the
     *         specified input line segment
     */
    const geom::Coordinate& getIntersectionAlongSegment(std::size_t segmentIndex, std::size_t intIndex);

    /** \brief
     * Computes the index of the intIndex'th intersection point in the direction of
     * a specified input line segment
     *
     * @param segmentIndex is 0 or 1
     * @param intIndex is 0 or 1
     *
     * @return the index of the intersection point along the segment (0 or 1)
     */
    std::size_t getIndexAlongSegment(std::size_t segmentIndex, std::size_t intIndex);

    /** \brief
     * Computes the "edge distance" of an intersection point along the specified
     * input line segment.
     *
     * @param geomIndex is 0 or 1
     * @param intIndex is 0 or 1
     *
     * @return the edge distance of the intersection point
     */
    double getEdgeDistance(std::size_t geomIndex, std::size_t intIndex) const;

private:

    /**
     * If makePrecise is true, computed intersection coordinates
     * will be made precise using Coordinate#makePrecise
     */
    const geom::PrecisionModel* precisionModel;

    std::size_t result;

    const geom::CoordinateXY* inputLines[2][2];

    /**
     * We store real Coordinates here because
     * we must compute the Z of intersection point.
     */
    geom::CoordinateXYZM intPt[2];

    /**
     * The indexes of the endpoints of the intersection lines, in order along
     * the corresponding line
     */
    std::size_t intLineIndex[2][2];

    bool isProperVar;
    //Coordinate &pa;
    //Coordinate &pb;

    bool
    isCollinear() const
    {
        return result == COLLINEAR_INTERSECTION;
    }

    template<typename C1, typename C2>
    uint8_t computeIntersect(const C1& p1, const C1& p2, const C2& q1, const C2& q2)
    {
        isProperVar = false;

        // first try a fast test to see if the envelopes of the lines intersect
        if(!geom::Envelope::intersects(p1, p2, q1, q2)) {
            return NO_INTERSECTION;
        }

        // for each endpoint, compute which side of the other segment it lies
        // if both endpoints lie on the same side of the other segment,
        // the segments do not intersect
        int Pq1 = Orientation::index(p1, p2, q1);
        int Pq2 = Orientation::index(p1, p2, q2);

        if((Pq1 > 0 && Pq2 > 0) || (Pq1 < 0 && Pq2 < 0)) {
            return NO_INTERSECTION;
        }

        int Qp1 = Orientation::index(q1, q2, p1);
        int Qp2 = Orientation::index(q1, q2, p2);

        if((Qp1 > 0 && Qp2 > 0) || (Qp1 < 0 && Qp2 < 0)) {
            return NO_INTERSECTION;
        }

        /**
         * Intersection is collinear if each endpoint lies on the other line.
         */
        bool collinear = Pq1 == 0 && Pq2 == 0 && Qp1 == 0 && Qp2 == 0;
        if(collinear) {
            return computeCollinearIntersection(p1, p2, q1, q2);
        }

        /*
         * At this point we know that there is a single intersection point
         * (since the lines are not collinear).
         */

        /*
         * Check if the intersection is an endpoint.
         * If it is, copy the endpoint as
         * the intersection point. Copying the point rather than
         * computing it ensures the point has the exact value,
         * which is important for robustness. It is sufficient to
         * simply check for an endpoint which is on the other line,
         * since at this point we know that the inputLines must
         *  intersect.
         */
        geom::CoordinateXYZM p;
        double z = DoubleNotANumber;
        double m = DoubleNotANumber;

        if(Pq1 == 0 || Pq2 == 0 || Qp1 == 0 || Qp2 == 0) {

        isProperVar = false;

        /* Check for two equal endpoints.
         * This is done explicitly rather than by the orientation tests
         * below in order to improve robustness.
         *
         * (A example where the orientation tests fail
         *  to be consistent is:
         *
         * LINESTRING ( 19.850257749638203 46.29709338043669,
         * 			20.31970698357233 46.76654261437082 )
         * and
         * LINESTRING ( -48.51001596420236 -22.063180333403878,
         * 			19.850257749638203 46.29709338043669 )
         *
         * which used to produce the INCORRECT result:
         * (20.31970698357233, 46.76654261437082, NaN)
         */

        if (p1.equals2D(q1)) {
            p = p1;
            z = Interpolate::zGet(p1, q1);
            m = Interpolate::mGet(p1, q1);
        }
        else if (p1.equals2D(q2)) {
            p = p1;
            z = Interpolate::zGet(p1, q2);
            m = Interpolate::mGet(p1, q2);
        }
        else if (p2.equals2D(q1)) {
            p = p2;
            z = Interpolate::zGet(p2, q1);
            m = Interpolate::mGet(p2, q1);
        }
        else if (p2.equals2D(q2)) {
            p = p2;
            z = Interpolate::zGet(p2, q2);
            m = Interpolate::mGet(p2, q2);
        }
        /*
         * Now check to see if any endpoint lies on the interior of the other segment.
         */
        else if(Pq1 == 0) {
            p = q1;
            z = Interpolate::zGetOrInterpolate(q1, p1, p2);
            m = Interpolate::mGetOrInterpolate(q1, p1, p2);
        }
        else if(Pq2 == 0) {
            p = q2;
            z = Interpolate::zGetOrInterpolate(q2, p1, p2);
            m = Interpolate::mGetOrInterpolate(q2, p1, p2);
        }
        else if(Qp1 == 0) {
            p = p1;
            z = Interpolate::zGetOrInterpolate(p1, q1, q2);
            m = Interpolate::mGetOrInterpolate(p1, q1, q2);
        }
        else if(Qp2 == 0) {
            p = p2;
            z = Interpolate::zGetOrInterpolate(p2, q1, q2);
            m = Interpolate::mGetOrInterpolate(p2, q1, q2);
        }
    } else {
        isProperVar = true;
        p = intersection(p1, p2, q1, q2);
        z = Interpolate::zInterpolate(p, p1, p2, q1, q2);
        m = Interpolate::mInterpolate(p, p1, p2, q1, q2);
        }
        intPt[0] = geom::CoordinateXYZM(p.x, p.y, z, m);
    #if GEOS_DEBUG
        std::cerr << " POINT_INTERSECTION; intPt[0]:" << intPt[0].toString() << std::endl;
    #endif // GEOS_DEBUG
        return POINT_INTERSECTION;
    }

    bool
    isEndPoint() const
    {
        return hasIntersection() && !isProperVar;
    }

    void computeIntLineIndex();

    void computeIntLineIndex(std::size_t segmentIndex);

    template<typename C1, typename C2>
    uint8_t computeCollinearIntersection(const C1& p1, const C1& p2, const C2& q1, const C2& q2)
    {
        bool q1inP = geom::Envelope::intersects(p1, p2, q1);
        bool q2inP = geom::Envelope::intersects(p1, p2, q2);
        bool p1inQ = geom::Envelope::intersects(q1, q2, p1);
        bool p2inQ = geom::Envelope::intersects(q1, q2, p2);

        if(q1inP && q2inP) {
            intPt[0] = zmGetOrInterpolateCopy(q1, p1, p2);
            intPt[1] = zmGetOrInterpolateCopy(q2, p1, p2);
            return COLLINEAR_INTERSECTION;
        }
        if(p1inQ && p2inQ) {
            intPt[0] = zmGetOrInterpolateCopy(p1, q1, q2);
            intPt[1] = zmGetOrInterpolateCopy(p2, q1, q2);
            return COLLINEAR_INTERSECTION;
        }
        if(q1inP && p1inQ) {
            // if pts are equal Z is chosen arbitrarily
            intPt[0] = zmGetOrInterpolateCopy(q1, p1, p2);
            intPt[1] = zmGetOrInterpolateCopy(p1, q1, q2);

            return (q1 == p1) && !q2inP && !p2inQ ? POINT_INTERSECTION : COLLINEAR_INTERSECTION;
        }
        if(q1inP && p2inQ) {
            // if pts are equal Z is chosen arbitrarily
            intPt[0] = zmGetOrInterpolateCopy(q1, p1, p2);
            intPt[1] = zmGetOrInterpolateCopy(p2, q1, q2);

            return (q1 == p2) && !q2inP && !p1inQ ? POINT_INTERSECTION : COLLINEAR_INTERSECTION;
        }
        if(q2inP && p1inQ) {
            // if pts are equal Z is chosen arbitrarily
            intPt[0] = zmGetOrInterpolateCopy(q2, p1, p2);
            intPt[1] = zmGetOrInterpolateCopy(p1, q1, q2);

            return (q2 == p1) && !q1inP && !p2inQ ? POINT_INTERSECTION : COLLINEAR_INTERSECTION;
        }
        if(q2inP && p2inQ) {
            // if pts are equal Z is chosen arbitrarily
            intPt[0] = zmGetOrInterpolateCopy(q2, p1, p2);
            intPt[1] = zmGetOrInterpolateCopy(p2, q1, q2);
            return (q2 == p2) && !q1inP && !p1inQ ? POINT_INTERSECTION : COLLINEAR_INTERSECTION;
        }
        return NO_INTERSECTION;
    }

    /** \brief
     * This method computes the actual value of the intersection point.
     *
     * To obtain the maximum precision from the intersection calculation,
     * the coordinates are normalized by subtracting the minimum
     * ordinate values (in absolute value).  This has the effect of
     * removing common significant digits from the calculation to
     * maintain more bits of precision.
     */
    template<typename C1, typename C2>
    geom::CoordinateXYZM intersection (const C1& p1, const C1& p2, const C2& q1, const C2& q2) const {
        auto intPtOut = intersectionSafe(p1, p2, q1, q2);

        /*
         * Due to rounding it can happen that the computed intersection is
         * outside the envelopes of the input segments.  Clearly this
         * is inconsistent.
         * This code checks this condition and forces a more reasonable answer
         *
         * MD - May 4 2005 - This is still a problem.  Here is a failure case:
         *
         * LINESTRING (2089426.5233462777 1180182.3877339689,
         *             2085646.6891757075 1195618.7333999649)
         * LINESTRING (1889281.8148903656 1997547.0560044837,
         *             2259977.3672235999 483675.17050843034)
         * int point = (2097408.2633752143,1144595.8008114607)
         */

        if(! isInSegmentEnvelopes(intPtOut)) {
            //intPt = CentralEndpointIntersector::getIntersection(p1, p2, q1, q2);
            intPtOut = nearestEndpoint(p1, p2, q1, q2);
        }

        if(precisionModel != nullptr) {
            precisionModel->makePrecise(intPtOut);
        }

        return intPtOut;
    }

    /**
     * Test whether a point lies in the envelopes of both input segments.
     * A correctly computed intersection point should return true
     * for this test.
     * Since this test is for debugging purposes only, no attempt is
     * made to optimize the envelope test.
     *
     * @return true if the input point lies within both
     *         input segment envelopes
     */
    bool isInSegmentEnvelopes(const geom::CoordinateXY& pt) const
    {
        geom::Envelope env0(*inputLines[0][0], *inputLines[0][1]);
        geom::Envelope env1(*inputLines[1][0], *inputLines[1][1]);
        return env0.contains(pt) && env1.contains(pt);
    };

    /**
     * Computes a segment intersection.
     * Round-off error can cause the raw computation to fail,
     * (usually due to the segments being approximately parallel).
     * If this happens, a reasonable approximation is computed instead.
     *
     * @param p1 a segment endpoint
     * @param p2 a segment endpoint
     * @param q1 a segment endpoint
     * @param q2 a segment endpoint
     * @return the computed intersection point is stored there
     */
    template<typename C1, typename C2>
    geom::CoordinateXYZM intersectionSafe(const C1& p1, const C1& p2,
                                          const C2& q1, const C2& q2) const
    {
        geom::CoordinateXYZM ptInt(Intersection::intersection(p1, p2, q1, q2));
        if (ptInt.isNull()) {
            // FIXME need to cast to correct type in mixed-dimensionality case
            ptInt = static_cast<const C1&>(nearestEndpoint(p1, p2, q1, q2));
        }
        return ptInt;
    }

    /**
     * Finds the endpoint of the segments P and Q which
     * is closest to the other segment.
     * This is a reasonable surrogate for the true
     * intersection points in ill-conditioned cases
     * (e.g. where two segments are nearly coincident,
     * or where the endpoint of one segment lies almost on the other segment).
     * <p>
     * This replaces the older CentralEndpoint heuristic,
     * which chose the wrong endpoint in some cases
     * where the segments had very distinct slopes
     * and one endpoint lay almost on the other segment.
     *
     * @param p1 an endpoint of segment P
     * @param p2 an endpoint of segment P
     * @param q1 an endpoint of segment Q
     * @param q2 an endpoint of segment Q
     * @return the nearest endpoint to the other segment
     */
    static const geom::CoordinateXY& nearestEndpoint(const geom::CoordinateXY& p1,
                                                     const geom::CoordinateXY& p2,
                                                     const geom::CoordinateXY& q1,
                                                     const geom::CoordinateXY& q2);


    template<typename C1, typename C2>
    static geom::CoordinateXYZM zmGetOrInterpolateCopy(
        const C1& p,
        const C2& p1,
        const C2& p2)
    {
        geom::CoordinateXYZM pCopy(p);
        pCopy.z = Interpolate::zGetOrInterpolate(p, p1, p2);
        pCopy.m = Interpolate::mGetOrInterpolate(p, p1, p2);
        return pCopy;
    }

};


} // namespace geos::algorithm
} // namespace geos
















