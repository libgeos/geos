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


#ifndef GEOS_OPPOLYGONIZE_H
#define GEOS_OPPOLYGONIZE_H

#include <geos/platform.h>
#include <geos/planargraph.h>
#include <geos/geosAlgorithm.h>
#include <geos/geom.h>
#include <vector>

namespace geos {

//using namespace planargraph;

/*
 * An edge of a polygonization graph.
 *
 * @version 1.4
 */
class PolygonizeEdge: public planarEdge {
private:
	const LineString *line;
public:
	PolygonizeEdge(const LineString *newLine);
	const LineString* getLine();
};


/*
 * Represents a ring of PolygonizeDirectedEdge which form
 * a ring of a polygon.  The ring may be either an outer shell or a hole.
 */
class polygonizeEdgeRing {
private:
	const GeometryFactory *factory;
	static CGAlgorithms cga;
	vector<const planarDirectedEdge*> *deList;

	// cache the following data for efficiency
	LinearRing *ring;
	CoordinateSequence *ringPts;
	vector<Geometry*> *holes;

	/*
	 * Computes the list of coordinates which are contained in this ring.
	 * The coordinatea are computed once only and cached.
	 *
	 * @return an array of the Coordinate in this ring
	 */
	CoordinateSequence* getCoordinates();

	static void addEdge(const CoordinateSequence *coords, bool isForward, CoordinateSequence *coordList);

public:
	/**
	 * Find the innermost enclosing shell polygonizeEdgeRing
	 * containing the argument polygonizeEdgeRing, if any.
	 * The innermost enclosing ring is the <i>smallest</i> enclosing ring.
	 * The algorithm used depends on the fact that:
	 * 
	 * ring A contains ring B iff envelope(ring A) contains envelope(ring B)
	 *
	 * This routine is only safe to use if the chosen point of the hole
	 * is known to be properly contained in a shell
	 * (which is guaranteed to be the case if the hole does not touch
	 * its shell)
	 *
	 * @return containing polygonizeEdgeRing, if there is one
	 * @return null if no containing polygonizeEdgeRing is found
	 */
	static polygonizeEdgeRing* findEdgeRingContaining(polygonizeEdgeRing *testEr, vector<polygonizeEdgeRing*> *shellList);

	/*
	 * \brief
	 * Finds a point in a list of points which is not contained in
	 * another list of points.
	 *
	 * @param testPts the CoordinateSequence to test
	 * @param pts the CoordinateSequence to test the input points against
	 * @return a Coordinate reference from <code>testPts</code> which is
	 * not in <code>pts</code>, or <code>Coordinate::nullCoord</code>
	 */
	static const Coordinate& ptNotInList(const CoordinateSequence *testPts, const CoordinateSequence *pts);

	/*
	 * Tests whether a given point is in an array of points.
	 * Uses a value-based test.
	 *
	 * @param pt a Coordinate for the test point
	 * @param pts an array of Coordinate to test
	 * @return <code>true</code> if the point is in the array
	 */
	static bool isInList(const Coordinate &pt, const CoordinateSequence *pts);
	polygonizeEdgeRing(const GeometryFactory *newFactory);
	~polygonizeEdgeRing();

	/*
	 * Adds a DirectedEdge which is known to form part of this ring.
	 * @param de the DirectedEdge to add. Ownership to the caller.
	 */
	void add(const planarDirectedEdge *de);

	/*
	 * Tests whether this ring is a hole.
	 * Due to the way the edges in the polyongization graph are linked,
	 * a ring is a hole if it is oriented counter-clockwise.
	 * @return <code>true</code> if this ring is a hole
	 */
	bool isHole();

	/*
	 * Adds a hole to the polygon formed by this ring.
	 * @param hole the LinearRing forming the hole.
	 */
	void addHole(LinearRing *hole);

	/*
	 * Computes the Polygon formed by this ring and any contained holes.
	 *
	 * @return the Polygon formed by this ring and its holes.
	 */
	Polygon* getPolygon();

