#include "../../headers/indexStrtree.h"
#include "../../headers/util.h"
#include "stdio.h"
#include <typeinfo>

namespace geos {

Interval::Interval(Interval *other) {
	Interval(other->imin,other->imax);
}

Interval::Interval(double newMin,double newMax) {
	Assert::isTrue(newMin<=newMax);
	imin=newMin;
	imax=newMax;
}

double Interval::getCentre() {
	return (imin+imax)/2;
}

Interval* Interval::expandToInclude(Interval *other) {
	imax=max(imax,other->imax);
	imin=min(imin,other->imin);
	return this;
}

bool Interval::intersects(Interval *other) {
	return !(other->imin>imax || other->imax<imin);
}

bool Interval::equals(void *o) {
	if (typeid(o)!=typeid(Interval)) {
		return false;
	}
	Interval *other=(Interval*) o;
	return imin==other->imin && imax==other->imax;
}
}