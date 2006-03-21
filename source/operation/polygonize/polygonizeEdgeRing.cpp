/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 * Copyright (C) 2005 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/opPolygonize.h>
#include <geos/planargraph/DirectedEdge.h>
#include <geos/geom/CoordinateSequence.h>

#include <vector>

//#define DEBUG_ALLOC 1

using namespace std;
using namespace geos::planargraph;
using namespace geos::algorithm;
using namespace geos::geom;

namespace geos {
namespace operation { // geos.operation
namespace polygonize { // geos.operation.polygonize

/*
 * Find the innermost enclosing shell polygonizeEdgeRing containing
 * the argument polygonizeEdgeRing, if any.
 * The innermost enclosing ring is the <i>smallest</i> enclosing ring.
 * The algorithm used depends on the fact that:
 * 
 * ring A contains ring B iff envelope(ring A) contains envelope(ring B)
 * 
 * This routine is only safe to use if the chosen point of the hole
 * is known to be properly contained in a shell
 * (which is guaranteed to be the case if the hole does not touch its shell)
 *
 * @return containing polygonizeEdgeRing, if there is one
 * @return null if no containing polygonizeEdgeRing is found
 */
polygonizeEdgeRing *
polygonizeEdgeRing::findEdgeRingContaining(polygonizeEdgeRing *testEr,
	vector<polygonizeEdgeRing*> *shellList)
{
	LinearRing *testRing=testEr->getRingInternal();
	if ( ! testRing ) return NULL;
	const Envelope *testEnv=testRing->getEnvelopeInternal();
	Coordinate testPt=testRing->getCoordinateN(0);
	polygonizeEdgeRing *minShell=NULL;
	const Envelope *minEnv=NULL;
	for(int i=0;i<(int)shellList->size();i++) {
		polygonizeEdgeRing *tryShell=(*shellList)[i];
		LinearRing *tryRing=tryShell->getRingInternal();
		const Envelope *tryEnv=tryRing->getEnvelopeInternal();
		if (minShell!=NULL) minEnv=minShell->getRingInternal()->getEnvelopeInternal();
		bool isContained=false;

		// the hole envelope cannot equal the shell envelope

		if (tryEnv->equals(testEnv)) continue;

		const CoordinateSequence *tryCoords =
			tryRing->getCoordinatesRO();

		testPt=ptNotInList(testRing->getCoordinatesRO(),
			tryCoords);

		if (tryEnv->contains(testEnv)
			&& CGAlgorithms::isPointInRing(testPt, tryCoords))
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

/*
 * Finds a point in a list of points which is not contained in another
 * list of points
 * @param testPts the CoordinateSequence to test
 * @param pts the CoordinateSequence to test the input points against
 * @return a Coordinate from <code>testPts</code> which is not
 * in <code>pts</code>, 
 * or <code>nullCoord</code>
 */
const Coordinate&
polygonizeEdgeRing::ptNotInList(const CoordinateSequence *testPts,
	const CoordinateSequence *pts)
{
	unsigned int npts=testPts->getSize();
	for (unsigned int i=0; i<npts; ++i)
	{
		const Coordinate& testPt=testPts->getAt(i);
		if (isInList(testPt, pts))
			return testPt;
	}
	return Coordinate::getNull();
}

/*
 * Tests whether a given point is in an array of points.
 * Uses a value-based test.
 *
 * @param pt a Coordinate for the test point
 * @param pts a CoordinateSequence to test pt against
 * @return <code>true</code> if the point is in the array
 */
bool
polygonizeEdgeRing::isInList(const Coordinate& pt,
	const CoordinateSequence *pts)
{
	unsigned int npts=pts->getSize();
	for (unsigned int i=0; i<npts; ++i)
	{
		if (pt==pts->getAt(i))
			return false;
	}
	return true;
}

polygonizeEdgeRing::polygonizeEdgeRing(const GeometryFactory *newFactory)
{
#ifdef DEBUG_ALLOC
	cerr<<"["<<this<<"] polygonizeEdgeRing(factory)"<<endl;
#endif // DEBUG_ALLOC

	deList=new vector<const DirectedEdge*>();
	// cache the following data for efficiency
	ring=NULL;
	ringPts=NULL;
	holes=NULL;
	factory=newFactory;
}

polygonizeEdgeRing::~polygonizeEdgeRing()
{
#ifdef DEBUG_ALLOC
	cerr<<"["<<this<<"] ~polygonizeEdgeRing()"<<endl;
#endif // DEBUG_ALLOC
	delete deList;
	if ( holes )
	{
		for (int i=0; i<(int)holes->size(); i++) delete (*holes)[i];
		delete holes;
	}
	delete ring;
	delete ringPts;
}

/*
 * Adds a DirectedEdge which is known to form part of this ring.
 * @param de the DirectedEdge to add.
 */
void
polygonizeEdgeRing::add(const DirectedEdge *de){
	deList->push_back(de);
}

/**
 * Tests whether this ring is a hole.
 * Due to the way the edges in the polyongization graph are linked,
 * a ring is a hole if it is oriented counter-clockwise.
 * @return <code>true</code> if this ring is a hole
 */
bool
polygonizeEdgeRing::isHole(){
	getRingInternal();
	return CGAlgorithms::isCCW(ring->getCoordinatesRO());
}

/**
 * Adds a hole to the polygon formed by this ring.
 * @param hole the {@link LinearRing} forming the hole.
 */
void
polygonizeEdgeRing::addHole(LinearRing *hole)
{
	if (holes==NULL)
		holes=new vector<Geometry*>();
	holes->push_back((Geometry *)hole);
}

/*
 * Computes the Polygon formed by this ring and any contained holes.
 * LinearRings ownership is transferred to returned polygon.
 * Subsequent calls to the function will return NULL.
 *
 * @return the Polygon formed by this ring and its holes.
 */
Polygon*
polygonizeEdgeRing::getPolygon()
{
	Polygon *poly=factory->createPolygon(ring, holes);
	ring=NULL;
	holes=NULL;
	return poly;
}

/*
 * Tests if the LinearRing formed by this edge ring is topologically valid.
 */
bool
polygonizeEdgeRing::isValid()
{
	if ( ! getRingInternal() ) return false; // computes cached ring
	return ring->isValid();
}

/*
 * Computes the list of coordinates which are contained in this ring.
 * The coordinatea are computed once only and cached.
 *
 * @return a CoordinateSequence for this ring
 */
CoordinateSequence*
polygonizeEdgeRing::getCoordinates()
{
	if (ringPts==NULL)
	{
		ringPts=factory->getCoordinateSequenceFactory()->create(NULL);
		for (int i=0;i<(int)deList->size();i++) {
			const DirectedEdge *de=(*deList)[i];
			PolygonizeEdge *edge=(PolygonizeEdge*) de->getEdge();
			addEdge(edge->getLine()->getCoordinatesRO(),
				de->getEdgeDirection(), ringPts);
		}
	}
	return ringPts;
}

/*
 * Gets the coordinates for this ring as a LineString.
 * Used to return the coordinates in this ring
 * as a valid geometry, when it has been detected that the ring
 * is topologically invalid.
 * @return a LineString containing the coordinates in this ring
 */
LineString*
polygonizeEdgeRing::getLineString()
{
	getCoordinates();
	return factory->createLineString(*ringPts);
}

/*
 * Returns this ring as a LinearRing, or null if an Exception occurs while
 * creating it (such as a topology problem). Details of problems are written to
 * standard output.
 */
LinearRing *
polygonizeEdgeRing::getRingInternal()
{
	if (ring!=NULL) return ring;

	getCoordinates();
	try {
		ring=factory->createLinearRing(*ringPts);
	} catch (...) {
		return NULL;
	}
	return ring;
}

/*
 * Returns this ring as a LinearRing, or null if an Exception occurs while
 * creating it (such as a topology problem). Details of problems are written to
 * standard output.
 * Caller gets ownership of ring.
 */
LinearRing *
polygonizeEdgeRing::getRingOwnership()
{
	LinearRing *ret = getRingInternal();
	ring = NULL;
	return ret;
}

void
polygonizeEdgeRing::addEdge(const CoordinateSequence *coords, bool isForward,
	CoordinateSequence *coordList)
{
	unsigned int npts=coords->getSize();
	if (isForward) {
		for (unsigned int i=0; i<npts; ++i)
		{
			coordList->add(coords->getAt(i), false);
		}
	} else {
		for (unsigned int i=npts; i>0; --i) {
			coordList->add(coords->getAt(i-1), false);
		}
	}
}

} // namespace geos.operation.polygonize
} // namespace geos.operation
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.15  2006/03/21 21:42:54  strk
 * planargraph.h header split, planargraph:: classes renamed to match JTS symbols
 *
 **********************************************************************/
