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
 * Revision 1.8  2004/03/17 02:00:33  ybychkov
 * "Algorithm" upgraded to JTS 1.4
 *
 * Revision 1.7  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 * Revision 1.6  2003/10/11 01:56:08  strk
 *
 * Code base padded with 'const' keywords ;)
 *
 *
 **********************************************************************/

#include "../headers/geosAlgorithm.h"
#include "../headers/platform.h"
#include "../headers/util.h"
#include <typeinfo>

namespace geos {

 /**
  * Create a new convex hull construction for the input {@link Geometry}.
  */
ConvexHull::ConvexHull(const Geometry *newGeometry) {
	geometry=newGeometry;
	pointLocator=new PointLocator();
}

ConvexHull::~ConvexHull() {
	delete pointLocator;
}


/**
* Returns a {@link Geometry} that represents the convex hull of the input
* geometry.
* The geometry will contain the minimal number of points needed to
* represent the convex hull.  In particular, no more than two consecutive
* points will be collinear.
*
* @return if the convex hull contains 3 or more points, a {@link Polygon};
* 2 points, a {@link LineString};
* 1 point, a {@link Point};
* 0 points, an empty {@link GeometryCollection}.
*/
Geometry* ConvexHull::getConvexHull() {
	factory=geometry->getFactory();
	UniqueCoordinateArrayFilter *filter=new UniqueCoordinateArrayFilter();
	geometry->apply_ro(filter);

	// Work with a copy of filtered coordinates
	CoordinateList *pts =
		CoordinateListFactory::internalFactory->createCoordinateList(
				filter->getCoordinates());

	if (pts->getSize()==0) {
		Geometry *g=factory->createGeometryCollection(NULL);
		delete pts;
		delete filter;
		return g;
	}
	if (pts->getSize()==1) {
		Geometry *g=factory->createPoint(pts->getAt(0));
		delete pts;
		delete filter;
		return g;
	}
	if (pts->getSize()==2) {
		Geometry *g=factory->createLineString(pts);
		delete pts;
		delete filter;
		return g;
	}
	// sort points for Graham scan.
	CoordinateList *cH;
	if (pts->getSize()>10) {
		//Probably should be somewhere between 50 and 100?
		CoordinateList *rpts=reduce(pts);
		// Use Graham scan to find convex hull.
		cH=grahamScan(preSort(rpts));
		delete filter;
		delete rpts; 
	} else {
		// Use Graham scan to find convex hull.
		cH=grahamScan(preSort(pts));
		delete filter;
	}
	// Convert array to linear ring.
	Geometry *g=lineOrPolygon(cH);
	delete pts;
	delete cH;
	return g;
}

// Always return a new CoordinateList --strk
CoordinateList* ConvexHull::reduce(const CoordinateList *pts) {
	auto_ptr<BigQuad> bigQuad(makeBigQuad(pts));
	// Build a linear ring defining a big poly.
	CoordinateList *bigPoly=CoordinateListFactory::internalFactory->createCoordinateList();
	bigPoly->add(bigQuad->westmost);
	if (CoordinateList::indexOf(&(bigQuad->northmost),bigPoly)==-1) {
		bigPoly->add(bigQuad->northmost);
	}
	if (CoordinateList::indexOf(&(bigQuad->eastmost),bigPoly)==-1) {
		bigPoly->add(bigQuad->eastmost);
	}
	if (CoordinateList::indexOf(&(bigQuad->southmost),bigPoly)==-1) {
		bigPoly->add(bigQuad->southmost);
	}
	if (bigPoly->getSize()<3) {
		delete bigPoly;
		return CoordinateListFactory::internalFactory->createCoordinateList(pts);
	}
	bigPoly->add(bigQuad->westmost);
	LinearRing *bQ=factory->createLinearRing(bigPoly);
	// load an array with all points not in the big poly
	// and the defining points.

//!!!Note to self: this might not work properly because of sorting.

	CoordinateList *cl=CoordinateListFactory::internalFactory->createCoordinateList(bigPoly);
	delete bigPoly;
	for(int i=0;i<pts->getSize();i++) {
		if (pointLocator->locate(pts->getAt(i),bQ)==Location::EXTERIOR) {
			cl->add(pts->getAt(i));
		}
	}
	// Return this array as the reduced problem.
	delete bQ;
	return cl;
}

CoordinateList* ConvexHull::preSort(CoordinateList *pts) {
	Coordinate t;
	// find the lowest point in the set. If two or more points have
	// the same minimum y coordinate choose the one with the minimu x.
	// This focal point is put in array location pts[0].
	for(int i=1;i<pts->getSize();i++) {
		if ((pts->getAt(i).y<pts->getAt(0).y) || ((pts->getAt(i).y==pts->getAt(0).y) && (pts->getAt(i).x<pts->getAt(0).x))) {
			t=pts->getAt(0);
			pts->setAt(pts->getAt(i),0);
			pts->setAt(t,i);
		}
	}
	// sort the points radially around the focal point.
	radialSort(pts);
	return pts;
}

// returns a newly allocated CoordinateList object
CoordinateList* ConvexHull::grahamScan(const CoordinateList *c) {
	Coordinate p;
	Coordinate p1;
	Coordinate p2;
	vector<Coordinate> *ps=new vector<Coordinate>();
	ps->push_back(c->getAt(0));
	ps->push_back(c->getAt(1));
	ps->push_back(c->getAt(2));
	p=c->getAt(2);
	for(int i=3;i<c->getSize();i++) {
		p=ps->back();
		ps->pop_back();
		while (CGAlgorithms::computeOrientation(ps->back(),p,c->getAt(i))>0) {
			p=ps->back();
			ps->pop_back();
		}
		ps->push_back(p);
		ps->push_back(c->getAt(i));
		p=c->getAt(i);
	}
	ps->push_back(c->getAt(0));
	p=c->getAt(0);
	CoordinateList *cl=CoordinateListFactory::internalFactory->createCoordinateList();
	cl->setPoints(*ps);
	delete ps;
	return cl;
}

void ConvexHull::radialSort(CoordinateList *p) {
	// A selection sort routine, assumes the pivot point is
	// the first point (i.e., p[0]).
	Coordinate t;
	for(int i=1;i<(p->getSize()-1);i++) {
		int min=i;
		for(int j=i+1;j<p->getSize();j++) {
			if (polarCompare(p->getAt(0),p->getAt(j),p->getAt(min))<0) {
				min=j;
			}
		}
		t=p->getAt(i);
		p->setAt(p->getAt(min),i);
		p->setAt(t,min);
	}
}

int ConvexHull::polarCompare(Coordinate o, Coordinate p, Coordinate q) {
	// Given two points p and q compare them with respect to their radial
	// ordering about point o. -1, 0 or 1 depending on whether p is less than,
	// equal to or greater than q. First checks radial ordering then if both
	// points lie on the same line, check distance to o.
	double dxp=p.x-o.x;
	double dyp=p.y-o.y;
	double dxq=q.x-o.x;
	double dyq=q.y-o.y;
	double alph=atan2(dxp, dyp);
	double beta=atan2(dxq, dyq);
	if (alph<beta) {
		return -1;
	}
	if (alph>beta) {
		return 1;
	}
	double op=dxp*dxp+dyp*dyp;
	double oq=dxq*dxq+dyq*dyq;
	if (op<oq) {
		return -1;
	}
	if (op>oq) {
		return 1;
	}
	return 0;
}

/**
*@return    whether the three coordinates are collinear and c2 lies between
*      c1 and c3 inclusive
*/
bool ConvexHull::isBetween(Coordinate c1, Coordinate c2, Coordinate c3) {
	if (CGAlgorithms::computeOrientation(c1, c2, c3)!=0) {
		return false;
	}
	if (c1.x!=c3.x) {
		if (c1.x<=c2.x && c2.x<=c3.x) {
			return true;
		}
		if (c3.x<=c2.x && c2.x<=c1.x) {
			return true;
		}
	}
	if (c1.y!=c3.y) {
		if (c1.y<=c2.y && c2.y<=c3.y) {
			return true;
		}
		if (c3.y<=c2.y && c2.y<=c1.y) {
			return true;
		}
	}
	return false;
}

BigQuad* ConvexHull::makeBigQuad(const CoordinateList *pts) {
	BigQuad *bigQuad=new BigQuad();
	bigQuad->northmost=pts->getAt(0);
	bigQuad->southmost=pts->getAt(0);
	bigQuad->westmost=pts->getAt(0);
	bigQuad->eastmost=pts->getAt(0);
	for (int i=1;i<pts->getSize();i++) {
		if (pts->getAt(i).x<bigQuad->westmost.x) {
			bigQuad->westmost=pts->getAt(i);
		}
		if (pts->getAt(i).x>bigQuad->eastmost.x) {
			bigQuad->eastmost=pts->getAt(i);
		}
		if (pts->getAt(i).y<bigQuad->southmost.y) {
			bigQuad->southmost=pts->getAt(i);
		}
		if (pts->getAt(i).y>bigQuad->northmost.y) {
			bigQuad->northmost=pts->getAt(i);
		}
	}
	return bigQuad;
}

/**
*@param  vertices  the vertices of a linear ring, which may or may not be
*      flattened (i.e. vertices collinear)
*@return           a 2-vertex <code>LineString</code> if the vertices are
*      collinear;otherwise, a <code>Polygon</code> with unnecessary
*      (collinear) vertices removed
*/
Geometry* ConvexHull::lineOrPolygon(CoordinateList *newCoordinates) {
	CoordinateList *coordinates=cleanRing(newCoordinates);
	if (coordinates->getSize()==3) { // shouldn't this be 2 ??
		CoordinateList *cl1=CoordinateListFactory::internalFactory->createCoordinateList();
		cl1->add(coordinates->getAt(0));
		cl1->add(coordinates->getAt(1));
		delete coordinates;
		LineString *ret =factory->createLineString(cl1);
		delete cl1;
		return ret;
	}
	LinearRing *linearRing=new LinearRing(coordinates,geometry->getPrecisionModel(),geometry->getSRID());
	delete coordinates;
	return factory->createPolygon(linearRing,NULL);
}

/**
*@param  vertices  the vertices of a linear ring, which may or may not be
*      flattened (i.e. vertices collinear)
*@return           the coordinates with unnecessary (collinear) vertices
*      removed
*/
CoordinateList* ConvexHull::cleanRing(CoordinateList *original) {
	Assert::equals(original->getAt(0),original->getAt(original->getSize()-1));
	CoordinateList *cleanedRing=CoordinateListFactory::internalFactory->createCoordinateList();
	Coordinate previousDistinctCoordinate;
	Coordinate currentCoordinate;
	Coordinate nextCoordinate;
	previousDistinctCoordinate.setNull();
	for(int i=0;i<=original->getSize()-2;i++) {
		currentCoordinate=original->getAt(i);
		nextCoordinate=original->getAt(i+1);
		if (currentCoordinate==nextCoordinate) {
			continue;
		}
		if (!(previousDistinctCoordinate==Coordinate::getNull()) && 
			isBetween(previousDistinctCoordinate,currentCoordinate, nextCoordinate)) {
			continue;
		}
		cleanedRing->add(currentCoordinate);
		previousDistinctCoordinate=currentCoordinate;
	}
	cleanedRing->add(original->getAt(original->getSize()-1));
	return cleanedRing;
}

}

