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
 **********************************************************************
 * $Log$
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


#include <geos/planargraph.h>

namespace geos {

/**
* Constructs a NodeMap without any Nodes.
*/
planarNodeMap::planarNodeMap() {
	nodeMap=new map<Coordinate,planarNode*,planarCoordLT>();	
}

planarNodeMap::~planarNodeMap() {
	delete nodeMap;	
}

map<Coordinate,planarNode*,planarCoordLT>* planarNodeMap::getNodeMap() {
	return nodeMap;	
}

/**
* Adds a node to the map, replacing any that is already at that location.
* @return the added node
*/
planarNode* planarNodeMap::add(planarNode *n){
	nodeMap->insert(pair<Coordinate,planarNode*>(n->getCoordinate(),n));
	return n;
}

/**
* Removes the Node at the given location, and returns it (or null if no Node was there).
*/
planarNode* planarNodeMap::remove(Coordinate& pt){
	planarNode *n=find(pt);
	nodeMap->erase(pt);
	return n;
}

vector<planarNode*>* planarNodeMap::getNodes(){
	vector<planarNode*> *values=new vector<planarNode*>();
	map<Coordinate,planarNode*,planarCoordLT>::iterator it=nodeMap->begin();
	while(it!=nodeMap->end()) {
		values->push_back(it->second);
		it++;
	}
	return values;
}

/**
* Returns the Node at the given location, or null if no Node was there.
*/
planarNode* planarNodeMap::find(Coordinate& coord)  {
	map<Coordinate,planarNode*,planarCoordLT>::iterator found=nodeMap->find(coord);
	if (found==nodeMap->end())
		return NULL;
	else
		return found->second;
}

/**
* Returns an Iterator over the Nodes in this NodeMap, sorted in ascending order
* by angle with the positive x-axis.
*/
map<Coordinate,planarNode*,planarCoordLT>::iterator planarNodeMap::iterator(){
	return nodeMap->begin();
}

}
