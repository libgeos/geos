#include "opValid.h"
#include "stdio.h"

string TopologyValidationError::errMsg[]={
    "Topology Validation Error",
    "Repeated Point",
    "Hole lies outside shell",
    "Holes are nested",
    "Interior is disconnected",
    "Self-intersection",
    "Ring Self-intersection",
    "Nested shells",
    "Duplicate Rings"
};

TopologyValidationError::TopologyValidationError(int newErrorType,Coordinate newPt) {
	errorType=newErrorType;
	pt=newPt;
}

TopologyValidationError::TopologyValidationError(int newErrorType) {
	TopologyValidationError(errorType,Coordinate::getNull());
}

Coordinate& TopologyValidationError::getCoordinate(){
	return pt;
}

string TopologyValidationError::getMessage(){
	return errMsg[errorType];
}

string TopologyValidationError::toString() {
	return (getMessage().append(" at or near point")).append(pt.toString());
}
