#include "../headers/geom.h"

void GeometryComponentFilter::filter(Geometry *geom) {
	geom->geometryChangedAction();
}
