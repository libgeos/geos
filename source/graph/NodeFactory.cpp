#include "../headers/graph.h"

namespace geos {

Node* NodeFactory::createNode(Coordinate coord) {
	return new Node(coord,NULL);
}

}

