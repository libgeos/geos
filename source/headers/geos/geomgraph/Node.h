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


#ifndef GEOS_GEOMGRAPH_NODE_H
#define GEOS_GEOMGRAPH_NODE_H

#include <string>

#include <geos/geomgraph/GraphComponent.h> // for inheritance
#include <geos/geom/Coordinate.h> // for member

#include <geos/inline.h>

// Forward declarations
namespace geos {
	namespace geom {
		class IntersectionMatrix;
	}
	namespace geomgraph {
		class Node;
		class EdgeEndStar;
		class EdgeEnd;
		class Label;
		class NodeFactory;
	}
}

namespace geos {
namespace geomgraph { // geos.geomgraph

class Node: public GraphComponent {
using GraphComponent::setLabel;

public:

	friend std::ostream& operator<< (std::ostream& os, const Node& node);

	Node(const geom::Coordinate& newCoord, EdgeEndStar* newEdges);

	virtual ~Node();

	virtual const geom::Coordinate& getCoordinate() const;

	virtual EdgeEndStar* getEdges();

	virtual bool isIsolated() const;

	virtual void add(EdgeEnd *e);

	virtual void mergeLabel(const Node* n);

	virtual void mergeLabel(const Label* label2);

	virtual void setLabel(int argIndex, int onLocation);

	virtual void setLabelBoundary(int argIndex);

	virtual int computeMergedLocation(const Label* label2, int eltIndex);

	virtual std::string print();

	virtual const std::vector<double> &getZ() const;

	virtual void addZ(double);

	virtual bool isIncidentEdgeInResult() const;

protected:

	geom::Coordinate coord;

	EdgeEndStar* edges;

	virtual void computeIM(geom::IntersectionMatrix *im) {};

private:

	std::vector<double>zvals;

	double ztot;

};

std::ostream& operator<< (std::ostream& os, const Node& node);

} // namespace geos.geomgraph
} // namespace geos

//#ifdef GEOS_INLINE
//# include "geos/geomgraph/Node.inl"
//#endif

#endif // ifndef GEOS_GEOMGRAPH_NODE_H

/**********************************************************************
 * $Log$
 * Revision 1.3  2006/03/24 09:52:41  strk
 * USE_INLINE => GEOS_INLINE
 *
 * Revision 1.2  2006/03/15 16:27:54  strk
 * operator<< for Node class
 *
 * Revision 1.1  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/

