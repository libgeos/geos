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

#include <geos/geomgraph.h>
#include <geos/util.h>
#include <vector>

using namespace std;
using namespace geos::algorithm;

namespace geos {
namespace geomgraph { // geos.geomgraph

EdgeRing::EdgeRing(DirectedEdge *newStart,
		const GeometryFactory *newGeometryFactory)
	:
        startDe(newStart),
        geometryFactory(newGeometryFactory),
        maxNodeDegree(-1),
	pts(newGeometryFactory->getCoordinateSequenceFactory()->create(NULL)),
        label(new Label(Location::UNDEF)),
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

}

EdgeRing::~EdgeRing()
{
	/*
	 * If we constructed a ring, we did by transferring
	 * ownership of the CoordinateSequence, so it will be
	 * destroyed by `ring' dtor and we must not destroy
	 * it twice.
	 */
	if ( ring == NULL ) delete pts;
	else delete ring;

	delete label;
	for(unsigned int i=0;i<holes.size(); ++i) {
		delete holes[i];
	}
}

bool
EdgeRing::isIsolated()
{
	return (label->getGeometryCount()==1);
}

bool
EdgeRing::isHole()
{
	return isHoleVar;
}


LinearRing*
EdgeRing::getLinearRing()
{
//	return new LinearRing(*ring);
	return ring;
}

Label*
EdgeRing::getLabel()
{
	return label;
}

bool
EdgeRing::isShell()
{
	return shell==NULL;
}

EdgeRing*
EdgeRing::getShell()
{
	return shell;
}

void
EdgeRing::setShell(EdgeRing *newShell)
{
	shell=newShell;
	if (shell!=NULL) shell->addHole(this);
}

void
EdgeRing::addHole(EdgeRing *edgeRing)
{
	holes.push_back(edgeRing);
}

Polygon*
EdgeRing::toPolygon(const GeometryFactory* geometryFactory)
{
	unsigned int nholes=holes.size();
	vector<Geometry *> *holeLR=new vector<Geometry *>(nholes);
	for (unsigned int i=0; i<nholes; ++i)
	{
		LinearRing *hole=new LinearRing(*(holes[i]->getLinearRing()));
        	(*holeLR)[i]=(hole);
	}
	LinearRing *shellLR=new LinearRing(*(getLinearRing()));
	return geometryFactory->createPolygon(shellLR, holeLR);
}

void
EdgeRing::computeRing()
{
	if (ring!=NULL) return;   // don't compute more than once
	ring=geometryFactory->createLinearRing(pts);
	isHoleVar=CGAlgorithms::isCCW(pts);
}

/**
 * Returns the list of DirectedEdges that make up this EdgeRing
 */
vector<DirectedEdge*>*
EdgeRing::getEdges()
{
	return &edges;
}

void
EdgeRing::computePoints(DirectedEdge *newStart)
{
	startDe=newStart;
	DirectedEdge *de=newStart;
	bool isFirstEdge=true;
	do {
		assert(de!=NULL); // EdgeRing::computePoints: found null Directed Edge
		if (de->getEdgeRing()==this)
			throw util::TopologyException("Directed Edge visited twice during ring-building at ",&(de->getCoordinate()));
		edges.push_back(de);
		Label *deLabel=de->getLabel();
		assert(deLabel->isArea());
		mergeLabel(deLabel);
		addPoints(de->getEdge(),de->isForward(),isFirstEdge);
		isFirstEdge=false;
		setEdgeRing(de,this);
		de=getNext(de);
	} while (de!=startDe);
}

int
EdgeRing::getMaxNodeDegree()
{
	if (maxNodeDegree<0) computeMaxNodeDegree();
	return maxNodeDegree;
}

void
EdgeRing::computeMaxNodeDegree()
{
	maxNodeDegree=0;
	DirectedEdge *de=startDe;
	do {
		Node *node=de->getNode();
		int degree=((DirectedEdgeStar*) node->getEdges())->getOutgoingDegree(this);
		if (degree>maxNodeDegree) maxNodeDegree=degree;
		de=getNext(de);
	} while (de!=startDe);
	maxNodeDegree *= 2;
}

void
EdgeRing::setInResult()
{
	DirectedEdge *de=startDe;
	do {
		(de->getEdge())->setInResult(true);
		de=de->getNext();
	} while (de!=startDe);
}

void
EdgeRing::mergeLabel(Label *deLabel)
{
	mergeLabel(deLabel, 0);
	mergeLabel(deLabel, 1);
}

/**
 * Merge the RHS label from a DirectedEdge into the label for this EdgeRing.
 * The DirectedEdge label may be null.  This is acceptable - it results
 * from a node which is NOT an intersection node between the Geometries
 * (e.g. the end node of a LinearRing).  In this case the DirectedEdge label
 * does not contribute any information to the overall labelling, and is
 * simply skipped.
 */
void
EdgeRing::mergeLabel(Label *deLabel, int geomIndex)
{
	int loc=deLabel->getLocation(geomIndex,Position::RIGHT);
	// no information to be had from this label
	if (loc==Location::UNDEF) return;
	// if there is no current RHS value, set it
	if (label->getLocation(geomIndex)==Location::UNDEF) {
		label->setLocation(geomIndex,loc);
		return;
	}
}

void
EdgeRing::addPoints(Edge *edge, bool isForward, bool isFirstEdge)
{
	// EdgeRing::addPoints: can't add points after LinearRing construction
	assert(ring==NULL);

	const CoordinateSequence* edgePts=edge->getCoordinates();

	unsigned int numEdgePts=edgePts->getSize();

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
}

/**
 * This method will use the computed ring.
 * It will also check any holes, if they have been assigned.
 */
bool
EdgeRing::containsPoint(const Coordinate& p)
{
	const Envelope* env=ring->getEnvelopeInternal();
	if ( ! env->contains(p) ) return false;
	if ( ! CGAlgorithms::isPointInRing(p, ring->getCoordinatesRO()) )
		return false;

	for (vector<EdgeRing*>::iterator i=holes.begin(); i<holes.end(); ++i)
	{
		EdgeRing *hole=*i;
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

