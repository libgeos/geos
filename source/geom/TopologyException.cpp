#include "../headers/geom.h"
#include "stdio.h"

string TopologyException::msgWithCoord(string msg,Coordinate *newPt){
	if (newPt!=NULL)
		return msg+newPt->toString();
	return msg;
}

TopologyException::TopologyException(string msg){
//	super(msg);
}

TopologyException::TopologyException(string msg,Coordinate *newPt) {
//	super(msgWithCoord(msg, pt));
	pt=new Coordinate(*newPt);
}

Coordinate* TopologyException::getCoordinate() {
	return pt;
}
