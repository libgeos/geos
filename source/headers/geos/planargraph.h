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
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#ifndef GEOS_PLANARGRAPH_H
#define GEOS_PLANARGRAPH_H

#include <geos/platform.h>
#include <geos/geosAlgorithm.h>
#include <vector>
#include <string>
#include <map>

using namespace std;

namespace geos {
//namespace planargraph {

class planarDirectedEdge;
class planarDirectedEdgeStar;
class planarEdge;
class planarGraphComponent;
class planarNode;
class planarNodeMap;
class planarPlanarGraph;
class planarSubgraph;


/**
 * \class planarGraphComponent planargraph.h geos/planargraph.h
 *
 * \brief The base class for all graph component classes.
 *
 * Maintains flags of use in generic graph algorithms.
 * Provides two flags:
 * 
 *  - <b>marked</b> - typically this is used to indicate a state that
 *    persists for the course of the graph's lifetime.  For instance,
 *    it can be used to indicate that a component has been logically
 *    deleted from the graph.
 *  - <b>visited</b> - this is used to indicate that a component has been
 *    processed or visited by an single graph algorithm.  For instance,
 *    a breadth-first traversal of the graph might use this to indicate
 *    that a node has already been traversed.
 *    The visited flag may be set and cleared many times during the
 *    lifetime of a graph.
 *
 * Last port: planargraph/GraphComponent.java rev. 1.7 (JTS-1.7)
 *
 */
class planarGraphComponent {

protected:

	bool isMarkedVar;

	bool isVisitedVar;

public:

	planarGraphComponent()
		:
		isMarkedVar(false),
		isVisitedVar(false)
		{}

	virtual ~planarGraphComponent() {};

	/** \brief
	 * Tests if a component has been visited during the course
	 * of a graph algorithm.
	 *
	 * @return <code>true</code> if the component has been visited
	 */
	virtual bool isVisited() { return isVisitedVar; }

	/** \brief
	 * Sets the visited flag for this component.
	 * @param isVisited the desired value of the visited flag
	 */
	virtual void setVisited(bool isVisited) { isVisitedVar=isVisited; }

	/** \brief
	 * Tests if a component has been marked at some point
	 * during the processing involving this graph.
	 * @return <code>true</code> if the component has been marked
	 */
	virtual bool isMarked() { return isMarkedVar; }

	/** \brief
	 * Sets the marked flag for this component.
	 * @param isMarked the desired value of the marked flag
	 */
	virtual void setMarked(bool isMarked) { isVisitedVar=isMarked; }

};

bool pdeLessThan(planarDirectedEdge *first,planarDirectedEdge * second);

/**
 * \class planarDirectedEdgeStar planargraph.h geos/planargraph.h
 *
 * \brief
 * A sorted collection of DirectedEdge which leave a Node in a PlanarGraph.
 */
class planarDirectedEdgeStar {
protected:
	/**
	 * \brief The underlying list of outgoing DirectedEdges
	 */
	vector<planarDirectedEdge*> outEdges;

private:
	bool sorted;
	void sortEdges();

public:
	/**
	 * \brief Constructs a DirectedEdgeStar with no edges.
	 */
	planarDirectedEdgeStar();

	virtual ~planarDirectedEdgeStar();

	/**
	 * \brief Adds a new member to this DirectedEdgeStar.
	 */
	void add(planarDirectedEdge *de);

	/**
	 * \brief Drops a member of this DirectedEdgeStar.
	 */
	void remove(planarDirectedEdge *de);

	/**
	 * \brief Returns an Iterator over the DirectedEdges,
	 * in ascending order by angle with the positive x-axis.
	 */
	vector<planarDirectedEdge*>::iterator iterator();

	/**
	 * \brief Returns the number of edges around the Node associated
	 * with this DirectedEdgeStar.
	 */
	unsigned int getDegree() const;

	/**
	 * \brief Returns the coordinate for the node at wich this
	 * star is based
	 */
	Coordinate& getCoordinate() const;

	/**
	 * \brief Returns the DirectedEdges, in ascending order
	 * by angle with the positive x-axis.
	 */
	vector<planarDirectedEdge*>& getEdges();

