#include "geom.h"
#include "time.h"
#include "bigtest.h"

#include "io.h"
/**
 * Run relate between two large geometries to test the performance
 * of the sweepline intersection detection algorithm
*/

void run(int nPts, GeometryFactory *fact) {
	time_t startTime, endTime;
	double size=100.0;
	double armLen=50.0;
	int nArms=10;
	Polygon *poly=GeometryTestFactory::createSineStar(fact,0.0,0.0,size,armLen,nArms,nPts);
	Polygon *box=GeometryTestFactory::createBox(fact,0,0,1,100.0);

	time(&startTime);
    poly->intersects(box);
    time(&endTime);
	double totalTime=difftime(endTime,startTime)*1000;
//    String totalTimeStr = totalTime < 10000 ? totalTime + " ms" : (double) totalTime / 1000.0 + " s";
	cout << "n Pts: " << nPts << "   Executed in " << totalTime << endl;
}

void main(int argC, char* argV[]) {
	GeometryFactory *fact=new GeometryFactory();

	run(10,fact);
	run(10,fact);
	run(100,fact);
	run(1000,fact);
	run(10000,fact);
	run(10000,fact);
}