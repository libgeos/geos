/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#ifndef GEOS_OP_BUFFER_DEPTHSEGMENT_H
#define GEOS_OP_BUFFER_DEPTHSEGMENT_H

#include <geos/geom/LineSegment.h> // for composition

// Forward declarations
namespace geos {
	namespace geom {
	}
}

namespace geos {
namespace operation { // geos.operation
namespace buffer { // geos.operation.buffer

/**
 * \class DepthSegment opBuffer.h geos/opBuffer.h
 *
 * \brief
 * A segment from a directed edge which has been assigned a depth value
 * for its sides.
 */
class DepthSegment {
private:
	geom::LineSegment upwardSeg;

	/**
	 * Compare two collinear segments for left-most ordering.
	 * If segs are vertical, use vertical ordering for comparison.
	 * If segs are equal, return 0.
	 * Segments are assumed to be directed so that the second
	 * coordinate is >= to the first
	 * (e.g. up and to the right).
	 *
	 * @param seg0 a segment to compare
	 * @param seg1 a segment to compare
	 * @return
	 */
	int compareX(geom::LineSegment *seg0, geom::LineSegment *seg1);

public:
	int leftDepth;
	DepthSegment(const geom::LineSegment &seg, int depth);
	~DepthSegment();

	/**
	 * Defines a comparision operation on DepthSegments
	 * which orders them left to right
	 *
	 * <pre>
	 * DS1 < DS2   if   DS1.seg is left of DS2.seg
	 * DS1 > DS2   if   DS1.seg is right of DS2.seg
	 * </pre>
	 *
	 * @param obj
	 * @return
	 */
	int compareTo(DepthSegment *);
};

bool DepthSegmentLT(DepthSegment *first, DepthSegment *second);


} // namespace geos::operation::buffer
} // namespace geos::operation
} // namespace geos

#endif // ndef GEOS_OP_BUFFER_DEPTHSEGMENT_H

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/03/14 00:19:40  strk
 * opBuffer.h split, streamlined headers in some (not all) files in operation/buffer/
 *
 **********************************************************************/

