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

#include <geos/index/quadtree/Key.h>
#include <geos/index/quadtree/DoubleBits.h>
#include <geos/geom/Envelope.h>
#include <geos/geom/Coordinate.h>

#include <cmath>

#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif

#ifdef GEOS_DEBUG
#include <iostream>
#endif

using namespace geos::geom;

namespace geos {
namespace index { // geos.index
namespace quadtree { // geos.index.quadtree

int
Key::computeQuadLevel(Envelope *env)
{
	double dx=env->getWidth();
	double dy=env->getHeight();
	double dMax=dx>dy?dx:dy;
	int level=DoubleBits::exponent(dMax)+1;
#if GEOS_DEBUG
	std::cerr<<"Maxdelta:"<<dMax<<" exponent:"<<(level-1)<<std::endl;
#endif
	return level;
}

Key::Key(Envelope *itemEnv){
	pt=new Coordinate();
	level=0;
	env=NULL;
	computeKey(itemEnv);
}

Key::~Key(){
	delete pt;
	delete env;
}

Coordinate* Key::getPoint() {
	return pt;
}

int Key::getLevel() {
	return level;
}

Envelope* Key::getEnvelope() {
	return env;
}

Coordinate* Key::getCentre() {
	return new Coordinate(
					(env->getMinX()+env->getMaxX())/2,
					(env->getMinY()+env->getMaxY())/2);
}

/**
 * return a square envelope containing the argument envelope,
 * whose extent is a power of two and which is based at a power of 2
 */
void Key::computeKey(Envelope *itemEnv) {
	level=computeQuadLevel(itemEnv);
	env=new Envelope();
	computeKey(level,itemEnv);
	// MD - would be nice to have a non-iterative form of this algorithm
	while (!env->contains(itemEnv)) {
		level+=1;
		computeKey(level,itemEnv);
	}
#if GEOS_DEBUG
	std::cerr<<"Key::computeKey:"<<std::endl;
	std::cerr<<" itemEnv: "<<itemEnv->toString()<<std::endl;
	std::cerr<<"  keyEnv: "<<env->toString()<<std::endl;
	std::cerr<<"  keyLvl: "<<level<<std::endl;

#endif
}

void
Key::computeKey(int level,Envelope *itemEnv)
{
	double quadSize=DoubleBits::powerOf2(level);
	//double quadSize=pow2.power(level);
	pt->x=std::floor(itemEnv->getMinX()/quadSize)*quadSize;
	pt->y=std::floor(itemEnv->getMinY()/quadSize)*quadSize;
	env->init(pt->x,pt->x+quadSize,pt->y,pt->y+quadSize);
}

} // namespace geos.index.quadtree
} // namespace geos.index
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/03/22 14:28:52  strk
 * Filenames renamed to match class names (matching JTS)
 *
 * Revision 1.13  2006/03/22 12:22:50  strk
 * indexQuadtree.h split
 *
 **********************************************************************/
