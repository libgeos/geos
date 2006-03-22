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

#ifndef GEOS_IDX_QUADTREE_INTERVALSIZE_H
#define GEOS_IDX_QUADTREE_INTERVALSIZE_H

namespace geos {
namespace index { // geos::index
namespace quadtree { // geos::index::quadtree

/**
 * \class IntervalSize indexQuadtree.h geos/indexQuadtree.h
 *
 * \brief
 * Provides a test for whether an interval is
 * so small it should be considered as zero for the purposes of
 * inserting it into a binary tree.
 *
 * The reason this check is necessary is that round-off error can
 * cause the algorithm used to subdivide an interval to fail, by
 * computing a midpoint value which does not lie strictly between the
 * endpoints.
 */
class IntervalSize {
public:
	/**
	 * This value is chosen to be a few powers of 2 less than the
	 * number of bits available in the double representation (i.e. 53).
	 * This should allow enough extra precision for simple computations
	 * to be correct,
	 * at least for comparison purposes.
	 */
	static const int MIN_BINARY_EXPONENT=-50;
	static bool isZeroWidth(double min, double max);
};

} // namespace geos::index::quadtree
} // namespace geos::index
} // namespace geos

#endif // GEOS_IDX_QUADTREE_INTERVALSIZE_H

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/03/22 12:22:50  strk
 * indexQuadtree.h split
 *
 **********************************************************************/

