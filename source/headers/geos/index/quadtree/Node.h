/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#ifndef GEOS_IDX_QUADTREE_NODE_H
#define GEOS_IDX_QUADTREE_NODE_H

#include <geos/index/quadtree/NodeBase.h> // for inheritance
#include <geos/geom/Coordinate.h> // for composition
#include <geos/geom/Envelope.h> // for inline

#include <string>
#include <memory>

// Forward declarations
namespace geos {
	namespace geom {
		//class Coordinate;
		class Envelope;
	}
}

namespace geos {
namespace index { // geos::index
namespace quadtree { // geos::index::quadtree

/**
 * \brief
 * Represents a node of a Quadtree.
 *
 * Nodes contain items which have a spatial extent corresponding to
 * the node's position in the quadtree.
 *
 */
class Node: public NodeBase {

private:

	/// Owned by this class
	geom::Envelope *env;

	geom::Coordinate centre;

	int level;

	Node* getSubnode(int index);

	Node* createSubnode(int index);

protected:

	bool isSearchMatch(const geom::Envelope& searchEnv) const {
		return env->intersects(searchEnv);
	}

public:

	static std::auto_ptr<Node> createNode(const geom::Envelope& env);

	static std::auto_ptr<Node> createExpanded(Node *node,
			const geom::Envelope *addEnv);

	// Takes ownership of envelope
	Node(geom::Envelope *nenv, int nlevel)
		:
		env(nenv),
		centre((nenv->getMinX()+nenv->getMaxX())/2,
			(nenv->getMinY()+nenv->getMaxY())/2),
		level(nlevel)
	{
	}

	virtual ~Node() { delete env; }

	/// Return Envelope associated with this node
	/// ownership retained by this object
	geom::Envelope* getEnvelope() { return env; }

	/** \brief
	 * Returns the subquad containing the envelope.
	 * Creates the subquad if
	 * it does not already exist.
	 */
	Node* getNode(const geom::Envelope *searchEnv);

	/** \brief
	 * Returns the smallest <i>existing</i>
	 * node containing the envelope.
	 */
	NodeBase* find(const geom::Envelope *searchEnv);

	void insertNode(Node *node);

	std::string toString() const;

};

} // namespace geos::index::quadtree
} // namespace geos::index
} // namespace geos

#endif // GEOS_IDX_QUADTREE_NODE_H

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/03/22 12:22:50  strk
 * indexQuadtree.h split
 *
 **********************************************************************/

