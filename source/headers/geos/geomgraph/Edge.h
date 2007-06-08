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


#ifndef GEOS_GEOMGRAPH_EDGE_H
#define GEOS_GEOMGRAPH_EDGE_H

#include <string>
#include <cassert>

#include <geos/geomgraph/GraphComponent.h> // for inheritance
#include <geos/geomgraph/Depth.h> // for member
#include <geos/geomgraph/EdgeIntersectionList.h> // for composition
#include <geos/geom/CoordinateSequence.h> // for inlines

#include <geos/inline.h>

// Forward declarations
namespace geos {
	namespace geom {
		class Envelope;
		class IntersectionMatrix;
		class Coordinate;
	}
	namespace algorithm {
		class LineIntersector;
	}
	namespace geomgraph {
		class Node;
		class EdgeEndStar;
		class Label;
		class NodeFactory;
		namespace index {
			class MonotoneChainEdge;
		}
	}
}

namespace geos {
namespace geomgraph { // geos.geomgraph

class Edge: public GraphComponent{
using GraphComponent::updateIM;

private:

	std::string name;

	/// Lazily-created, owned by Edge.
	index::MonotoneChainEdge *mce;

	/// Lazily-created, owned by Edge.
	geom::Envelope *env;

	bool isIsolatedVar;

	Depth depth;

	int depthDelta;   // the change in area depth from the R to L side of this edge

public:

	void testInvariant() const {
		assert(pts);
		assert(pts->size() > 1);
	}


	friend std::ostream& operator<< (std::ostream& os, const Edge& el);

	static void updateIM(Label *lbl,geom::IntersectionMatrix *im);

	/// Externally-set, owned by Edge. FIXME: refuse ownership
	geom::CoordinateSequence* pts;

	EdgeIntersectionList eiList;

	//Edge();

	Edge(geom::CoordinateSequence* newPts, Label *newLabel);

	Edge(geom::CoordinateSequence* newPts);

	virtual ~Edge();

	virtual int getNumPoints() const {
		return static_cast<int>(pts->getSize());
	}

	virtual void setName(const std::string &newName) {
		name=newName;
	}

	virtual const geom::CoordinateSequence* getCoordinates() const {
		testInvariant();
		return pts;
	}

	virtual const geom::Coordinate& getCoordinate(int i) const {
		testInvariant();
		return pts->getAt(i);
	}

	virtual const geom::Coordinate& getCoordinate() const {
		testInvariant();
		return pts->getAt(0);
	}


	virtual Depth &getDepth() { 
		testInvariant();
		return depth;
	}

	/** \brief
	 * The depthDelta is the change in depth as an edge is crossed from R to L
	 *
	 * @return the change in depth as the edge is crossed from R to L
	 */
	virtual int getDepthDelta() const {
		testInvariant();
		return depthDelta;
	}

	virtual void setDepthDelta(int newDepthDelta) {
		depthDelta=newDepthDelta;
		testInvariant();
	}

	virtual int getMaximumSegmentIndex() const {
		testInvariant();
		return getNumPoints()-1;
	}

	virtual EdgeIntersectionList& getEdgeIntersectionList() {
		testInvariant();
		return eiList;
	}

	/// \brief
	/// Return this Edge's index::MonotoneChainEdge,
	/// ownership is retained by this object.
	///
	virtual index::MonotoneChainEdge* getMonotoneChainEdge();

	virtual bool isClosed() const {
		testInvariant();
		return pts->getAt(0)==pts->getAt(getNumPoints()-1);
	}

	/** \brief
	 * An Edge is collapsed if it is an Area edge and it consists of
	 * two segments which are equal and opposite (eg a zero-width V).
	 */
	virtual bool isCollapsed() const;

	virtual Edge* getCollapsedEdge();

	virtual void setIsolated(bool newIsIsolated) {
		isIsolatedVar=newIsIsolated;
		testInvariant();
	}

	virtual bool isIsolated() const {
		testInvariant();
		return isIsolatedVar;
	}

	/** \brief
	 * Adds EdgeIntersections for one or both
	 * intersections found for a segment of an edge to the edge intersection list.
	 */
	virtual void addIntersections(algorithm::LineIntersector *li, int segmentIndex,
		int geomIndex);

	/// Add an EdgeIntersection for intersection intIndex.
	//
	/// An intersection that falls exactly on a vertex of the edge is normalized
	/// to use the higher of the two possible segmentIndexes
	///
	virtual void addIntersection(algorithm::LineIntersector *li, int segmentIndex,
		int geomIndex, int intIndex);

	/// Update the IM with the contribution for this component.
	//
	/// A component only contributes if it has a labelling for both parent geometries
	///
	virtual void computeIM(geom::IntersectionMatrix *im) {
		updateIM(label, im);
		testInvariant();
	}

	/// return true if the coordinate sequences of the Edges are identical
	virtual bool isPointwiseEqual(const Edge *e) const;

	virtual std::string print() const;

	virtual std::string printReverse() const;

	/**
	 * equals is defined to be:
	 * 
	 * e1 equals e2
	 * <b>iff</b>
	 * the coordinates of e1 are the same or the reverse of the coordinates in e2
	 */
	virtual bool equals(const Edge& e) const;

	virtual bool equals(const Edge* e) const {
		assert(e);
		return equals(*e);
	}

	virtual geom::Envelope* getEnvelope();
};


//Operators
inline bool operator==(const Edge &a, const Edge &b) {
	return a.equals(b);
}

std::ostream& operator<< (std::ostream& os, const Edge& el);


} // namespace geos.geomgraph
} // namespace geos

//#ifdef GEOS_INLINE
//# include "geos/geomgraph/Edge.inl"
//#endif

#endif // ifndef GEOS_GEOMGRAPH_EDGE_H

/**********************************************************************
 * $Log$
 * Revision 1.4  2006/04/05 18:28:42  strk
 * Moved testInvariant() methods from private to public, added
 * some comments about them.
 *
 * Revision 1.3  2006/03/24 09:52:41  strk
 * USE_INLINE => GEOS_INLINE
 *
 * Revision 1.2  2006/03/14 11:03:14  strk
 * Added operator<< for Edge and EdgeList
 *
 * Revision 1.1  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/

