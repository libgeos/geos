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
 **********************************************************************
 * $Log$
 * Revision 1.2  2004/04/07 06:55:50  ybychkov
 * "operation/linemerge" ported from JTS 1.4
 *
 * Revision 1.1  2004/04/04 06:29:11  ybychkov
 * "planargraph" and "geom/utill" upgraded to JTS 1.4
 *
 *
 **********************************************************************/


#ifndef GEOS_PLANARGRAPH_H
#define GEOS_PLANARGRAPH_H

#include "platform.h"
#include "geosAlgorithm.h"
#include <vector>
#include <string>
#include <map>

using namespace std;

namespace geos {
/**
 * The base class for all graph component classes.
 * Maintains flags of use in generic graph algorithms.
 * Provides two flags:
 * <ul>
 * <li><b>marked</b> - typically this is used to indicate a state that persists
 * for the course of the graph's lifetime.  For instance, it can be
 * used to indicate that a component has been logically deleted from the graph.
 * <li><b>visited</b> - this is used to indicate that a component has been processed
 * or visited by an single graph algorithm.  For instance, a breadth-first traversal of the
 * graph might use this to indicate that a node has already been traversed.
 * The visited flag may be set and cleared many times during the lifetime of a graph.
 *
 */
class planarGraphComponent {
protected:
	bool isMarkedVar;
	bool isVisitedVar;
public:
	planarGraphComponent();
	/**
	* Tests if a component has been visited during the course of a graph algorithm
	* @return <code>true</code> if the component has been visited
	*/
	virtual bool isVisited();
	/**
	* Sets the visited flag for this component.
	* @param isVisited the desired value of the visited flag
	*/
	virtual void setVisited(bool newIsVisited);
	/**
	* Tests if a component has been marked at some point during the processing
	* involving this graph.
	* @return <code>true</code> if the component has been marked
	*/
	virtual bool isMarked();
	/**
	* Sets the marked flag for this component.
	* @param isMarked the desired value of the marked flag
	*/
	virtual void setMarked(bool newIsMarked);
};
class planarDirectedEdge;
class planarEdge;

bool pdeLessThan(planarDirectedEdge *first,planarDirectedEdge * second);
/**
 * A sorted collection of {@link DirectedEdge}s which leave a {@link Node}
 * in a {@link PlanarGraph}.
 *
 */
class planarDirectedEdgeStar {
protected:
	/**
	* The underlying list of outgoing DirectedEdges
	*/
	vector<planarDirectedEdge*> *outEdges;
private:
	bool sorted;
	void sortEdges();
public:
	/**
	* Constructs a DirectedEdgeStar with no edges.
	*/
	planarDirectedEdgeStar();
	virtual ~planarDirectedEdgeStar();
	/**
	* Adds a new member to this DirectedEdgeStar.
	*/
	void add(planarDirectedEdge *de);
	/**
	* Drops a member of this DirectedEdgeStar.
	*/
	void remove(planarDirectedEdge *de);
	/**
	* Returns an Iterator over the DirectedEdges, in ascending order by angle with the positive x-axis.
	*/
	vector<planarDirectedEdge*>::iterator iterator();
	/**
	* Returns the number of edges around the Node associated with this DirectedEdgeStar.
	*/
	int getDegree();
	/**
	* Returns the coordinate for the node at wich this star is based
	*/
	Coordinate& getCoordinate();
	/**
	* Returns the DirectedEdges, in ascending order by angle with the positive x-axis.
	*/
	vector<planarDirectedEdge*>* getEdges();
	/**
	* Returns the zero-based index of the given Edge, after sorting in ascending order
	* by angle with the positive x-axis.
	*/
	int getIndex(planarEdge *edge);
	/**
	* Returns the zero-based index of the given DirectedEdge, after sorting in ascending order
	* by angle with the positive x-axis.
	*/  
	int getIndex(planarDirectedEdge *dirEdge);
	/**
	* Returns the remainder when i is divided by the number of edges in this
	* DirectedEdgeStar. 
	*/
	int getIndex(int i);
	/**
	* Returns the DirectedEdge on the left-hand side of the given DirectedEdge (which
	* must be a member of this DirectedEdgeStar). 
	*/
	planarDirectedEdge* getNextEdge(planarDirectedEdge *dirEdge);
};

/**
 * A node in a {@link PlanarGraph}is a location where 0 or more {@link Edge}s
 * meet. A node is connected to each of its incident Edges via an outgoing
 * DirectedEdge. Some clients using a <code>PlanarGraph</code> may want to
 * subclass <code>Node</code> to add their own application-specific
 * data and methods.
 *
 */
class planarNode: public planarGraphComponent {
protected:
	/** The location of this Node */
	Coordinate pt;
	/** The collection of DirectedEdges that leave this Node */
	planarDirectedEdgeStar *deStar;
public:
	/**
	* Returns all Edges that connect the two nodes (which are assumed to be different).
	*/
	static vector<planarEdge*>* getEdgesBetween(planarNode *node0, planarNode *node1);
	/**
	* Constructs a Node with the given location.
	*/
	planarNode(Coordinate& newPt);
	/**
	* Constructs a Node with the given location and collection of outgoing DirectedEdges.
	*/
	planarNode(Coordinate& newPt, planarDirectedEdgeStar *newDeStar);
	/**
	* Returns the location of this Node.
	*/
	Coordinate& getCoordinate();
	/**
	* Adds an outgoing DirectedEdge to this Node.
	*/
	void addOutEdge(planarDirectedEdge *de);
	/**
	* Returns the collection of DirectedEdges that leave this Node.
	*/
	planarDirectedEdgeStar* getOutEdges();
	/**
	* Returns the number of edges around this Node.
	*/
	int getDegree();
	/**
	* Returns the zero-based index of the given Edge, after sorting in ascending order
	* by angle with the positive x-axis.
	*/
	int getIndex(planarEdge *edge);
};
/**
 * Represents an undirected edge of a {@link PlanarGraph}. An undirected edge
 * in fact simply acts as a central point of reference for two opposite
 * {@link DirectedEdge}s.
 * <p>
 * Usually a client using a <code>PlanarGraph</code> will subclass <code>Edge</code>
 * to add its own application-specific data and methods.
 *
 */
class planarEdge: public planarGraphComponent {
protected:
	/** The two DirectedEdges associated with this Edge */
	vector<planarDirectedEdge*> *dirEdge;
	/**
	* Constructs an Edge whose DirectedEdges are not yet set. Be sure to call
	* {@link #setDirectedEdges(DirectedEdge, DirectedEdge)}
	*/
public:
	planarEdge();
	/**
	* Constructs an Edge initialized with the given DirectedEdges, and for each
	* DirectedEdge: sets the Edge, sets the symmetric DirectedEdge, and adds
	* this Edge to its from-Node.
	*/
	planarEdge(planarDirectedEdge *de0, planarDirectedEdge *de1);
	/**
	* Initializes this Edge's two DirectedEdges, and for each DirectedEdge: sets the
	* Edge, sets the symmetric DirectedEdge, and adds this Edge to its from-Node.
	*/
	void setDirectedEdges(planarDirectedEdge *de0,planarDirectedEdge *de1);
	/**
	* Returns one of the DirectedEdges associated with this Edge.
	* @param i 0 or 1
	*/
	planarDirectedEdge* getDirEdge(int i);
	/**
	* Returns the {@link DirectedEdge} that starts from the given node, or null if the
	* node is not one of the two nodes associated with this Edge.
	*/
	planarDirectedEdge* getDirEdge(planarNode *fromNode);
	/**
	* If <code>node</code> is one of the two nodes associated with this Edge,
	* returns the other node; otherwise returns null.
	*/
	planarNode* getOppositeNode(planarNode *node);
};


/**
 * Represents a directed edge in a {@link PlanarGraph}. A DirectedEdge may or
 * may not have a reference to a parent {@link Edge} (some applications of
 * planar graphs may not require explicit Edge objects to be created). Usually
 * a client using a <code>PlanarGraph</code> will subclass <code>DirectedEdge</code>
 * to add its own application-specific data and methods.
 *
 */
class planarDirectedEdge: public planarGraphComponent {
protected:
	static const CGAlgorithms* cga;
	planarEdge* parentEdge;
	planarNode* from;
	planarNode* to;
	Coordinate p0, p1;
	planarDirectedEdge* sym;  // optional
	bool edgeDirection;
	int quadrant;
	double angle;
public:
	/**
	* Returns a List containing the parent Edge (possibly null) for each of the given
	* DirectedEdges.
	*/
	static vector<planarEdge*>* toEdges(vector<planarDirectedEdge*> *dirEdges);
	/**
	* Constructs a DirectedEdge connecting the <code>from</code> node to the
	* <code>to</code> node.
	*
	* @param directionPt
	*                  specifies this DirectedEdge's direction (given by an imaginary
	*                  line from the <code>from</code> node to <code>directionPt</code>)
	* @param edgeDirection
	*                  whether this DirectedEdge's direction is the same as or
	*                  opposite to that of the parent Edge (if any)
	*/
	planarDirectedEdge(planarNode *newFrom,planarNode *newTo,Coordinate &directionPt,bool newEdgeDirection);
	/**
	* Returns this DirectedEdge's parent Edge, or null if it has none.
	*/
	planarEdge* getEdge();
	/**
	* Associates this DirectedEdge with an Edge (possibly null, indicating no associated
	* Edge).
	*/
	void setEdge(planarEdge* newParentEdge);
	/**
	* Returns 0, 1, 2, or 3, indicating the quadrant in which this DirectedEdge's
	* orientation lies.
	*/
	int getQuadrant();
	/**
	* Returns a point to which an imaginary line is drawn from the from-node to
	* specify this DirectedEdge's orientation.
	*/
	Coordinate& getDirectionPt();
	/**
	* Returns whether the direction of the parent Edge (if any) is the same as that
	* of this Directed Edge.
	*/
	bool getEdgeDirection();
	/**
	* Returns the node from which this DirectedEdge leaves.
	*/
	planarNode* getFromNode();
	/**
	* Returns the node to which this DirectedEdge goes.
	*/
	planarNode* getToNode();
	/**
	* Returns the coordinate of the from-node.
	*/
	Coordinate& getCoordinate();
	/**
	* Returns the angle that the start of this DirectedEdge makes with the
	* positive x-axis, in radians.
	*/
	double getAngle();
	/**
	* Returns the symmetric DirectedEdge -- the other DirectedEdge associated with
	* this DirectedEdge's parent Edge.
	*/
	planarDirectedEdge* getSym();
	/**
	* Sets this DirectedEdge's symmetric DirectedEdge, which runs in the opposite
	* direction.
	*/
	void setSym(planarDirectedEdge *newSym);

