/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2006      Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#ifndef GEOS_NODING_H
#define GEOS_NODING_H

#include <string>
#include <vector>
#include <set>
#include <cassert>
#include <geos/platform.h>
#include <geos/geom.h>
//#include <geos/geomgraph.h>
#include <geos/geosAlgorithm.h>


namespace geos {

/// Classes to compute nodings for arrangements of line segments and line segment sequences.
namespace noding { // geos.noding

class SegmentString;
class Octant;
class SegmentIntersector;

/// Temporary typedef for namespace transition
typedef SegmentIntersector nodingSegmentIntersector;

/** \brief
 * Methods for computing and working with octants of the Cartesian plane.
 *
 * Octants are numbered as follows:
 * 
 *   2|1 
 * 3  |  0
 * ---+--
 * 4  |  7
 *   5|6 
 *
 * If line segments lie along a coordinate axis, the octant is the lower of the two
 * possible values.
 *
 * Last port: noding/Octant.java rev. 1.2 (JTS-1.7)
 */
class Octant {
private:
	Octant() {} // Can't instanciate it
public:

	/**
	 * Returns the octant of a directed line segment (specified
	 * as x and y displacements, which cannot both be 0).
	 */
	static int octant(double dx, double dy);

	/**
	 * Returns the octant of a directed line segment from p0 to p1.
	 */
	static int octant(const Coordinate& p0, const Coordinate& p1);
	static int octant(const Coordinate* p0, const Coordinate* p1) {
		return octant(*p1, *p1);
	}
};


/// Represents an intersection point between two SegmentString.
//
/// Final class.
///
/// Last port: noding/SegmentNode.java rev. 1.5 (JTS-1.7)
///
class SegmentNode {
private:
	const SegmentString& segString;

	int segmentOctant;

	bool isInteriorVar;

public:

	/// the point of intersection (own copy)
	Coordinate coord;  

	/// the index of the containing line segment in the parent edge
	unsigned int segmentIndex;  

	/// Construct a node on the given SegmentString
	//
	/// @param ss the parent SegmentString 
	///
	/// @param coord the coordinate of the intersection, will be copied
	///
	/// @param nSegmentIndex the index of the segment on parent SegmentString
	///        where the Node is located.
	///
	/// @param nSegmentOctant
	///
	SegmentNode(const SegmentString& ss, const Coordinate& nCoord,
			unsigned int nSegmentIndex, int nSegmentOctant);

	~SegmentNode() {}

	/// \brief
	/// Return true if this Node is *internal* (not on the boundary)
	/// of the corresponding segment. Currently only the *first*
	/// segment endpoint is checked, actually.
	///
	bool isInterior() const { return isInteriorVar; }

	bool isEndPoint(unsigned int maxSegmentIndex) const;

	/**
	 * @return -1 this EdgeIntersection is located before
	 *            the argument location
	 * @return 0 this EdgeIntersection is at the argument location
	 * @return 1 this EdgeIntersection is located after the
	 *           argument location
	 */
	int compareTo(const SegmentNode& other);

	string print() const;
};

inline ostream& operator<< (ostream& os, const SegmentNode& n) {
	return os<<n.print();
}

struct SegmentNodeLT {
	bool operator()(SegmentNode *s1, SegmentNode *s2) const {
		return s1->compareTo(*s2)<0;
	}
};

/** \brief
 * A list of the SegmentNode present along a
 * noded SegmentString.
 *
 * Last port: noding/SegmentNodeList.java rev. 1.7 (JTS-1.7)
 */
class SegmentNodeList {
private:
	set<SegmentNode*,SegmentNodeLT> nodeMap;

	// the parent edge
	const SegmentString& edge; 

	// UNUSED
	//std::vector<SegmentNode*> *sortedNodes;

	// This vector is here to keep track of created splitEdges
	std::vector<SegmentString*> splitEdges;

	// This vector is here to keep track of created Coordinates
	std::vector<CoordinateSequence*> splitCoordLists;

	/**
	 * Checks the correctness of the set of split edges corresponding
	 * to this edge
	 *
	 * @param splitEdges the split edges for this edge (in order)
	 */
	void checkSplitEdgesCorrectness(std::vector<SegmentString*>& splitEdges);

	/**
	 * Create a new "split edge" with the section of points between
	 * (and including) the two intersections.
	 * The label for the new edge is the same as the label for the
	 * parent edge.
	 */
	SegmentString* createSplitEdge(SegmentNode *ei0, SegmentNode *ei1);

