#include "../headers/io.h"
#include "stdio.h"

namespace geos {

ParseException::ParseException(){
	GEOSException();
	setName("ParseException");
}

ParseException::ParseException(string msg){
	setName("ParseException");
	setMessage(msg);
}
ParseException::ParseException(string msg, string var){
	setName("ParseException");
	setMessage(msg+": "+var);
}
ParseException::ParseException(string msg, double num){
	setName("ParseException");
	string ntxt="";
	ntxt+=msg;
	ntxt+=": ";
	char buffer[255];
	sprintf(buffer,"%g",num);
	ntxt.append(buffer);
	setMessage(ntxt);
}

ParseException::~ParseException(){}
}

