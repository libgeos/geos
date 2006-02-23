/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#ifndef GEOS_INDEXQUADTREE_H
#define GEOS_INDEXQUADTREE_H

#include <memory>
#include <vector>
#include <geos/platform.h>
#include <geos/geom.h>
#include <geos/spatialIndex.h>

#if __STDC_IEC_559__
#define ASSUME_IEEE_DOUBLE 1
#else
#define ASSUME_IEEE_DOUBLE 0
#endif


namespace geos {
namespace index { // geos.index

/// Contains classes that implement a Quadtree spatial index
namespace quadtree { // geos.index.quadtree

/**
 * \class IntervalSize indexQuadtree.h geos/indexQuadtree.h
 *
 * \brief
 * Provides a test for whether an interval is
 * so small it should be considered as zero for the purposes of
 * inserting it into a binary tree.
 *
 * The reason this check is necessary is that round-off error can
 * cause the algorithm used to subdivide an interval to fail, by
 * computing a midpoint value which does not lie strictly between the
 * endpoints.
 */
class IntervalSize {
public:
	/**
	 * This value is chosen to be a few powers of 2 less than the
	 * number of bits available in the double representation (i.e. 53).
	 * This should allow enough extra precision for simple computations
	 * to be correct,
	 * at least for comparison purposes.
	 */
	static const int MIN_BINARY_EXPONENT=-50;
	static bool isZeroWidth(double min, double max);
};

class DoubleBits {
public:
	static const int EXPONENT_BIAS=1023;
	static double powerOf2(int exp);
	static int exponent(double d);
	static double truncateToPowerOfTwo(double d);
	static string toBinaryString(double d);
	static double maximumCommonMantissa(double d1, double d2);
	DoubleBits(double nx);
	double getDouble();
	int64 biasedExponent();
	int getExponent();
	void zeroLowerBits(int nBits);
	int getBit(int i);
	int numCommonMantissaBits(DoubleBits *db);
	string toString();
private:
	double x;
//	long long xBits;
//	long xBits;
	int64 xBits;
};

/**
 * \class QuadTreeKey indexQuadtree.h geos/indexQuadtree.h
 *
 * \brief
 * A QuadTreeKey is a unique identifier for a node in a quadtree.
 *
 * It contains a lower-left point and a level number. The level number
 * is the power of two for the size of the node envelope
 */
class QuadTreeKey {
public:
	static int computeQuadLevel(Envelope *env);
	QuadTreeKey(Envelope *itemEnv);
	virtual ~QuadTreeKey();
	Coordinate* getPoint();
	int getLevel();
	Envelope* getEnvelope();
	Coordinate* getCentre();
	void computeKey(Envelope *itemEnv);
private:	
	// the fields which make up the key
	Coordinate *pt;
	int level;
	// auxiliary data which is derived from the key for use in computation
	Envelope *env;
	void computeKey(int level,Envelope *itemEnv);
};

class QuadTreeNode;

/**
 * \class QuadTreeNodeBase indexQuadtree.h geos/indexQuadtree.h
 *
 * \brief
 * The base class for nodes in a Quadtree.
 *
 */
class QuadTreeNodeBase {

private:
	void visitItems(const Envelope* searchEnv, ItemVisitor& visitor);
	
public:
	static int getSubnodeIndex(const Envelope *env, const Coordinate& centre);
	QuadTreeNodeBase();
	virtual ~QuadTreeNodeBase();
	virtual std::vector<void*>* getItems();
	virtual void add(void* item);
	virtual std::vector<void*>* addAllItems(std::vector<void*> *resultItems);
	virtual void addAllItemsFromOverlapping(const Envelope *searchEnv,std::vector<void*> *resultItems);
	virtual int depth();
	virtual int size();
	virtual int nodeCount();
	virtual string toString() const;

	virtual void visit(const Envelope* searchEnv, ItemVisitor& visitor);

	/**
	 * Removes a single item from this subtree.
	 *
	 * @param searchEnv the envelope containing the item
	 * @param item the item to remove
	 * @return <code>true</code> if the item was found and removed
	 */
	bool remove(const Envelope* itemEnv, void* item);
 
	bool hasItems() const { return ! items->empty(); }