	/**
	 * Adds nodes for any collapsed edge pairs.
	 * Collapsed edge pairs can be caused by inserted nodes, or they
	 * can be pre-existing in the edge vertex list.
	 * In order to provide the correct fully noded semantics,
	 * the vertex at the base of a collapsed pair must also be added
	 * as a node.
	 */
	void addCollapsedNodes();

	/**
	 * Adds nodes for any collapsed edge pairs
	 * which are pre-existing in the vertex list.
	 */
	void findCollapsesFromExistingVertices(
			std::vector<unsigned int>& collapsedVertexIndexes);

	/**
	 * Adds nodes for any collapsed edge pairs caused by inserted nodes
	 * Collapsed edge pairs occur when the same coordinate is inserted
	 * as a node both before and after an existing edge vertex.
	 * To provide the correct fully noded semantics,
	 * the vertex must be added as a node as well.
	 */
	void findCollapsesFromInsertedNodes(
		std::vector<unsigned int>& collapsedVertexIndexes);

	bool findCollapseIndex(SegmentNode& ei0, SegmentNode& ei1,
		unsigned int& collapsedVertexIndex);
public:

	typedef set<SegmentNode*,SegmentNodeLT> container;
	typedef container::iterator iterator;
	typedef container::const_iterator const_iterator;


	SegmentNodeList(const SegmentString* newEdge): edge(*newEdge) {}

	SegmentNodeList(const SegmentString& newEdge): edge(newEdge) {}

	const SegmentString& getEdge() const { return edge; }

	// TODO: Is this a final class ?
	// Should remove the virtual in that case
	virtual ~SegmentNodeList();

	/**
	 * Adds an intersection into the list, if it isn't already there.
	 * The input segmentIndex is expected to be normalized.
	 *
	 * @return the SegmentIntersection found or added. It will be
	 *	   destroyed at SegmentNodeList destruction time.
	 *
	 * @param intPt the intersection Coordinate, will be copied
	 * @param segmentIndex 
	 */
	SegmentNode* add(const Coordinate& intPt, unsigned int segmentIndex);

	SegmentNode* add(const Coordinate *intPt, unsigned int segmentIndex) {
		return add(*intPt, segmentIndex);
	}

	/*
	 * returns the set of SegmentNodes
	 */
	//replaces iterator()
	// TODO: obsolete this function
	set<SegmentNode*,SegmentNodeLT>* getNodes() { return &nodeMap; }

	/// Return the number of nodes in this list
	unsigned int size() { return nodeMap.size(); }

	container::iterator begin() { return nodeMap.begin(); }
	container::const_iterator begin() const { return nodeMap.begin(); }
	container::iterator end() { return nodeMap.end(); }
	container::const_iterator end() const { return nodeMap.end(); }

	/**
	 * Adds entries for the first and last points of the edge to the list
	 */
	void addEndpoints();

	/**
	 * Creates new edges for all the edges that the intersections in this
	 * list split the parent edge into.
	 * Adds the edges to the input list (this is so a single list
	 * can be used to accumulate all split edges for a Geometry).
	 */
	void addSplitEdges(std::vector<SegmentString*>& edgeList);

	void addSplitEdges(std::vector<SegmentString*>* edgeList) {
		assert(edgeList);
		addSplitEdges(*edgeList);
	}

	string print();
};


/** \brief
 * Represents a list of contiguous line segments,
 * and supports noding the segments.
 *
 * The line segments are represented by a CoordinateSequence.
 *
 * TODO:
 * This should be changed to use a vector of Coordinate,
 * to optimize the noding of contiguous segments by
 * reducing the number of allocated objects.
 *
 * SegmentStrings can carry a context object, which is useful
 * for preserving topological or parentage information.
 * All noded substrings are initialized with the same context object.
 *
 * Final class.
 *
 * Last port: noding/SegmentString.java rev. 1.5 (JTS-1.7)
 */
class SegmentString {
public:
	typedef std::vector<const SegmentString*> ConstVect;
	typedef std::vector<SegmentString *> NonConstVect;

private:
	SegmentNodeList eiList;
	CoordinateSequence *pts;
	unsigned int npts;
	const void* context;
	bool isIsolatedVar;

	void testInvariant() const {
		assert(pts);
		assert(pts->size() > 1);
		assert(pts->size() == npts);
	}
public:

