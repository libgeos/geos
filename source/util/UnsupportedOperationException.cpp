/*
* $Log$
* Revision 1.4  2003/10/16 17:41:59  strk
* Fixed a bug in GEOSException that prevented print of the type of exception thrown.
*
*/
#include "../headers/util.h"

namespace geos {

UnsupportedOperationException::UnsupportedOperationException(): GEOSException() {
	setName("UnsupportedOperationException");
}
UnsupportedOperationException::UnsupportedOperationException(string msg): GEOSException("UnsupportedOperationException", msg) {
	//setName("UnsupportedOperationException");
	//setMessage(msg);
}

UnsupportedOperationException::~UnsupportedOperationException(){}
}

