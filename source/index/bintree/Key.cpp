/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/indexBintree.h>
#include <geos/index/quadtree/DoubleBits.h>

#include <cmath>

namespace geos {
namespace index { // geos.index
namespace bintree { // geos.index.bintree

int
Key::computeLevel(BinTreeInterval *newInterval)
{
	using geos::index::quadtree::DoubleBits;
	double dx=newInterval->getWidth();
	//int level = BinaryPower.exponent(dx) + 1;
	int level=DoubleBits::exponent(dx)+1;
	return level;
}

Key::Key(BinTreeInterval *newInterval){
	interval=NULL;
	pt=0.0;
	level=0;
	computeKey(newInterval);
}

Key::~Key(){
	delete interval;
}

double Key::getPoint() {
	return pt;
}

int Key::getLevel() {
	return level;
}

BinTreeInterval* Key::getInterval() {
	return interval;
}

/**
* return a square envelope containing the argument envelope,
* whose extent is a power of two and which is based at a power of 2
*/
void Key::computeKey(BinTreeInterval *itemInterval) {
	level=computeLevel(itemInterval);
	delete interval;
	interval=new BinTreeInterval();
	computeInterval(level,itemInterval);
	// MD - would be nice to have a non-iterative form of this algorithm
	while (!interval->contains(itemInterval)) {
		level+=1;
		computeInterval(level,itemInterval);
	}
}

void
Key::computeInterval(int level,BinTreeInterval *itemInterval)
{
	using geos::index::quadtree::DoubleBits;

	double size=DoubleBits::powerOf2(level);
	//double size = pow2.power(level);
	pt=floor(itemInterval->getMin()/size)*size;
	interval->init(pt,pt+size);
}

} // namespace geos.index.bintree
} // namespace geos.index
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.10  2006/03/22 12:22:50  strk
 * indexQuadtree.h split
 *
 * Revision 1.9  2006/03/15 18:44:52  strk
 * Bug #60 - Missing <cmath> header in some files
 *
 * Revision 1.8  2006/02/20 10:14:18  strk
 * - namespaces geos::index::*
 * - Doxygen documentation cleanup
 *
 * Revision 1.7  2004/07/02 13:28:27  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.6  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/

