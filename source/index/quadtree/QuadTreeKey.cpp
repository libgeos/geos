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

#include <geos/indexQuadtree.h>
#include <cmath>

#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif

#ifdef GEOS_DEBUG
#include <iostream>
#endif

namespace geos {
namespace index { // geos.index
namespace quadtree { // geos.index.quadtree

int QuadTreeKey::computeQuadLevel(Envelope *env){
	double dx=env->getWidth();
	double dy=env->getHeight();
	double dMax=dx>dy?dx:dy;
	int level=DoubleBits::exponent(dMax)+1;
#if GEOS_DEBUG
	std::cerr<<"Maxdelta:"<<dMax<<" exponent:"<<(level-1)<<std::endl;
#endif
	return level;
}

QuadTreeKey::QuadTreeKey(Envelope *itemEnv){
	pt=new Coordinate();
	level=0;
	env=NULL;
	computeKey(itemEnv);
}

QuadTreeKey::~QuadTreeKey(){
	delete pt;
	delete env;
}

Coordinate* QuadTreeKey::getPoint() {
	return pt;
}

int QuadTreeKey::getLevel() {
	return level;
}

Envelope* QuadTreeKey::getEnvelope() {
	return env;
}

Coordinate* QuadTreeKey::getCentre() {
	return new Coordinate(
					(env->getMinX()+env->getMaxX())/2,
					(env->getMinY()+env->getMaxY())/2);
}

/**
 * return a square envelope containing the argument envelope,
 * whose extent is a power of two and which is based at a power of 2
 */
void QuadTreeKey::computeKey(Envelope *itemEnv) {
	level=computeQuadLevel(itemEnv);
	env=new Envelope();
	computeKey(level,itemEnv);
	// MD - would be nice to have a non-iterative form of this algorithm
	while (!env->contains(itemEnv)) {
		level+=1;
		computeKey(level,itemEnv);
	}
#if GEOS_DEBUG
	std::cerr<<"QuadTreeKey::computeKey:"<<std::endl;
	std::cerr<<" itemEnv: "<<itemEnv->toString()<<std::endl;
	std::cerr<<"  keyEnv: "<<env->toString()<<std::endl;
	std::cerr<<"  keyLvl: "<<level<<std::endl;

#endif
}

void
QuadTreeKey::computeKey(int level,Envelope *itemEnv)
{
	double quadSize=DoubleBits::powerOf2(level);
	//double quadSize=pow2.power(level);
	pt->x=floor(itemEnv->getMinX()/quadSize)*quadSize;
	pt->y=floor(itemEnv->getMinY()/quadSize)*quadSize;
	env->init(pt->x,pt->x+quadSize,pt->y,pt->y+quadSize);
}

} // namespace geos.index.quadtree
} // namespace geos.index
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.11  2006/03/15 18:44:52  strk
 * Bug #60 - Missing <cmath> header in some files
 *
 * Revision 1.10  2006/03/09 15:34:57  strk
 * Fixed debugging lines
 *
 * Revision 1.9  2006/03/02 12:12:00  strk
 * Renamed DEBUG macros to GEOS_DEBUG, all wrapped in #ifndef block to allow global override (bug#43)
 *
 * Revision 1.8  2006/02/20 10:14:18  strk
 * - namespaces geos::index::*
 * - Doxygen documentation cleanup
 *
 * Revision 1.7  2004/11/01 16:43:04  strk
 * Added Profiler code.
 * Temporarly patched a bug in DoubleBits (must check drawbacks).
 * Various cleanups and speedups.
 *
 * Revision 1.6  2004/07/02 13:28:27  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.5  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/
