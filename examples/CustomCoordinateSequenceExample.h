/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 **********************************************************************/


#ifndef GEOS_EX2_H
#define GEOS_EX2_H

#include <geos/platform.h>
#include <geos/geom.h>
#include <geos/util.h>

using namespace std;
using namespace geos;

/*
 * This is an example of how you can create a custom CoordinateSequence class that wraps
 * your own way of storing lists of coordinates. Depending on your internal storage
 * format some methods in the wrapper class might not work properly (but have to be
 * preserved for the interface compatibility. In this example CustomPointCoordinateSequence
 * wraps an array of point_3d. Since the array is fixed length, methods like 'add' or
 * 'deleteAt' will not work.
 */
class CustomPointCoordinateSequence : public BasicCoordinateSequence {
public:
	CustomPointCoordinateSequence(point_3d *newPts,int newSize);
	CustomPointCoordinateSequence(const CustomPointCoordinateSequence &cl);
	bool isEmpty();
	void add(Coordinate& c); //NoOp (exception)
	void add(point_3d p); //NoOp (exception)
	int getSize();
	Coordinate& getAt(int pos);
	point_3d getPointAt(int pos);
	void setAt(Coordinate& c, int pos);
	void setAt(point_3d p, int pos);
	void deleteAt(int pos); //NoOp (exception)
	vector<Coordinate>* toVector();
	vector<point_3d>* toPointVector();
	string toString();
	void setPoints(const vector<Coordinate> &v);
	void setPoints(const vector<point_3d> &v);
private:
	point_3d *pts;
	int size;
};

class CPCLException: public GEOSException {
public:
	CPCLException();
	CPCLException(const string& msg);
	~CPCLException();
};

#endif

