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
 * Revision 1.6  2004/07/08 19:34:49  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added DefaultCoordinateSequenceFactory::instance() function.
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

	//Creating first CoordinateList
	//By default CoordinateListFactory will create a BasicCoordinateList
	CoordinateList *cl1=CoordinateListFactory::internalFactory->createCoordinateList();
	//It is also possible to explicitly create BasicCoordinateLists
	//CoordinateList *cl1=new BasicCoordinateList();

	//CoordinateList cl1 is empty
	cout << endl << "CoordinateList cl1: " << cl1->toString() << endl;
	//Adding points
	cl1->add(*(new Coordinate(140,120)));
	cl1->add(*(new Coordinate(160,20)));
	cl1->add(*(new Coordinate(33,33)));
	cl1->add(*(new Coordinate(20,20)));
	cl1->add(*(new Coordinate(11,11)));
	cl1->add(*(new Coordinate(140,120)));
	cout << "CoordinateList cl1: " << cl1->toString() << endl;
	//Changing point
	cl1->setAt(*(new Coordinate(20,120)),4);
	cout << "CoordinateList cl1: " << cl1->toString() << endl;
	//Deleting point
	cl1->deleteAt(2);
	cout << "CoordinateList cl1: " << cl1->toString() << endl;

	//Switching CoordinateListFactory to create PointCoordinateLists
	//PointCoordinateList is a sample implementation of a user-defined
	//CoordinateList based on a vector of struct {3 x double}
	CoordinateListFactory::internalFactory=new PointCoordinateListFactory();
	//Now calls to CoordinateListFactory would create PointCoordinateLists
	CoordinateList *cl2=CoordinateListFactory::internalFactory->createCoordinateList();
	//It is also possible to explicitly create PointCoordinateLists
	//CoordinateList *cl2=new PointCoordinateList();

	//CoordinateList cl2 is empty
	cout << endl << "CoordinateList cl2: " << cl2->toString() << endl;
	//Adding points
	//Points could be added as Coordinates
	cl2->add(*(new Coordinate(11,11)));
	cl2->add(*(new Coordinate(140,200)));
	cl2->add(*(new Coordinate(33,33)));
	//or using native PointCoordinateList format
	point_3d p1={240,200,DoubleNotANumber};
	point_3d p2={55,55,DoubleNotANumber};
	point_3d p3={140,120,DoubleNotANumber};
	((PointCoordinateList*) cl2)->add(p1);
	((PointCoordinateList*) cl2)->add(p2);
	((PointCoordinateList*) cl2)->add(p3);
	cout << "CoordinateList cl2: " << cl2->toString() << endl;
	//Changing point
	//Points can be set using Coordinates
	cl2->setAt(*(new Coordinate(240,120)),4);
	//or using native PointCoordinateList format
	point_3d pn={140,120,DoubleNotANumber};
	((PointCoordinateList*) cl2)->setAt(pn,0);
	cout << "CoordinateList cl2: " << cl2->toString() << endl;
	//Deleting point
	cl2->deleteAt(2);
	cout << "CoordinateList cl2: " << cl2->toString() << endl;

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
	} catch (GEOSException *ge) {
		cout << ge->toString() << endl;
	}

	return 0;
}