	/**
	 * \brief Returns the zero-based index of the given Edge,
	 * after sorting in ascending order by angle with the
	 * positive x-axis.
	 */
	int getIndex(const planarEdge *edge);

	/**
	 * \brief Returns the zero-based index of the given DirectedEdge,
	 * after sorting in ascending order
	 * by angle with the positive x-axis.
	 */  
	int getIndex(const planarDirectedEdge *dirEdge);

	/**
	 * \brief Returns the remainder when i is divided by the number of
	 * edges in this DirectedEdgeStar. 
	 */
	int getIndex(int i) const;

	/**
	 * \brief Returns the DirectedEdge on the left-hand side
	 * of the given DirectedEdge (which must be a member of this
	 * DirectedEdgeStar). 
	 */
	planarDirectedEdge* getNextEdge(planarDirectedEdge *dirEdge);
};

/**
 * \class planarNode planargraph.h geos/planargraph.h
 *
 * \brief A node in a PlanarGraph is a location where 0 or more Edge meet.
 *
 * A node is connected to each of its incident Edges via an outgoing
 * DirectedEdge. Some clients using a <code>PlanarGraph</code> may want to
 * subclass <code>Node</code> to add their own application-specific
 * data and methods.
 *
 */
class planarNode: public planarGraphComponent {
protected:

	/// The location of this Node 
	Coordinate pt;

	/// The collection of DirectedEdges that leave this Node 
	planarDirectedEdgeStar *deStar;

public:

	/**
	 * \brief Returns all Edges that connect the two nodes (which are
	 * assumed to be different).
	 */
	static vector<planarEdge*>* getEdgesBetween(planarNode *node0, planarNode *node1);

	/// Constructs a Node with the given location.
	planarNode(const Coordinate& newPt);

	~planarNode();

	/**
	 * \brief Constructs a Node with the given location and
	 * collection of outgoing planarDirectedEdges.
	 */
	planarNode(Coordinate& newPt, planarDirectedEdgeStar *newDeStar);

	/**
	 * \brief Returns the location of this Node.
	 */
	Coordinate& getCoordinate();

	/**
	 * \brief Adds an outgoing DirectedEdge to this Node.
	 */
	void addOutEdge(planarDirectedEdge *de);

	/**
	 * \brief Returns the collection of DirectedEdges that
	 * leave this Node.
	 */
	planarDirectedEdgeStar* getOutEdges();

	/**
	 * \brief Returns the number of edges around this Node.
	 */
	int getDegree();

	/**
	 * \brief Returns the zero-based index of the given Edge,
	 * after sorting in ascending order by angle with
	 * the positive x-axis.
	 */
	int getIndex(planarEdge *edge);

};

/**
 * \class planarEdge planargraph.h geos/planargraph.h
 *
 * \brief Represents an undirected edge of a PlanarGraph.
 *
 * An undirected edge in fact simply acts as a central point of reference
 * for two opposite DirectedEdge.
 * 
 * Usually a client using a PlanarGraph will subclass Edge
 * to add its own application-specific data and methods.
 */
class planarEdge: public planarGraphComponent {

public:
	typedef set<const planarEdge *> ConstSet;

protected:

	/** \brief The two DirectedEdges associated with this Edge */
	vector<planarDirectedEdge*> dirEdge;

	/**
	 * \brief Constructs an Edge whose DirectedEdges are not yet set.
	 *
	 * Be sure to call setDirectedEdges(DirectedEdge, DirectedEdge)
	 */

public:

	planarEdge();

	/**
	 * \brief Constructs an Edge initialized with the given DirectedEdges.
	 *
	 * For  each DirectedEdge: sets the Edge, sets the symmetric
	 * DirectedEdge, and adds this Edge to its from-Node.
	 */
	planarEdge(planarDirectedEdge *de0, planarDirectedEdge *de1);

	/**
	 * \brief Initializes this Edge's two DirectedEdges.
	 *
	 * For each DirectedEdge:
	 *  sets the Edge, sets the symmetric DirectedEdge, and
	 *  adds this Edge to its from-Node.
	 */
	void setDirectedEdges(planarDirectedEdge *de0, planarDirectedEdge *de1);

