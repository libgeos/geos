#include "graph.h"

Node NodeFactory::createNode(Coordinate coord) {
	return Node(coord,NULL);
}