	/**
	* Returns 1 if this DirectedEdge has a greater angle with the
	* positive x-axis than b", 0 if the DirectedEdges are collinear, and -1 otherwise.
	* <p>
	* Using the obvious algorithm of simply computing the angle is not robust,
	* since the angle calculation is susceptible to roundoff. A robust algorithm
	* is:
	* <ul>
	* <li>first compare the quadrants. If the quadrants are different, it it
	* trivial to determine which vector is "greater".
	* <li>if the vectors lie in the same quadrant, the robust
	* {@link RobustCGAlgorithms#computeOrientation(Coordinate, Coordinate, Coordinate)}
	* function can be used to decide the relative orientation of the vectors.
	* </ul>
	*/
	int compareTo(void* obj);
	/**
	* Returns 1 if this DirectedEdge has a greater angle with the
	* positive x-axis than b", 0 if the DirectedEdges are collinear, and -1 otherwise.
	* <p>
	* Using the obvious algorithm of simply computing the angle is not robust,
	* since the angle calculation is susceptible to roundoff. A robust algorithm
	* is:
	* <ul>
	* <li>first compare the quadrants. If the quadrants are different, it it
	* trivial to determine which vector is "greater".
	* <li>if the vectors lie in the same quadrant, the robust
	* {@link RobustCGAlgorithms#computeOrientation(Coordinate, Coordinate, Coordinate)}
	* function can be used to decide the relative orientation of the vectors.
	* </ul>
	*/
	int compareDirection(planarDirectedEdge *e);
	/**
	* Prints a detailed string representation of this DirectedEdge to the given PrintStream.
	*/
	string print();

};

struct planarCoordLT {
	bool operator()(Coordinate s1, Coordinate s2) const {
		return s1.compareTo(s2)<0;
	}
};

/**
 * A map of {@link Node}s, indexed by the coordinate of the node.
 *
 */
class planarNodeMap {
private:
	map<Coordinate,planarNode*,planarCoordLT> *nodeMap;
public:  
	/**
	* Constructs a NodeMap without any Nodes.
	*/
	planarNodeMap();
	map<Coordinate,planarNode*,planarCoordLT>* getNodeMap();
	virtual ~planarNodeMap();
	/**
	* Adds a node to the map, replacing any that is already at that location.
	* @return the added node
	*/
	planarNode* add(planarNode *n);
	/**
	* Removes the Node at the given location, and returns it (or null if no Node was there).
	*/
	planarNode* remove(Coordinate& pt);
	/**
	* Returns the Node at the given location, or null if no Node was there.
	*/
	planarNode* find(Coordinate& coord);
	/**
	* Returns an Iterator over the Nodes in this NodeMap, sorted in ascending order
	* by angle with the positive x-axis.
	*/
	map<Coordinate,planarNode*,planarCoordLT>::iterator iterator();
	/**
	* Returns the Nodes in this NodeMap, sorted in ascending order
	* by angle with the positive x-axis.
	*/
	vector<planarNode*>* getNodes();
};

/**
 * Represents a directed graph which is embeddable in a planar surface.
 * <p>
 * This class and the other classes in this package serve as a framework for
 * building planar graphs for specific algorithms. This class must be
 * subclassed to expose appropriate methods to construct the graph. This allows
 * controlling the types of graph components ({@link DirectedEdge}s,
 * {@link Edge}s and {@link Node}s) which can be added to the graph. An
 * application which uses the graph framework will almost always provide
 * subclasses for one or more graph components, which hold application-specific
 * data and graph algorithms.
 *
 */
class planarPlanarGraph {
protected:
	vector<planarEdge*> *edges;
	vector<planarDirectedEdge*> *dirEdges;
	planarNodeMap *nodeMap;
	/**
	* Adds a node to the map, replacing any that is already at that location.
	* Only subclasses can add Nodes, to ensure Nodes are of the right type.
	* @return the added node
	*/
	void add(planarNode *node);
	/**
	* Adds the Edge and its DirectedEdges with this PlanarGraph.
	* Assumes that the Edge has already been created with its associated DirectEdges.
	* Only subclasses can add Edges, to ensure the edges added are of the right class.
	*/
	void add(planarEdge *edge);
	/**
	* Adds the Edge to this PlanarGraph; only subclasses can add DirectedEdges,
	* to ensure the edges added are of the right class.
	*/
	void add(planarDirectedEdge *dirEdge);
public:
	/**
	* Constructs a PlanarGraph without any Edges, DirectedEdges, or Nodes.
	*/
	planarPlanarGraph();
	virtual ~planarPlanarGraph();
	/**
	* Returns the Node at the given location, or null if no Node was there.
	*/
	planarNode* findNode(Coordinate& pt);
	/**
	* Returns an Iterator over the Nodes in this PlanarGraph.
	*/
	map<Coordinate,planarNode*,planarCoordLT>::iterator nodeIterator();
	/**
	* Returns the Nodes in this PlanarGraph.
	*/  
	vector<planarNode*>* getNodes();
	/**
	* Returns an Iterator over the DirectedEdges in this PlanarGraph, in the order in which they
	* were added.
	*
	* @see #add(Edge)
	* @see #add(DirectedEdge)
	*/
	vector<planarDirectedEdge*>::iterator dirEdgeIterator();
	/**
	* Returns an Iterator over the Edges in this PlanarGraph, in the order in which they
	* were added.
	*
	* @see #add(Edge)
	*/
	vector<planarEdge*>::iterator edgeIterator();
	/**
	* Returns the Edges that have been added to this PlanarGraph
	* @see #add(Edge)
	*/
	vector<planarEdge*>* getEdges();
	/**
	* Removes an Edge and its associated DirectedEdges from their from-Nodes and
	* from this PlanarGraph. Note: This method does not remove the Nodes associated
	* with the Edge, even if the removal of the Edge reduces the degree of a
	* Node to zero.
	*/
	void remove(planarEdge *edge);
	/**
	* Removes DirectedEdge from its from-Node and from this PlanarGraph. Note:
	* This method does not remove the Nodes associated with the DirectedEdge,
	* even if the removal of the DirectedEdge reduces the degree of a Node to
	* zero.
	*/
	void remove(planarDirectedEdge *de);
	/**
	* Removes a node from the graph, along with any associated DirectedEdges and
	* Edges.
	*/
	void remove(planarNode *node);
	/**
	* Returns all Nodes with the given number of Edges around it.
	*/
	vector<planarNode*>* findNodesOfDegree(int degree);
};

}
#endif