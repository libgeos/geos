#include "indexQuadtree.h"
#include "stdio.h"

/**
* return a square envelope containing the argument envelope
*/
Envelope* Quadtree::computeInitialExtent(Envelope *env) {
	double dx=env->getWidth();
	double dy=env->getHeight();
	double size=dx>dy?dx:dy;
	double x=env->getMinX();
	double y=env->getMinY();
	return new Envelope(x,x+size,y,y+size);
}

Quadtree::Quadtree(Envelope *env) {
	Envelope *quadEnv=computeInitialExtent(env);
	root=new Quad(NULL,quadEnv);
}

Quadtree::~Quadtree() {
	delete root;
}

Quad* Quadtree::getRoot() {
	return root;
}

void Quadtree::insert(Envelope *itemEnv,void *item) {
	/**
	* Do NOT create a new quad for zero-area envelopes - this would lead
	* to infinite recursion. Instead, use a heuristic of simply returning
	* the smallest existing quad containing the query
	*/
	bool isValid=itemEnv->getHeight()>0 && itemEnv->getWidth()>0;
	Quad* quad;
	if (isValid)
		quad=root->getQuad(itemEnv);
	else
		quad=root->find(itemEnv);
	quad->add(item);
}

vector<void*>* Quadtree::query(Envelope *searchEnv){
	/**
	* the items that are matched are the items in quads which
	* overlap the search envelope
	*/
	vector<void*>* foundItems=new vector<void*>();
	root->addAllItemsFromOverlapping(searchEnv,foundItems);
	return foundItems;
}


/**
* Internal iterator for queries
*/
void Quadtree::query(Envelope *searchEnv,QuadtreeSelectAction *action){
	/**
	* the items that are selected are the items in quads which
	* overlap the search envelope
	*/
	root->selectAllItemsFromOverlapping(searchEnv,action);
}

vector<void*>* Quadtree::queryAll() {
	/**
	* the items that are matched are the items in quads which
	* overlap the search envelope
	*/
	vector<void*>* foundItems=new vector<void*>();
	root->addAllItems(foundItems);
	return foundItems;
}
