#include "../headers/geom.h"
#include "stdio.h"

namespace geos {

bool CoordinateList::hasRepeatedPoints(){
	int size=(int) getSize();
	for(int i=1; i<size; i++) {
		if (getAt(i-1)==getAt(i)) {
			return true;
		}
	}
	return false;
}

bool CoordinateList::hasRepeatedPoints(CoordinateList *cl){
	int size=(int) cl->getSize();
	for(int i=1;i<size; i++) {
		if (cl->getAt(i-1)==cl->getAt(i)) {
			return true;
		}
	}
	return false;
}

Coordinate* CoordinateList::minCoordinate(){
	Coordinate* minCoord=NULL;
	int size=(int) getSize();
	for(int i=0; i<size; i++) {
		if(minCoord==NULL || minCoord->compareTo(getAt(i))>0) {
			minCoord=&getAt(i);
		}
	}
	return minCoord;
}

Coordinate* CoordinateList::minCoordinate(CoordinateList *cl){
	Coordinate* minCoord=NULL;
	int size=(int) cl->getSize();
	for(int i=0;i<size; i++) {
		if(minCoord==NULL || minCoord->compareTo(cl->getAt(i))>0) {
			minCoord=&(cl->getAt(i));
		}
	}
	return minCoord;
}

int CoordinateList::indexOf(Coordinate *coordinate,CoordinateList *cl) {
	for (int i=0; i<cl->getSize(); i++) {
		if ((*coordinate)==cl->getAt(i)) {
			return i;
		}
	}
	return -1;
}

void CoordinateList::scroll(CoordinateList* cl,Coordinate* firstCoordinate) {
	int ind=indexOf(firstCoordinate,cl);
	if (ind<0) return;
	int length=cl->getSize();
	vector<Coordinate> v(length);
	for (int i=ind; i<length; i++) {
		v[i-ind]=cl->getAt(i);
	}
	for (int j=0; j<ind; j++) {
		v[length-ind+j]=cl->getAt(j);
	}
	cl->setPoints(v);
}

void CoordinateList::reverse(CoordinateList *cl){
	int last=cl->getSize()-1;
	int mid=last/2;
	for(int i=0;i<=mid;i++) {
		Coordinate& tmp=cl->getAt(i);
		cl->setAt(cl->getAt(last-i),i);
		cl->setAt(tmp,last-i);
	}
}

bool CoordinateList::equals(CoordinateList *cl1,CoordinateList *cl2){
	if (cl1==cl2) return true;
	if (cl1==NULL||cl2==NULL) return false;
	if (cl1->getSize()!=cl2->getSize()) return false;
	for (int i = 0; i<cl1->getSize(); i++) {
		if (!(cl1->getAt(i)==cl2->getAt(i))) return false;
	}
	return true;
}

void CoordinateList::add(vector<Coordinate>* vc,bool allowRepeated) {
	for(int i=0;i<(int)vc->size();i++) {
		add((*vc)[i],allowRepeated);
	}
}

void CoordinateList::add(Coordinate& c,bool allowRepeated) {
	if (!allowRepeated) {
		if (getSize()>=1) {
			Coordinate& last=getAt(getSize()-1);
			if (last==c) return;
		}
	}
	add(c);
}

CoordinateList* CoordinateList::removeRepeatedPoints(CoordinateList *cl){
	CoordinateList* ret=CoordinateListFactory::internalFactory->createCoordinateList();
	vector<Coordinate> *v=cl->toVector();
	ret->add(v,false);
	delete v;
	v=ret->toVector();
	cl->setPoints(*(v));
	delete v;
	delete ret;
//	return ret;
	return cl;
}
}