/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 * $Log$
 * Revision 1.12  2004/07/02 13:28:26  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.11  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/


//#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
//#include <crtdbg.h>

#include <stdio.h>
#include <time.h>

#include <geos/geom.h>
#include <geos/bigtest.h>
#include <geos/io.h>

using namespace geos;
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
//	Polygon *box=GeometryTestFactory::createBox(fact,0,0,1,100.0);

	startTime=clock();
    poly->intersects(box);
    endTime=clock();
	double totalTime=(double)(endTime-startTime);
	printf( "n Pts: %i  Executed in %6.0f ms.\n",nPts,totalTime);
//	cout << "n Pts: " << nPts << "   Executed in " << totalTime << endl;
}

int main(int argC, char* argV[]) {

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

	return 0;
}
