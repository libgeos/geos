#include "../../headers/indexQuadtree.h"
#include "../../headers/util.h"

QuadTreeNode* QuadTreeNode::createNode(Envelope *env) {
	QuadTreeKey* key=new QuadTreeKey(env);
	QuadTreeNode *node=new QuadTreeNode(key->getEnvelope(),key->getLevel());
	return node;
}

QuadTreeNode* QuadTreeNode::createExpanded(QuadTreeNode *node,Envelope *addEnv){
	Envelope *expandEnv=new Envelope(*addEnv);
	if (node!=NULL) expandEnv->expandToInclude(node->env);
	QuadTreeNode *largerNode=createNode(expandEnv);
	if (node!=NULL) largerNode->insertNode(node);
	return largerNode;
}

QuadTreeNode::QuadTreeNode(Envelope *nenv,int nlevel){
	env=nenv;
	level=nlevel;
	centre=new Coordinate();
	centre->x=(env->getMinX()+env->getMaxX())/2;
	centre->y=(env->getMinY()+env->getMaxY())/2;
}

Envelope* QuadTreeNode::getEnvelope() {
	return env;
}

bool QuadTreeNode::isSearchMatch(Envelope *searchEnv){
	return env->intersects(searchEnv);
}

/**
* Returns the subquad containing the envelope.
* Creates the subquad if
* it does not already exist.
*/
QuadTreeNode* QuadTreeNode::getNode(Envelope *searchEnv){
	int subnodeIndex=getSubnodeIndex(searchEnv,centre);
	// if subquadIndex is -1 searchEnv is not contained in a subquad
	if (subnodeIndex!=-1) {
		// create the quad if it does not exist
		QuadTreeNode *node=getSubnode(subnodeIndex);
		// recursively search the found/created quad
		return node->getNode(searchEnv);
	} else {
		return this;
	}
}

/**
* Returns the smallest <i>existing</i>
* node containing the envelope.
*/
QuadTreeNodeBase* QuadTreeNode::find(Envelope *searchEnv) {
	int subnodeIndex=getSubnodeIndex(searchEnv,centre);
	if (subnodeIndex==-1)
		return this;
	if (subnode[subnodeIndex]!=NULL) {
		// query lies in subquad, so search it
		QuadTreeNode *node=subnode[subnodeIndex];
		return node->find(searchEnv);
	}
	// no existing subquad, so return this one anyway
	return this;
}

void QuadTreeNode::insertNode(QuadTreeNode* node) {
	Assert::isTrue(env==NULL || env->contains(node->env));
	//System.out.println(env);
	//System.out.println(quad.env);
	int index=getSubnodeIndex(node->env,centre);
	//System.out.println(index);
	if (node->level==level-1) {
		subnode[index]=node;
		//System.out.println("inserted");
	} else {
		// the quad is not a direct child, so make a new child quad to contain it
		// and recursively insert the quad
		QuadTreeNode *childNode=createSubnode(index);
		childNode->insertNode(node);
		subnode[index]=childNode;
	}
}

/**
* get the subquad for the index.
* If it doesn't exist, create it
*/
QuadTreeNode* QuadTreeNode::getSubnode(int index){
	if (subnode[index]==NULL) {
		subnode[index]=createSubnode(index);
	}
	return subnode[index];
}

QuadTreeNode* QuadTreeNode::createSubnode(int index) {
	// create a new subquad in the appropriate quadrant
	double minx=0.0;
	double maxx=0.0;
	double miny=0.0;
	double maxy=0.0;

	switch (index) {
		case 0:
			minx=env->getMinX();
			maxx=centre->x;
			miny=env->getMinY();
			maxy=centre->y;
			break;
		case 1:
			minx=centre->x;
			maxx=env->getMaxX();
			miny=env->getMinY();
			maxy=centre->y;
			break;
	case 2:
			minx=env->getMinX();
			maxx=centre->x;
			miny=centre->y;
			maxy=env->getMaxY();
			break;
	case 3:
			minx=centre->x;
			maxx=env->getMaxX();
			miny=centre->y;
			maxy=env->getMaxY();
			break;
	}
	Envelope *sqEnv=new Envelope(minx,maxx,miny,maxy);
	QuadTreeNode *node=new QuadTreeNode(sqEnv,level-1);
	return node;
}
