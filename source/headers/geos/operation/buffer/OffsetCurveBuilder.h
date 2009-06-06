/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2009  Sandro Santilli <strk@keybit.net>
 * Copyright (C) 2006-2007 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: operation/buffer/OffsetCurveBuilder.java rev. 1.30 (JTS-1.9)
 *
 **********************************************************************/

#ifndef GEOS_OP_BUFFER_OFFSETCURVEBUILDER_H
#define GEOS_OP_BUFFER_OFFSETCURVEBUILDER_H

#include <geos/export.h>

#include <vector>

#include <geos/algorithm/LineIntersector.h> // for composition
#include <geos/geom/Coordinate.h> // for composition
#include <geos/geom/LineSegment.h> // for composition
#include <geos/operation/buffer/BufferParameters.h> // for composition

// Forward declarations
namespace geos {
	namespace geom {
		class CoordinateSequence;
		class PrecisionModel;
	}
	namespace operation {
		namespace buffer {
			class OffsetCurveVertexList;
		}
	}
}

namespace geos {
namespace operation { // geos.operation
namespace buffer { // geos.operation.buffer

/**
 * \class OffsetCurveBuilder opBuffer.h geos/opBuffer.h
 *
 * \brief
 * Computes the raw offset curve for a
 * single Geometry component (ring, line or point).
 *
 * A raw offset curve line is not noded -
 * it may contain self-intersections (and usually will).
 * The final buffer polygon is computed by forming a topological graph
 * of all the noded raw curves and tracing outside contours.
 * The points in the raw curve are rounded to the required precision model.
 *
 * Last port: operation/buffer/OffsetCurveBuilder.java rev. 1.9
 *
 */
class GEOS_DLL OffsetCurveBuilder {
public:

	/*
	 * @param nBufParams buffer parameters, this object will
	 *                   keep a reference to the passed parameters
	 *                   so caller must make sure the object is
	 *                   kept alive for the whole lifetime of
	 *                   the buffer builder.
	 */
	OffsetCurveBuilder(const geom::PrecisionModel *newPrecisionModel,
			const BufferParameters& bufParams);

	~OffsetCurveBuilder();

	/**
	 * This method handles single points as well as lines.
	 * Lines are assumed to <b>not</b> be closed (the function will not
	 * fail for closed lines, but will generate superfluous line caps).
	 *
	 * @param lineList the std::vector to which CoordinateSequences will
	 *                 be pushed_back
	 */
	void getLineCurve(const geom::CoordinateSequence* inputPts,
	                  double distance,
	                  std::vector<geom::CoordinateSequence*>& lineList);

	/**
	 * This method handles single points as well as lines.
	 *
	 * Lines are assumed to <b>not</b> be closed (the function will not
	 * fail for closed lines, but will generate superfluous line caps).
	 *
	 * @param lineList the std::vector to which CoordinateSequences will
	 *                 be pushed_back
	 * @param leftSide indicates that the left side buffer will be
	 *                 obtained/skipped
	 * @param rightSide indicates that the right side buffer will
	 *                  be obtained/skipped
	 *
	 * NOTE: this is a GEOS extension
	 */
	void getSingleSidedLineCurve(const geom::CoordinateSequence* inputPts, 
	     double distance, std::vector<geom::CoordinateSequence*>& lineList,
	     bool leftSide, bool rightSide ) ;

	/**
	 * This method handles the degenerate cases of single points and lines,
	 * as well as rings.
	 *
	 * @param lineList the std::vector to which CoordinateSequences will
	 *                 be pushed_back
	 */
	void getRingCurve(const geom::CoordinateSequence *inputPts, int side,
	                  double distance,
	                  std::vector<geom::CoordinateSequence*>& lineList);


private:

	/// The mitre will be beveled if it exceeds the mitre ratio limit.
	//
	/// @param offset0 the first offset segment
	/// @param offset1 the second offset segment
	/// @param distance the offset distance
	///
	void addMitreJoin(const geom::Coordinate& p,
	                  const geom::LineSegment& offset0,
	                  const geom::LineSegment& offset1,
	                  double distance);

	/// Adds a limited mitre join connecting the two reflex offset segments.
	//
	/// A limited mitre is a mitre which is beveled at the distance
	/// determined by the mitre ratio limit.
	///
	/// @param offset0 the first offset segment
	/// @param offset1 the second offset segment
	/// @param distance the offset distance
	/// @param mitreLimit the mitre limit ratio
	///
	void addLimitedMitreJoin(
	                  const geom::LineSegment& offset0,
	                  const geom::LineSegment& offset1,
	                  double distance, double mitreLimit);

	/// \brief
	/// Adds a bevel join connecting the two offset segments
	/// around a reflex corner.
	// 
	/// @param offset0 the first offset segment
	/// @param offset1 the second offset segment
	///
	void addBevelJoin(const geom::LineSegment& offset0,
	                  const geom::LineSegment& offset1);

   
	/**
	 * Factor which controls how close curve vertices can be to be snapped
	 */
	static const double CURVE_VERTEX_SNAP_DISTANCE_FACTOR; //  1.0E-6;

	static const double PI; //  3.14159265358979

	static const double MAX_CLOSING_SEG_LEN; // 3.0

	/**
	 * Factor which controls how close offset segments can be to
	 * skip adding a filler or mitre.
	 */
	static const double OFFSET_SEGMENT_SEPARATION_FACTOR; // 1.0E-3;

	/**
	 * Factor which controls how close curve vertices on inside turns
	 * can be to be snapped
	 */
	static const double INSIDE_TURN_VERTEX_SNAP_DISTANCE_FACTOR; // 1.0E-3;

