/**********************************************************************
 * $Id: MCIndexPointSnapper.h 3240 2011-02-23 14:37:39Z strk $
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
 * Last port: noding/snapround/MCIndexPointSnapper.java r320 (JTS-1.12)
 *
 **********************************************************************/

#ifndef GEOS_NODING_SNAPROUND_MCINDEXPOINTSNAPPER_H
#define GEOS_NODING_SNAPROUND_MCINDEXPOINTSNAPPER_H

#include <geos/export.h>

#include <geos/inline.h>

// Forward declarations
namespace geos {
	namespace index {
		class SpatialIndex;
	}
	namespace noding {
		class SegmentString;
		namespace snapround {
			class HotPixel;
		}
	}
}

namespace geos {
namespace noding { // geos::noding
namespace snapround { // geos::noding::snapround

/** \brief
 * "Snaps" all {@link SegmentString}s in a {@link SpatialIndex} containing
 * {@link MonotoneChain}s to a given {@link HotPixel}.
 *
 */
class GEOS_DLL MCIndexPointSnapper {

public:
 

	MCIndexPointSnapper(index::SpatialIndex& nIndex)
		:
		index(nIndex)
	{}

	/**
	 * Snaps (nodes) all interacting segments to this hot pixel.
	 * The hot pixel may represent a vertex of an edge,
	 * in which case this routine uses the optimization
	 * of not noding the vertex itself
	 *
	 * @param hotPixel the hot pixel to snap to
	 * @param parentEdge the edge containing the vertex,
	 *        if applicable, or <code>null</code>
	 * @param vertexIndex the index of the vertex, if applicable, or -1
	 * @return <code>true</code> if a node was added for this pixel
	 */
	bool snap(HotPixel& hotPixel, SegmentString* parentEdge,
			unsigned int vertexIndex);

	bool snap(HotPixel& hotPixel) {
		return snap(hotPixel, 0, 0);
	}


private:

	index::SpatialIndex& index;

    // Declare type as noncopyable
    MCIndexPointSnapper(const MCIndexPointSnapper& other);
    MCIndexPointSnapper& operator=(const MCIndexPointSnapper& rhs);
};


} // namespace geos::noding::snapround
} // namespace geos::noding
} // namespace geos

//#ifdef GEOS_INLINE
//# include "geos/noding/snapround/MCIndexPointSnapper.inl"
//#endif

#endif // GEOS_NODING_SNAPROUND_MCINDEXPOINTSNAPPER_H

/**********************************************************************
 * $Log$
 * Revision 1.3  2006/03/24 09:52:41  strk
 * USE_INLINE => GEOS_INLINE
 *
 * Revision 1.2  2006/03/22 18:12:31  strk
 * indexChain.h header split.
 *
 * Revision 1.1  2006/03/14 12:55:56  strk
 * Headers split: geomgraphindex.h, nodingSnapround.h
 *
 **********************************************************************/

