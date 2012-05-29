/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 **********************************************************************/


//#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
//#include <crtdbg.h>

#include <stdio.h>
#include <time.h>

#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Polygon.h>
#include "bigtest.h"

using namespace geos::geom;

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
	Polygon *box=GeometryTestFactory::createSineStar(fact,0.0,size/2,size,armLen,nArms,nPts); 
    //Polygon *box=GeometryTestFactory::createBox(fact,0,0,1,100.0);

	startTime=clock();
    poly->intersects(box);
    endTime=clock();
	double totalTime=(double)(endTime-startTime);
	printf( "n Pts: %i  Executed in %6.0f ms.\n",nPts,totalTime);
    //cout << "n Pts: " << nPts << "   Executed in " << totalTime << endl;
    
    // FIXME - mloskot: Why generated test geometries are not destroyed?"
}

int main(int /* argc */, char** /* argv[] */) {

	GeometryFactory *fact=new GeometryFactory();

	run(1000,fact);
	run(2000,fact);
	run(4000,fact);
	run(8000,fact);
	run(16000,fact);
	run(32000,fact);
	run(64000,fact);
	run(128000,fact);
	run(256000,fact);
	run(512000,fact);
	run(1024000,fact);

//	_CrtDumpMemoryLeaks();

	cout << "Done" << endl;

    // FIXME - mloskot: Who's gonna to eat the 'fact'? Mr. Leak!

	return 0;
}
