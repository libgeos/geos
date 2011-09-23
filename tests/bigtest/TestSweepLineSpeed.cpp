/**********************************************************************
 * $Id: TestSweepLineSpeed.cpp 2163 2008-08-18 16:32:50Z mloskot $
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
 * Revision 1.2  2006/06/12 16:57:51  strk
 * Removed unused parameter warnings
 *
 * Revision 1.1  2006/01/31 19:07:35  strk
 * - Renamed DefaultCoordinateSequence to CoordinateArraySequence.
 * - Moved GetNumGeometries() and GetGeometryN() interfaces
 *   from GeometryCollection to Geometry class.
 * - Added getAt(int pos, Coordinate &to) funtion to CoordinateSequence class.
 * - Reworked automake scripts to produce a static lib for each subdir and
 *   then link all subsystem's libs togheter
 * - Moved C-API in it's own top-level dir capi/
 * - Moved source/bigtest and source/test to tests/bigtest and test/xmltester
 * - Fixed PointLocator handling of LinearRings
 * - Changed CoordinateArrayFilter to reduce memory copies
 * - Changed UniqueCoordinateArrayFilter to reduce memory copies
 * - Added CGAlgorithms::isPointInRing() version working with
 *   Coordinate::ConstVect type (faster!)
 * - Ported JTS-1.7 version of ConvexHull with big attention to
 *   memory usage optimizations.
 * - Improved XMLTester output and user interface
 * - geos::geom::util namespace used for geom/util stuff
 * - Improved memory use in geos::geom::util::PolygonExtractor
 * - New ShortCircuitedGeometryVisitor class
 * - New operation/predicate package
 *
 * Revision 1.13  2004/07/19 10:35:23  strk
 * bigtest.h moved to local dir
 *
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
