/*
* $Log$
* Revision 1.4  2003/10/16 17:05:07  strk
* Made TopologyException inherit from GEOSException. Adjusted IllegalArgumentException subclassing.
*
*/
#include "../headers/util.h"

namespace geos {

IllegalArgumentException::IllegalArgumentException(): GEOSException() {
	setName("IllegalArgumentException");
}
IllegalArgumentException::IllegalArgumentException(string msg): GEOSException("IllegalArgumentException", msg){
	//setName("IllegalArgumentException");
	//setMessage(msg);
}

IllegalArgumentException::~IllegalArgumentException(){}
}

