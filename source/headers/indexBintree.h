#ifndef GEOS_INDEXBINTREE_H
#define GEOS_INDEXBINTREE_H

#include <vector>
#include "platform.h"
#include "geom.h"

using namespace std;

class BinTreeInterval {
public:
	double min, max;
	BinTreeInterval();
	BinTreeInterval(double nmin, double nmax);
	BinTreeInterval(const BinTreeInterval& interval);
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

/**
 * A Key is a unique identifier for a node in a tree.
 * It contains a lower-left point and a level number. The level number
 * is the power of two for the size of the node envelope
 */
class Key {
public:
	static int computeLevel(BinTreeInterval *interval);
	Key(BinTreeInterval *interval);
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
class NodeBase {
public:
	static int getSubnodeIndex(BinTreeInterval *interval, double centre);
	NodeBase();
	~NodeBase();
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

class BinTreeNode: public NodeBase {
public:
	static BinTreeNode* createNode(BinTreeInterval *itemInterval);
	static BinTreeNode* createExpanded(BinTreeNode *node,BinTreeInterval *addInterval);
	BinTreeNode(BinTreeInterval *newInterval,int newLevel);
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

/**
 * Root is the root of a single Bintree.  It is centred at the origin,
 * and does not have a defined extent.
 */
class Root: public NodeBase {
private:
	// the singleton root node is centred at the origin.
	static double origin;
	void insertContained(BinTreeNode *tree,BinTreeInterval *itemInterval,void* item);
public:
	Root();
	void insert(BinTreeInterval *itemInterval,void* item);
protected:
	bool isSearchMatch(BinTreeInterval *interval);
};

/**
 * An <code>BinTree</code> (or "Binary BinTreeInterval Tree")
 * is a 1-dimensional version of a quadtree.
 * It indexes 1-dimensional intervals (which of course may
 * be the projection of 2-D objects on an axis).
 * It supports range searching
 * (where the range may be a single point).
 * <p>
 * This implementation does not require specifying the extent of the inserted
 * items beforehand.  It will automatically expand to accomodate any extent
 * of dataset.
 * <p>
 * This index is different to the BinTreeInterval Tree of Edelsbrunner
 * or the Segment Tree of Bentley.
 */
class Bintree {
public:
	static BinTreeInterval* ensureExtent(BinTreeInterval *itemInterval, double minExtent);
	Bintree();
	int depth();
	int size();
	int nodeSize();
	void insert(BinTreeInterval *itemInterval,void* item);
	vector<void*>* iterator();
	vector<void*>* query(double x);
	vector<void*>* query(BinTreeInterval *interval);
	void query(BinTreeInterval *interval,vector<void*> *foundItems);
private:
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
#endif

