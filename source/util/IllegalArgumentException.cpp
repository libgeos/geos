#include "../headers/util.h"

namespace geos {

IllegalArgumentException::IllegalArgumentException(){
	GEOSException();
	setName("IllegalArgumentException");
}
IllegalArgumentException::IllegalArgumentException(string msg){
	setName("IllegalArgumentException");
	setMessage(msg);
}

IllegalArgumentException::~IllegalArgumentException(){}
}