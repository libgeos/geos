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

#ifndef GEOS_IDX_CHAIN_MONOTONECHAINBUILDER_H
#define GEOS_IDX_CHAIN_MONOTONECHAINBUILDER_H

#include <vector>

// Forward declarations
namespace geos {
	namespace geom {
		class CoordinateSequence;
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

/** \brief
 * A MonotoneChainBuilder implements functions to determine the monotone chains
 * in a sequence of points.
 *
 * TODO: use vector<const Coordinate*> instead
 *
 */
class MonotoneChainBuilder {

public:

	MonotoneChainBuilder(){}

	/** \brief
	 * Return a newly-allocated vector of newly-allocated
	 * MonotoneChain objects for the given CoordinateSequence.
	 * Remember to deep-delete the result.
	 */
	static std::vector<MonotoneChain*>* getChains(
			const geom::CoordinateSequence *pts,
			void* context);

	/** \brief
	 * Fill the provided vector with newly-allocated MonotoneChain objects
	 * for the given CoordinateSequence.
	 * Remember to delete vector elements!
	 */
	static void getChains(const geom::CoordinateSequence *pts,
			void* context,
			std::vector<MonotoneChain*>& mcList);

	static std::vector<MonotoneChain*>* getChains(const geom::CoordinateSequence *pts)
	{
		return getChains(pts, NULL);
	}

	/** \brief
	 * Fill the given vector with start/end indexes of the monotone chains
	 * for the given CoordinateSequence.
	 * The last entry in the array points to the end point of the point array,
	 * for use as a sentinel.
	 */
	static void getChainStartIndices(const geom::CoordinateSequence *pts,
			std::vector<int>& startIndexList);

	/** \brief
	 * @return the index of the last point in the monotone chain
	 * starting at <code>start</code>.
	 */
	static int findChainEnd(const geom::CoordinateSequence *pts, int start);
};

} // namespace geos::index::chain
} // namespace geos::index
} // namespace geos

#endif // GEOS_IDX_CHAIN_MONOTONECHAINBUILDER_H

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/03/22 18:12:31  strk
 * indexChain.h header split.
 *
 **********************************************************************/