	//SegmentString(const CoordinateSequence *newPts, const void* newContext)

	/// Construct a SegmentString.
	//
	/// @param newPts CoordinateSequence representing the string,
	/// externally owned
	///
	/// @param newContext the context associated to this SegmentString
	///
	SegmentString(CoordinateSequence *newPts, const void* newContext)
		:
		eiList(this),
		pts(newPts),
		npts(newPts->getSize()),
		context(newContext),
		isIsolatedVar(false)
	{
		testInvariant();
	}

	~SegmentString() {}

	const void* getContext() const {
		testInvariant();
		return getData();
	}

	const void* getData() const {
		testInvariant();
		return context;
	}

	const SegmentNodeList& getNodeList() const {
		testInvariant();
		return eiList;
	}
	SegmentNodeList& getNodeList() {
		testInvariant();
		return eiList;
	}

	unsigned int size() const {
		testInvariant();
		return npts;
	}

	const Coordinate& getCoordinate(unsigned int i) const {
		testInvariant();
		return pts->getAt(i);
	}

	/// \brief
	/// Return a pointer to the CoordinateSequence associated
	/// with this SegmentString.
	//
	/// Note that the CoordinateSequence is not owned by
	/// this SegmentString!
	///
	CoordinateSequence* getCoordinates() const {
		testInvariant();
		return pts;
	}

	// Return a read-only pointer to this SegmentString CoordinateSequence
	//const CoordinateSequence* getCoordinatesRO() const { return pts; }

	void setIsolated(bool isIsolated) {
		isIsolatedVar=isIsolated;
		testInvariant();
	}

	bool isIsolated() const {
		testInvariant();
		return isIsolatedVar;
	}
	
	bool isClosed() const {
		testInvariant();
		return pts->getAt(0)==pts->getAt(npts-1);
	}

	/** \brief
	 * Gets the octant of the segment starting at vertex <code>index</code>.
	 *
	 * @param index the index of the vertex starting the segment. 
	 *              Must not be the last index in the vertex list
	 * @return the octant of the segment at the vertex
	 */
	int getSegmentOctant(unsigned int index) const
	{
		testInvariant();
		if (index == size() - 1) return -1;
		return Octant::octant(getCoordinate(index), getCoordinate(index+1));
	}

	/** \brief
	 * Adds EdgeIntersections for one or both
	 * intersections found for a segment of an edge to the edge
	 * intersection list.
	 */
	void addIntersections(algorithm::LineIntersector *li, unsigned int segmentIndex,
			int geomIndex);

	/** \brief
	 * Add an SegmentNode for intersection intIndex.
	 *
	 * An intersection that falls exactly on a vertex
	 * of the SegmentString is normalized
	 * to use the higher of the two possible segmentIndexes
	 */
	void addIntersection(algorithm::LineIntersector *li, unsigned int segmentIndex,
			int geomIndex, int intIndex);

	/** \brief
	 * Add an EdgeIntersection for intersection intIndex.
	 *
	 * An intersection that falls exactly on a vertex of the
	 * edge is normalized
	 * to use the higher of the two possible segmentIndexes
	 */
	void addIntersection(const Coordinate& intPt, unsigned int segmentIndex);

	static void getNodedSubstrings(const SegmentString::NonConstVect& segStrings,
			SegmentString::NonConstVect* resultEdgeList);

	static SegmentString::NonConstVect* getNodedSubstrings(
			const SegmentString::NonConstVect& segStrings);
};

/**
 * \brief
 * Computes the intersections between two line segments in SegmentString
 * and adds them to each string.
 *
 * The SegmentIntersector is passed to a Noder.
 * The addIntersections method is called whenever the Noder
 * detects that two SegmentStrings <i>might</i> intersect.
 * This class is an example of the <i>Strategy</i> pattern.
 *
 * Last port: noding/SegmentIntersector.java rev. 1.7 (JTS-1.7)
 *
 */
class SegmentIntersector {

public:

	/**
	 * This method is called by clients
	 * of the {@link SegmentIntersector} interface to process
	 * intersections for two segments of the {@link SegmentStrings}
	 * being intersected.
	 */
	virtual void processIntersections(
		SegmentString* e0,  int segIndex0,
		SegmentString* e1,  int segIndex1)=0;

protected:

