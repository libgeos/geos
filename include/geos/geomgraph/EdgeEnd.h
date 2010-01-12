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
 **********************************************************************
 *
 * Last port: geomgraph/EdgeEnd.java rev. 1.6 (JTS-1.10)
 *
 **********************************************************************/


#ifndef GEOS_GEOMGRAPH_EDGEEND_H
#define GEOS_GEOMGRAPH_EDGEEND_H

#include <geos/export.h>
#include <geos/geom/Coordinate.h>  // for p0,p1
#include <geos/inline.h>

#include <string>

// Forward declarations
namespace geos {
	namespace algorithm {
		class BoundaryNodeRule;
	}
	namespace geomgraph {
		class Label;
		class Edge;
		class Node;
	}
}

namespace geos {
namespace geomgraph { // geos.geomgraph

/** \brief
 * Models the end of an edge incident on a node.
 *
 * EdgeEnds have a direction
 * determined by the direction of the ray from the initial
 * point to the next point.
 * EdgeEnds are comparable under the ordering
 * "a has a greater angle with the x-axis than b".
 * This ordering is used to sort EdgeEnds around a node.
 */
class GEOS_DLL EdgeEnd {

public:

	friend std::ostream& operator<< (std::ostream&, const EdgeEnd&);

	EdgeEnd();

	virtual ~EdgeEnd();

	/**
	 * NOTES:
	 *  - takes ownership of given Label (if any)
	 *  - keeps a pointer to given Edge, make sure it's
	 *    not deleted before the EdgeEnd.
	 *  - copies given Coordinates (maybe we should avoid that)
	 */
	EdgeEnd(Edge* newEdge, const geom::Coordinate& newP0,
			const geom::Coordinate& newP1,
			Label* newLabel=NULL);

	Edge* getEdge() { return edge; }
	//virtual Edge* getEdge() { return edge; }

	virtual Label* getLabel();

	virtual geom::Coordinate& getCoordinate();

	virtual geom::Coordinate& getDirectedCoordinate();

	virtual int getQuadrant();

	virtual double getDx();

	virtual double getDy();

	virtual void setNode(Node* newNode);

	virtual Node* getNode();

	virtual int compareTo(const EdgeEnd *e) const;

	/**
	 * Implements the total order relation:
	 * 
	 *    a has a greater angle with the positive x-axis than b
	 * 
	 * Using the obvious algorithm of simply computing the angle
	 * is not robust, since the angle calculation is obviously
	 * susceptible to roundoff.
	 * A robust algorithm is:
	 * - first compare the quadrant.  If the quadrants
	 *   are different, it it trivial to determine which vector
	 *   is "greater".
	 * - if the vectors lie in the same quadrant, the
	 *   computeOrientation function can be used to decide
	 *   the relative orientation of the vectors.
	 */
	virtual int compareDirection(const EdgeEnd *e) const;

	virtual void computeLabel(const algorithm::BoundaryNodeRule& bnr);

	virtual std::string print();

protected:

	Edge* edge;// the parent edge of this edge end

	Label* label;

	EdgeEnd(Edge* newEdge);

	virtual void init(const geom::Coordinate& newP0,
			const geom::Coordinate& newP1);

private:

	/// the node this edge end originates at
	Node* node;         

	/// points of initial line segment. FIXME: do we need a copy here ?
	geom::Coordinate p0, p1; 

	/// the direction vector for this edge from its starting point
	double dx, dy;     

	int quadrant;
};

std::ostream& operator<< (std::ostream&, const EdgeEnd&);

struct GEOS_DLL  EdgeEndLT {
	bool operator()(const EdgeEnd *s1, const EdgeEnd *s2) const {
		return s1->compareTo(s2)<0;
	}
};

} // namespace geos.geomgraph
} // namespace geos

//#ifdef GEOS_INLINE
//# include "geos/geomgraph/EdgeEnd.inl"
//#endif

#endif // ifndef GEOS_GEOMGRAPH_EDGEEND_H

/**********************************************************************
 * $Log$
 * Revision 1.6  2006/06/14 14:32:20  strk
 * EdgeEnd::getEdge() made non-virtual and inlined.
 *
 * Revision 1.5  2006/04/06 09:39:56  strk
 * Added operator<<
 *
 * Revision 1.4  2006/04/03 17:05:22  strk
 * Assertion checking, port info, cleanups
 *
 * Revision 1.3  2006/03/24 09:52:41  strk
 * USE_INLINE => GEOS_INLINE
 *
 * Revision 1.2  2006/03/15 15:27:24  strk
 * cleanups
 *
 * Revision 1.1  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/

