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
 **********************************************************************/


#include <geos/geomgraph.h>
#include <geos/indexQuadtree.h>
#include <geos/profiler.h>

#ifndef DEBUG
#define DEBUG 0
#endif

namespace geos {

#if PROFILE
static Profiler *profiler = Profiler::instance();
#endif

EdgeList::EdgeList(){
	edges=new vector<Edge*>();
	index=new Quadtree();
}

EdgeList::~EdgeList(){
	delete edges;
	delete index;
}
/**
 * Insert an edge unless it is already in the list
 */
void EdgeList::add(Edge *e) {
	edges->push_back(e);
	index->insert(e->getEnvelope(),e);
}

void EdgeList::addAll(vector<Edge*> *edgeColl) {
	for (int i=0; i<(int)edgeColl->size();i++) {
		add((*edgeColl)[i]);
	}
}

vector<Edge*>* EdgeList::getEdges() {
	return edges;
}

// <FIX> fast lookup for edges
/**
 * If there is an edge equal to e already in the list, return it.
 * Otherwise return null.
 * @return  equal edge, if there is one already in the list
 *          null otherwise
 */
Edge *
EdgeList::findEqualEdge(Edge *e)
{
#if PROFILE
	static Profile *prof = profiler->get("EdgeList::findEqualEdge(Edge *e)");
	prof->start();
#endif
	vector<void*> *testEdges=index->query(e->getEnvelope());
#if PROFILE
	prof->stop();
#endif

#if DEBUG
	cerr<<"EdgeList::findEqualEdge found "<<testEdges->size()<<" overlapping edges"<<endl;
#endif

	for (int i=0; i<(int)testEdges->size();i++) {
		Edge* testEdge=(Edge*) (*testEdges)[i];
		if (testEdge->equals(e))
		{
			delete testEdges;
			return testEdge;
		}
	}
	delete testEdges;
	return NULL;
}

Edge* EdgeList::get(int i) {
	return (*edges)[i];
}

/**
 * If the edge e is already in the list, return its index.
 * @return  index, if e is already in the list
 *          -1 otherwise
 */
int EdgeList::findEdgeIndex(Edge *e) {
	for (int i=0; i<(int)edges->size();i++) {
		if ( (*edges)[i]->equals(e) )
            return i;
	}
	return -1;
}

string EdgeList::print() {
	string out="EdgeList( ";
	for(unsigned int j=0; j<edges->size();j++)
	{
       		Edge *e=(*edges)[j];
		if (j) out+=",";
		out += e->print();
	}
	out+=")  ";
	return out;
}

} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.7  2005/02/01 13:44:59  strk
 * More profiling labels.
 *
 * Revision 1.6  2004/11/22 11:34:49  strk
 * More debugging lines and comments/indentation cleanups
 *
 * Revision 1.5  2004/11/01 16:43:04  strk
 * Added Profiler code.
 * Temporarly patched a bug in DoubleBits (must check drawbacks).
 * Various cleanups and speedups.
 *
 * Revision 1.4  2004/07/08 19:34:49  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added DefaultCoordinateSequenceFactory::instance() function.
 *
 * Revision 1.3  2004/07/02 13:28:26  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.2  2004/05/03 22:56:44  strk
 * leaks fixed, exception specification omitted.
 *
 * Revision 1.1  2004/03/19 09:48:45  ybychkov
 * "geomgraph" and "geomgraph/indexl" upgraded to JTS 1.4
 *
 * Revision 1.14  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/
