#include "CustomCoordinateListExample.h"
#include "../headers/util.h"

using namespace geos;

CPCLException::CPCLException(){
	GEOSException();
	setName("CPCLException");
}
CPCLException::CPCLException(string msg){
	setName("CPCLException");
	setMessage(msg);
}

CPCLException::~CPCLException(){}
