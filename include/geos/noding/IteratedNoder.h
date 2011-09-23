/**********************************************************************
 * $Id: IteratedNoder.h 2556 2009-06-06 22:22:28Z strk $
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2006      Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: noding/IteratedNoder.java rev. 1.6 (JTS-1.9)
 *
 **********************************************************************/

#ifndef GEOS_NODING_ITERATEDNODER_H
#define GEOS_NODING_ITERATEDNODER_H

#include <geos/export.h>

#include <vector>
#include <iostream>

#include <geos/inline.h>

#include <geos/algorithm/LineIntersector.h>
#include <geos/noding/SegmentString.h> // due to inlines
#include <geos/noding/Noder.h> // for inheritance

// Forward declarations
namespace geos {
	namespace geom {
		class PrecisionModel;
	}
}

namespace geos {
namespace noding { // geos::noding

/**
 * Nodes a set of SegmentStrings completely.
 * The set of segmentStrings is fully noded;
 * i.e. noding is repeated until no further
 * intersections are detected.
 * <p>
 * Iterated noding using a FLOATING precision model is not guaranteed to converge,
 * due to roundoff error.   This problem is detected and an exception is thrown.
 * Clients can choose to rerun the noding using a lower precision model.
 *
 */
class GEOS_DLL IteratedNoder : public Noder { // implements Noder

private:
	static const int MAX_ITER = 5;


	const geom::PrecisionModel *pm;
	algorithm::LineIntersector li;
	std::vector<SegmentString*>* nodedSegStrings;
	int maxIter;

	/**
	 * Node the input segment strings once
	 * and create the split edges between the nodes
	 */
	void node(std::vector<SegmentString*>* segStrings,
			int *numInteriorIntersections);

public:

	IteratedNoder(const geom::PrecisionModel *newPm)
		:
		pm(newPm),
		li(pm),
		maxIter(MAX_ITER)
	{
	}

	virtual ~IteratedNoder() {}

	/**
	 * Sets the maximum number of noding iterations performed before
	 * the noding is aborted.
	 * Experience suggests that this should rarely need to be changed
	 * from the default.
	 * The default is MAX_ITER.
	 *
	 * @param n the maximum number of iterations to perform
	 */
	void setMaximumIterations(int n) { maxIter = n; }

	std::vector<SegmentString*>* getNodedSubstrings() const {
		return nodedSegStrings;
	}
 

	/**
	 * Fully nodes a list of {@link SegmentStrings}, i.e. peforms noding iteratively
	 * until no intersections are found between segments.
	 * Maintains labelling of edges correctly through
	 * the noding.
	 *
	 * @param segStrings a collection of SegmentStrings to be noded
	 * @throws TopologyException if the iterated noding fails to converge.
	 */
	void computeNodes(std::vector<SegmentString*>* inputSegmentStrings); // throw(GEOSException);
};

} // namespace geos::noding
} // namespace geos


//#ifdef GEOS_INLINE
//# include "geos/noding/IteratedNoder.inl"
//#endif

#endif // GEOS_NODING_ITERATEDNODER_H

/**********************************************************************
 * $Log$
 * Revision 1.2  2006/03/24 09:52:41  strk
 * USE_INLINE => GEOS_INLINE
 *
 * Revision 1.1  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/

