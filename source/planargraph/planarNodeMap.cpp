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
 * the terms of the GNU Lesser General Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/planargraph.h>

namespace geos {
//namespace planargraph {

/**
 * Constructs a NodeMap without any Nodes.
 */
planarNodeMap::planarNodeMap()
{
	//nodeMap=new map<Coordinate,planarNode*,planarCoordLT>();	
}

planarNodeMap::~planarNodeMap()
{
	//delete nodeMap;	
}

map<Coordinate,planarNode*,planarCoordLT>&
planarNodeMap::getNodeMap()
{
	return nodeMap;	
}

/**
 * Adds a node to the map, replacing any that is already at that location.
 * @return the added node
 */
planarNode*
planarNodeMap::add(planarNode *n)
{
	nodeMap.insert(pair<Coordinate,planarNode*>(n->getCoordinate(),n));
	return n;
}

/**
 * Removes the Node at the given location, and returns it
 * (or null if no Node was there).
 */
planarNode *
planarNodeMap::remove(Coordinate& pt)
{
	planarNode *n=find(pt);
	nodeMap.erase(pt);
	return n;
}

vector<planarNode*>*
planarNodeMap::getNodes()
{
	vector<planarNode*> *values=new vector<planarNode*>();
	map<Coordinate,planarNode*,planarCoordLT>::iterator it=nodeMap.begin();
	while(it!=nodeMap.end()) {
		values->push_back(it->second);
		++it;
	}
	return values;
}

/**
 * Returns the Node at the given location, or null if no Node was there.
 */
planarNode*
planarNodeMap::find(const Coordinate& coord)
{
	map<Coordinate,planarNode*,planarCoordLT>::iterator found=nodeMap.find(coord);
	if (found==nodeMap.end())
		return NULL;
	else
		return found->second;
}

//} //namespace planargraph 
} //namespace geos 

/**********************************************************************
 * $Log$
 * Revision 1.8  2006/02/05 17:14:43  strk
 * - New ConnectedSubgraphFinder class.
 * - More iterators returning methods, inlining and cleanups
 *   in planargraph.
 *
 * Revision 1.7  2005/11/15 12:14:05  strk
 * Reduced heap allocations, made use of references when appropriate,
 * small optimizations here and there.
 *
 * Revision 1.6  2004/10/19 19:51:14  strk
 * Fixed many leaks and bugs in Polygonizer.
 * Output still bogus.
 *
 * Revision 1.5  2004/10/13 10:03:02  strk
 * Added missing linemerge and polygonize operation.
 * Bug fixes and leaks removal from the newly added modules and
 * planargraph (used by them).
 * Some comments and indentation changes.
 *
 * Revision 1.4  2004/07/02 13:28:29  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.3  2004/05/03 12:09:22  strk
 * newline added at end of file
 *
 * Revision 1.2  2004/04/07 06:55:50  ybychkov
 * "operation/linemerge" ported from JTS 1.4
 *
 * Revision 1.1  2004/04/04 06:29:11  ybychkov
 * "planargraph" and "geom/utill" upgraded to JTS 1.4
 *
 *
 **********************************************************************/

