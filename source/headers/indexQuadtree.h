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
 * Revision 1.13  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/


#ifndef GEOS_INDEXQUADTREE_H
#define GEOS_INDEXQUADTREE_H

#include <memory>
#include <vector>
#include "platform.h"
#include "geom.h"
#include "spatialIndex.h"

using namespace std;

namespace geos {

/**
* Provides a test for whether an interval is
* so small it should be considered as zero for the purposes of
* inserting it into a binary tree.
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
	* This should allow enough extra precision for simple computations to be correct,
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
 * A QuadTreeKey is a unique identifier for a node in a quadtree.
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
class QuadTreeNodeBase {
public:
	static int getSubnodeIndex(Envelope *env,Coordinate *centre);
	QuadTreeNodeBase();
	virtual ~QuadTreeNodeBase();
	virtual vector<void*>* getItems();
	virtual void add(void* item);
	virtual vector<void*>* addAllItems(vector<void*> *resultItems);
	virtual void addAllItemsFromOverlapping(Envelope *searchEnv,vector<void*> *resultItems);
	virtual int depth();
	virtual int size();
	virtual int nodeCount();
protected:
	vector<void*> *items;
	/**
	* subquads are numbered as follows:
	*  2 | 3
	*  --+--
	*  0 | 1
	*/
	QuadTreeNode* subnode[4];
	virtual bool isSearchMatch(Envelope *searchEnv)=0;
};

class QuadTreeNode: public QuadTreeNodeBase {
public:
	static QuadTreeNode* createNode(Envelope *env);
	static QuadTreeNode* createExpanded(QuadTreeNode *node,Envelope *addEnv);
	QuadTreeNode(Envelope *nenv,int nlevel);
	virtual ~QuadTreeNode();
	Envelope* getEnvelope();
	QuadTreeNode* getNode(Envelope *searchEnv);
	QuadTreeNodeBase* find(Envelope *searchEnv);
	void insertNode(QuadTreeNode *node);
private:
	Envelope *env;
	Coordinate *centre;
	int level;
	QuadTreeNode* getSubnode(int index);
	QuadTreeNode* createSubnode(int index);
protected:
	bool isSearchMatch(Envelope *searchEnv);
};

/**
 * QuadRoot is the root of a single Quadtree.  It is centred at the origin,
 * and does not have a defined extent.
 */
class QuadTreeRoot: public QuadTreeNodeBase {
friend class Unload;
private:
	static Coordinate *origin;
	void insertContained(QuadTreeNode *tree,Envelope *itemEnv,void* item);
public:
	QuadTreeRoot();
	virtual ~QuadTreeRoot();
	void insert(Envelope *itemEnv,void* item);
protected:
	bool isSearchMatch(Envelope *searchEnv);
};

/**
 * A Quadtree is a spatial index structure for efficient querying
 * of 2D rectangles.  If other kinds of spatial objects
 * need to be indexed they can be represented by their
 * envelopes
 * <p>
 * The quadtree structure is used to provide a primary filter
 * for range rectangle queries.  The query() method returns a list of
 * all objects which <i>may</i> intersect the query rectangle.  Note that
 * it may return objects which do not in fact intersect.
 * A secondary filter is required to test for exact intersection.
 * Of course, this secondary filter may consist of other tests besides
 * intersection, such as testing other kinds of spatial relationships.
 *
 * <p>
 * This implementation does not require specifying the extent of the inserted
 * items beforehand.  It will automatically expand to accomodate any extent
 * of dataset.
 * <p>
 * This data structure is also known as an <i>MX-CIF quadtree</i>
 * following the usage of Samet and others.
 */
class Quadtree: public SpatialIndex {
public:
	static Envelope* ensureExtent(Envelope *itemEnv,double minExtent);
	Quadtree();
	virtual ~Quadtree();
	int depth();
	int size();
	void insert(Envelope *itemEnv,void* item);
	vector<void*>* query(Envelope *searchEnv);
	vector<void*>* queryAll();
private:
	void collectStats(Envelope *itemEnv);
	QuadTreeRoot *root;
	/**
	*  Statistics
	*
	* minExtent is the minimum envelope extent of all items
	* inserted into the tree so far. It is used as a heuristic value
	* to construct non-zero envelopes for features with zero X and/or Y extent.
	* Start with a non-zero extent, in case the first feature inserted has
	* a zero extent in both directions.  This value may be non-optimal, but
	* only one feature will be inserted with this value.
	**/
	double minExtent;
};
}
#endif

