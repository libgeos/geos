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
 * Revision 1.5  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/


#include "../../headers/indexBintree.h"

namespace geos {

BinTreeInterval::BinTreeInterval(){
	min=0.0;
	max=0.0;
}

BinTreeInterval::BinTreeInterval(double nmin, double nmax){
	init(nmin, nmax);
}

BinTreeInterval::~BinTreeInterval(){
}

BinTreeInterval::BinTreeInterval(BinTreeInterval* interval){
	init(interval->min, interval->max);
}

void BinTreeInterval::init(double nmin, double nmax) {
	min=nmin;
	max=nmax;
	if (min>max) {
		min=nmax;
		max=nmin;
	}
}
 
double BinTreeInterval::getMin() {
	return min;
}

double BinTreeInterval::getMax() {
	return max;
}

double BinTreeInterval::getWidth() {
	return max-min;
}
 
void BinTreeInterval::expandToInclude(BinTreeInterval *interval) {
	if (interval->max>max) max=interval->max;
	if (interval->min<min) min=interval->min;
}
 
bool BinTreeInterval::overlaps(BinTreeInterval *interval) {
	return overlaps(interval->min,interval->max);
}
 
bool BinTreeInterval::overlaps(double nmin, double nmax){
	if (min>nmax || max<nmin) return false;
	return true;
}
 
bool BinTreeInterval::contains(BinTreeInterval *interval){
	return contains(interval->min,interval->max);
}

bool BinTreeInterval::contains(double nmin, double nmax){
	return (nmin>=min && nmax<=max);
}

bool BinTreeInterval::contains(double p) {
	return (p>=min && p<=max);
}
}

