#include "../../headers/opRelate.h"
#include "stdio.h"

namespace geos {

Node* RelateNodeFactory::createNode(Coordinate coord) {
	return new RelateNode(coord,new EdgeEndBundleStar());
}
}