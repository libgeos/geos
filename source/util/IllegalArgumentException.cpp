#include "../headers/util.h"

IllegalArgumentException::IllegalArgumentException(){
	GEOSException();
	setName("IllegalArgumentException");
}
IllegalArgumentException::IllegalArgumentException(string msg){
	setName("IllegalArgumentException");
	setMessage(msg);
}

IllegalArgumentException::~IllegalArgumentException(){}
