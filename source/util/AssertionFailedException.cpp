/*
* $Log$
* Revision 1.6  2003/10/20 13:56:34  strk
* fixed typo
*
*/
#include "../headers/util.h"

namespace geos {

AssertionFailedException::AssertionFailedException(){
	GEOSException();
	setName("AssertionFailedException");
}
AssertionFailedException::AssertionFailedException(string msg){
	setName("AssertionFailedException");
	setMessage(msg);
}

AssertionFailedException::~AssertionFailedException(){}
}

