#include "io.h"

ParseException::ParseException(): txt(""){}
ParseException::ParseException(string msg): txt(""){
	txt+="ParseException(";
	txt+=msg;
	txt+=")";
}
ParseException::ParseException(string msg, string var): txt(""){
	txt+="ParseException(";
	txt+=msg;
	txt+=": ";
	txt+=var;
	txt+=")";
}
ParseException::ParseException(string msg, double num): txt(""){
	txt+="ParseException(";
	txt+=msg;
	txt+=": ";
	char buffer[255];
	sprintf(buffer,"%g",num);
	txt.append(buffer);
	txt.append(")");
}
ParseException::~ParseException(){}
string ParseException::toString(){return txt;}