	SegmentIntersector() {}
	virtual ~SegmentIntersector() {}
 
};

/**
 * Computes the intersections between two line segments in SegmentString
 * and adds them to each string.
 * The {@link SegmentIntersector} is passed to a {@link Noder}.
 * The {@link addIntersections} method is called whenever the {@link Noder}
 * detects that two SegmentStrings <i>might</i> intersect.
 * This class is an example of the <i>Strategy</i> pattern.
 *
 * Last port: noding/IntersectionAdder.java rev. 1.4 (JTS-1.7)
 *
 */
class IntersectionAdder: public SegmentIntersector {

private:

	/**
	 * These variables keep track of what types of intersections were
	 * found during ALL edges that have been intersected.
	 */
	bool hasIntersectionVar;
	bool hasProper;
	bool hasProperInterior;
	bool hasInterior;

	// the proper intersection point found
	const Coordinate* properIntersectionPoint;

	algorithm::LineIntersector& li;
	bool isSelfIntersection;
	//bool intersectionFound;

	/**
	 * A trivial intersection is an apparent self-intersection which
	 * in fact is simply the point shared by adjacent line segments.
	 * Note that closed edges require a special check for the point
	 * shared by the beginning and end segments.
	 */
	bool isTrivialIntersection(const SegmentString* e0, int segIndex0,
			const SegmentString* e1, int segIndex1);
 
 

public:

	int numIntersections;
	int numInteriorIntersections;
	int numProperIntersections;

	// testing only
	int numTests;

	IntersectionAdder(algorithm::LineIntersector& newLi)
		:
		hasIntersectionVar(false),
		hasProper(false),
		hasProperInterior(false),
		hasInterior(false),
		properIntersectionPoint(NULL),
		li(newLi),
		numIntersections(0),
		numInteriorIntersections(0),
		numProperIntersections(0),
		numTests(0)
	{}

	algorithm::LineIntersector& getLineIntersector() { return li; }

	/**
	 * @return the proper intersection point, or <code>NULL</code>
	 *         if none was found
	 */
	const Coordinate* getProperIntersectionPoint()  {
		return properIntersectionPoint;
	}

	bool hasIntersection() { return hasIntersectionVar; }

	/**
	 * A proper intersection is an intersection which is interior to
	 * at least two line segments.  Note that a proper intersection
	 * is not necessarily in the interior of the entire Geometry,
	 * since another edge may have an endpoint equal to the intersection,
	 * which according to SFS semantics can result in the point being
	 * on the Boundary of the Geometry.
	 */
	bool hasProperIntersection() { return hasProper; }

	/**
	 * A proper interior intersection is a proper intersection which is
	 * <b>not</b> contained in the set of boundary nodes set for this
	 * SegmentIntersector.
	 */
	bool hasProperInteriorIntersection() { return hasProperInterior; }

	/**
	 * An interior intersection is an intersection which is
	 * in the interior of some segment.
	 */
	bool hasInteriorIntersection() { return hasInterior; }


	/**
	 * This method is called by clients
	 * of the {@link SegmentIntersector} class to process
	 * intersections for two segments of the SegmentStrings being
	 * intersected.
	 * Note that some clients (such as MonotoneChains) may optimize away
	 * this call for segment pairs which they have determined do not
	 * intersect (e.g. by an disjoint envelope test).
	 */
	void processIntersections(
		SegmentString* e0,  int segIndex0,
		SegmentString* e1,  int segIndex1);

 
	static bool isAdjacentSegments(int i1, int i2) {
		return abs(i1 - i2) == 1;
	}

};
 


/**
 * Finds proper and interior intersections in a set of SegmentStrings,
 * and adds them as nodes.
 *
 * Last port: noding/IntersectionFinderAdder.java rev. 1.2 (JTS-1.7)
 *
 */
class IntersectionFinderAdder: public SegmentIntersector {

private:
	algorithm::LineIntersector& li;
	std::vector<Coordinate>& interiorIntersections;

public:

	/**
	 * Creates an intersection finder which finds all proper intersections
	 * and stores them in the provided Coordinate array
	 *
	 * @param li the LineIntersector to use
	 */
	IntersectionFinderAdder(algorithm::LineIntersector& newLi, std::vector<Coordinate>& v)
		:
		li(newLi),
		interiorIntersections(v)
	{}

