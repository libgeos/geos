#include "geom.h"
#include "stdio.h"

BasicCoordinateList::BasicCoordinateList() {
	vect=new vector<Coordinate>();
}

BasicCoordinateList::BasicCoordinateList(int n) {
	vect=new vector<Coordinate>();
//	vect->reserve(n);
	vect->resize(n);
}

BasicCoordinateList::BasicCoordinateList(Coordinate& c) {
	vect=new vector<Coordinate>(1,c);
}

BasicCoordinateList::BasicCoordinateList(const BasicCoordinateList &c) {
	vect=new vector<Coordinate>(*(c.vect));
}

void BasicCoordinateList::setPoints(const vector<Coordinate> &v) {
	vect=new vector<Coordinate>(v);
}

vector<Coordinate>* BasicCoordinateList::toVector() {
	return new vector<Coordinate>(vect->begin(),vect->end());
}

bool BasicCoordinateList::isEmpty() {
	return vect->empty();
}

void BasicCoordinateList::add(Coordinate& c){
	vect->push_back(c);
}

int BasicCoordinateList::getSize(){
	return (int) vect->size();
}

Coordinate& BasicCoordinateList::getAt(int pos){
//	if (pos>=0 && pos<=vect->size()-1) 
		return (*vect)[pos];
//	else
//		throw "BasicCoordinateList exception: can't retrieve element\n";
}

void BasicCoordinateList::setAt(Coordinate& c, int pos){
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
