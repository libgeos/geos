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
 **********************************************************************
 * $Log$
 * Revision 1.6  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/


#include "../../headers/indexBintree.h"
#include "../../headers/indexQuadtree.h"

namespace geos {

int Key::computeLevel(BinTreeInterval *newInterval) {
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

void Key::computeInterval(int level,BinTreeInterval *itemInterval){
	double size=DoubleBits::powerOf2(level);
	//double size = pow2.power(level);
	pt=floor(itemInterval->getMin()/size)*size;
	interval->init(pt,pt+size);
}
}

