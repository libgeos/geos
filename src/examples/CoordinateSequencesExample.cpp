/**********************************************************************
 * WARNING! This example is obsoleted, read doc/example.cpp for
 * an updated example.
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
 * Revision 1.3  2006/02/09 15:52:47  strk
 * GEOSException derived from std::exception; always thrown and cought by const ref.
 *
 * Revision 1.2  2006/01/31 19:07:33  strk
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
 * Revision 1.1  2004/07/08 19:41:27  strk
 * renamed to reflect JTS API.
 *
 * Revision 1.5  2004/07/02 13:28:26  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.4  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/


#include <iostream>

#include <geos/io.h>
#include <geos/util.h>
#include <geos/geom.h>
#include <geos/geosAlgorithm.h>

using namespace std;
using namespace geos;

int main(int argc, char** argv)
{
	try {
	cout << "Start:" << endl << endl;

	CoordinateSequence *cl1=new CoordinateArraySequence();

	//CoordinateSequence cl1 is empty
	cout << endl << "CoordinateSequence cl1: " << cl1->toString() << endl;
	//Adding points
	cl1->add(*(new Coordinate(140,120)));
	cl1->add(*(new Coordinate(160,20)));
	cl1->add(*(new Coordinate(33,33)));
	cl1->add(*(new Coordinate(20,20)));
	cl1->add(*(new Coordinate(11,11)));
	cl1->add(*(new Coordinate(140,120)));
	cout << "CoordinateSequence cl1: " << cl1->toString() << endl;
	//Changing point
	cl1->setAt(*(new Coordinate(20,120)),4);
	cout << "CoordinateSequence cl1: " << cl1->toString() << endl;
	//Deleting point
	cl1->deleteAt(2);
	cout << "CoordinateSequence cl1: " << cl1->toString() << endl;

	//Switching CoordinateSequenceFactory to create PointCoordinateSequences
	//PointCoordinateSequence is a sample implementation of a user-defined
	//CoordinateSequence based on a vector of struct {3 x double}
	CoordinateSequenceFactory::internalFactory=new PointCoordinateSequenceFactory();
	//Now calls to CoordinateSequenceFactory would create PointCoordinateSequences
	CoordinateSequence *cl2=CoordinateSequenceFactory::internalFactory->createCoordinateSequence();
	//It is also possible to explicitly create PointCoordinateSequences
	//CoordinateSequence *cl2=new PointCoordinateSequence();

	//CoordinateSequence cl2 is empty
	cout << endl << "CoordinateSequence cl2: " << cl2->toString() << endl;
	//Adding points
	//Points could be added as Coordinates
	cl2->add(*(new Coordinate(11,11)));
	cl2->add(*(new Coordinate(140,200)));
	cl2->add(*(new Coordinate(33,33)));
	//or using native PointCoordinateSequence format
	point_3d p1={240,200,DoubleNotANumber};
	point_3d p2={55,55,DoubleNotANumber};
	point_3d p3={140,120,DoubleNotANumber};
	((PointCoordinateSequence*) cl2)->add(p1);
	((PointCoordinateSequence*) cl2)->add(p2);
	((PointCoordinateSequence*) cl2)->add(p3);
	cout << "CoordinateSequence cl2: " << cl2->toString() << endl;
	//Changing point
	//Points can be set using Coordinates
	cl2->setAt(*(new Coordinate(240,120)),4);
	//or using native PointCoordinateSequence format
	point_3d pn={140,120,DoubleNotANumber};
	((PointCoordinateSequence*) cl2)->setAt(pn,0);
	cout << "CoordinateSequence cl2: " << cl2->toString() << endl;
	//Deleting point
	cl2->deleteAt(2);
	cout << "CoordinateSequence cl2: " << cl2->toString() << endl;

	//To do the operations:
	//First we need to create a GeometryFactory
	GeometryFactory *gf=new GeometryFactory(new PrecisionModel(),0);
	//Now we can create Geometries
	Geometry *geom1=gf->createPolygon(gf->createLinearRing(cl1),NULL);
	cout << endl << "Geometry 1:" << endl << geom1->toString() << endl;
	Geometry *geom2=gf->createPolygon(gf->createLinearRing(cl2),NULL);
	cout << endl << "Geometry 2:" << endl << geom2->toString() << endl;
	//And see how they relate to each other
	IntersectionMatrix *im=geom1->relate(geom2);
	cout << endl << "Result of relate() operation should be \"FF2F01212\"" << endl;
	cout << "Result is: " << im->toString() << endl;

	cout << "End" << endl;
	} catch (const GEOSException& ge) {
		cout << ge->toString() << endl;
	}

	return 0;
}

