#include "geom.h"
#include "stdio.h"

CoordinateList::CoordinateList() {
	vect=new vector<Coordinate>();
	current=-1;
}

CoordinateList::CoordinateList(int n) {
	vect=new vector<Coordinate>();
	vect->reserve(n);
	current=-1;
}

CoordinateList::CoordinateList(Coordinate c) {
	vect=new vector<Coordinate>(1,c);
	current=-1;
}

CoordinateList::CoordinateList(const CoordinateList &c) {
	vect=new vector<Coordinate>(*(c.vect));
//	vect=c.vect->c;
	current=c.current;
}

void CoordinateList::setPoints(const vector<Coordinate> &v) {
	vect=new vector<Coordinate>(v);
}

void CoordinateList::reset(){
	current=-1;
}

vector<Coordinate> CoordinateList::toVector() {
	return vector<Coordinate>(vect->begin(),vect->end());
}

bool CoordinateList::isEmpty() {
	return vect->empty();
}

Coordinate& CoordinateList::getNext(){
	if (current==vect->size()-1) current=-1;
	current++;
	return (*vect)[current];
}

bool CoordinateList::hasNext(){
	if (current>=vect->size()-1) {
		return false;
	} else {
		return true;
	}
}

void CoordinateList::add(Coordinate c){
	vect->push_back(c);
}

int CoordinateList::getSize(){
	return (int) vect->size();
}

void CoordinateList::moveTo(int pos){
	if (pos>vect->size()-1) 
		throw "Invalid argument: out of bounds\n" ;
	else
		current=pos;
}

Coordinate& CoordinateList::getAt(int pos){
	if (pos>vect->size()-1) 
		throw "Invalid argument: out of bounds\n" ;
	else
		current=pos;
	if (current>=0 && current<=vect->size()-1) 
		return (*vect)[current];
	else
		throw "CoordinateList exception: can't retrieve element\n";
}

void CoordinateList::setAt(Coordinate c, int pos){
	if (pos>vect->size()-1) 
		throw "Invalid argument: out of bounds\n" ;
	else
		current=pos;
	if (current>=0 && current<=vect->size()-1) 
		(*vect)[current]=c;
	else
		throw "CoordinateList exception: can't change element\n";
}
void CoordinateList::deleteAt(int pos){
	if (pos>vect->size()-1) 
		throw "Invalid argument: out of bounds\n" ;
	else
		current=pos;
	remove();
}

Coordinate& CoordinateList::get(){
	if (current>=0 && current<=vect->size()-1) 
		return (*vect)[current];
	else
		throw "CoordinateList exception: can't retrieve element\n";
}

void CoordinateList::set(Coordinate c){
	if (current>=0 && current<=vect->size()-1) 
		(*vect)[current]=c;
	else
		throw "CoordinateList exception: can't change element\n";
}

void CoordinateList::remove(){
	if (current>=0 && current<=vect->size()-1) 
		vect->erase(vect->begin()+current);
	else
		throw "CoordinateList exception: can't remove element\n";
}

string CoordinateList::toString() {
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

void CoordinateList::swap(vector<Coordinate> v) {
	vect->swap(v);
}

CoordinateList::~CoordinateList() {
	delete vect;
}
