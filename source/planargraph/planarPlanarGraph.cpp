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


/*
 * Adds the Edge and its DirectedEdges with this PlanarGraph.
 * Assumes that the Edge has already been created with its associated
 * DirectEdges.
 * Only subclasses can add Edges, to ensure the edges added are of
 * the right class.
 */
void
planarPlanarGraph::add(planarEdge *edge)
{
	edges.push_back(edge);
	add(edge->getDirEdge(0));
	add(edge->getDirEdge(1));
}


/*
 * Removes an Edge and its associated DirectedEdges from their from-Nodes and
 * from this PlanarGraph. Note: This method does not remove the Nodes associated
 * with the Edge, even if the removal of the Edge reduces the degree of a
 * Node to zero.
 */
void
planarPlanarGraph::remove(planarEdge *edge)
{
	remove(edge->getDirEdge(0));
	remove(edge->getDirEdge(1));
	for(unsigned int i=0; i<edges.size();++i)
	{
		if(edges[i]==edge) {
			edges.erase(edges.begin()+i);
			--i;
		}
	}
}

/*
 * Removes DirectedEdge from its from-Node and from this PlanarGraph. Note:
 * This method does not remove the Nodes associated with the DirectedEdge,
 * even if the removal of the DirectedEdge reduces the degree of a Node to
 * zero.
 */
void
planarPlanarGraph::remove(planarDirectedEdge *de)
{
	planarDirectedEdge *sym = de->getSym();
	if (sym!=NULL) sym->setSym(NULL);
	de->getFromNode()->getOutEdges()->remove(de);
	for(unsigned int i=0; i<dirEdges.size(); ++i) {
		if(dirEdges[i]==de) {
			dirEdges.erase(dirEdges.begin()+i);
			--i;
		}
	}
}

/*
 * Removes a node from the graph, along with any associated
 * DirectedEdges and Edges.
 */
void
planarPlanarGraph::remove(planarNode *node)
{
	// unhook all directed edges
	vector<planarDirectedEdge*> &outEdges=node->getOutEdges()->getEdges();
	for(unsigned int i=0; i<outEdges.size(); ++i) {
		planarDirectedEdge *de =outEdges[i];
		planarDirectedEdge *sym = de->getSym();
		// remove the diredge that points to this node
		if (sym!=NULL) remove(sym);
		// remove this diredge from the graph collection
		for(unsigned int j=0; j<dirEdges.size(); ++j) {
			if (dirEdges[j]==de) {
				dirEdges.erase(dirEdges.begin()+j);
				--j;
			}
		}
		planarEdge *edge=de->getEdge();
		if (edge!=NULL) {
			for(unsigned int k=0; k<edges.size(); ++k) {
				if(edges[k]==edge) {
					edges.erase(edges.begin()+k);
					--k;
				}
			}
		}
	}
	// remove the node from the graph
	nodeMap.remove(node->getCoordinate());
	//nodes.remove(node);
}

/*
 * Returns all Nodes with the given number of Edges around it.
 * The return value is a newly allocated vector of existing nodes
 */
vector<planarNode*>*
planarPlanarGraph::findNodesOfDegree(int degree)
{
	vector<planarNode*> *nodesFound=new vector<planarNode*>();
	map<Coordinate,planarNode*,planarCoordLT> &nm=nodeMap.getNodeMap();
	map<Coordinate,planarNode*,planarCoordLT>::iterator it=nm.begin();
	for ( ; it!=nm.end(); ++it) {
		planarNode *node=it->second;
		if (node->getDegree()==degree)
			nodesFound->push_back(node);
	}
	return nodesFound;
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
 * Revision 1.4  2005/11/15 12:14:05  strk
 * Reduced heap allocations, made use of references when appropriate,
 * small optimizations here and there.
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
 * Revision 1.1  2004/04/07 06:55:50  ybychkov
 * "operation/linemerge" ported from JTS 1.4
 *
 * Revision 1.1  2004/04/04 06:29:11  ybychkov
 * "planargraph" and "geom/utill" upgraded to JTS 1.4
 *
 *
 **********************************************************************/

