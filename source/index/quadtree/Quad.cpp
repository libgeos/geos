#include "indexQuadtree.h"
#include "stdio.h"

Quad::Quad(Quad* newParent,Envelope *newEnv) {
	parent=newParent;
	env=newEnv;
	centre.x=(env->getMinX()+env->getMaxX())/2;
	centre.y=(env->getMinY()+env->getMaxY())/2;
	items=new vector<void*>();
}

Quad::~Quad() {
	delete items;
}

Quad* Quad::getParent() {
	return parent;
}

vector<void*>* Quad::getItems() {
	return items;
}

bool Quad::overlaps(Envelope *testEnv) {
	return env->overlaps(testEnv);
}

void Quad::add(void *item) {
	items->push_back(item);
}

vector<void*>* Quad::addAllItems(vector<void*> *newItems) {
	newItems->insert(newItems->end(),items->begin(),items->end());
	for(int i=0;i<4;i++) {
		if (subquad[i]!=NULL) {
			subquad[i]->addAllItems(newItems);
		}
	}
	return newItems;
}


vector<void*>* Quad::addAllItemsFromOverlapping(Envelope *searchEnv,vector<void*> *newItems) {
	if (!env->overlaps(searchEnv))
		return newItems;

	newItems->insert(newItems->end(),items->begin(),items->end());
	for (int i = 0; i < 4; i++) {
		if (subquad[i]!=NULL) {
			subquad[i]->addAllItemsFromOverlapping(searchEnv,newItems);
		}
	}
	return items;
}

void Quad::selectAllItemsFromOverlapping(Envelope *searchEnv,QuadtreeSelectAction *action) {
	if (!env->overlaps(searchEnv))
		return;

	// select all items from this quad
	for(int i=0; i<(int)items->size();i++) {
		action->select((*items)[i]);
	}
	for (int i = 0; i < 4; i++) {
		if (subquad[i]!=NULL) {
			subquad[i]->selectAllItemsFromOverlapping(searchEnv,action);
		}
	}
}

/**
* Returns the quad containing the envelope.  Creates the quad if
* it does not already exist.
*/
Quad* Quad::getQuad(Envelope *searchEnv) {
	int subquadIndex=getSubquadIndex(searchEnv);
	if (subquadIndex!=-1) {
		// create the quad if it does not exist
		Quad* quad=getSubquad(subquadIndex);
		// recursively search the found quad
		return quad->getQuad(searchEnv);
	} else {
		return this;
	}
}

/**
* Returns the smallest existing quad containing the envelope.
*/
Quad* Quad::find(Envelope *searchEnv) {
	int subquadIndex=getSubquadIndex(searchEnv);
	if (subquadIndex==-1)
		return this;
	if (subquad[subquadIndex]!=NULL) {
		// query lies in subquad, so search it
		Quad* quad=subquad[subquadIndex];
		return quad->find(searchEnv);
	}
	// no existing subquad, so return this one anyway
	return this;
}

/**
* Returns the index of the subquad that wholely contains the search envelope.
* If none does, returns -1
*/
int Quad::getSubquadIndex(Envelope *searchEnv) {
	int subquadIndex=-1;
	if (searchEnv->getMinX()>centre.x) {
		if (searchEnv->getMinY()>centre.y) subquadIndex=3;
		if (searchEnv->getMaxY()<centre.y) subquadIndex=1;
	}
	if (searchEnv->getMaxX()<centre.x) {
		if (searchEnv->getMinY()>centre.y) subquadIndex=2;
		if (searchEnv->getMaxY()<centre.y) subquadIndex=0;
	}
	return subquadIndex;
}

/**
* get the subquad for the index.
* If it doesn't exist, create it
*/
Quad* Quad::getSubquad(int ind) {
	if (subquad[ind]==NULL) {
		// create a new subquad in the appropriate quadrant
		double minx=0.0;
		double maxx=0.0;
		double miny=0.0;
		double maxy=0.0;
		switch(ind) {
			case 0:
				minx=env->getMinX();
				maxx=centre.x;
				miny=env->getMinY();
				maxy=centre.y;
				break;
			case 1:
				minx=centre.x;
				maxx=env->getMaxX();
				miny=env->getMinY();
				maxy=centre.y;
				break;
			case 2:
				minx=env->getMinX();
				maxx=centre.x;
				miny=centre.y;
				maxy=env->getMaxY();
				break;
			case 3:
				minx=centre.x;
				maxx=env->getMaxX();
				miny=centre.y;
				maxy=env->getMaxY();
				break;
		}
		Envelope *sqEnv=new Envelope(minx,maxx,miny,maxy);
		Quad *sq=new Quad(this,sqEnv);
		subquad[ind]=sq;
	}
	return subquad[ind];
}
