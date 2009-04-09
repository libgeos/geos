/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2006-2007 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: operation/buffer/OffsetCurveBuilder.java rev 1.10
 *
 **********************************************************************/

#ifndef GEOS_OP_BUFFER_OFFSETCURVEBUILDER_H
#define GEOS_OP_BUFFER_OFFSETCURVEBUILDER_H

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
class OffsetCurveBuilder {
public:
	/** \brief
	 * The default number of facets into which to divide a fillet
	 * of 90 degrees.
	 *
	 * A value of 8 gives less than 2% max error in the buffer distance.
	 * For a max error of < 1%, use QS = 12
	 */
	static const int DEFAULT_QUADRANT_SEGMENTS=8;

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
	void getLineCurve(const geom::CoordinateSequence* inputPts, double distance,
		std::vector<geom::CoordinateSequence*>& lineList);

	/**
	 * This method handles the degenerate cases of single points and lines,
	 * as well as rings.
	 *
	 * @param lineList the std::vector to which CoordinateSequences will
	 *                 be pushed_back
	 */
	void getRingCurve(const geom::CoordinateSequence *inputPts, int side,
		double distance, std::vector<geom::CoordinateSequence*>& lineList);


private:

	static const double MIN_CURVE_VERTEX_FACTOR; //  1.0E-6;

	static const double PI; //  3.14159265358979

	static const double MAX_CLOSING_SEG_LEN; // 3.0

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

	geom::Coordinate s0, s1, s2;

	geom::LineSegment seg0;

	geom::LineSegment seg1;

	geom::LineSegment offset0;

	geom::LineSegment offset1;

	int side;

//	static geom::CoordinateSequence* copyCoordinates(geom::CoordinateSequence *pts);

	void init(double newDistance);

	//geom::CoordinateSequence* getCoordinates();

	void computeLineBufferCurve(const geom::CoordinateSequence& inputPts);

	void computeRingBufferCurve(const geom::CoordinateSequence& inputPts, int side);

	//void addPt(const geom::Coordinate &pt);

	//void closePts();

	void initSideSegments(const geom::Coordinate &nS1, const geom::Coordinate &nS2, int nSide);

	void addNextSegment(const geom::Coordinate &p, bool addStartPoint);

	/// Add last offset point
	void addLastSegment();

	/**
	 * Compute an offset segment for an input segment on a given side and at a
	 * given distance.
	 * The offset points are computed in full double precision, for accuracy.
	 *
	 * @param seg the segment to offset
	 * @param side the side of the segment the offset lies on
	 * @param distance the offset distance
	 * @param offset the points computed for the offset segment
	 */
	void computeOffsetSegment(const geom::LineSegment& seg, int side, double distance,
			geom::LineSegment& offset);

	/// Add an end cap around point p1, terminating a line segment coming from p0
	void addLineEndCap(const geom::Coordinate &p0,const geom::Coordinate &p1);

	/**
	 * @param p base point of curve
	 * @param p0 start point of fillet curve
	 * @param p1 endpoint of fillet curve
	 */
	void addFillet(const geom::Coordinate &p, const geom::Coordinate &p0,
			const geom::Coordinate &p1, int direction, double distance);

	/** \brief
	 * Adds points for a fillet. 
	 * The start and end point for the fillet are not added -
	 * the caller must add them if required.
	 *
	 * @param direction is -1 for a CW angle, 1 for a CCW angle
	 */
	void addFillet(const geom::Coordinate &p, double startAngle, double endAngle,
			int direction, double distance);

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

