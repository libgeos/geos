#include "../../headers/opRelate.h"
#include "stdio.h"

Node* RelateNodeFactory::createNode(Coordinate coord) {
	return new RelateNode(coord,new EdgeEndBundleStar());
}
