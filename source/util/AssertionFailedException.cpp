#include "../headers/util.h"

namespace geos {

AssertionFailedException::AssertionFailedException(){
	GEOSException();
	setName("AssertionFaiedException");
}
AssertionFailedException::AssertionFailedException(string msg){
	setName("AssertionFaiedException");
	setMessage(msg);
}

AssertionFailedException::~AssertionFailedException(){}
}

