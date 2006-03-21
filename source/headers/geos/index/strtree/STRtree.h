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

#ifndef GEOS_INDEX_STRTREE_STRTREE_H
#define GEOS_INDEX_STRTREE_STRTREE_H

#include <geos/index/strtree/AbstractSTRtree.h> // for inheritance
#include <geos/index/SpatialIndex.h> // for inheritance
#include <geos/geom/Envelope.h> // for inlines

#include <vector>

// Forward declarations
namespace geos {
	namespace index { 
		namespace strtree { 
			class Boundable;
		}
	}
}


namespace geos {
namespace index { // geos::index
namespace strtree { // geos::index::strtree

/**
 * \brief
 * A query-only R-tree created using the Sort-Tile-Recursive (STR) algorithm. 
 * For two-dimensional spatial data. 
 *
 * The STR packed R-tree is simple to implement and maximizes space
 * utilization; that is, as many leaves as possible are filled to capacity.
 * Overlap between nodes is far less than in a basic R-tree. However, once the
 * tree has been built (explicitly or on the first call to #query), items may
 * not be added or removed. 
 * 
 * Described in: P. Rigaux, Michel Scholl and Agnes Voisard. Spatial
 * Databases With Application To GIS. Morgan Kaufmann, San Francisco, 2002. 
 *
 */
class STRtree: public AbstractSTRtree, public SpatialIndex
{
using AbstractSTRtree::insert;
using AbstractSTRtree::query;

private:
	class STRIntersectsOp: public AbstractSTRtree::IntersectsOp {
		public:
			bool intersects(const void* aBounds, const void* bBounds);
	};

	std::vector<Boundable*>* createParentBoundables(std::vector<Boundable*> *childBoundables, int newLevel);

	std::vector<Boundable*>* createParentBoundablesFromVerticalSlices(std::vector<std::vector<Boundable*>*>* verticalSlices, int newLevel);

	STRIntersectsOp intersectsOp;

protected:

	std::vector<Boundable*> *sortBoundables(
			const std::vector<Boundable*> *input);

	std::vector<Boundable*>* createParentBoundablesFromVerticalSlice(
			std::vector<Boundable*> *childBoundables,
			int newLevel);

	std::vector<std::vector<Boundable*>*>* verticalSlices(
			std::vector<Boundable*> *childBoundables,
			int sliceCount);

	AbstractNode* createNode(int level);
	
	IntersectsOp* getIntersectsOp() {
		return &intersectsOp;
	};

public:

	~STRtree();

	STRtree(int nodeCapacity=10);

	void insert(const geom::Envelope *itemEnv,void* item);

	static double centreX(geom::Envelope *e);

	static double avg(double a, double b) {
		return (a + b) / 2.0;
	}

	static double centreY(geom::Envelope *e) {
		return STRtree::avg(e->getMinY(), e->getMaxY());
	}

#if 0
	std::vector<void*>* query(const geom::Envelope *searchEnv) {
		return AbstractSTRtree::query(searchEnv);
	}
#endif

	void query(const geom::Envelope *searchEnv, std::vector<void*>& matches) {
		AbstractSTRtree::query(searchEnv, matches);
	}

	void query(const geom::Envelope *searchEnv, ItemVisitor& visitor) {
		return AbstractSTRtree::query(searchEnv, visitor);
	}

	bool remove(const geom::Envelope *itemEnv, void* item) {
		return AbstractSTRtree::remove(itemEnv, item);
	}
};

} // namespace geos::index::strtree
} // namespace geos::index
} // namespace geos

#endif // GEOS_INDEX_STRTREE_STRTREE_H

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/03/21 10:47:34  strk
 * indexStrtree.h split
 *
 **********************************************************************/