	/**
	 * \brief Returns one of the DirectedEdges associated with this Edge.
	 * @param i 0 or 1
	 */
	planarDirectedEdge* getDirEdge(int i);

	/**
	 * \brief Returns the DirectedEdge that starts from the given node,
	 * or null if the node is not one of the two nodes associated
	 * with this Edge.
	 */
	planarDirectedEdge* getDirEdge(planarNode *fromNode);

	/**
	 * \brief If <code>node</code> is one of the two nodes associated
	 * with this Edge, returns the other node; otherwise returns null.
	 */
	planarNode* getOppositeNode(planarNode *node);
};


/**
 * \class planarDirectedEdge planargraph.h geos/planargraph.h
 *
 * \brief Represents a directed edge in a PlanarGraph.
 *
 * A DirectedEdge may or may not have a reference to a parent Edge
 * (some applications of planar graphs may not require explicit Edge
 * objects to be created). Usually a client using a PlanarGraph
 * will subclass DirectedEdge to add its own application-specific
 * data and methods.
 */
class planarDirectedEdge: public planarGraphComponent {
//friend class Unload;

protected:
	//static const CGAlgorithms* cga;
	static CGAlgorithms cga;
	planarEdge* parentEdge;
	planarNode* from;
	planarNode* to;
	Coordinate p0, p1;
	planarDirectedEdge* sym;  // optional
	bool edgeDirection;
	int quadrant;
	double angle;
public:

	typedef vector<const planarDirectedEdge *> ConstVect;
	typedef vector<planarDirectedEdge *> Vect;

	/**
	 * \brief Returns a List containing the parent Edge (possibly null)
	 * for each of the given DirectedEdges.
	 */
	static vector<planarEdge*>* toEdges(vector<planarDirectedEdge*> *dirEdges);

	/**
	 * \brief Constructs a DirectedEdge connecting the <code>from</code>
	 * node to the <code>to</code> node.
	 *
	 * @param directionPt specifies this DirectedEdge's direction
	 *                    (given by an imaginary line from the
	 *		      <code>from</code> node to
	 *		      <code>directionPt</code>)
	 * @param edgeDirection whether this DirectedEdge's direction
	 *		       is the same as or opposite to that of the
	 *		       parent Edge (if any)
	 */
	planarDirectedEdge(planarNode *newFrom,planarNode *newTo, const Coordinate &directionPt,bool newEdgeDirection);

	/**
	 * \brief Returns this DirectedEdge's parent Edge,
	 * or null if it has none.
	 */
	planarEdge* getEdge() const;

	/**
	 * \brief Associates this DirectedEdge with an Edge
	 * (possibly null, indicating no associated Edge).
	 */
	void setEdge(planarEdge* newParentEdge);

	/**
	 * \brief Returns 0, 1, 2, or 3, indicating the quadrant in which
	 * this DirectedEdge's orientation lies.
	 */
	int getQuadrant() const;

	/**
	 * \brief Returns a point to which an imaginary line is drawn
	 * from the from-node to specify this DirectedEdge's orientation.
	 */
	const Coordinate& getDirectionPt() const;

	/**
	 * \brief Returns whether the direction of the parent Edge (if any)
	 * is the same as that of this Directed Edge.
	 */
	bool getEdgeDirection() const;

	/**
	 * \brief Returns the node from which this DirectedEdge leaves.
	 */
	planarNode* getFromNode() const;

	/**
	 * \brief Returns the node to which this DirectedEdge goes.
	 */
	planarNode* getToNode() const;

	/**
	 * \brief
	 * Returns the coordinate of the from-node.
	 */
	Coordinate& getCoordinate() const;

	/**
	 * \brief
	 * Returns the angle that the start of this DirectedEdge makes
	 * with the positive x-axis, in radians.
	 */
	double getAngle() const;

	/**
	 * \brief
	 * Returns the symmetric DirectedEdge -- the other DirectedEdge
	 * associated with this DirectedEdge's parent Edge.
	 */
	planarDirectedEdge* getSym() const;

	/**
	 * \brief
	 * Sets this DirectedEdge's symmetric DirectedEdge, which runs
	 * in the opposite direction.
	 */
	void setSym(planarDirectedEdge *newSym);

