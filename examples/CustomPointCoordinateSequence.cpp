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
 * Revision 1.3  2006/02/09 15:52:47  strk
 * GEOSException derived from std::exception; always thrown and cought by const ref.
 *
 * Revision 1.2  2005/01/28 09:47:51  strk
 * Replaced sprintf uses with ostringstream.
 *
 * Revision 1.1  2004/07/08 19:41:27  strk
 * renamed to reflect JTS API.
 *
 * Revision 1.6  2004/07/02 13:28:26  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.5  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/


#include "CustomCoordinateSequenceExample.h"
#include <sstream>
#include <iostream>

using namespace geos;

CustomPointCoordinateSequence::CustomPointCoordinateSequence(point_3d *newPts,int newSize) {
	pts=newPts;
	size=newSize;
}

CustomPointCoordinateSequence::CustomPointCoordinateSequence(const CustomPointCoordinateSequence &c) {
	pts=c.pts;
	size=c.size;
}

void CustomPointCoordinateSequence::setPoints(const vector<Coordinate> &v) {
	if(v.size()==size) {
		point_3d pt;
		for(int i=0;i<(int)v.size(); i++) {
			pt.x=v[i].x;
			pt.y=v[i].y;
			pt.z=v[i].z;
			pts[i]=pt;
		}
	} else {
		throw  CPCLException("size mismatch\n");
	}
}

void CustomPointCoordinateSequence::setPoints(const vector<point_3d> &v) {
	if(v.size()==size) {
		for(int i=0;i<(int)v.size(); i++) {
			pts[i]=v[i];
		}
	} else {
		throw  CPCLException("size mismatch\n");
	}
}

vector<Coordinate>* CustomPointCoordinateSequence::toVector() {
	vector<Coordinate>* v=new vector<Coordinate>();
	for(int i=0;i<size;i++) {
		v->push_back(*(new Coordinate(pts[i].x,pts[i].y,pts[i].z)));
	}
	return v;
}

vector<point_3d>* CustomPointCoordinateSequence::toPointVector() {
	vector<point_3d>* v=new vector<point_3d>();
	for(int i=0;i<size;i++) {
		v->push_back(pts[i]);
	}
	return v;
}

bool CustomPointCoordinateSequence::isEmpty() {
	return size==0;
}

void CustomPointCoordinateSequence::add(Coordinate& c){
	throw  CPCLException("list's size can't be modified\n");
}

void CustomPointCoordinateSequence::add(point_3d p){
	throw  CPCLException("list's size can't be modified\n");
}

int CustomPointCoordinateSequence::getSize(){
	return size;
}

Coordinate& CustomPointCoordinateSequence::getAt(int pos){
	point_3d pt;
	if (pos>=0 && pos<size) {
		pt=pts[pos];
		return *(new Coordinate(pt.x,pt.y,pt.z));
	} else
		throw  CPCLException("can't retrieve element\n");
}

point_3d CustomPointCoordinateSequence::getPointAt(int pos){
	if (pos>=0 && pos<size) {
		return pts[pos];
	} else
		throw  CPCLException("can't retrieve element\n");
}

void CustomPointCoordinateSequence::setAt(Coordinate& c, int pos){
	point_3d pt={c.x,c.y,c.z};
	if (pos>=0 && pos<size) 
		pts[pos]=pt;
	else
		throw  CPCLException("can't change element\n");
}

void CustomPointCoordinateSequence::setAt(point_3d p, int pos){
	if (pos>=0 && pos<size) 
		pts[pos]=p;
	else
		throw  CPCLException("can't change element\n");
}

void CustomPointCoordinateSequence::deleteAt(int pos){
	throw  CPCLException("list's size can't be modified\n");
}

string CustomPointCoordinateSequence::toString() {
	ostringstream s;
	//string result("");
	//char buffer[100];
	for (int i=0;i<size;i++) {
		point_3d c=pts[i];
		//sprintf(buffer,"(%g,%g,%g) ",c.x,c.y,c.z);
		s<<"("<<c.x<<","<<c.y<<","<<c.z<<") ";
		//result.append(buffer);
	}
	//result.append("");
	//return result;
	return s.str();
}

