#include "../../headers/indexStrtree.h"
#include "../../headers/util.h"
#include "stdio.h"

namespace geos {

AbstractNode::AbstractNode(int newLevel) {
	childBoundables=new vector<Boundable*>();
	bounds=NULL;
	level=newLevel;
}

AbstractNode::~AbstractNode() {
	delete childBoundables;
}

vector<Boundable*>* AbstractNode::getChildBoundables() {
	return childBoundables;
}

void* AbstractNode::getBounds() {
	if (bounds==NULL) {
		bounds = computeBounds();
	}
	return bounds;
}

int AbstractNode::getLevel() {
	return level;
}

/**
*@param  childBoundable  either a Node or an ItemBoundable
*/
void AbstractNode::addChildBoundable(Boundable *childBoundable) {
	Assert::isTrue(bounds==NULL);
	childBoundables->push_back(childBoundable);
}
}