	/**
	 * \brief
	 * Returns 1 if this DirectedEdge has a greater angle with the
	 * positive x-axis than b", 0 if the DirectedEdges are collinear,
	 * and -1 otherwise.
	 *
	 * Using the obvious algorithm of simply computing the angle is
	 * not robust, since the angle calculation is susceptible to roundoff.
	 * A robust algorithm is:
	 * 
	 * - first compare the quadrants.
	 *   If the quadrants are different, it it
	 *   trivial to determine which vector is "greater".
	 * - if the vectors lie in the same quadrant, the robust
	 *   RobustCGAlgorithms::computeOrientation(Coordinate, Coordinate, Coordinate)
	 *   function can be used to decide the relative orientation of
	 *   the vectors.
	 * 
	 */
	int compareTo(const planarDirectedEdge* obj) const;

	/**
	 * \brief
	 * Returns 1 if this DirectedEdge has a greater angle with the
	 * positive x-axis than b", 0 if the DirectedEdges are collinear,
	 * and -1 otherwise.
	 *
	 * Using the obvious algorithm of simply computing the angle is
	 * not robust, since the angle calculation is susceptible to roundoff.
	 * A robust algorithm is:
	 * 
	 * - first compare the quadrants.
	 *   If the quadrants are different, it it trivial to determine
	 *   which vector is "greater".
	 * - if the vectors lie in the same quadrant, the robust
	 *   RobustCGAlgorithms::computeOrientation(Coordinate, Coordinate, Coordinate)
	 *   function can be used to decide the relative orientation of
	 *   the vectors.
	 *
	 */
	int compareDirection(const planarDirectedEdge *e) const;

	/**
	 * \brief
	 * Prints a detailed string representation of this DirectedEdge
	 * to the given PrintStream.
	 */
	string print() const;

};

struct planarCoordLT {
	bool operator()(Coordinate s1, Coordinate s2) const {
		return s1.compareTo(s2)<0;
	}
};

/**
 * \class planarNodeMap planargraph.h geos/planargraph.h
 * \brief
 * A map of Node, indexed by the coordinate of the node.
 *
 */
class planarNodeMap {
private:
	map<Coordinate,planarNode*, planarCoordLT> nodeMap;
public:  
	/**
	 * \brief Constructs a NodeMap without any Nodes.
	 */
	planarNodeMap();

	map<Coordinate,planarNode*,planarCoordLT>& getNodeMap();

	virtual ~planarNodeMap();

	/**
	 * \brief
	 * Adds a node to the map, replacing any that is already
	 * at that location.
	 * @return the added node
	 */
	planarNode* add(planarNode *n);

	/**
	 * \brief
	 * Removes the Node at the given location, and returns it
	 * (or null if no Node was there).
	 */
	planarNode* remove(Coordinate& pt);

	/**
	 * \brief
	 * Returns the Node at the given location,
	 * or null if no Node was there.
	 */
	planarNode* find(const Coordinate& coord);

	/**
	 * \brief
	 * Returns an Iterator over the Nodes in this NodeMap,
	 * sorted in ascending order
	 * by angle with the positive x-axis.
	 */
	map<Coordinate,planarNode*,planarCoordLT>::iterator iterator();

	/**
	 * \brief
	 * Returns the Nodes in this NodeMap, sorted in ascending order
	 * by angle with the positive x-axis.
	 */
	vector<planarNode*>* getNodes();
};

/**
 * \class planarPlanarGraph planargraph.h geos/planargraph.h
 * \brief
 * Represents a directed graph which is embeddable in a planar surface.
 * 
 * This class and the other classes in this package serve as a framework for
 * building planar graphs for specific algorithms. This class must be
 * subclassed to expose appropriate methods to construct the graph. This allows
 * controlling the types of graph components (DirectedEdge, Edge and Node)
 * which can be added to the graph. An application which uses the graph
 * framework will almost always provide subclasses for one or more graph
 * components, which hold application-specific data and graph algorithms.
 */
class planarPlanarGraph {

protected:

