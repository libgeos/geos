/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/


#include <geos/planargraph.h>

namespace geos {
//namespace planargraph {

/**
* Returns all Edges that connect the two nodes (which are assumed to be different).
*/
vector<planarEdge*>*
planarNode::getEdgesBetween(planarNode *node0, planarNode *node1)
{
	return NULL;
}


//} // namespace planargraph 
} // namespace geos 

/**********************************************************************
 * $Log$
 * Revision 1.5  2006/02/05 17:14:43  strk
 * - New ConnectedSubgraphFinder class.
 * - More iterators returning methods, inlining and cleanups
 *   in planargraph.
 *
 * Revision 1.4  2004/10/19 19:51:14  strk
 * Fixed many leaks and bugs in Polygonizer.
 * Output still bogus.
 *
 * Revision 1.3  2004/10/13 10:03:02  strk
 * Added missing linemerge and polygonize operation.
 * Bug fixes and leaks removal from the newly added modules and
 * planargraph (used by them).
 * Some comments and indentation changes.
 *
 * Revision 1.2  2004/07/02 13:28:29  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.1  2004/04/04 06:29:11  ybychkov
 * "planargraph" and "geom/utill" upgraded to JTS 1.4
 *
 *
 **********************************************************************/