	/*
	 * Tests if the LinearRing ring formed by this edge ring
	 * is topologically valid.
	 */
	bool isValid();

	/*
	 * Gets the coordinates for this ring as a LineString.
	 * Used to return the coordinates in this ring
	 * as a valid geometry, when it has been detected that the ring
	 * is topologically invalid.
	 * @return a LineString containing the coordinates in this ring
	 */
	LineString* getLineString();

	/*
	 * Returns this ring as a LinearRing, or null if an Exception
	 * occurs while creating it (such as a topology problem).
	 * Ownership of ring is retained by the object.
	 * Details of problems are written to standard output.
	 */
	LinearRing* getRingInternal();

	/*
	 * Returns this ring as a LinearRing taking ownership
	 * of it. 
	 */
	LinearRing* getRingOwnership();
};


/*
 * A DirectedEdge of a PolygonizeGraph, which represents
 * an edge of a polygon formed by the graph.
 * May be logically deleted from the graph by setting the
 * <code>marked</code> flag.
 */
class PolygonizeDirectedEdge: public planarDirectedEdge {
private:
	polygonizeEdgeRing *edgeRing;
	PolygonizeDirectedEdge *next;
	long label;
public:
	/*
	 * \brief
	 * Constructs a directed edge connecting the <code>from</code> node
	 * to the <code>to</code> node.
	 *
	 * @param directionPt
	 *    specifies this DirectedEdge's direction (given by an imaginary
	 *    line from the <code>from</code> node to <code>directionPt</code>)
	 *
	 * @param edgeDirection
	 *    whether this DirectedEdge's direction is the same as or
	 *    opposite to that of the parent Edge (if any)
	 */
	PolygonizeDirectedEdge(planarNode *newFrom,planarNode *newTo, const Coordinate& newDirectionPt,bool nEdgeDirection);

	/*
	 * Returns the identifier attached to this directed edge.
	 */
	long getLabel() const;

	/*
	 * Attaches an identifier to this directed edge.
	 */
	void setLabel(long newLabel);

	/*
	 * Returns the next directed edge in the EdgeRing that this
	 * directed edge is a member of.
	 */
	PolygonizeDirectedEdge* getNext() const;

	/*
	 * Sets the next directed edge in the EdgeRing that this
	 * directed edge is a member of.
	 */
	void setNext(PolygonizeDirectedEdge *newNext);

	/*
	 * Returns the ring of directed edges that this directed edge is
	 * a member of, or null if the ring has not been set.
	 * @see #setRing(EdgeRing)
	 */
	bool isInRing() const;

	/*
	 * Sets the ring of directed edges that this directed edge is
	 * a member of.
	 */
	void setRing(polygonizeEdgeRing *newEdgeRing);
};


/*
 * Represents a planar graph of edges that can be used to compute a
 * polygonization, and implements the algorithms to compute the
 * EdgeRings formed by the graph.
 * 
 * The marked flag on DirectedEdge is used to indicate that a directed edge
 * has be logically deleted from the graph.
 *
 */
class PolygonizeGraph: public planarPlanarGraph {
public:
	/*
	 * \brief
	 * Deletes all edges at a node
	 */
	static void deleteAllEdges(planarNode *node);

	/*
	 * \brief
	 * Create a new polygonization graph.
	 */
	PolygonizeGraph(const GeometryFactory *newFactory);

	/*
	 * \brief
	 * Destroy a polygonization graph.
	 */
	~PolygonizeGraph();

	/*
	 * \brief
	 * Add a LineString forming an edge of the polygon graph.
	 * @param line the line to add
	 */
	void addEdge(const LineString *line);

	/*
	 * \brief
	 * Computes the EdgeRings formed by the edges in this graph.
	 *
	 * @return a list of the EdgeRing found by the
	 * 	polygonization process.
	 */
	vector<polygonizeEdgeRing*>* getEdgeRings();