	/**
	 * This method is called by clients
	 * of the {@link SegmentIntersector} class to process
	 * intersections for two segments of the {@link SegmentStrings}
	 * being intersected.
	 * Note that some clients (such as {@link MonotoneChain}s) may
	 * optimize away this call for segment pairs which they have
	 * determined do not intersect
	 * (e.g. by an disjoint envelope test).
	 */
	void processIntersections(
		SegmentString* e0,  int segIndex0,
		SegmentString* e1,  int segIndex1);

	std::vector<Coordinate>& getInteriorIntersections() {
		return interiorIntersections;
	}
	
};


/** \brief
 * Computes all intersections between segments in a set of SegmentString.
 *
 * Intersections found are represented as {@link SegmentNode}s and added to the
 * {@link SegmentString}s in which they occur.
 * As a final step in the noding a new set of segment strings split
 * at the nodes may be returned.
 *
 * Last port: noding/Noder.java rev. 1.8 (JTS-1.7)
 *
 * TODO: this was really an interface, we should avoid making it a Base class
 *
 */
class Noder {
public:
	/** \brief
	 * Computes the noding for a collection of {@link SegmentString}s.
	 *
	 * Some Noders may add all these nodes to the input SegmentStrings;
	 * others may only add some or none at all.
	 *
	 * @param segStrings a collection of {@link SegmentString}s to node
	 */
	virtual void computeNodes(SegmentString::NonConstVect* segStrings)=0;

	/** \brief
	 * Returns a {@link Collection} of fully noded {@link SegmentStrings}.
	 * The SegmentStrings have the same context as their parent.
	 *
	 * @return a newly allocated std::vector of const SegmentStrings.
	 *         Caller is responsible to delete it
	 */
	virtual SegmentString::NonConstVect* getNodedSubstrings() const=0;

	virtual ~Noder() {}

protected:
	Noder(){};
};


/**
 * Base class for {@link Noder}s which make a single
 * pass to find intersections.
 * This allows using a custom {@link SegmentIntersector}
 * (which for instance may simply identify intersections, rather than
 * insert them).
 *
 * Last port: noding/SinglePassNoder.java rev. 1.3 (JTS-1.7)
 *
 * TODO: Noder inheritance (that's just an interface!)
 *
 */
class SinglePassNoder : public Noder { // implements Noder

protected:

	/// Externally owned
	SegmentIntersector* segInt;

public:

	SinglePassNoder(SegmentIntersector* nSegInt=NULL): segInt(nSegInt) {}

	virtual ~SinglePassNoder() {}

	/**
	 * Sets the SegmentIntersector to use with this noder.
	 * A SegmentIntersector will normally add intersection nodes
	 * to the input segment strings, but it may not - it may
	 * simply record the presence of intersections.
	 * However, some Noders may require that intersections be added.
	 *
	 * @param newSegInt
	 */
	virtual void setSegmentIntersector(SegmentIntersector* newSegInt) {
	  segInt = newSegInt;
	}

	/**
	 * Computes the noding for a collection of {@link SegmentString}s.
	 *
	 * @param segStrings a collection of {@link SegmentString}s to node
	 */
	virtual void computeNodes(SegmentString::NonConstVect* segStrings)=0;

	/**
	 * Returns a {@link Collection} of fully noded {@link SegmentStrings}.
	 * The SegmentStrings have the same context as their parent.
	 *
	 * @return a Collection of SegmentStrings
	 */
	virtual SegmentString::NonConstVect* getNodedSubstrings() const=0;
	
};

/**
 * Wraps a {@link Noder} and transforms its input
 * into the integer domain.
 * This is intended for use with Snap-Rounding noders,
 * which typically are only intended to work in the integer domain.
 * Offsets can be provided to increase the number of digits of
 * available precision.
 *
 * Last port: noding/ScaledNoder.java rev. 1.2 (JTS-1.7)
 */
class ScaledNoder : public Noder, public CoordinateFilter { // implements Noder

private:

	Noder& noder;
	double scaleFactor;
	double offsetX;
	double offsetY;
	bool isScaled;

	void rescale(SegmentString::NonConstVect& segStrings) const;

public:

	bool isIntegerPrecision() { return (scaleFactor == 1.0); }

	ScaledNoder(Noder& n, double nScaleFactor,
			double nOffsetX=0.0, double nOffsetY=0.0)
		:
		noder(n),
		scaleFactor(nScaleFactor),
		offsetX(nOffsetX),
		offsetY(nOffsetY),
		isScaled(nScaleFactor!=1.0)
	{}

	SegmentString::NonConstVect* getNodedSubstrings() const;

