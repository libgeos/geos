/**********************************************************************
 * $Id: NodeMap.cpp 2733 2009-11-20 19:58:33Z strk $
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

#include <geos/planargraph/NodeMap.h>
#include <geos/planargraph/Node.h>

#include <map>

using namespace std;

namespace geos {
namespace planargraph {

/**
 * Constructs a NodeMap without any Nodes.
 */
NodeMap::NodeMap()
{
}

NodeMap::~NodeMap()
{
}

NodeMap::container& 
NodeMap::getNodeMap()
{
	return nodeMap;	
}

/**
 * Adds a node to the map, replacing any that is already at that location.
 * @return the added node
 */
Node*
NodeMap::add(Node *n)
{
	nodeMap.insert(pair<geom::Coordinate, Node*>(n->getCoordinate(),n));
	return n;
}

/**
 * Removes the Node at the given location, and returns it
 * (or null if no Node was there).
 */
Node *
NodeMap::remove(geom::Coordinate& pt)
{
	Node *n=find(pt);
	nodeMap.erase(pt);
	return n;
}

/* public */
void
NodeMap::getNodes(vector<Node*>& values)
{
	NodeMap::container::iterator it=nodeMap.begin(), itE=nodeMap.end();
	while (it != itE) {
		values.push_back(it->second);
		++it;
	}
}

/**
 * Returns the Node at the given location, or null if no Node was there.
 */
Node*
NodeMap::find(const geom::Coordinate& coord)
{
	container::iterator found=nodeMap.find(coord);
	if (found==nodeMap.end())
		return NULL;
	else
		return found->second;
}

} //namespace planargraph 
} //namespace geos 

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/03/21 21:42:54  strk
 * planargraph.h header split, planargraph:: classes renamed to match JTS symbols
 *
 **********************************************************************/

