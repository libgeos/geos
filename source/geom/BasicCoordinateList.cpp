/*
* $Log$
* Revision 1.12  2003/10/15 11:23:00  strk
* Formalized const nature of toVector() method and of first argument to static removeRepeatedPoints().
*
* Revision 1.11  2003/10/15 10:17:36  strk
* Made setPoints() get a const vector<Coordinate>.
*
*/
#include "../headers/geom.h"
#include "stdio.h"

namespace geos {

BasicCoordinateList::BasicCoordinateList(const CoordinateList *c){
	vect=new vector<Coordinate>();
	int size=c->getSize();
	for(int i=0; i<size; i++) {
		vect->push_back(c->getAt(i));
	}
}

BasicCoordinateList::BasicCoordinateList() {
	vect=new vector<Coordinate>();
}

BasicCoordinateList::BasicCoordinateList(int n) {
	vect=new vector<Coordinate>();
//	vect->reserve(n);
	vect->resize(n);
}

BasicCoordinateList::BasicCoordinateList(const Coordinate& c) {
	vect=new vector<Coordinate>(1,c);
}

BasicCoordinateList::BasicCoordinateList(const BasicCoordinateList &c) {
	vect=new vector<Coordinate>(*(c.vect));
}

void BasicCoordinateList::setPoints(const vector<Coordinate> &v) {
	//vect->swap(v);
	delete vect;
	vect=new vector<Coordinate>(v);
}

vector<Coordinate>* BasicCoordinateList::toVector() const {
	return new vector<Coordinate>(vect->begin(),vect->end());
}

bool BasicCoordinateList::isEmpty() const {
	return vect->empty();
}

void BasicCoordinateList::add(const Coordinate& c){
	vect->push_back(c);
}

int BasicCoordinateList::getSize() const {
	return (int) vect->size();
}

const Coordinate& BasicCoordinateList::getAt(int pos) const {
//	if (pos>=0 && pos<=vect->size()-1) 
		return (*vect)[pos];
//	else
//		throw "BasicCoordinateList exception: can't retrieve element\n";
}

void BasicCoordinateList::setAt(const Coordinate& c, int pos){
//	if (pos>=0 && pos<=vect->size()-1) 
		(*vect)[pos]=c;
//	else
//		throw "BasicCoordinateList exception: can't change element\n";
}
void BasicCoordinateList::deleteAt(int pos){
//	if (pos>=0 && pos<=vect->size()-1) 
		vect->erase(vect->begin()+pos);
//	else
//		throw "BasicCoordinateList exception: can't remove element\n";
}

string BasicCoordinateList::toString() {
	string result("");
	char buffer[100];
	for (unsigned int i=0; i<vect->size(); i++) {
		Coordinate& c=(*vect)[i];
		sprintf(buffer,"(%g,%g,%g) ",c.x,c.y,c.z);
		result.append(buffer);
	}
	result.append("");
	return result;
}

BasicCoordinateList::~BasicCoordinateList() {
	delete vect;
}
}
