#ifndef GEOS_EX2_H
#define GEOS_EX2_H

#include "../headers/platform.h"
#include "../headers/geom.h"

using namespace std;

/*
 * This is an example of how you can create a custom CoordinateList class that wraps
 * your own way of storing lists of coordinates. Depending on your internal storage
 * format some methods in the wrapper class might not work properly (but have to be
 * preserved for the interface compatibility. In this example CustomPointCoordinateList
 * wraps an array of point_3d. Since the array is fixed length, methods like 'add' or
 * 'deleteAt' will not work.
 */
class CustomPointCoordinateList : public CoordinateList {
public:
	CustomPointCoordinateList(point_3d *newPts,int newSize);
	CustomPointCoordinateList(const CustomPointCoordinateList &cl);
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


#endif

