/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2005-2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: operation/polygonize/PolygonizeGraph.java rev. 6/138 (JTS-1.10)
 *
 **********************************************************************/

#include <geos/operation/polygonize/PolygonizeGraph.h>
#include <geos/operation/polygonize/PolygonizeDirectedEdge.h>
#include <geos/operation/polygonize/PolygonizeEdge.h>
#include <geos/operation/polygonize/EdgeRing.h>
#include <geos/planargraph/Node.h>
#include <geos/planargraph/DirectedEdgeStar.h>
#include <geos/planargraph/DirectedEdge.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/LineString.h>

#include <cassert>
#include <vector>
#include <set>

//using namespace std;
using namespace geos::planargraph;
using namespace geos::geom;

namespace geos {
namespace operation { // geos.operation
namespace polygonize { // geos.operation.polygonize

int
PolygonizeGraph::getDegreeNonDeleted(Node *node) const
{
	std::vector<DirectedEdge*> &edges=node->getOutEdges()->getEdges();
	int degree=0;
	for (auto e : edges) {
		if (!dynamic_cast<PolygonizeDirectedEdge*>(e)->isMarked()) ++degree;
	}
	return degree;
}

int
PolygonizeGraph::getDegree(Node *node, long label) const
{
	std::vector<DirectedEdge*> &edges=node->getOutEdges()->getEdges();
	int degree=0;
	for (auto e : edges) {
		if (dynamic_cast<PolygonizeDirectedEdge*>(e)->getLabel() == label) ++degree;
	}
	return degree;
}

/**
 * marks them as Deleted
 */
void
PolygonizeGraph::deleteAllEdges(Node *node)
{
	std::vector<DirectedEdge*> &edges=node->getOutEdges()->getEdges();
  for (auto e : edges) {
		auto de = dynamic_cast<PolygonizeDirectedEdge*>(e);
		de->setMarked(true);
		if (de->getSym() != nullptr) de->getSym()->setMarked(true);
	}
}

/*
 * Create a new polygonization graph.
 */
PolygonizeGraph::PolygonizeGraph(const GeometryFactory *newFactory):
	factory(newFactory)
{
}

/*
 * Destroy a PolygonizeGraph
 */
PolygonizeGraph::~PolygonizeGraph()
{
	for (auto e : newEdges) delete e;
	for (auto e : newDirEdges) delete e;
	for (auto n : newNodes) delete n;
	for (auto e : newEdgeRings) delete e;
	for (auto c : newCoords) delete c;
}

/*
 * Add a LineString forming an edge of the polygon graph.
 * @param line the line to add
 */
void
PolygonizeGraph::addEdge(const LineString *line)
{
	if (line->isEmpty()) return;

	CoordinateSequence *linePts=CoordinateSequence::removeRepeatedPoints(line->getCoordinatesRO());

	/*
	 * This would catch invalid linestrings
	 * (containing duplicated points only)
	 */
	if ( linePts->getSize() < 2 )
	{
		delete linePts;
		return;
	}

	const Coordinate& startPt=linePts->getAt(0);
	const Coordinate& endPt=linePts->getAt(linePts->getSize()-1);
	Node *nStart=getNode(startPt);
	Node *nEnd=getNode(endPt);
	DirectedEdge *de0=new PolygonizeDirectedEdge(nStart, nEnd, linePts->getAt(1), true);
	newDirEdges.push_back(de0);
	DirectedEdge *de1=new PolygonizeDirectedEdge(nEnd, nStart,
			linePts->getAt(linePts->getSize()-2), false);
	newDirEdges.push_back(de1);
	Edge *edge=new PolygonizeEdge(line);
	newEdges.push_back(edge);
	edge->setDirectedEdges(de0, de1);
	add(edge);

	newCoords.push_back(linePts);
}

Node *
PolygonizeGraph::getNode(const Coordinate& pt)
{
	Node *node=findNode(pt);
	if (node==nullptr) {
		node=new Node(pt);
		newNodes.push_back(node);
		// ensure node is only added once to graph
		add(node);
	}
	return node;
}

void
PolygonizeGraph::computeNextCWEdges()
{
	typedef std::vector<Node*> Nodes;
	Nodes pns; getNodes(pns);
	// set the next pointers for the edges around each node
	for(Nodes::size_type i=0, in=pns.size(); i<in; ++i) {
		Node *node=pns[i];
		computeNextCWEdges(node);
	}
}

/* private */
void
PolygonizeGraph::convertMaximalToMinimalEdgeRings(
		std::vector<PolygonizeDirectedEdge*> &ringEdges)
{
	for (auto e : ringEdges) {
		auto de = dynamic_cast<PolygonizeDirectedEdge*>(e);
		auto label = de->getLabel();
		auto intNodes = findIntersectionNodes(de, label);
		for (auto n : intNodes) {
			computeNextCCWEdges(n, label);
		}
		intNodes.clear();
	}
}

std::vector<Node*>
PolygonizeGraph::findIntersectionNodes(
		PolygonizeDirectedEdge *startDE,
		long label) {
	std::vector<Node*> intNodes;
	auto de = startDE;
	do {
		auto node = de->getFromNode();
		if (getDegree(node, label) > 1) {
			intNodes.push_back(node);
		}
		de = de->getNext();
		assert(de != nullptr); // found NULL DE in ring
		assert(de == startDE || !de->isInRing()); // found DE already in ring
	} while (de != startDE);
	return intNodes;
}

/* public */
void
PolygonizeGraph::getEdgeRings(std::vector<EdgeRing*>& edgeRingList)
{
	// maybe could optimize this, since most of these pointers should
	// be set correctly already
	// by deleteCutEdges()
	computeNextCWEdges();

	// clear labels of all edges in graph
	label(dirEdges, -1);
	auto maximalRings = findLabeledEdgeRings(dirEdges);
	convertMaximalToMinimalEdgeRings(maximalRings);
	maximalRings.clear(); // not needed anymore

	// find all edgerings
	for (auto e : dirEdges) {
		auto de = dynamic_cast<PolygonizeDirectedEdge*>(e);

		if (de->isMarked()) continue;
		if (de->isInRing()) continue;

		auto er = findEdgeRing(de);
		edgeRingList.push_back(er);
	}
}

std::vector<PolygonizeDirectedEdge*>
PolygonizeGraph::findLabeledEdgeRings(
	std::vector<DirectedEdge*> &dirEdges) {

	std::vector<PolygonizeDirectedEdge*> edgeRingStarts;

	// label the edge rings formed
	long currLabel(1);
	for (auto e : dirEdges) {
		auto de = dynamic_cast<PolygonizeDirectedEdge*>(e);

		if (de->isMarked()) continue;
		if (de->getLabel() >= 0) continue;

		edgeRingStarts.push_back(de);

		auto edges = findDirEdgesInRing(de);

		label(edges, currLabel);

		++currLabel;
	}
	return edgeRingStarts;
}

/* public */
void
PolygonizeGraph::deleteCutEdges(std::vector<const LineString*> &cutLines)
{
	computeNextCWEdges();

	typedef std::vector<PolygonizeDirectedEdge*> DirEdges;

	// label the current set of edgerings
	/* even that is a find it has side efects on the lables */
	findLabeledEdgeRings(dirEdges); // ignoring the result

	/*
	 * Cut Edges are edges where both dirEdges have the same label.
	 * Delete them, and record them
	 */
	for (DirEdges::size_type i=0, in=dirEdges.size(); i<in; ++i)
	{
		DirectedEdge *de_ = dirEdges[i];
		PolygonizeDirectedEdge *de =
			dynamic_cast<PolygonizeDirectedEdge*>(de_);

		if (de->isMarked()) continue;

		DirectedEdge *sym_ = de->getSym();
		PolygonizeDirectedEdge *sym =
			dynamic_cast<PolygonizeDirectedEdge*>(sym_);

		if (de->getLabel()==sym->getLabel())
		{
			de->setMarked(true);
			sym->setMarked(true);

			// save the line as a cut edge
			Edge *e_ = de->getEdge();
			PolygonizeEdge *e = dynamic_cast<PolygonizeEdge*>(e_);

			cutLines.push_back(e->getLine());
		}
	}
}

void
PolygonizeGraph::label(
		std::vector<DirectedEdge*> &dirEdges,
	 	long label) {
	for(auto e : dirEdges) {
		dynamic_cast<PolygonizeDirectedEdge*>(e)->setLabel(label);
	}
}

void
PolygonizeGraph::computeNextCWEdges(Node *node)
{
	DirectedEdgeStar *deStar = node->getOutEdges();
	PolygonizeDirectedEdge *startDE = nullptr;
	PolygonizeDirectedEdge *prevDE = nullptr;

	// the edges are stored in CCW order around the star
	std::vector<DirectedEdge*> &pde=deStar->getEdges();
	for(auto e : pde) {
		auto outDE= dynamic_cast<PolygonizeDirectedEdge*>(e);
		if (outDE->isMarked()) continue;
		if (startDE == nullptr) startDE = outDE;
		if (prevDE != nullptr) {
			dynamic_cast<PolygonizeDirectedEdge*>(prevDE->getSym())->setNext(outDE);
		}
		prevDE = outDE;
	}
	if (prevDE != nullptr) {
		dynamic_cast<PolygonizeDirectedEdge*>(prevDE->getSym())->setNext(startDE);
	}
}

/**
 * Computes the next edge pointers going CCW around the given node, for the
 * given edgering label.
 * This algorithm has the effect of converting maximal edgerings into
 * minimal edgerings
 */
void
PolygonizeGraph::computeNextCCWEdges(Node *node, long label)
{
	auto deStar = node->getOutEdges();

	PolygonizeDirectedEdge *firstOutDE = nullptr;
	PolygonizeDirectedEdge *prevInDE = nullptr;

	// the edges are stored in CCW order around the star
	auto edges = deStar->getEdges();

	/*
	 * Must use a SIGNED int here to allow for beak condition
	 * to be true.
	 */
	for(auto i = edges.size(); i > 0; --i)
	{
		auto de=dynamic_cast<PolygonizeDirectedEdge*>(edges[i - 1]);
		auto sym=dynamic_cast<PolygonizeDirectedEdge*>(de->getSym());

		auto outDE = (de->getLabel() == label)? de : nullptr;
		auto inDE = (sym->getLabel() == label)? sym : nullptr;

		if (!outDE && !inDE) continue; // this edge is not in edgering

		if (inDE) prevInDE = inDE;
		if (outDE) {
			if (prevInDE) {
				prevInDE->setNext(outDE);
				prevInDE = nullptr;
			}
			if (!firstOutDE) firstOutDE = outDE;
		}
	}
	if (prevInDE) {
		assert(firstOutDE);
		prevInDE->setNext(firstOutDE);
	}
}

std::vector<DirectedEdge*>
PolygonizeGraph::findDirEdgesInRing(PolygonizeDirectedEdge *startDE) const {
	auto de = startDE;
	std::vector<DirectedEdge*> edges;
	do {
		edges.push_back(de);
		de = de->getNext();
		assert(de); // found NULL DE in ring
		assert(de == startDE || !de->isInRing()); // found DE already in ring
	} while (de != startDE);
	return edges;
}

EdgeRing *
PolygonizeGraph::findEdgeRing(PolygonizeDirectedEdge *startDE)
{
	auto de = startDE;
	EdgeRing *er = new EdgeRing(factory);
	// Now, when will we delete those EdgeRings ?
	newEdgeRings.push_back(er);
	do {
		er->add(de);
		de->setRing(er);
		de = de->getNext();
		assert(de); // found NULL DE in ring
		assert(de == startDE || !de->isInRing()); // found DE already in ring
	} while (de != startDE);
	return er;
}

/* public */
void
PolygonizeGraph::deleteDangles(std::vector<const LineString*>& dangleLines)
{
	std::vector<Node*> nodeStack;
	findNodesOfDegree(1, nodeStack);

	std::set<const LineString*> uniqueDangles;

	while (!nodeStack.empty()) {
		auto node = nodeStack.back();
		nodeStack.pop_back();
		deleteAllEdges(node);

		auto nodeOutEdges = node->getOutEdges()->getEdges();
		for(auto oe : nodeOutEdges)
		{
			auto de = dynamic_cast<PolygonizeDirectedEdge*>(oe);
			// delete this edge and its sym
			de->setMarked(true);
			auto sym = dynamic_cast<PolygonizeDirectedEdge*>(de->getSym());
			if (sym) sym->setMarked(true);
			// save the line as a dangle
			auto e = dynamic_cast<PolygonizeEdge*>(de->getEdge());
			auto ls = e->getLine();
			if (uniqueDangles.insert(ls).second) {
				dangleLines.push_back(ls);
			}

			auto toNode=de->getToNode();
			// add the toNode to the list to be processed,
			// if it is now a dangle
			if (getDegreeNonDeleted(toNode) == 1) {
				nodeStack.push_back(toNode);
			}
		}
	}
}

} // namespace geos.operation.polygonize
} // namespace geos.operation
} // namespace geos