	vector<planarEdge*> edges;
	vector<planarDirectedEdge*> dirEdges;
	planarNodeMap nodeMap;

	/**
	 * \brief
	 * Adds a node to the map, replacing any that is already at that
	 * location.
	 *
	 * Only subclasses can add Nodes, to ensure Nodes are
	 * of the right type.
	 * @return the added node
	 */
	void add(planarNode *node);

	/**
	 * \brief
	 * Adds the Edge and its DirectedEdges with this PlanarGraph.
	 *
	 * Assumes that the Edge has already been created with its associated
	 * DirectEdges.
	 * Only subclasses can add Edges, to ensure the edges added are of
	 * the right class.
	 */
	void add(planarEdge *edge);

	/**
	 * \brief
	 * Adds the Edge to this PlanarGraph.
	 *
	 * Only subclasses can add DirectedEdges,
	 * to ensure the edges added are of the right class.
	 */
	void add(planarDirectedEdge *dirEdge);

public:

	/**
	 * \brief
	 * Constructs a PlanarGraph without any Edges, DirectedEdges, or Nodes.
	 */
	planarPlanarGraph();

	virtual ~planarPlanarGraph();

	/**
	 * \brief
	 * Returns the Node at the given location,
	 * or null if no Node was there.
	 */
	planarNode* findNode(const Coordinate& pt);

	/**
	 * \brief
	 * Returns an Iterator over the Nodes in this PlanarGraph.
	 */
	map<Coordinate,planarNode*,planarCoordLT>::iterator nodeIterator();

	/**
	 * \brief
	 * Returns the Nodes in this PlanarGraph.
	 */  
	vector<planarNode*>* getNodes();

	/**
	 * \brief
	 * Returns an Iterator over the DirectedEdges in this PlanarGraph,
	 * in the order in which they were added.
	 *
	 * @see add(Edge)
	 * @see add(DirectedEdge)
	 */
	vector<planarDirectedEdge*>::iterator dirEdgeIterator();

	/**
	 * \brief
	 * Returns an Iterator over the Edges in this PlanarGraph,
	 * in the order in which they were added.
	 *
	 * @see #add(Edge)
	 */
	vector<planarEdge*>::iterator edgeIterator();

	/**
	 * \brief
	 * Returns the Edges that have been added to this PlanarGraph
	 * @see #add(Edge)
	 */
	vector<planarEdge*>* getEdges();

	/**
	 * \brief
	 * Removes an Edge and its associated DirectedEdges from their
	 * from-Nodes and from this PlanarGraph.
	 *
	 * Note: This method does not remove the Nodes associated
	 * with the Edge, even if the removal of the Edge reduces the
	 * degree of a Node to zero.
	 */
	void remove(planarEdge *edge);

	/**
	 * \brief
	 * Removes DirectedEdge from its from-Node and from this PlanarGraph.
	 *
	 * Note:
	 * This method does not remove the Nodes associated with the
	 * DirectedEdge, even if the removal of the DirectedEdge reduces
	 * the degree of a Node to zero.
	 */
	void remove(planarDirectedEdge *de);

	/**
	 * \brief
	 * Removes a node from the graph, along with any associated
	 * DirectedEdges and Edges.
	 */
	void remove(planarNode *node);

	/**
	 * \brief
	 * Returns all Nodes with the given number of Edges around it.
	 */
	vector<planarNode*>* findNodesOfDegree(int degree);
};

/// A subgraph of a PlanarGraph.
//
/// A subgraph may contain any subset of {@link Edges}
/// from the parent graph.
/// It will also automatically contain all {@link DirectedEdge}s
/// and {@link Node}s associated with those edges.
/// No new objects are created when edges are added -
/// all associated components must already exist in the parent graph.
///
class planarSubgraph
{
protected:
	planarPlanarGraph &parentGraph;
	planarEdge::ConstSet edges;
	planarDirectedEdge::ConstVect dirEdges;
	planarNodeMap nodeMap;

public:
	/**
	 * Creates a new subgraph of the given {@link PlanarGraph}
	 *
	 * @param parentGraph the parent graph
	 */
	planarSubgraph(planarPlanarGraph &parent)
		:
		parentGraph(parent)
		{}

