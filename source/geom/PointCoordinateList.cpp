/*
* $Log$
* Revision 1.15  2003/10/15 11:23:00  strk
* Formalized const nature of toVector() method and of first argument to static removeRepeatedPoints().
*
* Revision 1.14  2003/10/15 10:17:36  strk
* Made setPoints() get a const vector<Coordinate>.
*
*/
#include "../headers/geom.h"
#include "stdio.h"

namespace geos {

PointCoordinateList::PointCoordinateList(const CoordinateList *c){
	vect=new vector<point_3d>();
	point_3d pt;
	int size=c->getSize();
	for(int i=0; i<size; i++) {
		pt.x=c->getAt(i).x;
		pt.y=c->getAt(i).y;
		pt.z=c->getAt(i).z;
		vect->push_back(pt);
	}
}

PointCoordinateList::PointCoordinateList() {
	vect=new vector<point_3d>();
}

PointCoordinateList::PointCoordinateList(int n) {
	vect=new vector<point_3d>();
//	vect->reserve(n);
	vect->resize(n);
}

PointCoordinateList::PointCoordinateList(const Coordinate& c) {
	point_3d pt={c.x,c.y,c.z};
	vect=new vector<point_3d>(1,pt);
}

PointCoordinateList::PointCoordinateList(const PointCoordinateList &c) {
	vect=new vector<point_3d>(*(c.vect));
}

void PointCoordinateList::setPoints(const vector<Coordinate> &v) {
	delete vect;
	vect=new vector<point_3d>();
	point_3d pt;
	for(unsigned int i=0; i<v.size(); i++) {
		pt.x=v[i].x;
		pt.y=v[i].y;
		pt.z=v[i].z;
		vect->push_back(pt);
	}
}

void PointCoordinateList::setPoints(vector<point_3d> &v) {
	vect=new vector<point_3d>(v);
}

vector<Coordinate>* PointCoordinateList::toVector() const {
	vector<Coordinate>* v=new vector<Coordinate>();
	for(unsigned int i=0; i<vect->size(); i++) {
		v->push_back(*(new Coordinate((*vect)[i].x,(*vect)[i].y,(*vect)[i].z)));
	}
	return v;
}

vector<point_3d>* PointCoordinateList::toPointVector() {
	return vect;
}

bool PointCoordinateList::isEmpty() const {
	return vect->empty();
}

void PointCoordinateList::add(const Coordinate& c){
	point_3d pt={c.x,c.y,c.z};
	vect->push_back(pt);
}

void PointCoordinateList::add(point_3d p){
	vect->push_back(p);
}

int PointCoordinateList::getSize() const {
	return (int) vect->size();
}

const Coordinate& PointCoordinateList::getAt(int pos) const {
	point_3d pt;
//	if (pos>=0 && pos<=vect->size()-1) {
		pt=(*vect)[pos];
		return *(new Coordinate(pt.x,pt.y,pt.z));
//	} else
//		throw "PointCoordinateList exception: can't retrieve element\n";
}

point_3d PointCoordinateList::getPointAt(int pos){
//	if (pos>=0 && pos<=vect->size()-1) {
		return (*vect)[pos];
//	} else
//		throw "PointCoordinateList exception: can't retrieve element\n";
}

void PointCoordinateList::setAt(const Coordinate& c, int pos){
	point_3d pt={c.x,c.y,c.z};
//	if (pos>=0 && pos<=vect->size()-1) 
		(*vect)[pos]=pt;
//	else
//		throw "PointCoordinateList exception: can't change element\n";
}

void PointCoordinateList::setAt(point_3d p, int pos){
//	if (pos>=0 && pos<=vect->size()-1) 
		(*vect)[pos]=p;
//	else
//		throw "PointCoordinateList exception: can't change element\n";
}

void PointCoordinateList::deleteAt(int pos){
//	if (pos>=0 && pos<=vect->size()-1) 
		vect->erase(vect->begin()+pos);
//	else
//		throw "PointCoordinateList exception: can't remove element\n";
}

string PointCoordinateList::toString() {
	string result("");
	char buffer[100];
	for (unsigned int i=0; i<vect->size(); i++) {
		point_3d c=(*vect)[i];
		sprintf(buffer,"(%g,%g,%g) ",c.x,c.y,c.z);
		result.append(buffer);
	}
	result.append("");
	return result;
}

PointCoordinateList::~PointCoordinateList() {
	delete vect;
}
}

