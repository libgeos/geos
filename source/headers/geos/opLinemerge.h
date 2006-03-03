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

#ifndef GEOS_OPLINEMERGE_H
#define GEOS_OPLINEMERGE_H

#include <geos/platform.h>
#include <geos/planargraph.h>
#include <geos/geom.h>
#include <memory>
#include <vector>
#include <cassert>

namespace geos {
namespace operation { // geos.operation

/// Line merging package
namespace linemerge { // geos.operation.linemerge

//using namespace planargraph;

class EdgeString;
class LineMergeDirectedEdge;
class LineMergeEdge;
class LineMergeGraph;
class LineMerger;
class LineSequencer;

/*
 * An edge of a LineMergeGraph. The <code>marked</code> field indicates
 * whether this Edge has been logically deleted from the graph.
 */
class LineMergeEdge: public planargraph::Edge {
private:
	const LineString *line;
public:
	/**
	 * Constructs a LineMergeEdge with vertices given by the specified
	 * LineString.
	 */
	LineMergeEdge(const LineString *newLine);

	/**
	 * Returns the LineString specifying the vertices of this edge.
	 */
	const LineString* getLine() const;
};


/**
 * \class LineMergeDirectedEdge opLinemerge.h geos/opLinemerge.h
 * \brief
 * A planargraph::DirectedEdge of a LineMergeGraph. 
 *
 */
class LineMergeDirectedEdge: public planargraph::DirectedEdge {
public:
	/**
	 * Constructs a LineMergeDirectedEdge connecting the <code>from</code>
	 * node to the <code>to</code> node.
	 *
	 * @param directionPt
	 *        specifies this DirectedEdge's direction (given by an
	 *	  imaginary line from the <code>from</code> node to
	 *	  <code>directionPt</code>)
	 *
	 * @param edgeDirection
	 *        whether this DirectedEdge's direction is the same as or
	 *        opposite to that of the parent Edge (if any)
	 */  
	LineMergeDirectedEdge(planargraph::Node *from, planargraph::Node *to, const Coordinate& directionPt, bool edgeDirection);

	/**
	* Returns the directed edge that starts at this directed edge's end point, or null
	* if there are zero or multiple directed edges starting there.  
	* @return
	*/
	LineMergeDirectedEdge* getNext();
};

/*
 * \brief
 * A sequence of LineMergeDirectedEdge forming one of the lines that will
 * be output by the line-merging process.
 */
class EdgeString {
private:
	const GeometryFactory *factory;
	std::vector<LineMergeDirectedEdge*> *directedEdges;
	CoordinateSequence *coordinates;
	CoordinateSequence* getCoordinates();
public:
	/*
	 * \brief
	 * Constructs an EdgeString with the given factory used to
	 * convert this EdgeString to a LineString
	 */
	EdgeString(const GeometryFactory *newFactory);

	~EdgeString();

	/**
	* Adds a directed edge which is known to form part of this line.
	*/
	void add(LineMergeDirectedEdge *directedEdge);

	/*
	 * Converts this EdgeString into a LineString.
	 */
	LineString* toLineString();
};

/**
 * A planar graph of edges that is analyzed to sew the edges together. The 
 * <code>marked</code> flag on planargraph::Edge
 * and planargraph::Node indicates whether they have been
 * logically deleted from the graph.
 */
class LineMergeGraph: public planargraph::PlanarGraph {

private:

	planargraph::Node* getNode(const Coordinate &coordinate);

	std::vector<planargraph::Node*> newNodes;

	std::vector<planargraph::Edge*> newEdges;

	std::vector<planargraph::DirectedEdge*> newDirEdges;

public:

	/**
	 * Adds an Edge, DirectedEdges, and Nodes for the given
	 * LineString representation of an edge. 
	 */
	void addEdge(const LineString *lineString);

	~LineMergeGraph();
};

/**
 * \class LineMerger opLinemerge.h geos/opLinemerge.h
 *
 * \brief
 * Sews together a set of fully noded LineStrings.
 *
 * Sewing stops at nodes of degree 1 or 3 or more.
 * The exception is an isolated loop, which only has degree-2 nodes,
 * in which case a node is simply chosen as a starting point.
 * The direction of each merged LineString will be that of the majority
 * of the LineStrings from which it was derived.
 * 
 * Any dimension of Geometry is handled.
 * The constituent linework is extracted to form the edges.
 * The edges must be correctly noded; that is, they must only meet
 * at their endpoints. 
 *
 * The LineMerger will still run on incorrectly noded input
 * but will not form polygons from incorrected noded edges.
 *
 */
class LineMerger {

private:

	LineMergeGraph graph;

	std::vector<LineString*> *mergedLineStrings;

	std::vector<EdgeString*> edgeStrings;

	const GeometryFactory *factory;

	void merge();

