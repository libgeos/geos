#include "../headers/util.h"

namespace geos {

UnsupportedOperationException::UnsupportedOperationException(){
	GEOSException();
	setName("UnsupportedOperationException");
}
UnsupportedOperationException::UnsupportedOperationException(string msg){
	setName("UnsupportedOperationException");
	setMessage(msg);
}

UnsupportedOperationException::~UnsupportedOperationException(){}
}