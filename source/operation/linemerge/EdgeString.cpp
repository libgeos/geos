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
 * Revision 1.9  2006/03/03 10:46:22  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.8  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.7  2006/01/31 19:07:34  strk
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
 * Revision 1.6  2005/09/26 11:01:32  strk
 * Const correctness changes in LineMerger package, and a few speedups.
 *
 * Revision 1.5  2004/10/13 10:03:02  strk
 * Added missing linemerge and polygonize operation.
 * Bug fixes and leaks removal from the newly added modules and
 * planargraph (used by them).
 * Some comments and indentation changes.
 *
 * Revision 1.4  2004/07/08 19:34:50  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added CoordinateArraySequenceFactory::instance() function.
 *
 * Revision 1.3  2004/07/02 13:28:28  strk
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
 * Revision 1.1  2004/04/07 06:55:50  ybychkov
 * "operation/linemerge" ported from JTS 1.4
 *
 *
 **********************************************************************/

#include <geos/opLinemerge.h>
#include <vector>

using namespace std;

namespace geos {
namespace operation { // geos.operation
namespace linemerge { // geos.operation.linemerge

/**
 * Constructs an EdgeString with the given factory used to convert
 * this EdgeString to a LineString
 */
EdgeString::EdgeString(const GeometryFactory *newFactory):
	factory(newFactory),
	directedEdges(new vector<LineMergeDirectedEdge*>()),
	coordinates(NULL)
{
}

EdgeString::~EdgeString() {
	delete directedEdges;
}

/**
 * Adds a directed edge which is known to form part of this line.
 */
void
EdgeString::add(LineMergeDirectedEdge *directedEdge)
{
	directedEdges->push_back(directedEdge);
}

CoordinateSequence *
EdgeString::getCoordinates()
{
	if (coordinates==NULL) {
		int forwardDirectedEdges = 0;
		int reverseDirectedEdges = 0;
		coordinates=factory->getCoordinateSequenceFactory()->create(NULL);
		for (int i=0;i<(int)directedEdges->size();i++) {
			LineMergeDirectedEdge* directedEdge=(*directedEdges)[i];
			if (directedEdge->getEdgeDirection()) {
				forwardDirectedEdges++;
			} else {
				reverseDirectedEdges++;
			}
			coordinates->add((CoordinateSequence *)((LineMergeEdge*)directedEdge->getEdge())->getLine()->getCoordinatesRO(),false,directedEdge->getEdgeDirection());
		}
		if (reverseDirectedEdges > forwardDirectedEdges) {
			CoordinateSequence::reverse(coordinates);
		}
	}
	return coordinates;
}

/*
 * Converts this EdgeString into a new LineString.
 */
LineString*
EdgeString::toLineString()
{
	//return factory->createLineString(*(getCoordinates()));
	return factory->createLineString(getCoordinates());
}

} // namespace geos.operation.linemerge
} // namespace geos.operation
} // namespace geos
