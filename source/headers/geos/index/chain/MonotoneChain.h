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
 *
 * Last port: index/chain/MonotoneChain.java rev. 1.15 (JTS-1.10)
 *
 **********************************************************************/

#ifndef GEOS_IDX_CHAIN_MONOTONECHAIN_H
#define GEOS_IDX_CHAIN_MONOTONECHAIN_H

#include <geos/geom/Envelope.h> // for inline

// Forward declarations
namespace geos {
	namespace geom {
		class Envelope;
		class LineSegment;
		class CoordinateSequence;
	}
	namespace index { 
		namespace chain { 
			class MonotoneChainSelectAction;
			class MonotoneChainOverlapAction;
		}
	}
}

namespace geos {
namespace index { // geos::index
namespace chain { // geos::index::chain

/** \brief
 * Monotone Chains are a way of partitioning the segments of a linestring to
 * allow for fast searching of intersections.
 *
 * They have the following properties:
 * 
 * - the segments within a monotone chain never intersect each other
 * - the envelope of any contiguous subset of the segments in a monotone
 *   chain is equal to the envelope of the endpoints of the subset.
 * 
 * Property 1 means that there is no need to test pairs of segments from
 * within the same monotone chain for intersection.
 * Property 2 allows an efficient binary search to be used to find the
 * intersection points of two monotone chains.
 *
 * For many types of real-world data, these properties eliminate
 * a large number of segment comparisons, producing substantial speed gains.
 *
 * One of the goals of this implementation of MonotoneChains is to be
 * as space and time efficient as possible. One design choice that aids this
 * is that a MonotoneChain is based on a subarray of a list of points.
 * This means that new arrays of points (potentially very large) do not
 * have to be allocated.
 *
 * MonotoneChains support the following kinds of queries:
 * 
 * - Envelope select: determine all the segments in the chain which
 *   intersect a given envelope
 * - Overlap: determine all the pairs of segments in two chains whose
 *   envelopes overlap
 *
 * This implementation of MonotoneChains uses the concept of internal iterators
 * to return the resultsets for the above queries.
 * This has time and space advantages, since it
 * is not necessary to build lists of instantiated objects to represent the segments
 * returned by the query.
 * However, it does mean that the queries are not thread-safe.
 *
 */
class MonotoneChain
{
public:

	MonotoneChain(const geom::CoordinateSequence* pts,
                  size_t start, size_t end, void* context);

	~MonotoneChain();

	/// Returned envelope is owned by this class
	geom::Envelope* getEnvelope();

	size_t getStartIndex() { return start; }

	size_t getEndIndex() { return end; }

	/// Set given LineSegment with points of the segment starting
	/// at the given index.
	void getLineSegment(unsigned int index, geom::LineSegment *ls);

	/**
	 * Return the subsequence of coordinates forming this chain.
	 * Allocates a new CoordinateSequence to hold the Coordinates
	 *
	 * TODO: return by auto_ptr
	 *
	 */
	geom::CoordinateSequence* getCoordinates();

	/**
	 * Determine all the line segments in the chain whose envelopes overlap
	 * the searchEnvelope, and process them
	 */
	void select(const geom::Envelope& searchEnv,
			MonotoneChainSelectAction& mcs);

	void computeOverlaps(MonotoneChain *mc,
			MonotoneChainOverlapAction *mco);

	void setId(int nId) { id=nId; }

	inline int getId() { return id; }

	void* getContext() { return context; }

private:

	void computeSelect(const geom::Envelope& searchEnv,
			unsigned int start0,
			unsigned int end0,
			MonotoneChainSelectAction& mcs);

	void computeOverlaps(int start0, int end0, MonotoneChain* mc,
			int start1, int end1, MonotoneChainOverlapAction* mco);

	/// Externally owned (could be a reference)
	const geom::CoordinateSequence* pts;

	/// Owned by this class, lazely created
	geom::Envelope* env;

	/// user-defined information
	void* context;

	/// Index of chain start vertex into the CoordinateSequence, 0 based.
	size_t start;

	/// Index of chain end vertex into the CoordinateSequence, 0 based.
	size_t end;

	/// useful for optimizing chain comparisons
	int id;

};

} // namespace geos::index::chain
} // namespace geos::index
} // namespace geos

#endif // GEOS_IDX_CHAIN_MONOTONECHAIN_H

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/03/22 18:12:31  strk
 * indexChain.h header split.
 *
 **********************************************************************/