	void computeNodes(SegmentString::NonConstVect* inputSegStr);

	//void filter(Coordinate& c);

	void filter_ro(const Coordinate* c) { assert(0); }
	void filter_rw(Coordinate* c) const;

};

/** \brief
 * Nodes a set of SegmentString using a index based
 * on index::chain::MonotoneChain and a index::SpatialIndex.
 *
 * The {@link SpatialIndex} used should be something that supports
 * envelope (range) queries efficiently (such as a index::quadtree::Quadtree
 * or index::strtree::STRtree.
 *
 * Last port: noding/MCIndexNoder.java rev. 1.4 (JTS-1.7)
 */
class MCIndexNoder : public SinglePassNoder {

private:
	std::vector<index::chain::MonotoneChain*> monoChains;
	index::strtree::STRtree index;
	int idCounter;
	SegmentString::NonConstVect* nodedSegStrings;
	// statistics
	int nOverlaps;

	void intersectChains();

	void add(SegmentString* segStr);

public:

	MCIndexNoder(SegmentIntersector *nSegInt=NULL)
		:
		SinglePassNoder(nSegInt),
		idCounter(0),
		nodedSegStrings(NULL),
		nOverlaps(0)
	{}

	~MCIndexNoder();

	/// Return a reference to this instance's std::vector of MonotoneChains
	std::vector<index::chain::MonotoneChain*>& getMonotoneChains() { return monoChains; }

	index::SpatialIndex& getIndex() { return index; }

	SegmentString::NonConstVect* getNodedSubstrings() const {
		assert(nodedSegStrings); // must have colled computeNodes before!
		return SegmentString::getNodedSubstrings(*nodedSegStrings);
	}

	void computeNodes(SegmentString::NonConstVect* inputSegmentStrings);

	class SegmentOverlapAction : public index::chain::MonotoneChainOverlapAction {
	private:
		SegmentIntersector& si;
	public:
		SegmentOverlapAction(SegmentIntersector& newSi)
			:
			si(newSi)
		{}

		void overlap(index::chain::MonotoneChain* mc1, int start1,
				index::chain::MonotoneChain* mc2, int start2);

		void overlap(LineSegment* s1, LineSegment* s2) { assert(0); }
	};
	
};


/** \brief
 * Nodes a set of {@link SegmentString}s by
 * performing a brute-force comparison of every segment to every other one.
 *
 * This has n^2 performance, so is too slow for use on large numbers
 * of segments.
 *
 * @version 1.7
 */
class SimpleNoder: public SinglePassNoder {
private:
	SegmentString::NonConstVect* nodedSegStrings;
	virtual void computeIntersects(SegmentString *e0, SegmentString *e1);

public:
	SimpleNoder(SegmentIntersector* nSegInt=NULL)
		:
		SinglePassNoder(nSegInt)
	{};

	void computeNodes(SegmentString::NonConstVect* inputSegmentStrings);

	SegmentString::NonConstVect* getNodedSubstrings() const {
		return SegmentString::getNodedSubstrings(*nodedSegStrings);
	}
};

/**
 * Validates that a collection of {@link SegmentString}s is correctly noded.
 * Throws an appropriate exception if an noding error is found.
 *
 * Last port: noding/NodingValidator.java rev. 1.6 (JTS-1.7)
 *
 */
class NodingValidator {
private:
	algorithm::LineIntersector li;
	const SegmentString::NonConstVect& segStrings;

	/**
	 * Checks if a segment string contains a segment
	 * pattern a-b-a (which implies a self-intersection)
	 */
	void checkCollapses() const;

	void checkCollapses(const SegmentString& ss) const;

	void checkCollapse(const Coordinate& p0, const Coordinate& p1,
			const Coordinate& p2) const;

	/**
	 * Checks all pairs of segments for intersections at an
	 * interior point of a segment
	 */
	void checkInteriorIntersections();

	void checkInteriorIntersections(const SegmentString& ss0,
			const SegmentString& ss1);

	void checkInteriorIntersections(
			const SegmentString& e0, unsigned int segIndex0,
			const SegmentString& e1, unsigned int segIndex1);

	/**
	 * Checks for intersections between an endpoint of a segment string
	 * and an interior vertex of another segment string
	 */
	void checkEndPtVertexIntersections() const;

	void checkEndPtVertexIntersections(const Coordinate& testPt,
			const SegmentString::NonConstVect& segStrings) const;

