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
 * Revision 1.5  2004/10/13 10:03:02  strk
 * Added missing linemerge and polygonize operation.
 * Bug fixes and leaks removal from the newly added modules and
 * planargraph (used by them).
 * Some comments and indentation changes.
 *
 * Revision 1.4  2004/07/08 19:34:50  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added DefaultCoordinateSequenceFactory::instance() function.
 *
 * Revision 1.3  2004/07/02 13:28:29  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.2  2004/07/01 14:12:44  strk
 *
 * Geometry constructors come now in two flavors:
 * 	- deep-copy args (pass-by-reference)
 * 	- take-ownership of args (pass-by-pointer)
 * Same functionality is available through GeometryFactory,
 * including buildGeometry().
 *
 * Revision 1.1  2004/04/08 04:53:56  ybychkov
 * "operation/polygonize" ported from JTS 1.4
 *
 *
 **********************************************************************/


#include <geos/opPolygonize.h>

namespace geos {

/**
* Find the innermost enclosing shell polygonizeEdgeRing containing the argument polygonizeEdgeRing, if any.
* The innermost enclosing ring is the <i>smallest</i> enclosing ring.
* The algorithm used depends on the fact that:
* <br>
*  ring A contains ring B iff envelope(ring A) contains envelope(ring B)
* <br>
* This routine is only safe to use if the chosen point of the hole
* is known to be properly contained in a shell
* (which is guaranteed to be the case if the hole does not touch its shell)
*
* @return containing polygonizeEdgeRing, if there is one
* @return null if no containing polygonizeEdgeRing is found
*/
polygonizeEdgeRing* polygonizeEdgeRing::findEdgeRingContaining(polygonizeEdgeRing *testEr,vector<polygonizeEdgeRing*> *shellList){
	LinearRing *testRing=testEr->getRing();
	const Envelope *testEnv=testRing->getEnvelopeInternal();
	Coordinate testPt=testRing->getCoordinateN(0);
	polygonizeEdgeRing *minShell=NULL;
	const Envelope *minEnv=NULL;
	for(int i=0;i<(int)shellList->size();i++) {
		polygonizeEdgeRing *tryShell=(*shellList)[i];
		LinearRing *tryRing=tryShell->getRing();
		const Envelope *tryEnv=tryRing->getEnvelopeInternal();
		if (minShell!=NULL) minEnv=minShell->getRing()->getEnvelopeInternal();
		bool isContained=false;
		// the hole envelope cannot equal the shell envelope
		if (tryEnv==testEnv)
			continue;
		testPt=ptNotInList(testRing->getCoordinates(),tryRing->getCoordinates());
		if (tryEnv->contains(testEnv)
			&& cga.isPointInRing(testPt,tryRing->getCoordinates()))
				isContained=true;
		// check if this new containing ring is smaller than the current minimum ring
		if (isContained) {
			if (minShell==NULL || minEnv->contains(tryEnv)) {
				minShell=tryShell;
			}
		}
	}
	return minShell;
}

/**
* Finds a point in a list of points which is not contained in another list of points
* @param testPts the {@link Coordinate}s to test
* @param pts an array of {@link Coordinate}s to test the input points against
* @return a {@link Coordinate} from <code>testPts</code> which is not in <code>pts</code>, '
* or <code>null</code>
*/
const Coordinate&
polygonizeEdgeRing::ptNotInList(CoordinateSequence *testPts,CoordinateSequence *pts)
{
	for (int i=0; i<testPts->getSize(); i++) {
		const Coordinate& testPt=testPts->getAt(i);
		if (isInList(testPt,pts))
			return testPt;
	}
	return Coordinate::nullCoord;
}

/**
* Tests whether a given point is in an array of points.
* Uses a value-based test.
*
* @param pt a {@link Coordinate} for the test point
* @param pts an array of {@link Coordinate}s to test
* @return <code>true</code> if the point is in the array
*/
bool polygonizeEdgeRing::isInList(const Coordinate& pt, const CoordinateSequence *pts){
	for (int i=0; i < pts->getSize(); i++) {
		if (pt==pts->getAt(i))
			return false;
	}
	return true;
}

//CGAlgorithms* polygonizeEdgeRing::cga=new CGAlgorithms();

polygonizeEdgeRing::polygonizeEdgeRing(const GeometryFactory *newFactory) {
	deList=new vector<planarDirectedEdge*>();
	// cache the following data for efficiency
	ring=NULL;
	ringPts=NULL;
	holes=NULL;
	factory=newFactory;
}

polygonizeEdgeRing::~polygonizeEdgeRing() {
	delete deList;
//	ring=NULL;
//	ringPts=NULL;
	delete holes;
}

/**
* Adds a {@link DirectedEdge} which is known to form part of this ring.
* @param de the {@link DirectedEdge} to add.
*/
void polygonizeEdgeRing::add(planarDirectedEdge *de){
	deList->push_back(de);
}

/**
* Tests whether this ring is a hole.
* Due to the way the edges in the polyongization graph are linked,
* a ring is a hole if it is oriented counter-clockwise.
* @return <code>true</code> if this ring is a hole
*/
bool polygonizeEdgeRing::isHole(){
	LinearRing *ring=getRing();
	return cga.isCCW(ring->getCoordinates());
}

/**
* Adds a hole to the polygon formed by this ring.
* @param hole the {@link LinearRing} forming the hole.
*/
void polygonizeEdgeRing::addHole(LinearRing *hole) {
	if (holes==NULL)
		holes=new vector<LinearRing*>();
	holes->push_back(hole);
}

/**
* Computes the {@link Polygon formed by this ring and any contained holes.
*
* @return the {@link Polygon} formed by this ring and its holes.
*/
Polygon* polygonizeEdgeRing::getPolygon() {
	vector<Geometry*> *holeLR=NULL;
	if (holes!=NULL) {
		holeLR=new vector<Geometry*>(holes->size());
		for (int i=0; i < (int)holes->size(); i++) {
			(*holeLR)[i]=(*holes)[i];
		}
	}
	Polygon *poly=factory->createPolygon(ring, holeLR);
	return poly;
}

/**
* Tests if the {@link LinearRing} ring formed by this edge ring is topologically valid.
* @return
*/
bool polygonizeEdgeRing::isValid() {
	getCoordinates();
	if (ringPts->getSize() <= 3) return false;
	getRing();
	return ring->isValid();
}

/**
* Computes the list of coordinates which are contained in this ring.
* The coordinatea are computed once only and cached.
*
* @return an array of the {@link Coordinate}s in this ring
*/
CoordinateSequence*
polygonizeEdgeRing::getCoordinates() {
	if (ringPts==NULL) {
		ringPts=factory->getCoordinateSequenceFactory()->create(NULL);
		for (int i=0;i<(int)deList->size();i++) {
			planarDirectedEdge *de=(*deList)[i];
			PolygonizeEdge *edge=(PolygonizeEdge*) de->getEdge();
			addEdge(edge->getLine()->getCoordinates(), de->getEdgeDirection(), ringPts);
		}
	}
	return ringPts;
}

/**
* Gets the coordinates for this ring as a {@link LineString}.
* Used to return the coordinates in this ring
* as a valid geometry, when it has been detected that the ring is topologically
* invalid.
* @return a {@link LineString} containing the coordinates in this ring
*/
LineString* polygonizeEdgeRing::getLineString(){
	getCoordinates();
	return factory->createLineString(*ringPts);
}

/**
* Returns this ring as a {@link LinearRing}, or null if an Exception occurs while
* creating it (such as a topology problem). Details of problems are written to
* standard output.
*/
LinearRing* polygonizeEdgeRing::getRing(){
	if (ring!=NULL) return ring;
	getCoordinates();
	if (ringPts->getSize() < 3) cout<<ringPts->toString();
	try {
		ring=factory->createLinearRing(*ringPts);
	} catch (void* x) {
		cout << ringPts->toString();
	}
	return ring;
}

void polygonizeEdgeRing::addEdge(CoordinateSequence *coords, bool isForward, CoordinateSequence *coordList){
	if (isForward) {
		for (int i=0; i < coords->getSize(); i++) {
			coordList->add(coords->getAt(i), false);
		}
	} else {
		for (int i=coords->getSize()-1; i >= 0; i--) {
			coordList->add(coords->getAt(i), false);
		}
	}
}
}
