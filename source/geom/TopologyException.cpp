#include "../headers/geom.h"
#include "../headers/util.h"
#include "stdio.h"

namespace geos {

TopologyException::TopologyException(string msg): GEOSException("TopologyException", msg) {
}

TopologyException::TopologyException(string msg,const Coordinate *newPt): GEOSException("TopologyException", msg+" "+newPt->toString()) {
	pt=new Coordinate(*newPt);
}

TopologyException::~TopologyException() {
	delete pt;
}

Coordinate* TopologyException::getCoordinate() {
	return pt;
}


}