	/**
	 * @return true if there is an intersection point which is not an
	 *         endpoint of the segment p0-p1
	 */
	bool hasInteriorIntersection(const algorithm::LineIntersector& aLi,
			const Coordinate& p0, const Coordinate& p1) const;

public:

	NodingValidator(const SegmentString::NonConstVect& newSegStrings):
		segStrings(newSegStrings)
	{}

	~NodingValidator() {}

	void checkValid();

};


/**
 * Nodes a set of SegmentStrings completely.
 * The set of segmentStrings is fully noded;
 * i.e. noding is repeated until no further
 * intersections are detected.
 * <p>
 * Iterated noding using a FLOATING precision model is not guaranteed to converge,
 * due to roundoff error.   This problem is detected and an exception is thrown.
 * Clients can choose to rerun the noding using a lower precision model.
 *
 * Last port: noding/IteratedNoder.java rev. 1.6 (JTS-1.7)
 */
class IteratedNoder : public Noder { // implements Noder

private:
	static const int MAX_ITER = 5;


	const PrecisionModel *pm;
	algorithm::LineIntersector li;
	SegmentString::NonConstVect* nodedSegStrings;
	int maxIter;

	/**
	 * Node the input segment strings once
	 * and create the split edges between the nodes
	 */
	void node(SegmentString::NonConstVect* segStrings,
			int *numInteriorIntersections);

public:

	IteratedNoder(const PrecisionModel *newPm)
		:
		pm(newPm),
		li(pm),
		maxIter(MAX_ITER)
	{
	}

	virtual ~IteratedNoder() {}

	/**
	 * Sets the maximum number of noding iterations performed before
	 * the noding is aborted.
	 * Experience suggests that this should rarely need to be changed
	 * from the default.
	 * The default is MAX_ITER.
	 *
	 * @param n the maximum number of iterations to perform
	 */
	void setMaximumIterations(int n) { maxIter = n; }

	SegmentString::NonConstVect* getNodedSubstrings() const {
		return SegmentString::getNodedSubstrings(*nodedSegStrings);
	}
 

	/**
	 * Fully nodes a list of {@link SegmentStrings}, i.e. peforms noding iteratively
	 * until no intersections are found between segments.
	 * Maintains labelling of edges correctly through
	 * the noding.
	 *
	 * @param segStrings a collection of SegmentStrings to be noded
	 * @throws TopologyException if the iterated noding fails to converge.
	 */
	void computeNodes(SegmentString::NonConstVect* inputSegmentStrings); // throw(GEOSException);
};

} // namespace geos.noding
} // namespace geos

#endif

