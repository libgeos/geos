#include "../headers/geom.h"
#include "stdio.h"

namespace geos {

string TopologyException::msgWithCoord(string msg,Coordinate *newPt){
	if (newPt!=NULL)
		return msg+newPt->toString();
	return msg;
}

TopologyException::TopologyException(string msg){
	setName("TopologyException");
	setMessage(msg);
}

TopologyException::TopologyException(string msg,Coordinate *newPt) {
	setName("TopologyException");
	pt=new Coordinate(*newPt);
	setMessage(msgWithCoord(msg,pt));
	delete pt;
}

Coordinate* TopologyException::getCoordinate() {
	return pt;
}

void TopologyException::setName(string nname){
	name=nname;
}

void TopologyException::setMessage(string msg){
	txt=msg;
}

}

