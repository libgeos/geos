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

#ifndef GEOS_IDX_BINTREE_ROOT_H
#define GEOS_IDX_BINTREE_ROOT_H

#include <geos/index/bintree/NodeBase.h> // for inheritance

// Forward declarations
namespace geos {
	namespace index { 
		namespace bintree { 
			class Interval;
			class Node;
		}
	}
}

namespace geos {
namespace index { // geos::index
namespace bintree { // geos::index::bintree

/** \brief
 * The root node of a single Bintree.
 *
 * It is centred at the origin,
 * and does not have a defined extent.
 */
class Root: public NodeBase {

private:

	// the singleton root node is centred at the origin.
	static double origin;

	void insertContained(Node *tree,
			Interval *itemInterval,
			void* item);

public:

	Root() {}

	~Root() {}

	void insert(Interval *itemInterval,void* item);

protected:

	bool isSearchMatch(Interval *interval) { return true; }
};

} // namespace geos::index::bintree
} // namespace geos::index
} // namespace geos

#endif // GEOS_IDX_BINTREE_ROOT_H

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/03/22 16:01:33  strk
 * indexBintree.h header split, classes renamed to match JTS
 *
 **********************************************************************/