	/** \brief
	 * Factor which determines how short closing segs can be
	 * for round buffers
	 */
	static const int MAX_CLOSING_SEG_FRACTION = 80;

	algorithm::LineIntersector li;

	/** \brief
	 * The angle quantum with which to approximate a fillet curve
	 * (based on the input # of quadrant segments)
	 */
	double filletAngleQuantum;

	/** \brief
	 * the max error of approximation (distance) between a quad segment and
	 * the true fillet curve
	 */
	double maxCurveSegmentError;

	/// Owned by this object, destroyed by dtor 
	//
	/// This actually gets created multiple times
	/// and each of the old versions is pushed
	/// to the ptLists std::vector to ensure all
	/// created CoordinateSequences are properly 
	/// destroyed.
	///
	OffsetCurveVertexList* vertexList;

	double distance;

	const geom::PrecisionModel* precisionModel;

	const BufferParameters& bufParams; 

	/// The Closing Segment Factor controls how long "closing
	/// segments" are.  Closing segments are added at the middle of
	/// inside corners to ensure a smoother boundary for the buffer
	/// offset curve.  In some cases (particularly for round joins
	/// with default-or-better quantization) the closing segments
	/// can be made quite short.  This substantially improves
	/// performance (due to fewer intersections being created).
	/// 
	/// A closingSegFactor of 0 results in lines to the corner vertex.
	/// A closingSegFactor of 1 results in lines halfway
	/// to the corner vertex.
	/// A closingSegFactor of 80 results in lines 1/81 of the way
	/// to the corner vertex (this option is reasonable for the very
	/// common default situation of round joins and quadrantSegs >= 8).
	///
	/// The default is 1.
	///
	int closingSegFactor; // 1;

	geom::Coordinate s0, s1, s2;

	geom::LineSegment seg0;

	geom::LineSegment seg1;

	geom::LineSegment offset0;

	geom::LineSegment offset1;

	int side;

	// Not in JTS, used for single-sided buffers
	int endCapIndex;

	void init(double newDistance);

	/**
	 * Use a value which results in a potential distance error which is
	 * significantly less than the error due to
	 * the quadrant segment discretization.
	 * For QS = 8 a value of 100 is reasonable.
	 * This should produce a maximum of 1% distance error.
	 */
	static const double SIMPLIFY_FACTOR; // 100.0;

	/** \brief
	 * Computes the distance tolerance to use during input
	 * line simplification.
	 *
	 * @param distance the buffer distance
	 * @return the simplification tolerance
	 */
	double simplifyTolerance(double bufDistance);

	void computeLineBufferCurve(const geom::CoordinateSequence& inputPts);

	void computeRingBufferCurve(const geom::CoordinateSequence& inputPts,
	                            int side);

	void initSideSegments(const geom::Coordinate &nS1,
	                      const geom::Coordinate &nS2, int nSide);

	void addNextSegment(const geom::Coordinate &p, bool addStartPoint);

	void addCollinear(bool addStartPoint);

	/// Adds the offset points for an outside (convex) turn
	//
	/// @param orientation 
	/// @param addStartPoint  
	///
	void addOutsideTurn(int orientation, bool addStartPoint);

	/// Adds the offset points for an inside (concave) turn
	//
	/// @param orientation 
	/// @param addStartPoint  
	///
	void addInsideTurn(int orientation, bool addStartPoint);

	/// Add last offset point
	void addLastSegment();

	/** \brief
	 * Compute an offset segment for an input segment on a given
	 * side and at a given distance.
	 *
	 * The offset points are computed in full double precision,
	 * for accuracy.
	 *
	 * @param seg the segment to offset
	 * @param side the side of the segment the offset lies on
	 * @param distance the offset distance
	 * @param offset the points computed for the offset segment
	 */
	void computeOffsetSegment(const geom::LineSegment& seg,
	                          int side, double distance,
	                          geom::LineSegment& offset);

	/// \brief
	/// Add an end cap around point p1, terminating a line segment
	/// coming from p0
	void addLineEndCap(const geom::Coordinate &p0,
	                   const geom::Coordinate &p1);

	/**
	 * Adds points for a circular fillet around a reflex corner.
	 * 
	 * Adds the start and end points
	 *
	 * @param p base point of curve
	 * @param p0 start point of fillet curve
	 * @param p1 endpoint of fillet curve
	 * @param direction the orientation of the fillet
	 * @param radius the radius of the fillet
	 */
	void addFillet(const geom::Coordinate &p, const geom::Coordinate &p0,
	               const geom::Coordinate &p1,
	               int direction, double radius);

	/** 
	 * Adds points for a circular fillet arc between two specified angles.
	 *
	 * The start and end point for the fillet are not added -
	 * the caller must add them if required.
	 *
	 * @param direction is -1 for a CW angle, 1 for a CCW angle
	 * @param radius the radius of the fillet
	 */
	void addFillet(const geom::Coordinate &p, double startAngle,
	               double endAngle, int direction, double radius);

	/// Adds a CW circle around a point
	void addCircle(const geom::Coordinate &p, double distance);

	/// Adds a CW square around a point
	void addSquare(const geom::Coordinate &p, double distance);

	std::vector<OffsetCurveVertexList*> vertexLists;
};

} // namespace geos::operation::buffer
} // namespace geos::operation
} // namespace geos

#endif // ndef GEOS_OP_BUFFER_OFFSETCURVEBUILDER_H

/**********************************************************************
 * $Log$
 * Revision 1.2  2006/03/27 17:04:17  strk
 * Cleanups and explicit initializations
 *
 * Revision 1.1  2006/03/14 00:19:40  strk
 * opBuffer.h split, streamlined headers in some (not all) files in operation/buffer/
 *
 **********************************************************************/

