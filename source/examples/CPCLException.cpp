#include "CustomCoordinateListExample.h"
#include "../headers/util.h"

CPCLException::CPCLException(){
	GEOSException();
	setName("CPCLException");
}
CPCLException::CPCLException(string msg){
	setName("CPCLException");
	setMessage(msg);
}

CPCLException::~CPCLException(){}
