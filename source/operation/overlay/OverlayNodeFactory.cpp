#include "../../headers/opOverlay.h"
#include "stdio.h"

namespace geos {

Node* OverlayNodeFactory::createNode(Coordinate coord) {
	return new Node(coord,new DirectedEdgeStar());
}
}