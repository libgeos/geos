/**********************************************************************
 * $Id: EdgeRing.cpp 3078 2010-07-01 21:44:33Z strk $
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2005-2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: operation/polygonize/EdgeRing.java rev. 109/138 (JTS-1.10)
 *
 **********************************************************************/

#include <geos/operation/polygonize/EdgeRing.h>
#include <geos/operation/polygonize/PolygonizeEdge.h>
#include <geos/planargraph/DirectedEdge.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/Envelope.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/CoordinateSequenceFactory.h>
#include <geos/algorithm/CGAlgorithms.h>

#include <vector>
#include <cassert>

//#define DEBUG_ALLOC 1
//#define GEOS_PARANOIA_LEVEL 2

using namespace std;
using namespace geos::planargraph;
using namespace geos::algorithm;
using namespace geos::geom;

namespace geos {
namespace operation { // geos.operation
namespace polygonize { // geos.operation.polygonize

/*public*/
EdgeRing *
EdgeRing::findEdgeRingContaining(EdgeRing *testEr,
	vector<EdgeRing*> *shellList)
{
	const LinearRing *testRing=testEr->getRingInternal();
	if ( ! testRing ) return NULL;
	const Envelope *testEnv=testRing->getEnvelopeInternal();
	Coordinate testPt=testRing->getCoordinateN(0);
	EdgeRing *minShell=NULL;
	const Envelope *minEnv=NULL;

	typedef std::vector<EdgeRing*> ERList;
	for(ERList::size_type i=0, e=shellList->size(); i<e; ++i) {
		EdgeRing *tryShell=(*shellList)[i];
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
		// check if this new containing ring is smaller
		// than the current minimum ring
		if (isContained) {
			if (minShell==NULL || minEnv->contains(tryEnv)) {
				minShell=tryShell;
			}
		}
	}
	return minShell;
}

/*public static*/
const Coordinate&
EdgeRing::ptNotInList(const CoordinateSequence *testPts,
                      const CoordinateSequence *pts)
{
    const std::size_t npts = testPts->getSize();
    for (std::size_t i = 0; i < npts; ++i)
    {
        const Coordinate& testPt = testPts->getAt(i);
        if (isInList(testPt, pts))
            return testPt;
    }
    return Coordinate::getNull();
}

/*public static*/
bool
EdgeRing::isInList(const Coordinate& pt,
                   const CoordinateSequence *pts)
{
    const std::size_t npts = pts->getSize();
    for (std::size_t i = 0; i < npts; ++i)
    {
        if (pt == pts->getAt(i))
            return false;
    }
    return true;
}

/*public*/
EdgeRing::EdgeRing(const GeometryFactory *newFactory)
	:
	factory(newFactory),
	ring(0),
	ringPts(0),
	holes(0)
{
#ifdef DEBUG_ALLOC
	cerr<<"["<<this<<"] EdgeRing(factory)"<<endl;
#endif // DEBUG_ALLOC
}

EdgeRing::~EdgeRing()
{
#ifdef DEBUG_ALLOC
	cerr<<"["<<this<<"] ~EdgeRing()"<<endl;
#endif // DEBUG_ALLOC
	if ( holes )
	{
		for (GeomVect::size_type i=0, e=holes->size(); i<e; ++i)
			delete (*holes)[i];
		delete holes;
	}
	delete ring;
	delete ringPts;
}

/*public*/
void
EdgeRing::add(const DirectedEdge *de){
	deList.push_back(de);
}

/*public*/
bool
EdgeRing::isHole(){
	getRingInternal();
	return CGAlgorithms::isCCW(ring->getCoordinatesRO());
}

/*public*/
void
EdgeRing::addHole(LinearRing *hole)
{
	if (holes==NULL)
		holes=new vector<Geometry*>();
	holes->push_back(hole);
}

/*public*/
Polygon*
EdgeRing::getPolygon()
{
	Polygon *poly=factory->createPolygon(ring, holes);
	ring=NULL;
	holes=NULL;
	return poly;
}

/*public*/
bool
EdgeRing::isValid()
{
	if ( ! getRingInternal() ) return false; // computes cached ring
	return ring->isValid();
}

/*private*/
CoordinateSequence*
EdgeRing::getCoordinates()
{
	if (ringPts==NULL)
	{
		ringPts=factory->getCoordinateSequenceFactory()->create(NULL);
		for (DeList::size_type i=0, e=deList.size(); i<e; ++i) {
			const DirectedEdge *de=deList[i];
			assert(dynamic_cast<PolygonizeEdge*>(de->getEdge()));
			PolygonizeEdge *edge=static_cast<PolygonizeEdge*>(de->getEdge());
			addEdge(edge->getLine()->getCoordinatesRO(),
				de->getEdgeDirection(), ringPts);
		}
	}
	return ringPts;
}

/*public*/
LineString*
EdgeRing::getLineString()
{
	getCoordinates();
	return factory->createLineString(*ringPts);
}

/*public*/
LinearRing *
EdgeRing::getRingInternal()
{
	if (ring!=NULL) return ring;

	getCoordinates();
	try {
		ring=factory->createLinearRing(*ringPts);
	} catch (const std::exception& e) {
		// FIXME: print also ringPts
		std::cerr << "EdgeRing::getRingInternal: "
		          << e.what()
		          << endl;
	}
	return ring;
}

/*public*/
LinearRing *
EdgeRing::getRingOwnership()
{
	LinearRing *ret = getRingInternal();
	ring = NULL;
	return ret;
}

/*private*/
void
EdgeRing::addEdge(const CoordinateSequence *coords, bool isForward,
                  CoordinateSequence *coordList)
{
    const std::size_t npts=coords->getSize();
    if (isForward)
    {
        for (std::size_t i = 0; i < npts; ++i)
        {
            coordList->add(coords->getAt(i), false);
        }
    }
    else
    {
        for (std::size_t i = npts; i > 0; --i)
        {
            coordList->add(coords->getAt(i-1), false);
        }
    }
}

} // namespace geos.operation.polygonize
} // namespace geos.operation
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.2  2006/05/02 14:32:49  strk
 * Added port info for polygonize/EdgeRing class, polygonize/EdgeRing.h header
 * not installed.
 *
 * Revision 1.1  2006/05/02 14:22:30  strk
 * * source/operation/polygonize/: polygonizeEdgeRing.cpp renamed to EdgeRing.cpp, to follow JTS naming.
 *
 * Revision 1.16  2006/03/22 11:19:06  strk
 * opPolygonize.h headers split.
 *
 **********************************************************************/
