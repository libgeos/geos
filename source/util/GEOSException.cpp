/*
* $Log$
* Revision 1.4  2003/10/16 17:41:59  strk
* Fixed a bug in GEOSException that prevented print of the type of exception thrown.
*
*/
#include "../headers/util.h"

namespace geos {

GEOSException::GEOSException(){
	setName("GEOSException");
	setMessage("unknown error");
}
GEOSException::GEOSException(string msg){
	setName("GEOSException");
	setMessage(msg);
}
GEOSException::GEOSException(string nname,string msg){
	setName(nname);
	setMessage(msg);
}
GEOSException::~GEOSException(){}
string GEOSException::toString(){
	string result(name);
	result+=": ";
	result+=txt;
	return result;
}

void GEOSException::setName(string nname){
	name=nname;
}

void GEOSException::setMessage(string msg){
	txt=msg;
}
}

