#include "../headers/geom.h"
#include "../headers/util.h"
#include "stdio.h"

const double maximumPreciseValue=9007199254740992.0;

//double PrecisionModel::makePrecise(double val){
//	//return rint(val);
//	return ((val >= 0.0) ? floor(val+0.5) : - floor(-val+0.5));
//    /*
//     * Other options:
//     * - Math.floor(a + 0.5d);
//     * - Math.floor(a);
//     * -  (val >= 0.0) ? Math.floor(val) : - Math.floor(-val);
//     */
//}

/**
* Rounds an numeric value to the PrecisionModel grid.
*/
double PrecisionModel::makePrecise(double val) {
	double v=val*scale;
	if (val>=0.0) 
		v=floor(v+0.5);
	else 
		v=-floor(-v+0.5);
	return v/scale;
}

/**
* Rounds a Coordinate to the PrecisionModel grid.
*/
void PrecisionModel::makePrecise(Coordinate *coord) {
	if (modelType==FLOATING) return;
	coord->x=makePrecise(coord->x);
	coord->y=makePrecise(coord->y);
}


PrecisionModel::PrecisionModel(){
	modelType=FLOATING;
	scale=0.0;
	offsetX=0.0;
	offsetY=0.0;
}

/**
*  Creates a <code>PrecisionModel</code> that specifies Fixed precision.
*  Fixed-precision coordinates are represented as precise internal coordinates,
*  which are rounded to the grid defined by the scale factor.
*
*@param  scale    amount by which to multiply a coordinate after subtracting
*      the offset, to obtain a precise coordinate
*@param  offsetX  not used.
*@param  offsetY  not used.
*
* @deprecated
*/
PrecisionModel::PrecisionModel(double newScale, double newOffsetX, double newOffsetY) {
	modelType = FIXED;
	setScale(newScale);
	offsetX = newOffsetX;
	offsetY = newOffsetY;
}

/**
*  Creates a <code>PrecisionModel</code> that specifies Fixed precision.
*  Fixed-precision coordinates are represented as precise internal coordinates,
*  which are rounded to the grid defined by the scale factor.
*
*@param  scale    amount by which to multiply a coordinate after subtracting
*      the offset, to obtain a precise coordinate
*/
PrecisionModel::PrecisionModel(double newScale) {
	modelType=FIXED;
	setScale(scale);
	offsetX=0;
	offsetY=0;
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

/**
*  Returns the multiplying factor used to obtain a precise coordinate.
* This method is private because PrecisionModel is intended to
* be an immutable (value) type.
*
*@return    the amount by which to multiply a coordinate after subtracting
*      the offset
*/
double PrecisionModel::getScale(){
	return scale;
}

/**
*  Sets the multiplying factor used to obtain a precise coordinate.
* This method is private because PrecisionModel is intended to
* be an immutable (value) type.
*
*/
void PrecisionModel::setScale(double newScale) {
	scale=fabs(scale);
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
		internal->x=makePrecise(external.x);
		internal->y=makePrecise(external.y);
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
	external->x = internal.x;
	external->y = internal.y;
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

PrecisionModel::~PrecisionModel(){}

bool operator==(PrecisionModel a, PrecisionModel b) {
	return a.isFloating() == b.isFloating() &&
			a.getOffsetX() == b.getOffsetX() &&
			a.getOffsetY() == b.getOffsetY() &&
			a.getScale() == b.getScale();
}

/**
*  Compares this {@link PrecisionModel} object with the specified object for order.
* A PrecisionModel is greater than another if it provides greater precision.
*
*@param  o  the <code>PrecisionModel</code> with which this <code>PrecisionModel</code>
*      is being compared
*@return    a negative integer, zero, or a positive integer as this <code>PrecisionModel</code>
*      is less than, equal to, or greater than the specified <code>PrecisionModel</code>
*/
int PrecisionModel::compareTo(void* o) {
	PrecisionModel *other=(PrecisionModel*) o;
	if (modelType==FLOATING && other->modelType==FLOATING) return 0;
	if (modelType==FLOATING && other->modelType!=FLOATING) return 1;
	if (modelType!=FLOATING && other->modelType==FLOATING) return -1;
	if (modelType==FIXED && other->modelType==FIXED) {
		if (scale>other->scale)
			return 1;
		else if (scale<other->scale)
			return -1;
		else
			return 0;
	}
	Assert::shouldNeverReachHere("Unknown Precision Model type encountered");
	return 0;
}