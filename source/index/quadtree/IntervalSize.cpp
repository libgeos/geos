#include "../../headers/indexQuadtree.h"

bool IntervalSize::isZeroWidth(double min,double max){
	double width=max-min;
	if (width==0.0) return true;
	double maxAbs=__max(fabs(min),fabs(max));
	double scaledInterval=width/maxAbs;
	int level=DoubleBits::exponent(scaledInterval);
	return level<=MIN_BINARY_EXPONENT;
}