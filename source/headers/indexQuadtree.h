#ifndef GEOS_INDEXQUADTREE_H
#define GEOS_INDEXQUADTREE_H

#include <vector>
#include "platform.h"
#include "geom.h"

using namespace std;

class QuadtreeSelectAction {
public:
	virtual void select(void *obj)=0;
};

class Quad {
public:
	Quad(Quad *newParent,Envelope *newEnv);
	~Quad();
	Quad* getParent();
	vector<void*>* getItems();
	bool overlaps(Envelope *testEnv);
	void add(void *item);
	vector<void*>* addAllItems(vector<void*> *newItems);
	vector<void*>* addAllItemsFromOverlapping(Envelope *searchEnv,vector<void*>* newItems);
	void selectAllItemsFromOverlapping(Envelope *searchEnv,QuadtreeSelectAction *action);
	/**
	* Returns the quad containing the envelope.  Creates the quad if
	* it does not already exist.
	*/
	Quad* getQuad(Envelope *searchEnv);
	/**
	* Returns the smallest existing quad containing the envelope.
	*/
	Quad* find(Envelope *searchEnv);
private:
	Envelope *env;
	Coordinate centre;
	Quad *parent;
	vector<void*> *items;
	/**
	* subquads are numbered as follows:
	*
	*  2 | 3
	*  --+--
	*  0 | 1
	*/
	Quad* subquad[4];
	/**
	* Returns the index of the subquad that wholely contains the search envelope.
	* If none does, returns -1
	*/
	int getSubquadIndex(Envelope *searchEnv);
	/**
	* get the subquad for the index.
	* If it doesn't exist, create it
	*/
	Quad* getSubquad(int ind);
};

class Quadtree {
public:
	Quadtree(Envelope *env);
	~Quadtree();
	Quad* getRoot();
	void insert(Envelope *itemEnv,void* item);
	vector<void*>* query(Envelope *searchEnv);
	/**
	* Internal iterator for queries
	*/
	void query(Envelope *searchEnv,QuadtreeSelectAction *action);
	vector<void*>* queryAll();
private:
	/**
   * return a square envelope containing the argument envelope
   */
	static Envelope* computeInitialExtent(Envelope *env);
	Quad *root;
	//Doesn't seem to be used
	//	class QuadtreeIterator {
	//	}
};

#endif

