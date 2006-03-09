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

#include <geos/opLinemerge.h>
#include <geos/planargraph.h>

#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif

#ifdef GEOS_DEBUG
#include <iostream>
#endif

using namespace std;
using namespace geos::planargraph;

namespace geos {
namespace operation { // geos.operation
namespace linemerge { // geos.operation.linemerge

void
LineMergeGraph::addEdge(const LineString *lineString)
{
	if (lineString->isEmpty()) return;

#if GEOS_DEBUG
	cerr<<"Adding LineString "<<lineString->toString()<<endl;
#endif

	CoordinateSequence *coordinates = 
		CoordinateSequence::removeRepeatedPoints(lineString->getCoordinatesRO());

	const Coordinate& startCoordinate=coordinates->getAt(0);
	const Coordinate& endCoordinate=coordinates->getAt(coordinates->getSize()-1);

	planarNode* startNode=getNode(startCoordinate);
	planarNode* endNode=getNode(endCoordinate);
#if GEOS_DEBUG
	cerr<<" startNode: "<<*startNode<<endl;
	cerr<<" endNode: "<<*endNode<<endl;
#endif

	planarDirectedEdge *directedEdge0=new LineMergeDirectedEdge(startNode,
			endNode,coordinates->getAt(1),
			true);
	newDirEdges.push_back(directedEdge0);

	planarDirectedEdge *directedEdge1=new LineMergeDirectedEdge(endNode,
			startNode,coordinates->getAt(coordinates->getSize()-2),
			false);
	newDirEdges.push_back(directedEdge1);

	planarEdge *edge=new LineMergeEdge(lineString);
	newEdges.push_back(edge);
	edge->setDirectedEdges(directedEdge0, directedEdge1);

#if GEOS_DEBUG
	cerr<<" planarEdge: "<<*edge<<endl;
#endif

	add(edge);

#if GEOS_DEBUG
	cerr<<" After addition to the graph:"<<endl;
	cerr<<"  startNode: "<<*startNode<<endl;
	cerr<<"  endNode: "<<*endNode<<endl;
#endif

	delete coordinates;
}

planarNode *
LineMergeGraph::getNode(const Coordinate &coordinate)
{
	planarNode *node=findNode(coordinate);
	if (node==NULL) {
		node=new planarNode(coordinate);
		newNodes.push_back(node);
		add(node);
	}
	return node;
}

LineMergeGraph::~LineMergeGraph()
{
	unsigned int i;
	for (i=0; i<newNodes.size(); i++)
		delete newNodes[i];
	for (i=0; i<newEdges.size(); i++)
		delete newEdges[i];
	for (i=0; i<newDirEdges.size(); i++)
		delete newDirEdges[i];
}

} // namespace geos.operation.linemerge
} // namespace geos.operation
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.13  2006/03/09 15:47:19  strk
 * Fixed debugging lines
 *
 * Revision 1.12  2006/03/02 12:12:01  strk
 * Renamed DEBUG macros to GEOS_DEBUG, all wrapped in #ifndef block to allow global override (bug#43)
 *
 * Revision 1.11  2006/02/24 15:39:07  strk
 * - operator>> for Coordinate, planarNode and planarEdge
 * - Fixed bug in planarGraphComponent::setMarked
 * - Added linemerge.xml test (single test, should grow a bit)
 *
 * Revision 1.10  2006/02/23 23:17:52  strk
 * - Coordinate::nullCoordinate made private
 * - Simplified Coordinate inline definitions
 * - LMGeometryComponentFilter definition moved to LineMerger.cpp file
 * - Misc cleanups
 *
 * Revision 1.9  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.8  2006/01/31 19:07:34  strk
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
 * Revision 1.7  2005/11/15 12:14:05  strk
 * Reduced heap allocations, made use of references when appropriate,
 * small optimizations here and there.
 *
 * Revision 1.6  2005/09/26 11:01:32  strk
 * Const correctness changes in LineMerger package, and a few speedups.
 *
 * Revision 1.5  2004/12/08 13:54:44  strk
 * gcc warnings checked and fixed, general cleanups.
 *
 * Revision 1.4  2004/10/13 10:03:02  strk
 * Added missing linemerge and polygonize operation.
 * Bug fixes and leaks removal from the newly added modules and
 * planargraph (used by them).
 * Some comments and indentation changes.
 *
 * Revision 1.3  2004/07/08 19:34:50  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added CoordinateArraySequenceFactory::instance() function.
 *
 * Revision 1.2  2004/07/02 13:28:28  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.1  2004/04/07 06:55:50  ybychkov
 * "operation/linemerge" ported from JTS 1.4
 **********************************************************************/

