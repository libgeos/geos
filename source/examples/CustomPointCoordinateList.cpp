#include "CustomCoordinateListExample.h"
#include "stdio.h"

using namespace geos;

CustomPointCoordinateList::CustomPointCoordinateList(point_3d *newPts,int newSize) {
	pts=newPts;
	size=newSize;
}

CustomPointCoordinateList::CustomPointCoordinateList(const CustomPointCoordinateList &c) {
	pts=c.pts;
	size=c.size;
}

void CustomPointCoordinateList::setPoints(const vector<Coordinate> &v) {
	if(v.size()==size) {
		point_3d pt;
		for(int i=0;i<(int)v.size(); i++) {
			pt.x=v[i].x;
			pt.y=v[i].y;
			pt.z=v[i].z;
			pts[i]=pt;
		}
	} else {
		throw new CPCLException("size mismatch\n");
	}
}

void CustomPointCoordinateList::setPoints(const vector<point_3d> &v) {
	if(v.size()==size) {
		for(int i=0;i<(int)v.size(); i++) {
			pts[i]=v[i];
		}
	} else {
		throw new CPCLException("size mismatch\n");
	}
}

vector<Coordinate>* CustomPointCoordinateList::toVector() {
	vector<Coordinate>* v=new vector<Coordinate>();
	for(int i=0;i<size;i++) {
		v->push_back(*(new Coordinate(pts[i].x,pts[i].y,pts[i].z)));
	}
	return v;
}

vector<point_3d>* CustomPointCoordinateList::toPointVector() {
	vector<point_3d>* v=new vector<point_3d>();
	for(int i=0;i<size;i++) {
		v->push_back(pts[i]);
	}
	return v;
}

bool CustomPointCoordinateList::isEmpty() {
	return size==0;
}

void CustomPointCoordinateList::add(Coordinate& c){
	throw new CPCLException("list's size can't be modified\n");
}

void CustomPointCoordinateList::add(point_3d p){
	throw new CPCLException("list's size can't be modified\n");
}

int CustomPointCoordinateList::getSize(){
	return size;
}

Coordinate& CustomPointCoordinateList::getAt(int pos){
	point_3d pt;
	if (pos>=0 && pos<size) {
		pt=pts[pos];
		return *(new Coordinate(pt.x,pt.y,pt.z));
	} else
		throw new CPCLException("can't retrieve element\n");
}

point_3d CustomPointCoordinateList::getPointAt(int pos){
	if (pos>=0 && pos<size) {
		return pts[pos];
	} else
		throw new CPCLException("can't retrieve element\n");
}

void CustomPointCoordinateList::setAt(Coordinate& c, int pos){
	point_3d pt={c.x,c.y,c.z};
	if (pos>=0 && pos<size) 
		pts[pos]=pt;
	else
		throw new CPCLException("can't change element\n");
}

void CustomPointCoordinateList::setAt(point_3d p, int pos){
	if (pos>=0 && pos<size) 
		pts[pos]=p;
	else
		throw new CPCLException("can't change element\n");
}

void CustomPointCoordinateList::deleteAt(int pos){
	throw new CPCLException("list's size can't be modified\n");
}

string CustomPointCoordinateList::toString() {
	string result("");
	char buffer[100];
	for (int i=0;i<size;i++) {
		point_3d c=pts[i];
		sprintf(buffer,"(%g,%g,%g) ",c.x,c.y,c.z);
		result.append(buffer);
	}
	result.append("");
	return result;
}

