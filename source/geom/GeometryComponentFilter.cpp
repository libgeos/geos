#include "../headers/geom.h"

namespace geos {

void GeometryComponentFilter::filter(Geometry *geom) {
	geom->geometryChangedAction();
}
}