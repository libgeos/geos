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

#ifndef GEOS_IDX_QUADTREE_KEY_H
#define GEOS_IDX_QUADTREE_KEY_H

// Forward declarations
namespace geos {
	namespace geom {
		class Envelope;
		class Coordinate;
	}
}

namespace geos {
namespace index { // geos::index
namespace quadtree { // geos::index::quadtree

/**
 * \brief
 * A Key is a unique identifier for a node in a quadtree.
 *
 * It contains a lower-left point and a level number. The level number
 * is the power of two for the size of the node envelope
 */
class Key {
public:
	static int computeQuadLevel(geom::Envelope *env);
	Key(geom::Envelope *itemEnv);
	virtual ~Key();
	geom::Coordinate* getPoint();
	int getLevel();
	geom::Envelope* getEnvelope();
	geom::Coordinate* getCentre();
	void computeKey(geom::Envelope *itemEnv);
private:	
	// the fields which make up the key
	geom::Coordinate *pt;
	int level;
	// auxiliary data which is derived from the key for use in computation
	geom::Envelope *env;
	void computeKey(int level,geom::Envelope *itemEnv);
};

} // namespace geos::index::quadtree
} // namespace geos::index
} // namespace geos

#endif // GEOS_IDX_QUADTREE_KEY_H

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/03/22 12:22:50  strk
 * indexQuadtree.h split
 *
 **********************************************************************/

