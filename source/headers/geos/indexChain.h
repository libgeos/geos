/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 * $Log$
 * Revision 1.7  2005/02/15 17:15:13  strk
 * Inlined most Envelope methods, reserved() memory for some vectors when
 * the usage was known a priori.
 *
 * Revision 1.6  2004/11/04 19:08:06  strk
 * Cleanups, initializers list, profiling.
 *
 * Revision 1.5  2004/10/26 17:46:18  strk
 * Removed slash-stars in comments to remove annoying compiler warnings.
 *
 * Revision 1.4  2004/07/19 13:19:31  strk
 * Documentation fixes
 *
 * Revision 1.3  2004/07/13 08:33:52  strk
 * Added missing virtual destructor to virtual classes.
 * Fixed implicit unsigned int -> int casts
 *
 * Revision 1.2  2004/07/08 19:34:49  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added DefaultCoordinateSequenceFactory::instance() function.
 *
 * Revision 1.1  2004/07/02 13:20:42  strk
 * Header files moved under geos/ dir.
 *
 * Revision 1.8  2004/05/27 09:53:49  strk
 * MonotoneChainOverlapAction::overlap(*) funx made virtual
 * as they are supposed to be.
 *
 * Revision 1.7  2004/03/25 02:23:55  ybychkov
 * All "index/" packages upgraded to JTS 1.4
 *
 * Revision 1.6  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/


#ifndef GEOS_INDEXCHAIN_H
#define GEOS_INDEXCHAIN_H

#include <memory>
#include <vector>
#include <geos/platform.h>
#include <geos/geom.h>

using namespace std;

namespace geos {

class indexMonotoneChain;
/*
 * The action for the internal iterator for performing
 * envelope select queries on a MonotoneChain
 */
class MonotoneChainSelectAction {
protected:
	LineSegment *selectedSegment;
public:
	MonotoneChainSelectAction();
	virtual ~MonotoneChainSelectAction();
	/**
	* This function can be overridden if the original chain is needed
	*/
	virtual void select(indexMonotoneChain *mc,int start);
	/**
	* This is a convenience function which can be overridden to obtain the actual
	* line segment which is selected
	* @param seg
	*/
	virtual void select(LineSegment *newSeg){}
	// these envelopes are used during the MonotoneChain search process
	Envelope *tempEnv1;
};

/*
 * The action for the internal iterator for performing
 * overlap queries on a MonotoneChain
 */
class MonotoneChainOverlapAction {
protected:
	LineSegment *overlapSeg1;
	LineSegment *overlapSeg2;
public:
	MonotoneChainOverlapAction();
	virtual ~MonotoneChainOverlapAction();

	/**
	* This function can be overridden if the original chains are needed
	*
	* @param start1 the index of the start of the overlapping segment from mc1
	* @param start2 the index of the start of the overlapping segment from mc2
	*/
	virtual void overlap(indexMonotoneChain *mc1,int start1,indexMonotoneChain *mc2,int start2);
	/**
	* This is a convenience function which can be overridden to obtain the actual
	* line segments which overlap
	* @param seg1
	* @param seg2
	*/
	virtual void overlap(LineSegment *newSeg1,LineSegment *newSeg2){}
	// these envelopes are used during the MonotoneChain search process
	Envelope *tempEnv1;
	Envelope *tempEnv2;
};


/*
 * MonotoneChains are a way of partitioning the segments of a linestring to
 * allow for fast searching of intersections.
 * They have the following properties:
 * <ol>
 * <li>the segments within a monotone chain will never intersect each other
 * <li>the envelope of any contiguous subset of the segments in a monotone chain
 * is equal to the envelope of the endpoints of the subset.
 * </ol>
 * Property 1 means that there is no need to test pairs of segments from within
 * the same monotone chain for intersection.
 * Property 2 allows
 * binary search to be used to find the intersection points of two monotone chains.
 * For many types of real-world data, these properties eliminate a large number of
 * segment comparisons, producing substantial speed gains.
 * <p>
 * One of the goals of this implementation of MonotoneChains is to be
 * as space and time efficient as possible. One design choice that aids this
 * is that a MonotoneChain is based on a subarray of a list of points.
 * This means that new arrays of points (potentially very large) do not
 * have to be allocated.
 * <p>
 *
 * MonotoneChains support the following kinds of queries:
 * <ul>
 * <li>Envelope select: determine all the segments in the chain which
 * intersect a given envelope
 * <li>Overlap: determine all the pairs of segments in two chains whose
 * envelopes overlap
 * </ul>
 *
 * This implementation of MonotoneChains uses the concept of internal iterators
 * to return the resultsets for the above queries.
 * This has time and space advantages, since it
 * is not necessary to build lists of instantiated objects to represent the segments
 * returned by the query.
 * However, it does mean that the queries are not thread-safe.
 *
 * @version 1.4
 */
class indexMonotoneChain {
public:
	indexMonotoneChain(CoordinateSequence *newPts,int nstart,int nend, void* nContext);
	~indexMonotoneChain();
	Envelope* getEnvelope();
	int getStartIndex();
	int getEndIndex();
	void getLineSegment(int index,LineSegment *ls);
	/**
	* Return the subsequence of coordinates forming this chain.
	* Allocates a new array to hold the Coordinates
	*/
	CoordinateSequence* getCoordinates();
	/**
	* Determine all the line segments in the chain whose envelopes overlap
	* the searchEnvelope, and process them
	*/
	void select(Envelope *searchEnv,MonotoneChainSelectAction *mcs);
	void computeOverlaps(indexMonotoneChain *mc,MonotoneChainOverlapAction *mco);

	void setId(int nId);

	inline int getId();

	void* getContext();

private:
	void computeSelect(Envelope *searchEnv,int start0,int end0,MonotoneChainSelectAction *mcs);
	void computeOverlaps(int start0,int end0,indexMonotoneChain *mc,int start1,int end1,MonotoneChainOverlapAction *mco);
	CoordinateSequence *pts;
	int start, end;
	Envelope *env;
	void *context;// user-defined information
	int id; // useful for optimizing chain comparisons
};

// INLINE FUNCTIONS
inline int
indexMonotoneChain::getId() { return id; }


/*
 * A MonotoneChainBuilder implements functions to determine the monotone chains
 * in a sequence of points.
 */
class MonotoneChainBuilder {
public:
//	static int[] toIntArray(List list); //Not needed
	MonotoneChainBuilder(){}
	static vector<indexMonotoneChain*>* getChains(CoordinateSequence *pts);
	/**
	* Return a list of the {@link MonotoneChain}s
	* for the given list of coordinates.
	*/
	static vector<indexMonotoneChain*>* getChains(CoordinateSequence *pts,void* context);
	/**
	* Return an array containing lists of start/end indexes of the monotone chains
	* for the given list of coordinates.
	* The last entry in the array points to the end point of the point array,
	* for use as a sentinel.
	*/
	static vector<int>* getChainStartIndices(CoordinateSequence *pts);
	/**
	* @return the index of the last point in the monotone chain starting at <code>start</code>.
	*/
	static int findChainEnd(CoordinateSequence *pts,int start);
};
}
#endif