	/**
	 * Gets the {@link PlanarGraph} which this subgraph
	 * is part of.
	 *
	 * @return the parent PlanarGraph
	 */
	planarPlanarGraph& getParent() const { return parentGraph; }

	/**
	 * Adds an {@link Edge} to the subgraph.
	 * The associated {@link DirectedEdge}s and {@link Node}s
	 * are also added.
	 *
	 * @param e the edge to add
	 *
	 * @return a pair with first element being an iterator
	 *         to the planarEdge in set and second element
	 *	   being a boolean value indicating wheter
	 *	   the planarEdge has been inserted now or was
	 *	   already in the set.
	 */
	pair<planarEdge::ConstSet::iterator, bool> add(planarEdge *e);

	/**
	 * Returns an iterator over the planarDirectedEdge in this graph,
	 * in the order in which they were added.
	 *
	 * @return an iterator over the directed edges
	 *
	 * @see add(planarEdge)
	 */
	planarDirectedEdge::ConstVect::iterator getDirEdgeBegin() {
		return dirEdges.begin();
	}

	
	/**
	 * Returns an {@link Iterator} over the {@link Edge}s in this graph,
	 * in the order in which they were added.
	 *
	 * @return an iterator over the edges
	 *
	 * @see add(planarEdge)
	 */
	planarEdge::ConstSet::iterator edgeBegin() { return edges.begin(); }

	/**
	 * Returns an {@link Iterator} over the {@link Nodes} in this graph.
	 * @return an iterator over the nodes
	 */
	map<Coordinate,planarNode*,planarCoordLT>::iterator nodeIterator() {
		return nodeMap.iterator(); 
	}

	/**
	 * Tests whether an {@link Edge} is contained in this subgraph
	 * @param e the edge to test
	 * @return <code>true</code> if the edge is contained in this subgraph
	 */
	bool contains(planarEdge *e) { return (edges.find(e) != edges.end()); }

	
};

//} // namespace planargraph
} // namespace geos
#endif

/**********************************************************************
 * $Log$
 * Revision 1.9  2006/02/04 00:54:57  strk
 * - Doxygen dox updated
 * - LineStringLT struct moved from geomgraph.h to geom.h
 * - New planarSubgraph class
 * - Fixed ruby Makefiles to avoid running tests when disabled
 * - Renamed TESTS variable to XMLTESTS to not confuse 'make check' rule
 *
 * Revision 1.8  2006/02/01 22:21:29  strk
 * - Added rectangle-based optimizations of intersects() and contains() ops
 * - Inlined all planarGraphComponent class
 *
 * Revision 1.7  2005/11/15 12:14:05  strk
 * Reduced heap allocations, made use of references when appropriate,
 * small optimizations here and there.
 *
 * Revision 1.6  2004/12/14 10:35:44  strk
 * Comments cleanup. PolygonizeGraph keeps track of generated CoordinateSequence
 * for delayed destruction.
 *
 * Revision 1.5  2004/10/19 19:51:14  strk
 * Fixed many leaks and bugs in Polygonizer.
 * Output still bogus.
 *
 * Revision 1.4  2004/10/13 10:03:02  strk
 * Added missing linemerge and polygonize operation.
 * Bug fixes and leaks removal from the newly added modules and
 * planargraph (used by them).
 * Some comments and indentation changes.
 *
 * Revision 1.3  2004/07/19 13:19:31  strk
 * Documentation fixes
 *
 * Revision 1.2  2004/07/13 08:33:52  strk
 * Added missing virtual destructor to virtual classes.
 * Fixed implicit unsigned int -> int casts
 *
 * Revision 1.1  2004/07/02 13:20:42  strk
 * Header files moved under geos/ dir.
 *
 * Revision 1.3  2004/04/16 08:52:52  strk
 * Unload::Release final delete (static heap allocations should be gone now)
 *
 * Revision 1.2  2004/04/07 06:55:50  ybychkov
 * "operation/linemerge" ported from JTS 1.4
 *
 * Revision 1.1  2004/04/04 06:29:11  ybychkov
 * "planargraph" and "geom/utill" upgraded to JTS 1.4
 *
 *
 **********************************************************************/

