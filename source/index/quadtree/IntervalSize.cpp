#include "../../headers/indexQuadtree.h"

namespace geos {

bool IntervalSize::isZeroWidth(double mn,double mx){
	double width=mx-mn;
	if (width==0.0) return true;
	double maxAbs=max(fabs(mn),fabs(mx));
	double scaledInterval=width/maxAbs;
	int level=DoubleBits::exponent(scaledInterval);
	return level<=MIN_BINARY_EXPONENT;
}
}