#include "../../headers/opRelate.h"
#include "stdio.h"

RelateNode::RelateNode(Coordinate& coord,EdgeEndStar *edges): Node(coord,edges) {}

/**
* Update the IM with the contribution for this component.
* A component only contributes if it has a labelling for both parent geometries
*/
void RelateNode::computeIM(IntersectionMatrix *im) {
	im->setAtLeastIfValid(label->getLocation(0),label->getLocation(1),0);
}

/**
* Update the IM with the contribution for the EdgeEnds incident on this node.
*/
void RelateNode::updateIMFromEdges(IntersectionMatrix *im) {
	((EdgeEndBundleStar*) edges)->updateIM(im);
}
