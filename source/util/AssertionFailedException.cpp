#include "../headers/util.h"

AssertionFailedException::AssertionFailedException(){
	GEOSException();
	setName("AssertionFaiedException");
}
AssertionFailedException::AssertionFailedException(string msg){
	setName("AssertionFaiedException");
	setMessage(msg);
}

AssertionFailedException::~AssertionFailedException(){}
