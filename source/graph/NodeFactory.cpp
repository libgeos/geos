#include "../headers/graph.h"

Node* NodeFactory::createNode(Coordinate coord) {
	return new Node(coord,NULL);
}

