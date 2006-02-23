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

#include <typeinfo>
#include <cassert>
#include <functional>
#include <geos/opLinemerge.h>
#include <geos/planargraph.h>
#include <geos/util.h>

using namespace geos::planargraph;

namespace geos {
namespace operation { // geos.operation
namespace linemerge { // geos.operation.linemerge

void
LineMerger::add(vector<Geometry*> *geometries)
{
	for(unsigned int i=0, n=geometries->size(); i<n; i++) {
		Geometry *geometry=(*geometries)[i];
		add(geometry);
	}
}

LineMerger::LineMerger():
	mergedLineStrings(NULL),
	factory(NULL)
{
}

LineMerger::~LineMerger()
{
	for (unsigned int i=0, n=edgeStrings.size(); i<n; ++i) {
		delete edgeStrings[i];
	}
}


struct LMGeometryComponentFilter: public GeometryComponentFilter {
	LineMerger *lm;

	LMGeometryComponentFilter(LineMerger *newLm): lm(newLm) {}

	void filter(const Geometry *geom) {
		const LineString *ls = dynamic_cast<const LineString *>(geom);
		if ( ls ) lm->add(ls);
	}
};


/**
 * Adds a Geometry to be processed. May be called multiple times.
 * Any dimension of Geometry may be added; the constituent linework will be
 * extracted.
 */  
void
LineMerger::add(const Geometry *geometry)
{
	LMGeometryComponentFilter lmgcf(this);
	geometry->applyComponentFilter(lmgcf);
}

void
LineMerger::add(const LineString *lineString)
{
	if (factory==NULL) factory=lineString->getFactory();
	graph.addEdge(lineString);
}

void
LineMerger::merge()
{
	if (mergedLineStrings!=NULL) return;

	buildEdgeStringsForObviousStartNodes();
	buildEdgeStringsForIsolatedLoops();

	unsigned numEdgeStrings = edgeStrings.size();
	mergedLineStrings=new vector<LineString*>(numEdgeStrings);
	for (unsigned int i=0; i<numEdgeStrings; ++i)
	{
		EdgeString *edgeString=edgeStrings[i];
		(*mergedLineStrings)[i]=edgeString->toLineString();
	}    
}

void
LineMerger::buildEdgeStringsForObviousStartNodes()
{
	buildEdgeStringsForNonDegree2Nodes();
}

void
LineMerger::buildEdgeStringsForIsolatedLoops()
{
	buildEdgeStringsForUnprocessedNodes();
}  

void
LineMerger::buildEdgeStringsForUnprocessedNodes()
{
	vector<planarNode*> *nodes=graph.getNodes();
	for (unsigned int i=0; i<nodes->size(); ++i) {
		planarNode *node=(*nodes)[i];
		if (!node->isMarked()) { 
			assert(node->getDegree()==2);
			buildEdgeStringsStartingAt(node);
			node->setMarked(true);
		}
	}
	delete nodes;
}

void
LineMerger::buildEdgeStringsForNonDegree2Nodes()
{
	vector<planarNode*> *nodes=graph.getNodes();
	unsigned int size=nodes->size();
	for (unsigned int i=0; i<size; i++) {
		planarNode *node=(*nodes)[i];
		if (node->getDegree()!=2) { 
			buildEdgeStringsStartingAt(node);
			node->setMarked(true);
		}
	}
	delete nodes;
}

void
LineMerger::buildEdgeStringsStartingAt(planarNode *node)
{
	vector<planarDirectedEdge*> &edges=node->getOutEdges()->getEdges();
	unsigned int size = edges.size();
	for (unsigned int i=0; i<size; i++)
	{
		LineMergeDirectedEdge *directedEdge=(LineMergeDirectedEdge*) edges[i];
		if (directedEdge->getEdge()->isMarked()) {
			continue;
		}
		edgeStrings.push_back(buildEdgeStringStartingWith(directedEdge));
	}
}

EdgeString*
LineMerger::buildEdgeStringStartingWith(LineMergeDirectedEdge *start)
{    
	EdgeString *edgeString = new EdgeString(factory);
	LineMergeDirectedEdge *current=start;
	do {
		edgeString->add(current);
		current->getEdge()->setMarked(true);
		current=current->getNext();      
	} while (current!=NULL && current!=start);
	return edgeString;
}

/**
 * Returns the LineStrings built by the merging process.
 */
vector<LineString*>*
LineMerger::getMergedLineStrings()
{
	merge();
	return mergedLineStrings;
}

} // namespace geos.operation.linemerge
} // namespace geos.operation
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.9  2006/02/23 23:17:52  strk
 * - Coordinate::nullCoordinate made private
 * - Simplified Coordinate inline definitions
 * - LMGeometryComponentFilter definition moved to LineMerger.cpp file
 * - Misc cleanups
 *
 * Revision 1.8  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.7  2005/11/15 12:14:05  strk
 * Reduced heap allocations, made use of references when appropriate,
 * small optimizations here and there.
 *
 * Revision 1.6  2005/09/26 11:01:32  strk
 * Const correctness changes in LineMerger package, and a few speedups.
 *
 * Revision 1.5  2005/09/23 17:20:13  strk
 * Made LineMerger graph be a real object (rather then a pointer to it)
 *
 * Revision 1.4  2004/12/08 13:54:44  strk
 * gcc warnings checked and fixed, general cleanups.
 *
 * Revision 1.3  2004/10/13 10:03:02  strk
 * Added missing linemerge and polygonize operation.
 * Bug fixes and leaks removal from the newly added modules and
 * planargraph (used by them).
 * Some comments and indentation changes.
 *
 * Revision 1.2  2004/07/02 13:28:28  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.1  2004/04/07 06:55:50  ybychkov
 * "operation/linemerge" ported from JTS 1.4
 *
 *
 **********************************************************************/
 

