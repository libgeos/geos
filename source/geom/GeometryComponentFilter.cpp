#include "../headers/geom.h"
#include "../headers/util.h"

namespace geos {

void GeometryComponentFilter::filter_rw(Geometry *geom) {
	geom->geometryChangedAction();
}

void GeometryComponentFilter::filter_ro(const Geometry *geom) {
	throw new UnsupportedOperationException("GeometryComponentFilter only sets a geometry-changed flag, cannot be read-only\n");
}



}

