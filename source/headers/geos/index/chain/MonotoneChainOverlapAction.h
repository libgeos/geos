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
 **********************************************************************/

#ifndef GEOS_IDX_CHAIN_MONOTONECHAINOVERLAPACTION_H
#define GEOS_IDX_CHAIN_MONOTONECHAINOVERLAPACTION_H


// Forward declarations
namespace geos {
	namespace geom {
		class Envelope;
		class LineSegment;
	}
	namespace index { 
		namespace chain { 
			class MonotoneChain;
		}
	}
}

namespace geos {
namespace index { // geos::index
namespace chain { // geos::index::chain

/**
 * The action for the internal iterator for performing
 * overlap queries on a MonotoneChain
 */
class MonotoneChainOverlapAction {

protected:

	geom::LineSegment *overlapSeg1;

	geom::LineSegment *overlapSeg2;

public:

	MonotoneChainOverlapAction();

	virtual ~MonotoneChainOverlapAction();

	/**
	 * This function can be overridden if the original chains are needed
	 *
	 * @param start1 the index of the start of the overlapping segment from mc1
	 * @param start2 the index of the start of the overlapping segment from mc2
	 */
	virtual void overlap(MonotoneChain *mc1, int start1,
			MonotoneChain *mc2, int start2);

	/**
	 * This is a convenience function which can be overridden to obtain the actual
	 * line segments which overlap
	 * @param seg1
	 * @param seg2
	 */
	virtual void overlap(geom::LineSegment *newSeg1,
			geom::LineSegment *newSeg2)
	{}

	// these envelopes are used during the MonotoneChain search process
	geom::Envelope *tempEnv1;
	geom::Envelope *tempEnv2;
};

} // namespace geos::index::chain
} // namespace geos::index
} // namespace geos

#endif // GEOS_IDX_CHAIN_MONOTONECHAINOVERLAPACTION_H

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/03/22 18:12:31  strk
 * indexChain.h header split.
 *
 **********************************************************************/

