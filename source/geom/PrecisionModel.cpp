#include "../headers/geom.h"
#include "stdio.h"

const double maximumPreciseValue=9007199254740992.0;

double PrecisionModel::makePrecise(double val){
	//return rint(val);
	return ((val >= 0.0) ? floor(val+0.5) : - floor(-val+0.5));
    /*
     * Other options:
     * - Math.floor(a + 0.5d);
     * - Math.floor(a);
     * -  (val >= 0.0) ? Math.floor(val) : - Math.floor(-val);
     */
}

PrecisionModel::PrecisionModel(){
	modelType=FLOATING;
	scale=0.0;
	offsetX=0.0;
	offsetY=0.0;
}
PrecisionModel::PrecisionModel(double newScale, double newOffsetX, double newOffsetY) {
	modelType = FIXED;
	scale = newScale;
	offsetX = newOffsetX;
	offsetY = newOffsetY;
}

PrecisionModel::PrecisionModel(const PrecisionModel &pm) {
	modelType = pm.modelType;
	scale = pm.scale;
	offsetX = pm.offsetX;
	offsetY = pm.offsetY;
}

bool PrecisionModel::isFloating(){
	return modelType == FLOATING;
}

double PrecisionModel::getScale(){
	return scale;
}

double PrecisionModel::getOffsetX(){
	return offsetX;
}

double PrecisionModel::getOffsetY() {
	return offsetY;
}

void PrecisionModel::toInternal (Coordinate& external, Coordinate* internal) {
	if (isFloating()) {
		internal->x = external.x;
		internal->y = external.y;
	} else {
		internal->x = makePrecise((external.x - offsetX)*scale);
		internal->y = makePrecise((external.y - offsetY)*scale);
	}
	internal->z = external.z;
}

Coordinate* PrecisionModel::toInternal(Coordinate& external) {
	Coordinate* internal=new Coordinate();
	toInternal(external, internal);
	return internal;
}

Coordinate* PrecisionModel::toExternal(Coordinate& internal) {
	Coordinate* external=new Coordinate();
	toExternal(internal, external);
	return external;
}

void PrecisionModel::toExternal(Coordinate& internal, Coordinate* external) {
	if (isFloating()) {
		external->x = internal.x;
		external->y = internal.y;
	}else {
		external->x = (internal.x / scale) + offsetX;
		external->y = (internal.y / scale) + offsetY;
	}
	external->z = internal.z;
}
  
string PrecisionModel::toString() {
	string result("");
	char buffer[255];
    if (isFloating()) {
		result="Floating";
	}
    else {
		sprintf(buffer,"Fixed (Scale=%g, Offset X=%g, Offset Y=%g)",scale,offsetX,offsetY);
		result.append(buffer);
		result.append("");
    }
	return result;
}

/**
*  Sets <code>p1</code> to the value of external point p0
*  rounded to this precision model.
*
*@param  p0  the original coordinate
*@param  p1  the coordinate whose values will be changed to the
*      external representation of <code>internal</code>
*/
void PrecisionModel::round(Coordinate& p0,Coordinate& p1) {
	toInternal(p0,&p1);
	toExternal(p1,&p1);
}


PrecisionModel::~PrecisionModel(){}

bool operator==(PrecisionModel a, PrecisionModel b) {
	return a.isFloating() == b.isFloating() &&
			a.getOffsetX() == b.getOffsetX() &&
			a.getOffsetY() == b.getOffsetY() &&
			a.getScale() == b.getScale();
}
