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
 * Revision 1.1  2004/07/02 13:20:42  strk
 * Header files moved under geos/ dir.
 *
 * Revision 1.1  2004/04/07 06:55:50  ybychkov
 * "operation/linemerge" ported from JTS 1.4
 *
 *
 **********************************************************************/


#ifndef GEOS_OPLINEMERGE_H
#define GEOS_OPLINEMERGE_H

#include <geos/platform.h>
#include <geos/planargraph.h>
#include <geos/geom.h>
#include <vector>

namespace geos {

/**
 * An edge of a {@link LineMergeGraph}. The <code>marked</code> field indicates
 * whether this Edge has been logically deleted from the graph.
 *
 */
class LineMergeEdge: public planarEdge {
private:
	LineString *line;
public:
	/**
	* Constructs a LineMergeEdge with vertices given by the specified LineString.
	*/
	LineMergeEdge(LineString *newLine);
	/**
	* Returns the LineString specifying the vertices of this edge.
	*/
	LineString* getLine();
};


/**
 * A {@link com.vividsolutions.jts.planargraph.DirectedEdge} of a 
 * {@link LineMergeGraph}. 
 *
 */
class LineMergeDirectedEdge: public planarDirectedEdge {
public:
	/**
	* Constructs a LineMergeDirectedEdge connecting the <code>from</code> node to the
	* <code>to</code> node.
	*
	* @param directionPt
	*                  specifies this DirectedEdge's direction (given by an imaginary
	*                  line from the <code>from</code> node to <code>directionPt</code>)
	* @param edgeDirection
	*                  whether this DirectedEdge's direction is the same as or
	*                  opposite to that of the parent Edge (if any)
	*/  
	LineMergeDirectedEdge(planarNode *newFrom,planarNode *newTo,Coordinate& newDirectionPt,bool nEdgeDirection);
	/**
	* Returns the directed edge that starts at this directed edge's end point, or null
	* if there are zero or multiple directed edges starting there.  
	* @return
	*/
	LineMergeDirectedEdge* getNext();
};

/**
 * A sequence of {@link LineMergeDirectedEdge}s forming one of the lines that will
 * be output by the line-merging process.
 *
 */
class EdgeString {
private:
	GeometryFactory *factory;
	vector<LineMergeDirectedEdge*> *directedEdges;
	CoordinateList *coordinates;
	const CoordinateList* getCoordinates();
public:
	/**
	* Constructs an EdgeString with the given factory used to convert this EdgeString
	* to a LineString
	*/
	EdgeString(GeometryFactory *newFactory);
	~EdgeString();
	/**
	* Adds a directed edge which is known to form part of this line.
	*/
	void add(LineMergeDirectedEdge *directedEdge);
	/**
	* Converts this EdgeString into a LineString.
	*/
	LineString* toLineString();
};

/**
 * A planar graph of edges that is analyzed to sew the edges together. The 
 * <code>marked</code> flag on @{link com.vividsolutions.planargraph.Edge}s 
 * and @{link com.vividsolutions.planargraph.Node}s indicates whether they have been
 * logically deleted from the graph.
 *
 */
class LineMergeGraph: public planarPlanarGraph {
public:
	/**
	* Adds an Edge, DirectedEdges, and Nodes for the given LineString representation
	* of an edge. 
	*/
	void addEdge(LineString *lineString);
private:
	planarNode* getNode(Coordinate &coordinate);
};

/**
 * Sews together a set of fully noded LineStrings. Sewing stops at nodes of degree 1
 * or 3 or more -- the exception is an isolated loop, which only has degree-2 nodes,
 * in which case a node is simply chosen as a starting point. The direction of each
 * merged LineString will be that of the majority of the LineStrings from which it
 * was derived.
 * <p>
 * Any dimension of Geometry is handled -- the constituent linework is extracted to 
 * form the edges. The edges must be correctly noded; that is, they must only meet
 * at their endpoints.  The LineMerger will still run on incorrectly noded input
 * but will not form polygons from incorrected noded edges.
 *
 */
class LineMerger {
public:
	LineMerger();
	~LineMerger();
	/**
	* Adds a collection of Geometries to be processed. May be called multiple times.
	* Any dimension of Geometry may be added; the constituent linework will be
	* extracted.
	*/
	void add(vector<Geometry*> *geometries);
	/**
	* Adds a Geometry to be processed. May be called multiple times.
	* Any dimension of Geometry may be added; the constituent linework will be
	* extracted.
	*/  
	void add(Geometry *geometry);
	/**
	* Returns the LineStrings built by the merging process.
	*/
	vector<LineString*>* getMergedLineStrings();
	void add(LineString *lineString);
private:
	LineMergeGraph *graph;
	vector<LineString*> *mergedLineStrings;
	vector<EdgeString*> *edgeStrings;
	GeometryFactory *factory;
	void merge();
	void buildEdgeStringsForObviousStartNodes();
	void buildEdgeStringsForIsolatedLoops();
	void buildEdgeStringsForUnprocessedNodes();
	void buildEdgeStringsForNonDegree2Nodes();
	void buildEdgeStringsStartingAt(planarNode *node);
	EdgeString* buildEdgeStringStartingWith(LineMergeDirectedEdge *start);
};

class LMGeometryComponentFilter: public GeometryComponentFilter {
public:
	LineMerger *lm;
	LMGeometryComponentFilter(LineMerger *newLm);
	virtual void filter_rw(Geometry *geom);
	virtual void filter_ro(const Geometry *geom){}; // Unsupported
};

}
#endif