	/*
	 * \brief
	 * Finds and removes all cut edges from the graph.
	 *
	 * @return a list of the LineString forming the removed cut edges
	 */
	vector<const LineString*>* deleteCutEdges();

	/*
	 * Marks all edges from the graph which are "dangles".
	 * Dangles are which are incident on a node with degree 1.
	 * This process is recursive, since removing a dangling edge
	 * may result in another edge becoming a dangle.
	 * In order to handle large recursion depths efficiently,
	 * an explicit recursion stack is used
	 *
	 * @return a List containing the LineStrings that formed dangles
	 */
	vector<const LineString*>* deleteDangles();

private:
	static int getDegreeNonDeleted(planarNode *node);
	static int getDegree(planarNode *node, long label);
	const GeometryFactory *factory;
	planarNode* getNode(const Coordinate& pt);
	void computeNextCWEdges();

	/*
	 * \brief
	 * Convert the maximal edge rings found by the initial graph traversal
	 * into the minimal edge rings required by JTS polygon topology rules.
	 *
	 * @param ringEdges
	 * 	the list of start edges for the edgeRings to convert.
	 */
	void convertMaximalToMinimalEdgeRings(vector<PolygonizeDirectedEdge*> *ringEdges);

	/*
	 * \brief
	 * Finds all nodes in a maximal edgering which are self-intersection
	 * nodes
	 *
	 * @param startDE
	 * @param label
	 * @return the list of intersection nodes found,
	 * or <code>null</code> if no intersection nodes were found.
	 * Ownership of returned vector goes to caller.
	 */
	static vector<planarNode*>* findIntersectionNodes(PolygonizeDirectedEdge *startDE, long label);

	/*
	 * @param dirEdges a List of the DirectedEdges in the graph
	 * @return a List of DirectedEdges, one for each edge ring found
	 */
	static vector<PolygonizeDirectedEdge*>* findLabeledEdgeRings(vector<planarDirectedEdge*> *dirEdges);

	static void label(vector<planarDirectedEdge*> *dirEdges, long label);

	static void computeNextCWEdges(planarNode *node);

	/**
	 * \brief
	 * Computes the next edge pointers going CCW around the given node,
	 * for the given edgering label.
	 * This algorithm has the effect of converting maximal edgerings
	 * into minimal edgerings
	 */
	static void computeNextCCWEdges(planarNode *node, long label);

	/**
	 * \brief
	 * Traverse a ring of DirectedEdges, accumulating them into a list.
	 * This assumes that all dangling directed edges have been removed
	 * from the graph, so that there is always a next dirEdge.
	 *
	 * @param startDE the DirectedEdge to start traversing at
	 * @return a List of DirectedEdges that form a ring
	 */
	static vector<planarDirectedEdge*>* findDirEdgesInRing(PolygonizeDirectedEdge *startDE);

	polygonizeEdgeRing* findEdgeRing(PolygonizeDirectedEdge *startDE);

	/* Tese are for memory management */
	vector<planarEdge *>newEdges;
	vector<planarDirectedEdge *>newDirEdges;
	vector<planarNode *>newNodes;
	vector<polygonizeEdgeRing *>newEdgeRings;
};

/*
 * Polygonizes a set of Geometrys which contain linework that
 * represents the edges of a planar graph.
 * Any dimension of Geometry is handled - the constituent linework is extracted
 * to form the edges.
 * The edges must be correctly noded; that is, they must only meet
 * at their endpoints.  The Polygonizer will still run on incorrectly noded input
 * but will not form polygons from incorrected noded edges.
 * <p>
 * The Polygonizer reports the follow kinds of errors:
 * <ul>
 * <li><b>Dangles</b> - edges which have one or both ends which are not incident on another edge endpoint
 * <li><b>Cut Edges</b> - edges which are connected at both ends but which do not form part of polygon
 * <li><b>Invalid Ring Lines</b> - edges which form rings which are invalid
 * (e.g. the component lines contain a self-intersection)
 * </ul>
 *
 */
class Polygonizer {
private:
	/**
	* Add every linear element in a geometry into the polygonizer graph.
	*/
	class LineStringAdder: public GeometryComponentFilter {
	public:
		Polygonizer *pol;
		LineStringAdder(Polygonizer *p);
		void filter_rw(Geometry *g);
		void filter_ro(const Geometry *g){};
	};

