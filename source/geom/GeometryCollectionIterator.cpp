#include "geom.h"
#include <typeinfo>

GeometryCollectionIterator::GeometryCollectionIterator(){
	parent=NULL;
	subcollectionIterator=NULL;
	atStart=true;
    index=0;
    max=0;
}
GeometryCollectionIterator::GeometryCollectionIterator(const GeometryCollectionIterator &gci) {
	subcollectionIterator=gci.subcollectionIterator;
	parent=gci.parent;
	atStart=gci.atStart;
	index=gci.index;
	max=gci.max;
}
GeometryCollectionIterator::GeometryCollectionIterator(GeometryCollection *newParent){
	parent=newParent;
	subcollectionIterator=NULL;
    atStart=true;
    index=0;
    max=newParent->getNumGeometries();
}

bool GeometryCollectionIterator::hasNext() {
	if (atStart) {
		return true;
	}
	if (subcollectionIterator!=NULL) {
		if (subcollectionIterator->hasNext()) {
			return true;
		}
		subcollectionIterator=NULL;
	}
	if (index>=max) {
		return false;
	}
	return true;
}

Geometry *GeometryCollectionIterator::next() {
	// the parent GeometryCollection is the first object returned
	if (atStart) {
		atStart=false;
		return parent;
	}
	if (subcollectionIterator!=NULL) {
		if (subcollectionIterator->hasNext()) {
			return subcollectionIterator->next();
		} else {
			subcollectionIterator=NULL;
		}
	}
	if (index>=max) {
		throw "NoSuchElementException";
	}
	Geometry *obj=parent->getGeometryN(index++);
	if ((typeid(*obj)==typeid(GeometryCollection)) ||
			   (typeid(*obj)==typeid(MultiPoint)) ||
			   (typeid(*obj)==typeid(MultiLineString)) ||
			   (typeid(*obj)==typeid(MultiPolygon))) {
		subcollectionIterator=new GeometryCollectionIterator((GeometryCollection *)obj);
		// there will always be at least one element in the sub-collection
		return subcollectionIterator->next();
	}
	return obj;
}

/**
 *  Not implemented.
 *
 *@throws  UnsupportedOperationException  This method is not implemented.
 */
void GeometryCollectionIterator::remove() {
	throw "UnsupportedOperationException\n";
}

GeometryCollectionIterator::~GeometryCollectionIterator(){
	delete subcollectionIterator;
}