	bool hasChildren() const {
		for (int i = 0; i < 4; i++) 
			if (subnode[i]) return true;
		return false;
	}

	bool isPrunable() const {
		return ! (hasChildren() || hasItems());
	}

protected:
	std::vector<void*> *items;

	/**
	 * subquads are numbered as follows:
	 * <pre>
	 *  2 | 3
	 *  --+--
	 *  0 | 1
	 * </pre>
	 */
	QuadTreeNode* subnode[4];
	virtual bool isSearchMatch(const Envelope *searchEnv)=0;
};

/**
 * \class QuadTreeNode indexQuadtree.h geos/indexQuadtree.h
 *
 * \brief
 * Represents a node of a Quadtree.
 *
 * Nodes contain items which have a spatial extent corresponding to
 * the node's position in the quadtree.
 *
 */
class QuadTreeNode: public QuadTreeNodeBase {

private:

	Envelope *env;

	Coordinate centre;

	int level;

	QuadTreeNode* getSubnode(int index);

	QuadTreeNode* createSubnode(int index);

protected:

	bool isSearchMatch(const Envelope *searchEnv) {
		return env->intersects(searchEnv);
	}

public:

	static QuadTreeNode* createNode(Envelope *env);

	static QuadTreeNode* createExpanded(QuadTreeNode *node, const Envelope *addEnv);

	// Takes ownership of envelope
	QuadTreeNode(Envelope *nenv, int nlevel)
		:
		env(nenv),
		centre((nenv->getMinX()+nenv->getMaxX())/2,
			(nenv->getMinY()+nenv->getMaxY())/2),
		level(nlevel)
	{
	}

	virtual ~QuadTreeNode() { delete env; }

	Envelope* getEnvelope() { return env; }

	/**
	 * Returns the subquad containing the envelope.
	 * Creates the subquad if
	 * it does not already exist.
	 */
	QuadTreeNode* getNode(const Envelope *searchEnv);

	/**
	 * Returns the smallest <i>existing</i>
	 * node containing the envelope.
	 */
	QuadTreeNodeBase* find(const Envelope *searchEnv);

	void insertNode(QuadTreeNode *node);

	string toString() const;

};

/**
 * \class QuadTreeRoot indexQuadtree.h geos/indexQuadtree.h
 *
 * \brief
 * QuadRoot is the root of a single Quadtree.  It is centred at the origin,
 * and does not have a defined extent.
 */
class QuadTreeRoot: public QuadTreeNodeBase {
friend class Unload;

private:

	//static Coordinate *origin;
	static Coordinate origin;

	/**
	 * insert an item which is known to be contained in the tree rooted at
	 * the given QuadNode root.  Lower levels of the tree will be created
	 * if necessary to hold the item.
	 */
	void insertContained(QuadTreeNode *tree, const Envelope *itemEnv, void* item);

public:

	QuadTreeRoot() {}

	virtual ~QuadTreeRoot() {}

	/**
	 * Insert an item into the quadtree this is the root of.
	 */
	void insert(const Envelope *itemEnv, void* item);

protected:

	bool isSearchMatch(const Envelope *searchEnv) { return true; }

};

/**
 * \class Quadtree indexQuadtree.h geos/indexQuadtree.h
 *
 * \brief
 * A Quadtree is a spatial index structure for efficient querying
 * of 2D rectangles.  If other kinds of spatial objects
 * need to be indexed they can be represented by their
 * envelopes
 * 
 * The quadtree structure is used to provide a primary filter
 * for range rectangle queries.  The query() method returns a list of
 * all objects which <i>may</i> intersect the query rectangle.  Note that
 * it may return objects which do not in fact intersect.
 * A secondary filter is required to test for exact intersection.
 * Of course, this secondary filter may consist of other tests besides
 * intersection, such as testing other kinds of spatial relationships.
 *
 * This implementation does not require specifying the extent of the inserted
 * items beforehand.  It will automatically expand to accomodate any extent
 * of dataset.
 * 
 * This data structure is also known as an <i>MX-CIF quadtree</i>
 * following the usage of Samet and others.
 */
class Quadtree: public SpatialIndex {
private:
	std::vector<Envelope *>newEnvelopes;
	void collectStats(const Envelope *itemEnv);
	QuadTreeRoot *root;