	// default factory
	LineStringAdder *lineStringAdder;

	/**
	* Add a linestring to the graph of polygon edges.
	*
	* @param line the {@link LineString} to add
	*/
	void add(LineString *line);
	/**
	* Perform the polygonization, if it has not already been carried out.
	*/
	void polygonize();
	void findValidRings(vector<polygonizeEdgeRing*> *edgeRingList, vector<polygonizeEdgeRing*> *validEdgeRingList, vector<LineString*> *invalidRingList);
	void findShellsAndHoles(vector<polygonizeEdgeRing*> *edgeRingList);
	static void assignHolesToShells(vector<polygonizeEdgeRing*> *holeList,vector<polygonizeEdgeRing*> *shellList);
	static void assignHoleToShell(polygonizeEdgeRing *holeER,vector<polygonizeEdgeRing*> *shellList);
protected:
	PolygonizeGraph *graph;

	// initialize with empty collections, in case nothing is computed
	vector<const LineString*> *dangles;
	vector<const LineString*> *cutEdges;
	vector<LineString*> *invalidRingLines;

	vector<polygonizeEdgeRing*> *holeList;
	vector<polygonizeEdgeRing*> *shellList;
	vector<Polygon*> *polyList;

public:

	/*
	 * Create a polygonizer with the same GeometryFactory
	 * as the input Geometry
	 */
	Polygonizer();

	~Polygonizer();

	/*
	 * Add a collection of geometries to be polygonized.
	 * May be called multiple times.
	 * Any dimension of Geometry may be added;
	 * the constituent linework will be extracted and used
	 *
	 * @param geomList a list of Geometry with linework to be polygonized
	 */
	void add(vector<Geometry*> *geomList);

	/**
	 * Add a geometry to the linework to be polygonized.
	 * May be called multiple times.
	 * Any dimension of Geometry may be added;
	 * the constituent linework will be extracted and used
	 *
	 * @param g a Geometry with linework to be polygonized
	 */
	void add(Geometry *g);

	/**
	 * Gets the list of polygons formed by the polygonization.
	 * Ownership of vector is transferred to caller, subsequent
	 * calls will return NULL.
	 * @return a collection of Polygons
	 */
	vector<Polygon*>* getPolygons();

	/**
	 * Get the list of dangling lines found during polygonization.
	 * @return a collection of the input LineStrings which are dangles
	 */
	vector<const LineString*>* getDangles();


	/**
	 * Get the list of cut edges found during polygonization.
	 * @return a collection of the input LineStrings which are cut edges
	 */
	vector<const LineString*>* getCutEdges();

	/*
	 * Get the list of lines forming invalid rings found during
	 * polygonization.
	 * Ownership is tranferred to caller, second call will return
	 * NULL (unless polygonize is called again).
	 * @return a collection of LineStrings which form
	 * invalid rings
	 */
	vector<LineString*>* getInvalidRingLines();
};

} // namespace geos
#endif

/**********************************************************************
 * $Log$
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
 * Revision 1.2  2004/07/08 19:34:49  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added DefaultCoordinateSequenceFactory::instance() function.
 *
 * Revision 1.1  2004/07/02 13:20:42  strk
 * Header files moved under geos/ dir.
 *
 * Revision 1.1  2004/04/08 04:53:56  ybychkov
 * "operation/polygonize" ported from JTS 1.4
 *
 *
 **********************************************************************/
