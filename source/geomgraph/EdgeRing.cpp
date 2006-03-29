/**********************************************************************
 * $Id$
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
 **********************************************************************/

#include <geos/util/Assert.h>
#include <geos/util/TopologyException.h>
#include <geos/algorithm/CGAlgorithms.h>
#include <geos/geomgraph/EdgeRing.h>
#include <geos/geomgraph/DirectedEdge.h>
#include <geos/geomgraph/DirectedEdgeStar.h>
#include <geos/geomgraph/Edge.h>
#include <geos/geomgraph/Node.h>
#include <geos/geomgraph/Label.h>
#include <geos/geomgraph/Position.h>
#include <geos/geom/CoordinateSequenceFactory.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/Location.h>
#include <geos/geom/Envelope.h>

#include <vector>
#include <cassert>

#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif

#if GEOS_DEBUG
#include <iostream>
#endif

using namespace std;
using namespace geos::algorithm;
using namespace geos::geom;

namespace geos {
namespace geomgraph { // geos.geomgraph

EdgeRing::EdgeRing(DirectedEdge *newStart,
		const GeometryFactory *newGeometryFactory)
	:
        startDe(newStart),
        geometryFactory(newGeometryFactory),
	holes(),
        maxNodeDegree(-1),
	edges(),
	pts(newGeometryFactory->getCoordinateSequenceFactory()->create(NULL)),
        label(Location::UNDEF), // new Label(Location::UNDEF)),
        ring(NULL),
        isHoleVar(false),
        shell(NULL)
{
	/*
	 * Commented out to fix different polymorphism in C++ (from Java)
	 * Make sure these calls are made by derived classes !
	 */
	//computePoints(start);
	//computeRing();
#if GEOS_DEBUG
	cerr << "EdgeRing[" << this << "] ctor" << endl;
#endif
	testInvariant();
}

EdgeRing::~EdgeRing()
{
	testInvariant();

	/*
	 * If we constructed a ring, we did by transferring
	 * ownership of the CoordinateSequence, so it will be
	 * destroyed by `ring' dtor and we must not destroy
	 * it twice.
	 */
	if ( ring == NULL )
	{
		delete pts;
	}
	else 
	{
		delete ring;
	}

	for(unsigned int i=0;i<holes.size(); ++i) {
		delete holes[i];
	}

#if GEOS_DEBUG
	cerr << "EdgeRing[" << this << "] dtor" << endl;
#endif
}

bool
EdgeRing::isIsolated()
{
	testInvariant();
	return (label.getGeometryCount()==1);
}

bool
EdgeRing::isHole()
{
	testInvariant();

	// We can't tell if this is an hole
	// unless we computed the ring
	// see computeRing()
	assert(ring);

	return isHoleVar;
}


LinearRing*
EdgeRing::getLinearRing()
{
	testInvariant();
//	return new LinearRing(*ring);
	return ring;
}

Label&
EdgeRing::getLabel()
{
	testInvariant();
	return label;
}

bool
EdgeRing::isShell()
{
	testInvariant();
	return (shell==NULL);
}

EdgeRing*
EdgeRing::getShell()
{
	testInvariant();
	return shell;
}

void
EdgeRing::setShell(EdgeRing *newShell)
{
	shell=newShell;
	if (shell!=NULL) shell->addHole(this);
	testInvariant();
}

void
EdgeRing::addHole(EdgeRing *edgeRing)
{
	holes.push_back(edgeRing);
	testInvariant();
}

/*public*/
Polygon*
EdgeRing::toPolygon(const GeometryFactory* geometryFactory)
{
	testInvariant();

	unsigned int nholes=holes.size();
	vector<Geometry *> *holeLR=new vector<Geometry *>(nholes);
	for (unsigned int i=0; i<nholes; ++i)
	{
		Geometry *hole=holes[i]->getLinearRing()->clone();
        	(*holeLR)[i]=hole;
	}

	// We don't use "clone" here because
	// GeometryFactory::createPolygon really
	// wants a LinearRing
	//
	LinearRing *shellLR=new LinearRing(*(getLinearRing()));
	return geometryFactory->createPolygon(shellLR, holeLR);
}

/*public*/
void
EdgeRing::computeRing()
{
	testInvariant();

	if (ring!=NULL) return;   // don't compute more than once
	ring=geometryFactory->createLinearRing(pts);
	isHoleVar=CGAlgorithms::isCCW(pts);

	testInvariant();

}

/*public*/
vector<DirectedEdge*>&
EdgeRing::getEdges()
{
	testInvariant();

	return edges;
}

/*protected*/
void
EdgeRing::computePoints(DirectedEdge *newStart)
{
	startDe=newStart;
	DirectedEdge *de=newStart;
	bool isFirstEdge=true;
	do {
		//util::Assert::isTrue(de!=NULL,"EdgeRing::computePoints: found null Directed Edge");
		//assert(de!=NULL); // EdgeRing::computePoints: found null Directed Edge
		if(de==NULL)
			throw util::TopologyException(
				"EdgeRing::computePoints: found null Directed Edge");

		if (de->getEdgeRing()==this)
			throw util::TopologyException(
				"Directed Edge visited twice during ring-building",
				de->getCoordinate());

		edges.push_back(de);
		Label *deLabel=de->getLabel();
		assert(deLabel);
		assert(deLabel->isArea());
		mergeLabel(*deLabel);
		addPoints(de->getEdge(),de->isForward(),isFirstEdge);
		isFirstEdge=false;
		setEdgeRing(de,this);
		de=getNext(de);
	} while (de!=startDe);

	testInvariant();

}

/*public*/
int
EdgeRing::getMaxNodeDegree()
{

	testInvariant();

	if (maxNodeDegree<0) computeMaxNodeDegree();
	return maxNodeDegree;
}

/*private*/
void
EdgeRing::computeMaxNodeDegree()
{
	maxNodeDegree=0;
	DirectedEdge *de=startDe;
	do {
		Node *node=de->getNode();
		EdgeEndStar* ees = node->getEdges();
		assert(dynamic_cast<DirectedEdgeStar*>(ees));
		DirectedEdgeStar* des = static_cast<DirectedEdgeStar*>(ees);
		int degree=des->getOutgoingDegree(this);
		if (degree>maxNodeDegree) maxNodeDegree=degree;
		de=getNext(de);
	} while (de!=startDe);
	maxNodeDegree *= 2;

	testInvariant();

}

/*public*/
void
EdgeRing::setInResult()
{
	DirectedEdge *de=startDe;
	do {
		de->getEdge()->setInResult(true);
		de=de->getNext();
	} while (de!=startDe);

	testInvariant();

}

/*protected*/
void
EdgeRing::mergeLabel(Label& deLabel)
{
	mergeLabel(deLabel, 0);
	mergeLabel(deLabel, 1);

	testInvariant();

}

/*protected*/
void
EdgeRing::mergeLabel(Label& deLabel, int geomIndex)
{

	testInvariant();

	int loc=deLabel.getLocation(geomIndex, Position::RIGHT);
	// no information to be had from this label
	if (loc==Location::UNDEF) return;

	// if there is no current RHS value, set it
	if (label.getLocation(geomIndex)==Location::UNDEF) {
		label.setLocation(geomIndex,loc);
		return;
	}
}

/*protected*/
void
EdgeRing::addPoints(Edge *edge, bool isForward, bool isFirstEdge)
{
	// EdgeRing::addPoints: can't add points after LinearRing construction
	assert(ring==NULL);

	assert(edge);
	const CoordinateSequence* edgePts=edge->getCoordinates();

	assert(edgePts);
	unsigned int numEdgePts=edgePts->getSize();

	assert(pts);

	if (isForward) {
		unsigned int startIndex=1;
		if (isFirstEdge) startIndex=0;
		for (unsigned int i=startIndex; i<numEdgePts; ++i)
		{
			pts->add(edgePts->getAt(i));
		}
	}
	
	else { // is backward
		unsigned int startIndex=numEdgePts-1;
		if (isFirstEdge) startIndex=numEdgePts;
		//for (int i=startIndex;i>=0;i--) 
		for (unsigned int i=startIndex; i>0; --i)
		{
			pts->add(edgePts->getAt(i-1));
		}
	}

	testInvariant();

}

/*public*/
bool
EdgeRing::containsPoint(const Coordinate& p)
{

	testInvariant();

	assert(ring);

	const Envelope* env=ring->getEnvelopeInternal();
	assert(env);
	if ( ! env->contains(p) ) return false;

	if ( ! CGAlgorithms::isPointInRing(p, ring->getCoordinatesRO()) )
		return false;

	for (vector<EdgeRing*>::iterator i=holes.begin(); i<holes.end(); ++i)
	{
		EdgeRing *hole=*i;
		assert(hole);
		if (hole->containsPoint(p))
		{
			return false;
		}
	}
	return true;
}

} // namespace geos.geomgraph
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.22  2006/03/29 13:53:56  strk
 * EdgeRing equipped with Invariant testing function and lots of exceptional assertions. Removed useless heap allocations, and pointers usages.
 *
 * Revision 1.21  2006/03/27 16:02:33  strk
 * Added INL file for MinimalEdgeRing, added many debugging blocks,
 * fixed memory leak in ConnectedInteriorTester (bug #59)
 *
 * Revision 1.20  2006/03/23 15:10:29  strk
 * Dropped by-pointer TopologyException constructor, various small cleanups
 *
 * Revision 1.19  2006/03/15 17:16:29  strk
 * streamlined headers inclusion
 *
 * Revision 1.18  2006/03/15 15:26:58  strk
 * Cleanups
 *
 * Revision 1.17  2006/03/10 15:50:26  strk
 * Changed 'found null Directed Edge' from an Assertion to a TopologyException, to give 'precision-reducing' overlay operation a chance to handle it (it seems to work)
 *
 * Revision 1.16  2006/03/10 13:07:32  strk
 * fine-tuned includes, reverted Assert=>assert due to user-input based failure
 *
 * Revision 1.15  2006/03/09 16:46:47  strk
 * geos::geom namespace definition, first pass at headers split
 *
 * Revision 1.14  2006/03/06 19:40:46  strk
 * geos::util namespace. New GeometryCollection::iterator interface, many cleanups.
 *
 * Revision 1.13  2006/03/03 10:46:21  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.12  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.11  2006/02/09 15:52:47  strk
 * GEOSException derived from std::exception; always thrown and cought by const ref.
 *
 * Revision 1.10  2006/01/31 19:07:34  strk
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
 * Revision 1.9  2005/11/24 23:09:15  strk
 * CoordinateSequence indexes switched from int to the more
 * the correct unsigned int. Optimizations here and there
 * to avoid calling getSize() in loops.
 * Update of all callers is not complete yet.
 *
 * Revision 1.8  2005/11/18 00:55:29  strk
 *
 * Fixed a bug in EdgeRing::containsPoint().
 * Changed EdgeRing::getLinearRing() to avoid LinearRing copy and updated
 * usages from PolygonBuilder.
 * Removed CoordinateSequence copy in EdgeRing (ownership is transferred
 * to its LinearRing).
 * Removed heap allocations for EdgeRing containers.
 * Initialization lists and cleanups.
 *
 * Revision 1.7  2005/05/19 10:29:28  strk
 * Removed some CGAlgorithms instances substituting them with direct calls
 * to the static functions. Interfaces accepting CGAlgorithms pointers kept
 * for backward compatibility but modified to make the argument optional.
 * Fixed a small memory leak in OffsetCurveBuilder::getRingCurve.
 * Inlined some smaller functions encountered during bug hunting.
 * Updated Copyright notices in the touched files.
 *
 * Revision 1.6  2004/07/27 16:35:46  strk
 * Geometry::getEnvelopeInternal() changed to return a const Envelope *.
 * This should reduce object copies as once computed the envelope of a
 * geometry remains the same.
 *
 * Revision 1.5  2004/07/08 19:34:49  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added CoordinateArraySequenceFactory::instance() function.
 *
 * Revision 1.4  2004/07/02 13:28:26  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.3  2004/07/01 14:12:44  strk
 *
 * Geometry constructors come now in two flavors:
 * 	- deep-copy args (pass-by-reference)
 * 	- take-ownership of args (pass-by-pointer)
 * Same functionality is available through GeometryFactory,
 * including buildGeometry().
 *
 * Revision 1.2  2004/06/30 20:59:12  strk
 * Removed GeoemtryFactory copy from geometry constructors.
 * Enforced const-correctness on GeometryFactory arguments.
 *
 * Revision 1.1  2004/03/19 09:48:45  ybychkov
 * "geomgraph" and "geomgraph/indexl" upgraded to JTS 1.4
 *
 * Revision 1.22  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 * Revision 1.21  2003/11/06 17:59:03  strk
 * Memory leaks fixed in ::containsPoint()
 *
 * Revision 1.20  2003/10/20 14:02:14  strk
 * more explicit exception thrown on null Directed Edge detection
 *
 * Revision 1.19  2003/10/15 16:39:03  strk
 * Made Edge::getCoordinates() return a 'const' value. Adapted code set.
 * $Log$
 * Revision 1.22  2006/03/29 13:53:56  strk
 * EdgeRing equipped with Invariant testing function and lots of exceptional assertions. Removed useless heap allocations, and pointers usages.
 *
 * Revision 1.21  2006/03/27 16:02:33  strk
 * Added INL file for MinimalEdgeRing, added many debugging blocks,
 * fixed memory leak in ConnectedInteriorTester (bug #59)
 *
 * Revision 1.20  2006/03/23 15:10:29  strk
 * Dropped by-pointer TopologyException constructor, various small cleanups
 *
 * Revision 1.19  2006/03/15 17:16:29  strk
 * streamlined headers inclusion
 *
 * Revision 1.18  2006/03/15 15:26:58  strk
 * Cleanups
 *
 * Revision 1.17  2006/03/10 15:50:26  strk
 * Changed 'found null Directed Edge' from an Assertion to a TopologyException, to give 'precision-reducing' overlay operation a chance to handle it (it seems to work)
 *
 * Revision 1.16  2006/03/10 13:07:32  strk
 * fine-tuned includes, reverted Assert=>assert due to user-input based failure
 *
 * Revision 1.15  2006/03/09 16:46:47  strk
 * geos::geom namespace definition, first pass at headers split
 *
 * Revision 1.14  2006/03/06 19:40:46  strk
 * geos::util namespace. New GeometryCollection::iterator interface, many cleanups.
 *
 * Revision 1.13  2006/03/03 10:46:21  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.12  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.11  2006/02/09 15:52:47  strk
 * GEOSException derived from std::exception; always thrown and cought by const ref.
 *
 * Revision 1.10  2006/01/31 19:07:34  strk
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
 * Revision 1.9  2005/11/24 23:09:15  strk
 * CoordinateSequence indexes switched from int to the more
 * the correct unsigned int. Optimizations here and there
 * to avoid calling getSize() in loops.
 * Update of all callers is not complete yet.
 *
 * Revision 1.8  2005/11/18 00:55:29  strk
 *
 * Fixed a bug in EdgeRing::containsPoint().
 * Changed EdgeRing::getLinearRing() to avoid LinearRing copy and updated
 * usages from PolygonBuilder.
 * Removed CoordinateSequence copy in EdgeRing (ownership is transferred
 * to its LinearRing).
 * Removed heap allocations for EdgeRing containers.
 * Initialization lists and cleanups.
 *
 * Revision 1.7  2005/05/19 10:29:28  strk
 * Removed some CGAlgorithms instances substituting them with direct calls
 * to the static functions. Interfaces accepting CGAlgorithms pointers kept
 * for backward compatibility but modified to make the argument optional.
 * Fixed a small memory leak in OffsetCurveBuilder::getRingCurve.
 * Inlined some smaller functions encountered during bug hunting.
 * Updated Copyright notices in the touched files.
 *
 * Revision 1.6  2004/07/27 16:35:46  strk
 * Geometry::getEnvelopeInternal() changed to return a const Envelope *.
 * This should reduce object copies as once computed the envelope of a
 * geometry remains the same.
 *
 * Revision 1.5  2004/07/08 19:34:49  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added CoordinateArraySequenceFactory::instance() function.
 *
 * Revision 1.4  2004/07/02 13:28:26  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.3  2004/07/01 14:12:44  strk
 *
 * Geometry constructors come now in two flavors:
 * 	- deep-copy args (pass-by-reference)
 * 	- take-ownership of args (pass-by-pointer)
 * Same functionality is available through GeometryFactory,
 * including buildGeometry().
 *
 * Revision 1.2  2004/06/30 20:59:12  strk
 * Removed GeoemtryFactory copy from geometry constructors.
 * Enforced const-correctness on GeometryFactory arguments.
 *
 * Revision 1.1  2004/03/19 09:48:45  ybychkov
 * "geomgraph" and "geomgraph/indexl" upgraded to JTS 1.4
 *
 * Revision 1.22  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/

