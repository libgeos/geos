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

#ifndef GEOS_IDX_CHAIN_MONOTONECHAINSELECTACTION_H
#define GEOS_IDX_CHAIN_MONOTONECHAINSELECTACTION_H


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
 *  The action for the internal iterator for performing
 *  Envelope select queries on a MonotoneChain
 *
 * Last port: index/chain/MonotoneChainSelectAction.java rev. 1.6 (JTS-1.7)
 */
class MonotoneChainSelectAction {

protected:

	geom::LineSegment* selectedSegment;

public:

	MonotoneChainSelectAction();

	virtual ~MonotoneChainSelectAction();

	/// This function can be overridden if the original chain is needed
	virtual void select(MonotoneChain& mc, unsigned int start);

	/**
	 * This is a convenience function which can be overridden to obtain the actual
	 * line segment which is selected
	 * @param seg
	 */
	virtual void select(geom::LineSegment* newSeg)=0;

	// these envelopes are used during the MonotoneChain search process
	geom::Envelope* tempEnv1;
};


} // namespace geos::index::chain
} // namespace geos::index
} // namespace geos

#endif // GEOS_IDX_CHAIN_MONOTONECHAINSELECTACTION_H

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/03/22 18:12:31  strk
 * indexChain.h header split.
 *
 **********************************************************************/

