#include "indexSweepline.h"
#include "stdio.h"

SweepLineInterval::SweepLineInterval(double newMin, double newMax) {
	SweepLineInterval(newMin,newMax,NULL);
}

SweepLineInterval::SweepLineInterval(double newMin, double newMax, void* newItem){
	min=newMin<newMax?newMin:newMax;
	max=newMax>newMin?newMax:newMin;
	item=newItem;
}

double SweepLineInterval::getMin() {
	return min;
}
double SweepLineInterval::getMax() {
	return max;
}
void* SweepLineInterval::getItem() {
	return item;
}
