#include "geom.h"
#include "stdio.h"

BasicCoordinateList::BasicCoordinateList() {
	vect=new vector<Coordinate>();
	current=-1;
}

BasicCoordinateList::BasicCoordinateList(int n) {
	vect=new vector<Coordinate>();
	vect->reserve(n);
	current=-1;
}

BasicCoordinateList::BasicCoordinateList(Coordinate c) {
	vect=new vector<Coordinate>(1,c);
	current=-1;
}

BasicCoordinateList::BasicCoordinateList(const BasicCoordinateList &c) {
	vect=new vector<Coordinate>(*(c.vect));
//	vect=c.vect->c;
	current=c.current;
}

void BasicCoordinateList::setPoints(const vector<Coordinate> &v) {
	vect=new vector<Coordinate>(v);
}

void BasicCoordinateList::reset(){
	current=-1;
}

vector<Coordinate> BasicCoordinateList::toVector() {
	return vector<Coordinate>(vect->begin(),vect->end());
}

bool BasicCoordinateList::isEmpty() {
	return vect->empty();
}

Coordinate& BasicCoordinateList::getNext(){
	if (current==vect->size()-1) current=-1;
	current++;
	return (*vect)[current];
}

bool BasicCoordinateList::hasNext(){
	if (current>=vect->size()-1) {
		return false;
	} else {
		return true;
	}
}

void BasicCoordinateList::add(Coordinate c){
	vect->push_back(c);
}

int BasicCoordinateList::getSize(){
	return (int) vect->size();
}

void BasicCoordinateList::moveTo(int pos){
	if (pos>vect->size()-1) 
		throw "Invalid argument: out of bounds\n" ;
	else
		current=pos;
}

Coordinate& BasicCoordinateList::getAt(int pos){
	if (pos>vect->size()-1) 
		throw "Invalid argument: out of bounds\n" ;
	else
		current=pos;
	if (current>=0 && current<=vect->size()-1) 
		return (*vect)[current];
	else
		throw "BasicCoordinateList exception: can't retrieve element\n";
}

void BasicCoordinateList::setAt(Coordinate c, int pos){
	if (pos>vect->size()-1) 
		throw "Invalid argument: out of bounds\n" ;
	else
		current=pos;
	if (current>=0 && current<=vect->size()-1) 
		(*vect)[current]=c;
	else
		throw "BasicCoordinateList exception: can't change element\n";
}
void BasicCoordinateList::deleteAt(int pos){
	if (pos>vect->size()-1) 
		throw "Invalid argument: out of bounds\n" ;
	else
		current=pos;
	remove();
}

Coordinate& BasicCoordinateList::get(){
	if (current>=0 && current<=vect->size()-1) 
		return (*vect)[current];
	else
		throw "BasicCoordinateList exception: can't retrieve element\n";
}

void BasicCoordinateList::set(Coordinate c){
	if (current>=0 && current<=vect->size()-1) 
		(*vect)[current]=c;
	else
		throw "BasicCoordinateList exception: can't change element\n";
}

void BasicCoordinateList::remove(){
	if (current>=0 && current<=vect->size()-1) 
		vect->erase(vect->begin()+current);
	else
		throw "BasicCoordinateList exception: can't remove element\n";
}

string BasicCoordinateList::toString() {
	string result("");
	char buffer[100];
	for (unsigned int i=0; i<vect->size(); i++) {
		Coordinate c=(*vect)[i];
		sprintf(buffer,"(%g,%g,%g) ",c.x,c.y,c.z);
		result.append(buffer);
	}
	result.append("");
	return result;
}

void BasicCoordinateList::swap(vector<Coordinate> v) {
	vect->swap(v);
}

BasicCoordinateList::~BasicCoordinateList() {
	delete vect;
}