	void buildEdgeStringsForObviousStartNodes();

	void buildEdgeStringsForIsolatedLoops();

	void buildEdgeStringsForUnprocessedNodes();

	void buildEdgeStringsForNonDegree2Nodes();

	void buildEdgeStringsStartingAt(planargraph::Node *node);

	EdgeString* buildEdgeStringStartingWith(LineMergeDirectedEdge *start);

public:
	LineMerger();
	~LineMerger();

	/**
	 * \brief
	 * Adds a collection of Geometries to be processed.
	 * May be called multiple times.
	 *
	 * Any dimension of Geometry may be added; the constituent
	 * linework will be extracted.
	 */
	void add(std::vector<Geometry*> *geometries);

	/**
	 * \brief
	 * Adds a Geometry to be processed.
	 * May be called multiple times.
	 *
	 * Any dimension of Geometry may be added; the constituent
	 * linework will be extracted.
	 */  
	void add(const Geometry *geometry);

	/**
	 * \brief
	 * Returns the LineStrings built by the merging process.
	 */
	std::vector<LineString*>* getMergedLineStrings();

	void add(const LineString *lineString);

};

/**
 * Builds a sequence from a set of LineStrings so that
 * they are ordered end to end.
 * A sequence is a complete non-repeating list of the linear
 * components of the input.  Each linestring is oriented
 * so that identical endpoints are adjacent in the list.
 *
 * The input linestrings may form one or more connected sets.
 * The input linestrings should be correctly noded, or the results may
 * not be what is expected.
 * The output of this method is a single MultiLineString containing the ordered
 * linestrings in the sequence.
 * 
 * The sequencing employs the classic <b>Eulerian path</b> graph algorithm.
 * Since Eulerian paths are not uniquely determined,
 * further rules are used to
 * make the computed sequence preserve as much as possible of the input
 * ordering.
 * Within a connected subset of lines, the ordering rules are:
 *
 * - If there is degree-1 node which is the start
 *   node of an linestring, use that node as the start of the sequence
 * - If there is a degree-1 node which is the end
 *   node of an linestring, use that node as the end of the sequence
 * - If the sequence has no degree-1 nodes, use any node as the start
 *
 * Not all arrangements of lines can be sequenced.
 * For a connected set of edges in a graph,
 * Euler's Theorem states that there is a sequence containing each edge once
 * if and only if there are no more than 2 nodes of odd degree.
 * If it is not possible to find a sequence, the isSequenceable method
 * will return <code>false</code>.
 *
 * Last port: operation/linemerge/LineSequencer.java rev. 1.5 (JTS-1.7)
 */
class LineSequencer {

private:
	typedef std::vector<planargraph::DirectedEdge::NonConstList *> Sequences;

	LineMergeGraph graph;
	const GeometryFactory *factory;
	unsigned int lineCount;
	bool isRun;
	std::auto_ptr<Geometry> sequencedGeometry;
	bool isSequenceableVar;

	void addLine(const LineString *lineString);
	void computeSequence();
	Sequences* findSequences();
	planargraph::DirectedEdge::NonConstList* findSequence(planargraph::Subgraph& graph);

	/// return a newly allocated LineString
	static LineString* reverse(const LineString *line);

	/**
	 * Builds a geometry ({@link LineString} or {@link MultiLineString} )
	 * representing the sequence.
	 *
	 * @param sequences a vector of vectors of const planarDirectedEdges
	 *                  with LineMergeEdges as their parent edges.
	 * @return the sequenced geometry, possibly NULL
	 *         if no sequence exists
	 */
	Geometry* buildSequencedGeometry(const Sequences& sequences);

	static const planargraph::Node* findLowestDegreeNode(const planargraph::Subgraph& graph);

	void addReverseSubpath(const planargraph::DirectedEdge *de,
		planargraph::DirectedEdge::NonConstList& deList,
		planargraph::DirectedEdge::NonConstList::iterator lit,
		bool expectedClosed);
	
	/**
	 * Finds an {@link DirectedEdge} for an unvisited edge (if any),
	 * choosing the dirEdge which preserves orientation, if possible.
	 *
	 * @param node the node to examine
	 * @return the dirEdge found, or <code>null</code>
	 *         if none were unvisited
	 */
	static const planargraph::DirectedEdge* findUnvisitedBestOrientedDE(const planargraph::Node* node);

	/**
	 * Computes a version of the sequence which is optimally
	 * oriented relative to the underlying geometry.
	 * 
	 * Heuristics used are:
	 * 
	 * - If the path has a degree-1 node which is the start
	 *   node of an linestring, use that node as the start of the sequence
	 * - If the path has a degree-1 node which is the end
	 *   node of an linestring, use that node as the end of the sequence
	 * - If the sequence has no degree-1 nodes, use any node as the start
	 *   (NOTE: in this case could orient the sequence according to the
	 *   majority of the linestring orientations)
	 *
	 * @param seq a List of planarDirectedEdges 
	 * @return the oriented sequence, possibly same as input if already
	 *         oriented
	 */
	planargraph::DirectedEdge::NonConstList*
	orient(planargraph::DirectedEdge::NonConstList* seq);

