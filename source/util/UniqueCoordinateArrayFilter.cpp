#include "../headers/util.h"
#include "../headers/geom.h"

namespace geos {
UniqueCoordinateArrayFilter::UniqueCoordinateArrayFilter() {
	list=CoordinateListFactory::internalFactory->createCoordinateList();
}
UniqueCoordinateArrayFilter::~UniqueCoordinateArrayFilter() {
	delete list;
}
/**
*  Returns the gathered <code>Coordinate</code>s.
*
*@return    the <code>Coordinate</code>s collected by this <code>CoordinateArrayFilter</code>
*/
const CoordinateList* UniqueCoordinateArrayFilter::getCoordinates() const {
	return list;
}

void UniqueCoordinateArrayFilter::filter_ro(const Coordinate &coord) {
	if (CoordinateList::indexOf(&coord,list)==-1) {
		list->add(coord);
	}
}

void UniqueCoordinateArrayFilter::filter_rw(Coordinate &coord) {
	throw new UnsupportedOperationException("UniqueCoordinateArrayFilter is a read-only filter");
}

}

