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
 * Revision 1.3  2004/10/26 17:46:18  strk
 * Removed slash-stars in comments to remove annoying compiler warnings.
 *
 * Revision 1.2  2004/07/19 13:19:31  strk
 * Documentation fixes
 *
 * Revision 1.1  2004/07/02 13:20:42  strk
 * Header files moved under geos/ dir.
 *
 * Revision 1.6  2004/05/06 16:30:58  strk
 * Kept track of newly allocated objects by ensureExtent for Bintree and Quadtree,
 * deleted at destruction time. doc/example.cpp runs with no leaks.
 *
 * Revision 1.5  2004/03/25 02:23:55  ybychkov
 * All "index/" packages upgraded to JTS 1.4
 *
 * Revision 1.4  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/


#ifndef GEOS_INDEXBINTREE_H
#define GEOS_INDEXBINTREE_H

#include <memory>
#include <vector>
#include <geos/platform.h>
#include <geos/geom.h>

using namespace std;

namespace geos {

/*
 * \class BinTreeInterval indexBintree.h geos/indexBintree.h
 * \brief
 * Represents an (1-dimensional) closed interval on the Real number line.
 */
class BinTreeInterval {
public:
	double min, max;
	BinTreeInterval();
	virtual ~BinTreeInterval();
	BinTreeInterval(double nmin, double nmax);
	BinTreeInterval(BinTreeInterval *interval);
	void init(double nmin, double nmax);
	double getMin();
	double getMax();
	double getWidth();
	void expandToInclude(BinTreeInterval *interval);
	bool overlaps(BinTreeInterval *interval);
	bool overlaps(double nmin, double nmax);
	bool contains(BinTreeInterval *interval);
	bool contains(double nmin, double nmax);
	bool contains(double p);
};

/*
 * \class Key indexBintree.h geos/indexBintree.h
 * \brief
 * A Key is a unique identifier for a node in a tree.
 *
 * It contains a lower-left point and a level number.
 * The level number is the power of two for the size of the node envelope
 */
class Key {
public:
	static int computeLevel(BinTreeInterval *newInterval);
	Key(BinTreeInterval *newInterval);
	virtual ~Key();
	double getPoint();
	int getLevel();
	BinTreeInterval* getInterval();
	void computeKey(BinTreeInterval *itemInterval);
private:
	// the fields which make up the key
	double pt;
	int level;
	// auxiliary data which is derived from the key for use in computation
	BinTreeInterval *interval;
	void computeInterval(int level,BinTreeInterval *itemInterval);
};

class BinTreeNode;

/*
 * \class NodeBase indexBintree.h geos/indexBintree.h
 * \brief The base class for nodes in a Bintree.
 */
class NodeBase {
public:
	static int getSubnodeIndex(BinTreeInterval *interval, double centre);
	NodeBase();
	virtual ~NodeBase();
	virtual vector<void*> *getItems();
	virtual void add(void* item);
	virtual vector<void*>* addAllItems(vector<void*> *newItems);
	virtual vector<void*>* addAllItemsFromOverlapping(BinTreeInterval *interval,vector<void*> *resultItems);
	virtual int depth();
	virtual int size();
	virtual int nodeSize();
protected:	
	vector<void*>* items;
	/**
	* subnodes are numbered as follows:
	*
	*  0 | 1
	*/
	BinTreeNode* subnode[2];
	virtual bool isSearchMatch(BinTreeInterval *interval)=0;
};

/*
 * \class BinTreeNode indexBintree.h geos/indexBintree.h
 * \brief A node of a Bintree.
 */
class BinTreeNode: public NodeBase {
public:
	static BinTreeNode* createNode(BinTreeInterval *itemInterval);
	static BinTreeNode* createExpanded(BinTreeNode *node,BinTreeInterval *addInterval);
	BinTreeNode(BinTreeInterval *newInterval,int newLevel);
	virtual ~BinTreeNode();
	BinTreeInterval* getInterval();
	BinTreeNode* getNode(BinTreeInterval *searchInterval);
	NodeBase* find(BinTreeInterval *searchInterval);
	void insert(BinTreeNode *node);
private:
	BinTreeInterval *interval;
	double centre;
	int level;
	BinTreeNode* getSubnode(int index);
	BinTreeNode* createSubnode(int index);
protected:
	bool isSearchMatch(BinTreeInterval *itemInterval);
};

/*
 * \class Root indexBintree.h geos/indexBintree.h
 * \brief The root node of a single Bintree.
 *
 * It is centred at the origin,
 * and does not have a defined extent.
 */
class Root: public NodeBase {
private:
	// the singleton root node is centred at the origin.
	static double origin;
	void insertContained(BinTreeNode *tree,BinTreeInterval *itemInterval,void* item);
public:
	Root();
	virtual ~Root();
	void insert(BinTreeInterval *itemInterval,void* item);
protected:
	bool isSearchMatch(BinTreeInterval *interval);
};

/*
 * \class Bintree indexBintree.h geos/indexBintree.h
 *
 * \brief An BinTree (or "Binary BinTreeInterval Tree")
 * is a 1-dimensional version of a quadtree.
 *
 * It indexes 1-dimensional intervals (which of course may
 * be the projection of 2-D objects on an axis).
 * It supports range searching
 * (where the range may be a single point).
 *
 * This implementation does not require specifying the extent of the inserted
 * items beforehand.  It will automatically expand to accomodate any extent
 * of dataset.
 * 
 * This index is different to the BinTreeInterval Tree of Edelsbrunner
 * or the Segment Tree of Bentley.
 */
class Bintree {
public:
	static BinTreeInterval* ensureExtent(BinTreeInterval *itemInterval, double minExtent);
	Bintree();
	virtual ~Bintree();
	int depth();
	int size();
	int nodeSize();
	void insert(BinTreeInterval *itemInterval,void* item);
	vector<void*>* iterator();
	vector<void*>* query(double x);
	vector<void*>* query(BinTreeInterval *interval);
	void query(BinTreeInterval *interval,vector<void*> *foundItems);
private:
	vector<BinTreeInterval *>newIntervals;
	Root *root;
	/**
	*  Statistics
	*
	* minExtent is the minimum extent of all items
	* inserted into the tree so far. It is used as a heuristic value
	* to construct non-zero extents for features with zero extent.
	* Start with a non-zero extent, in case the first feature inserted has
	* a zero extent in both directions.  This value may be non-optimal, but
	* only one feature will be inserted with this value.
	**/
	double minExtent;
	void collectStats(BinTreeInterval *interval);
};
}
#endif

