#include "../../headers/indexStrtree.h"
#include "../../headers/util.h"
#include "stdio.h"
#include <typeinfo>

Interval::Interval(Interval *other) {
	Interval(other->min,other->max);
}

Interval::Interval(double newMin,double newMax) {
	Assert::isTrue(newMin<=newMax);
	min=newMin;
	max=newMax;
}

double Interval::getCentre() {
	return (min+max)/2;
}

Interval* Interval::expandToInclude(Interval *other) {
	max=max(max,other->max);
	min=min(min,other->min);
	return this;
}

bool Interval::intersects(Interval *other) {
	return !(other->min>max || other->max<min);
}

bool Interval::equals(void *o) {
	if (typeid(o)!=typeid(Interval)) {
		return false;
	}
	Interval *other=(Interval*) o;
	return min==other->min && max==other->max;
}
