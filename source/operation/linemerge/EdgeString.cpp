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
 * Added DefaultCoordinateSequenceFactory::instance() function.
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

namespace geos {

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

} // namespace geos
