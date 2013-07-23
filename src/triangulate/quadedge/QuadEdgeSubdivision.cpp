/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2012 Excensus LLC.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: triangulate/quadedge/QuadEdgeSubdivision.java r524
 *
 **********************************************************************/
#include <geos/triangulate/quadedge/QuadEdgeSubdivision.h>

#include <vector>

#include <geos/geom/Polygon.h>
#include <geos/geom/LineSegment.h>
#include <geos/geom/LineString.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/CoordinateArraySequence.h>
#include <geos/geom/CoordinateSequenceFactory.h>
#include <geos/geom/CoordinateArraySequenceFactory.h>
#include <geos/geom/GeometryCollection.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/util/IllegalArgumentException.h>
#include <geos/util/GEOSException.h>
#include <geos/triangulate/quadedge/QuadEdge.h>
#include <geos/triangulate/quadedge/QuadEdgeLocator.h>
#include <geos/triangulate/quadedge/LastFoundQuadEdgeLocator.h>
#include <geos/triangulate/quadedge/LocateFailureException.h>
#include <geos/triangulate/quadedge/TriangleVisitor.h>

using namespace geos::geom;

namespace geos {
namespace triangulate { //geos.triangulate
namespace quadedge { //geos.triangulate.quadedge

void
QuadEdgeSubdivision::getTriangleEdges(const QuadEdge &startQE,
		const QuadEdge* triEdge[3])
{
	triEdge[0] = &startQE;
	triEdge[1] = &triEdge[0]->lNext();
	triEdge[2] = &triEdge[1]->lNext();
	if (&triEdge[2]->lNext() != triEdge[0]) {
		throw new
			util::IllegalArgumentException("Edges do not form a triangle");
	}
}

QuadEdgeSubdivision::QuadEdgeSubdivision(const geom::Envelope &env, double tolerance) :
		tolerance(tolerance),
		locator(new LastFoundQuadEdgeLocator(this))
{
	edgeCoincidenceTolerance = tolerance / EDGE_COINCIDENCE_TOL_FACTOR;
	createFrame(env);
	initSubdiv(startingEdges);
	quadEdges.push_back(startingEdges[0]);
	createdEdges.push_back(startingEdges[0]);
	quadEdges.push_back(startingEdges[1]);
	createdEdges.push_back(startingEdges[1]);
	quadEdges.push_back(startingEdges[2]);
	createdEdges.push_back(startingEdges[2]);
}

QuadEdgeSubdivision::~QuadEdgeSubdivision()
{
	for(QuadEdgeList::iterator iter=createdEdges.begin(); iter!=createdEdges.end(); ++iter)
	{
		(*iter)->free();
		delete *iter;
	}
}

void
QuadEdgeSubdivision::createFrame(const geom::Envelope &env)
{
	double deltaX = env.getWidth();
	double deltaY = env.getHeight();
	double offset = 0.0;
	if (deltaX > deltaY) {
		offset = deltaX * 10.0;
	} else {
		offset = deltaY * 10.0;
	}

	frameVertex[0] = Vertex((env.getMaxX() + env.getMinX()) / 2.0, env
			.getMaxY() + offset);
	frameVertex[1] = Vertex(env.getMinX() - offset, env.getMinY() - offset);
	frameVertex[2] = Vertex(env.getMaxX() + offset, env.getMinY() - offset);

	frameEnv = Envelope(frameVertex[0].getCoordinate(), frameVertex[1]
			.getCoordinate());
	frameEnv.expandToInclude(frameVertex[2].getCoordinate());
}
void
QuadEdgeSubdivision::initSubdiv(QuadEdge* initEdges[3])
{
	std::auto_ptr<QuadEdge> tmp_auto_ptr;
	// build initial subdivision from frame
	tmp_auto_ptr = QuadEdge::makeEdge(frameVertex[0], frameVertex[1]);
	initEdges[0] = tmp_auto_ptr.get();
	tmp_auto_ptr.release();


	tmp_auto_ptr = QuadEdge::makeEdge(frameVertex[1], frameVertex[2]);
	initEdges[1] = tmp_auto_ptr.get();
	tmp_auto_ptr.release();

	QuadEdge::splice(initEdges[0]->sym(), *initEdges[1]);

	tmp_auto_ptr = QuadEdge::makeEdge(frameVertex[2], frameVertex[0]);
	initEdges[2] = tmp_auto_ptr.get();
	tmp_auto_ptr.release();

	QuadEdge::splice(initEdges[1]->sym(), *initEdges[2]);
	QuadEdge::splice(initEdges[2]->sym(), *initEdges[0]);
}

QuadEdge&
QuadEdgeSubdivision::makeEdge(const Vertex &o, const Vertex &d)
{
	std::auto_ptr<QuadEdge> q0 = QuadEdge::makeEdge(o, d);
	QuadEdge *q0_ptr = q0.get();
	q0.release();

	createdEdges.push_back(q0_ptr);
	quadEdges.push_back(q0_ptr);
	return *q0_ptr;
}

QuadEdge&
QuadEdgeSubdivision::connect(QuadEdge &a, QuadEdge &b)
{
	std::auto_ptr<QuadEdge> q0 = QuadEdge::connect(a, b);
	QuadEdge *q0_ptr = q0.get();
	q0.release();

	createdEdges.push_back(q0_ptr);
	quadEdges.push_back(q0_ptr);
	return *q0_ptr;
}

void
QuadEdgeSubdivision::remove(QuadEdge &e)
{
	QuadEdge::splice(e, e.oPrev());
	QuadEdge::splice(e.sym(), e.sym().oPrev());

	// this is inefficient on an ArrayList, but this method should be called infrequently
	quadEdges.remove(&e);

	//mark these edges as removed
	e.remove();

}

QuadEdge*
QuadEdgeSubdivision::locateFromEdge(const Vertex &v,
		const QuadEdge &startEdge) const
{
	int iter = 0;
	int maxIter = quadEdges.size();

	QuadEdge *e = startingEdges[0];

	while (true) {
		++iter;
		/**
		 * So far it has always been the case that failure to locate indicates an
		 * invalid subdivision. So just fail completely. (An alternative would be
		 * to perform an exhaustive search for the containing triangle, but this
		 * would mask errors in the subdivision topology)
		 * 
		 * This can also happen if two vertices are located very close together,
		 * since the orientation predicates may experience precision failures.
		 */
		if (iter > maxIter) {
			throw LocateFailureException("");
		}

		if ((v.equals(e->orig())) || (v.equals(e->dest()))) {
			break;
		} else if (v.rightOf(*e)) {
			e = &e->sym();
		} else if (!v.rightOf(e->oNext())) {
			e = &e->oNext();
		} else if (!v.rightOf(e->dPrev())) {
			e = &e->dPrev();
		} else {
			// on edge or in triangle containing edge
			break;
		}
	}
	return e;
}

QuadEdge*
QuadEdgeSubdivision::locate(const Coordinate &p0, const Coordinate &p1)
{
	// find an edge containing one of the points
	QuadEdge *e = locator->locate(Vertex(p0));
	if (e == NULL)
		return NULL;

	// normalize so that p0 is origin of base edge
	QuadEdge *base = e;
	if (e->dest().getCoordinate().equals2D(p0))
		base = &e->sym();
	// check all edges around origin of base edge
	QuadEdge *locEdge = base;
	do {
		if (locEdge->dest().getCoordinate().equals2D(p1))
			return locEdge;
		locEdge = &locEdge->oNext();
	} while (locEdge != base);
	return NULL;
}

QuadEdge&
QuadEdgeSubdivision::insertSite(const Vertex &v)
{
	QuadEdge *e = locate(v);

	if ((v.equals(e->orig(), tolerance)) || (v.equals(e->dest(), tolerance))) {
		return *e; // point already in subdivision.
	}

	// Connect the new point to the vertices of the containing
	// triangle (or quadrilateral, if the new point fell on an
	// existing edge.)
	QuadEdge *base = &makeEdge(e->orig(), v);
	QuadEdge::splice(*base, *e);
	QuadEdge *startEdge = base;
	do {
		base = &connect(*e, base->sym());
		e = &base->oPrev();
	} while (&e->lNext() != startEdge);

	return *startEdge;
}

bool
QuadEdgeSubdivision::isFrameEdge(const QuadEdge &e) const
{
	if (isFrameVertex(e.orig()) || isFrameVertex(e.dest()))
		return true;
	return false;
}

bool
QuadEdgeSubdivision::isFrameBorderEdge(const QuadEdge &e) const
{
	// check other vertex of triangle to left of edge
	Vertex vLeftTriOther = e.lNext().dest();
	if (isFrameVertex(vLeftTriOther))
		return true;
	// check other vertex of triangle to right of edge
	Vertex vRightTriOther = e.sym().lNext().dest();
	if (isFrameVertex(vRightTriOther))
		return true;

	return false;
}

bool
QuadEdgeSubdivision::isFrameVertex(const Vertex &v) const
{
	if (v.equals(frameVertex[0]))
		return true;
	if (v.equals(frameVertex[1]))
		return true;
	if (v.equals(frameVertex[2]))
		return true;
	return false;
}

bool
QuadEdgeSubdivision::isOnEdge(const QuadEdge &e, const Coordinate &p) const
{
	geom::LineSegment seg;
	seg.setCoordinates(e.orig().getCoordinate(), e.dest().getCoordinate());
	double dist = seg.distance(p);
	// heuristic (hack?)
	return dist < edgeCoincidenceTolerance;
}

bool
QuadEdgeSubdivision::isVertexOfEdge(const QuadEdge &e, const Vertex &v) const
{
	if ((v.equals(e.orig(), tolerance)) || (v.equals(e.dest(), tolerance))) {
		return true;
	}
	return false;
}

std::auto_ptr<QuadEdgeSubdivision::QuadEdgeList>
QuadEdgeSubdivision::getPrimaryEdges(bool includeFrame)
{
	QuadEdgeList *edges = new QuadEdgeList();
	QuadEdgeStack edgeStack;
	QuadEdgeSet visitedEdges;

	edgeStack.push(startingEdges[0]);

	while (!edgeStack.empty())
	{
		QuadEdge *edge = edgeStack.top();
		edgeStack.pop();
		if (visitedEdges.find(edge) == visitedEdges.end())
		{
			QuadEdge* priQE = (QuadEdge*)&edge->getPrimary();

			if (includeFrame || ! isFrameEdge(*priQE))
				edges->push_back(priQE);

			edgeStack.push(&edge->oNext());
			edgeStack.push(&edge->sym().oNext());

			visitedEdges.insert(edge);
			visitedEdges.insert(&edge->sym());
		}
	}
	return std::auto_ptr<QuadEdgeList>(edges);
}

QuadEdge**
QuadEdgeSubdivision::fetchTriangleToVisit(QuadEdge *edge,
		QuadEdgeStack &edgeStack, bool includeFrame, QuadEdgeSet &visitedEdges)
{
	QuadEdge *curr = edge;
	int edgeCount = 0;
	bool isFrame = false;
	do
	{
		triEdges[edgeCount] = curr;

		if (isFrameEdge(*curr))
			isFrame = true;

		// push sym edges to visit next
		QuadEdge *sym = &curr->sym();
		if (visitedEdges.find(sym) == visitedEdges.end())
			edgeStack.push(sym);

		// mark this edge as visited
		visitedEdges.insert(curr);

		edgeCount++;
		curr = &curr->lNext();

	} while (curr != edge);

	if (isFrame && !includeFrame)
		return NULL;
	return triEdges;
}

class
QuadEdgeSubdivision::TriangleCoordinatesVisitor : public TriangleVisitor {
private:
	QuadEdgeSubdivision::TriList *triCoords;
	CoordinateArraySequenceFactory coordSeqFact;

public:
	TriangleCoordinatesVisitor(QuadEdgeSubdivision::TriList *triCoords): triCoords(triCoords)
	{
	}

	void visit(QuadEdge* triEdges[3])
	{
		geom::CoordinateSequence *coordSeq = coordSeqFact.create(4,0);
		for (int i = 0; i < 3; i++) {
			Vertex v = triEdges[i]->orig();
			coordSeq->setAt(v.getCoordinate(), i);
		}
		coordSeq->setAt(triEdges[0]->orig().getCoordinate(), 3);
		triCoords->push_back(coordSeq);
	}
}; 

void
QuadEdgeSubdivision::getTriangleCoordinates(QuadEdgeSubdivision::TriList* triList, bool includeFrame)
{
	TriangleCoordinatesVisitor visitor(triList);
	visitTriangles((TriangleVisitor*)&visitor, includeFrame);
}

void
QuadEdgeSubdivision::visitTriangles(TriangleVisitor *triVisitor, bool includeFrame)
{

	QuadEdgeStack edgeStack;
	edgeStack.push(startingEdges[0]);

	QuadEdgeSet visitedEdges;

	while (!edgeStack.empty()) {
		QuadEdge *edge = edgeStack.top();
		edgeStack.pop();
		if (visitedEdges.find(edge) == visitedEdges.end()) {
			QuadEdge **triEdges = fetchTriangleToVisit(edge, edgeStack,
					includeFrame, visitedEdges);
			if (triEdges != NULL)
				triVisitor->visit(triEdges);
		}
	}
}

std::auto_ptr<geom::MultiLineString>
QuadEdgeSubdivision::getEdges(const geom::GeometryFactory& geomFact)
{
	std::auto_ptr<QuadEdgeList> quadEdges(getPrimaryEdges(false));
	std::vector<Geometry *> edges(quadEdges->size());
	const CoordinateSequenceFactory *coordSeqFact = geomFact.getCoordinateSequenceFactory();
	int i = 0;
	for (QuadEdgeSubdivision::QuadEdgeList::iterator it = quadEdges->begin(); it != quadEdges->end(); ++it)
	{
		QuadEdge *qe = *it;
		CoordinateSequence *coordSeq = coordSeqFact->create((std::vector<geom::Coordinate>*)NULL);;

		coordSeq->add(qe->orig().getCoordinate());
		coordSeq->add(qe->dest().getCoordinate());

		edges[i++] = static_cast<Geometry*>(geomFact.createLineString(*coordSeq));

		delete coordSeq;
	}

	geom::MultiLineString* result = geomFact.createMultiLineString(edges);

	for(std::vector<Geometry*>::iterator it=edges.begin(); it!=edges.end(); ++it)
		delete *it;

	return std::auto_ptr<MultiLineString>(result);
}

std::auto_ptr<GeometryCollection>
QuadEdgeSubdivision::getTriangles( const GeometryFactory &geomFact)
{
	TriList triPtsList;
	getTriangleCoordinates(&triPtsList, false);
	std::vector<Geometry*> tris;

	for(TriList::const_iterator it = triPtsList.begin();
			it != triPtsList.end(); ++it)
	{
		CoordinateSequence *coordSeq = *it;
		Polygon *tri = geomFact.createPolygon(
				geomFact.createLinearRing(coordSeq), NULL);
		tris.push_back(static_cast<Geometry*>(tri));
	}
	GeometryCollection* ret =  geomFact.createGeometryCollection(tris);

	//release memory
	for(std::vector<Geometry*>::iterator it=tris.begin(); it!=tris.end(); ++it)
		delete *it;
	tris.clear();

	return std::auto_ptr<GeometryCollection>(ret);
}

} //namespace geos.triangulate.quadedge
} //namespace geos.triangulate
} //namespace goes
