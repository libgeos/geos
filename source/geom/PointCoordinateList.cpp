#include "geom.h"
#include "stdio.h"

PointCoordinateList::PointCoordinateList() {
	vect=new vector<point_3d>();
}

PointCoordinateList::PointCoordinateList(int n) {
	vect=new vector<point_3d>();
	vect->reserve(n);
}

PointCoordinateList::PointCoordinateList(Coordinate c) {
	point_3d pt={c.x,c.y,c.z};
	vect=new vector<point_3d>(1,pt);
}

PointCoordinateList::PointCoordinateList(const PointCoordinateList &c) {
	vect=new vector<point_3d>(*(c.vect));
}

void PointCoordinateList::setPoints(const vector<Coordinate> &v) {
	vect=new vector<point_3d>();
	point_3d pt;
	for(unsigned int i=0; i<v.size(); i++) {
		pt.x=v[i].x;
		pt.y=v[i].y;
		pt.z=v[i].z;
		vect->push_back(pt);
	}
}

vector<Coordinate>* PointCoordinateList::toVector() {
	vector<Coordinate>* v=new vector<Coordinate>();
	for(unsigned int i=0; i<vect->size(); i++) {
		v->push_back(Coordinate((*vect)[i].x,(*vect)[i].y,(*vect)[i].z));
	}
	return v;
}

bool PointCoordinateList::isEmpty() {
	return vect->empty();
}

void PointCoordinateList::add(Coordinate c){
	point_3d pt={c.x,c.y,c.z};
	vect->push_back(pt);
}

int PointCoordinateList::getSize(){
	return (int) vect->size();
}

Coordinate& PointCoordinateList::getAt(int pos){
	point_3d pt;
	if (pos>=0 && pos<=vect->size()-1) {
		pt=(*vect)[pos];
		return *(new Coordinate(pt.x,pt.y,pt.z));
	} else
		throw "PointCoordinateList exception: can't retrieve element\n";
}

void PointCoordinateList::setAt(Coordinate c, int pos){
	point_3d pt={c.x,c.y,c.z};
	if (pos>=0 && pos<=vect->size()-1) 
		(*vect)[pos]=pt;
	else
		throw "PointCoordinateList exception: can't change element\n";
}
void PointCoordinateList::deleteAt(int pos){
	if (pos>=0 && pos<=vect->size()-1) 
		vect->erase(vect->begin()+pos);
	else
		throw "PointCoordinateList exception: can't remove element\n";
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