/**********************************************************************
 * $Log$
 * Revision 1.23  2006/02/28 14:34:05  strk
 * Added many assertions and debugging output hunting for a bug in BufferOp
 *
 * Revision 1.22  2006/02/28 13:08:23  strk
 * Added missing definition of IteratedNoder::MAX_ITER, moving it to private access level
 *
 * Revision 1.21  2006/02/23 20:05:19  strk
 * Fixed bug in MCIndexNoder constructor making memory checker go crazy, more
 * doxygen-friendly comments, miscellaneous cleanups
 *
 * Revision 1.20  2006/02/23 11:54:20  strk
 * - MCIndexPointSnapper
 * - MCIndexSnapRounder
 * - SnapRounding BufferOp
 * - ScaledNoder
 * - GEOSException hierarchy cleanups
 * - SpatialIndex memory-friendly query interface
 * - GeometryGraph::getBoundaryNodes memory-friendly
 * - NodeMap::getBoundaryNodes memory-friendly
 * - Cleanups in geomgraph::Edge
 * - Added an XML test for snaprounding buffer (shows leaks, working on it)
 *
 * Revision 1.19  2006/02/21 16:53:49  strk
 * MCIndexPointSnapper, MCIndexSnapRounder
 *
 * Revision 1.18  2006/02/20 21:04:37  strk
 * - namespace geos::index
 * - SpatialIndex interface synced
 *
 * Revision 1.17  2006/02/20 10:14:18  strk
 * - namespaces geos::index::*
 * - Doxygen documentation cleanup
 *
 * Revision 1.16  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.15  2006/02/18 21:08:09  strk
 * - new CoordinateSequence::applyCoordinateFilter method (slow but useful)
 * - SegmentString::getCoordinates() doesn't return a clone anymore.
 * - SegmentString::getCoordinatesRO() obsoleted.
 * - SegmentString constructor does not promises constness of passed
 *   CoordinateSequence anymore.
 * - NEW ScaledNoder class
 * - Stubs for MCIndexPointSnapper and  MCIndexSnapRounder
 * - Simplified internal interaces of OffsetCurveBuilder and OffsetCurveSetBuilder
 *
 * Revision 1.14  2006/02/15 17:19:18  strk
 * NodingValidator synced with JTS-1.7, added CoordinateSequence::operator[]
 * and size() to easy port maintainance.
 *
 * Revision 1.13  2006/02/15 14:59:03  strk
 * JTS-1.7 sync for:
 * noding/SegmentNode.cpp
 * noding/SegmentNodeList.cpp
 * noding/SegmentString.cpp
 *
 * Revision 1.12  2006/02/14 13:28:25  strk
 * New SnapRounding code ported from JTS-1.7 (not complete yet).
 * Buffer op optimized by using new snaprounding code.
 * Leaks fixed in XMLTester.
 *
 * Revision 1.11  2005/12/08 01:39:28  strk
 * SegmentString::eiList made a real object rather then a pointer.
 * Adde getter for const and non-const references of it (dropping get by pointer)
 *
 * Revision 1.10  2005/11/10 15:20:32  strk
 * Made virtual overloads explicit.
 *
 * Revision 1.9  2005/11/07 18:05:28  strk
 * Reduced set<> lookups
 *
 * Revision 1.8  2005/06/24 11:09:43  strk
 * Dropped RobustLineIntersector, made LineIntersector a concrete class.
 * Added LineIntersector::hasIntersection(Coordinate&,Coordinate&,Coordinate&)
 * to avoid computing intersection point (Z) when it's not necessary.
 *
 * Revision 1.7  2005/02/22 18:21:46  strk
 * Changed SegmentNode to contain a *real* Coordinate (not a pointer) to reduce
 * construction costs.
 *
 * Revision 1.6  2005/02/22 16:23:28  strk
 * Cached number of points in CoordinateSequence.
 *
 * Revision 1.5  2004/11/04 19:08:07  strk
 * Cleanups, initializers list, profiling.
 *
 * Revision 1.4  2004/11/01 16:43:04  strk
 * Added Profiler code.
 * Temporarly patched a bug in DoubleBits (must check drawbacks).
 * Various cleanups and speedups.
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
 * Revision 1.12  2004/07/01 14:12:44  strk
 *
 * Geometry constructors come now in two flavors:
 * 	- deep-copy args (pass-by-reference)
 * 	- take-ownership of args (pass-by-pointer)
 * Same functionality is available through GeometryFactory,
 * including buildGeometry().
 *
 * Revision 1.11  2004/06/16 13:13:25  strk
 * Changed interface of SegmentString, now copying CoordinateSequence argument.
 * Fixed memory leaks associated with this and MultiGeometry constructors.
 * Other associated fixes.
 *
 * Revision 1.10  2004/05/07 07:57:27  strk
 * Added missing EdgeNodingValidator to build scripts.
 * Changed SegmentString constructor back to its original form
 * (takes const void *), implemented local tracking of "contexts"
 * in caller objects for proper destruction.
 *
 * Revision 1.9  2004/05/06 15:54:15  strk
 * SegmentNodeList keeps track of created splitEdges for later destruction.
 * SegmentString constructor copies given Label.
 * Buffer operation does no more leaks for doc/example.cpp
 *
 * Revision 1.8  2004/05/03 22:56:44  strk
 * leaks fixed, exception specification omitted.
 *
 * Revision 1.7  2004/04/30 09:15:28  strk
 * Enlarged exception specifications to allow for AssertionFailedException.
 * Added missing initializers.
 *
 * Revision 1.6  2004/04/23 00:02:18  strk
 * const-correctness changes
 *
 * Revision 1.5  2004/04/19 16:14:52  strk
 * Some memory leaks plugged in noding algorithms.
 *
 * Revision 1.4  2004/04/19 12:51:01  strk
 * Memory leaks fixes. Throw specifications added.
 *
 * Revision 1.3  2004/04/14 09:30:48  strk
 * Private iterated noding funx now use int* instead of vector to know
 * when it's time to stop.
 *
 * Revision 1.2  2004/03/26 07:48:30  ybychkov
 * "noding" package ported (JTS 1.4)
 *
 *
 **********************************************************************/