	/**
	 *  Statistics
	 *
	 * minExtent is the minimum envelope extent of all items
	 * inserted into the tree so far. It is used as a heuristic value
	 * to construct non-zero envelopes for features with zero X and/or
	 * Y extent.
	 * Start with a non-zero extent, in case the first feature inserted has
	 * a zero extent in both directions.  This value may be non-optimal, but
	 * only one feature will be inserted with this value.
	 */
	double minExtent;

public:
	/**
	 * \brief
	 * Ensure that the envelope for the inserted item has non-zero extents.
	 * Use the current minExtent to pad the envelope, if necessary.
	 * Can return a new Envelope or the given one (casted to non-const).
	 */
	static Envelope* ensureExtent(const Envelope *itemEnv, double minExtent);

	/**
	 * \brief
	 * Constructs a Quadtree with zero items.
	 */
	Quadtree()
		:
		root(new QuadTreeRoot()),
		minExtent(1.0)
	{}

	virtual ~Quadtree();

	/// Returns the number of levels in the tree.
	int depth();

	/// Returns the number of items in the tree.
	int size();
	
	void insert(const Envelope *itemEnv, void *item);

	void query(const Envelope *searchEnv, std::vector<void*>& ret);
#if 0
	std::vector<void*>* query(const Envelope *searchEnv) {
		vector<void*> *foundItems=new vector<void*>();
		query(searchEnv, *foundItems);
		return foundItems;
	}
#endif

	void query(const Envelope *searchEnv, ItemVisitor& visitor) {
		root->visit(searchEnv, visitor);
	}

	bool remove(const Envelope* itemEnv, void* item);

	/// Return a list of all items in the Quadtree
	std::vector<void*>* queryAll();

	string toString() const;

};

} // namespace geos.index.quadtree
} // namespace geos.index
} // namespace geos

#endif

/**********************************************************************
 * $Log$
 * Revision 1.12  2006/02/23 11:54:20  strk
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
 * Revision 1.11  2006/02/20 21:04:37  strk
 * - namespace geos::index
 * - SpatialIndex interface synced
 *
 * Revision 1.10  2006/02/20 10:14:18  strk
 * - namespaces geos::index::*
 * - Doxygen documentation cleanup
 *
 * Revision 1.9  2004/11/17 08:13:16  strk
 * Indentation changes.
 * Some Z_COMPUTATION activated by default.
 *
 * Revision 1.8  2004/11/04 08:49:13  strk
 * Unlinked new documentation.
 *
 * Revision 1.7  2004/11/02 16:38:45  strk
 * Fixed ieee-754 detection switch
 *
 * Revision 1.6  2004/11/02 16:05:49  strk
 * Autodetect availability of IEEE-754 FP
 *
 * Revision 1.5  2004/11/02 15:49:59  strk
 *
 * Moved ASSUME_IEEE_DOUBLE define from DoubleBits.cpp to indexQuadtree.h.
 * Fixed a bug in powerOf2(). Made the !IEEE version less prone to
 * round-offs (still has approximation errors).
 *
 * Revision 1.4  2004/11/01 16:43:04  strk
 * Added Profiler code.
 * Temporarly patched a bug in DoubleBits (must check drawbacks).
 * Various cleanups and speedups.
 *
 * Revision 1.3  2004/07/27 16:35:46  strk
 * Geometry::getEnvelopeInternal() changed to return a const Envelope *.
 * This should reduce object copies as once computed the envelope of a
 * geometry remains the same.
 *
 * Revision 1.2  2004/07/19 13:19:31  strk
 * Documentation fixes
 *
 * Revision 1.1  2004/07/02 13:20:42  strk
 * Header files moved under geos/ dir.
 *
 * Revision 1.16  2004/05/06 16:30:58  strk
 * Kept track of newly allocated objects by ensureExtent for Bintree and Quadtree,
 * deleted at destruction time. doc/example.cpp runs with no leaks.
 *
 * Revision 1.15  2004/04/19 15:14:45  strk
 * Added missing virtual destructor in SpatialIndex class.
 * Memory leaks fixes. Const and throw specifications added.
 *
 * Revision 1.14  2004/03/25 02:23:55  ybychkov
 * All "index/" packages upgraded to JTS 1.4
 *
 * Revision 1.13  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/