	/**
	 * Reverse the sequence.
	 * This requires reversing the order of the dirEdges, and flipping
	 * each dirEdge as well
	 *
	 * @param seq a List of DirectedEdges, in sequential order
	 * @return the reversed sequence
	 */
	planargraph::DirectedEdge::NonConstList* reverse(planargraph::DirectedEdge::NonConstList& seq);

	/**
	 * Tests whether a complete unique path exists in a graph
	 * using Euler's Theorem.
	 *
	 * @param graph the subgraph containing the edges
	 * @return <code>true</code> if a sequence exists
	 */
	bool hasSequence(planargraph::Subgraph& graph);

public:

	LineSequencer()
		:
		factory(NULL),
		lineCount(0),
		isRun(false),
		sequencedGeometry(NULL),
		isSequenceableVar(false)
		{}

	/**
	 * Tests whether a {@link Geometry} is sequenced correctly.
	 * {@llink LineString}s are trivially sequenced.
	 * {@link MultiLineString}s are checked for correct sequencing.
	 * Otherwise, <code>isSequenced</code> is defined
	 * to be <code>true</code> for geometries that are not lineal.
	 *
	 * @param geom the geometry to test
	 * @return true if the geometry is sequenced or is not lineal
	 */
	static bool isSequenced(const Geometry* geom);

	/**
	 * Tests whether the arrangement of linestrings has a valid
	 * sequence.
	 *
	 * @return <code>true</code> if a valid sequence exists.
	 */
	bool isSequenceable() {
		computeSequence();
		return isSequenceableVar;
	}

	/**
	 * Adds a {@link Geometry} to be sequenced.
	 * May be called multiple times.
	 * Any dimension of Geometry may be added; the constituent
	 * linework will be extracted.
	 *
	 * @param geometry the geometry to add
	 */
	void add(const Geometry& geometry) {
		geometry.applyComponentFilter(*this);
	}

	/**
	 * Act as a GeometryComponentFilter so to extract
	 * the linearworks
	 */
	void filter(const Geometry* g)
	{
		if (const LineString *ls=dynamic_cast<const LineString *>(g))
		{
			addLine(ls);
		}
	}

	/**
	 * Returns the LineString or MultiLineString
	 * built by the sequencing process, if one exists.
	 *
	 * @param release release ownership of computed Geometry
	 * @return the sequenced linestrings,
	 *         or <code>null</code> if a valid sequence
	 *         does not exist.
	 */
	Geometry* getSequencedLineStrings(bool release=1) {
		computeSequence();
		if (release) return sequencedGeometry.release();
		else return sequencedGeometry.get();
	}


};

} // namespace geos.operation.linemerge
} // namespace geos.operation
} // namespace geos

#endif

/**********************************************************************
 * $Log$
 * Revision 1.11  2006/03/03 10:46:21  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.10  2006/02/24 15:39:07  strk
 * - operator>> for Coordinate, planarNode and planarEdge
 * - Fixed bug in planarGraphComponent::setMarked
 * - Added linemerge.xml test (single test, should grow a bit)
 *
 * Revision 1.9  2006/02/23 23:17:52  strk
 * - Coordinate::nullCoordinate made private
 * - Simplified Coordinate inline definitions
 * - LMGeometryComponentFilter definition moved to LineMerger.cpp file
 * - Misc cleanups
 *
 * Revision 1.8  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.7  2006/02/08 12:59:55  strk
 * - NEW Geometry::applyComponentFilter() templated method
 * - Changed Geometry::getGeometryN() to take unsigned int and getNumGeometries
 *   to return unsigned int.
 * - Changed planarNode::getDegree() to return unsigned int.
 * - Added Geometry::NonConstVect typedef
 * - NEW LineSequencer class
 * - Changed planarDirectedEdgeStar::outEdges from protected to private
 * - added static templated setVisitedMap to change Visited flag
 *   for all values in a map
 * - Added const versions of some planarDirectedEdgeStar methods.
 * - Added containers typedefs for planarDirectedEdgeStar
 *
 * Revision 1.6  2005/09/26 11:01:32  strk
 * Const correctness changes in LineMerger package, and a few speedups.
 *
 * Revision 1.5  2005/09/23 17:20:13  strk
 * Made LineMerger graph be a real object (rather then a pointer to it)
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
 * Revision 1.1  2004/04/07 06:55:50  ybychkov
 * "operation/linemerge" ported from JTS 1.4
 *
 *
 **********************************************************************/
