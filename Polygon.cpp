#include "geom.h"

Polygon::Polygon(){}
Polygon::Polygon(const Polygon &p){}
Polygon::Polygon(LinearRing shell, PrecisionModel precisionModel, int SRID){
	Polygon(&shell, vector<LinearRing>(), precisionModel, SRID);
}

Polygon::Polygon(LinearRing *newShell, vector<LinearRing> newHoles,
				 PrecisionModel precisionModel, int SRID):
				Geometry(precisionModel, SRID) {
/*	if (newShell==NULL) {
		LinearRing newShell(CoordinateList(), precisionModel, SRID);
	}
	if (newHoles==NULL) {
		newHoles=new LinearRing[];
	}
	if (hasNullElements(newHoles)) {
		throw "IllegalArgumentException: holes must not contain null elements";
	}
	if (newShell->isEmpty() && hasNonEmptyElements(newHoles)) {
		throw "IllegalArgumentException: shell is empty but holes are not";
	}
	LinearRing shell(*newShell);
	holes=newHoles;
*/
}


Polygon::~Polygon(){}

Geometry Polygon::getBoundary(){
	return Geometry();
}

