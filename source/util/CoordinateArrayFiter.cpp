#include "../headers/util.h"
#include "../headers/geom.h"

namespace geos {
/**
*  Constructs a <code>CoordinateArrayFilter</code>.
*
*@param  size  the number of points that the <code>CoordinateArrayFilter</code>
*      will collect
*/
CoordinateArrayFilter::CoordinateArrayFilter(int size) {
	n=0;
	pts=CoordinateListFactory::internalFactory->createCoordinateList(size);
}

/**
*  Returns the gathered <code>Coordinate</code>s.
*
*@return    the <code>Coordinate</code>s collected by this <code>CoordinateArrayFilter</code>
*/
CoordinateList* CoordinateArrayFilter::getCoordinates() {
	return pts;
}

void CoordinateArrayFilter::filter(Coordinate &coord) {
	pts->setAt(coord,n++);
}

}

