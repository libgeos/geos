#ifndef GEOS_OPOVERLAY_H
#define GEOS_OPOVERLAY_H

#include <string>
#include <vector>
#include <map>
#include "platform.h"
#include "graph.h"


class OverlayNodeFactory: public NodeFactory {
public:
	OverlayNodeFactory() {};
};

class MaximalEdgeRing: public EdgeRing {
public:
	MaximalEdgeRing(DirectedEdge *de, GeometryFactory *gf, CGAlgorithms *cga) {};
};
#endif
