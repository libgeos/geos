#include "geom.h"
#include "time.h"
#include "bigtest.h"

#include "io.h"
/**
 * Run relate between two large geometries to test the performance
 * of the sweepline intersection detection algorithm
*/

void run(int nPts, GeometryFactory *fact) {
	clock_t startTime, endTime;
	double size=100.0;
	double armLen=50.0;
	int nArms=10;
	Polygon *poly=GeometryTestFactory::createSineStar(fact,0.0,0.0,size,armLen,nArms,nPts);
	Polygon *box=GeometryTestFactory::createBox(fact,0,0,1,100.0);

	startTime=clock();
    poly->intersects(box);
    endTime=clock();
	double totalTime=(double)(endTime-startTime);
	printf( "n Pts: %i  Executed in %6.0f ms.\n",nPts,totalTime);
//	cout << "n Pts: " << nPts << "   Executed in " << totalTime << endl;
}

void main(int argC, char* argV[]) {
	GeometryFactory *fact=new GeometryFactory();

	run(10,fact);
	run(100,fact);
	run(1000,fact);
	run(2000,fact);
	run(3000,fact);
	run(4000,fact);
	run(5000,fact);
	run(6000,fact);
	run(7000,fact);
	run(8000,fact);
	run(9000,fact);
	run(10000,fact);

	cout << "Done" << endl;
}