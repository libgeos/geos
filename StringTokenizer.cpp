#include "io.h"

StringTokenizer::StringTokenizer(): str(""){
	currPos=-1;
}
StringTokenizer::StringTokenizer(string txt): str(txt) {
	currPos=-1;
}
StringTokenizer::~StringTokenizer(){}

int StringTokenizer::nextToken(){
	return 0;
}

double StringTokenizer::getNVal(){
	return 0.0;
}

string StringTokenizer::getSVal(){
	return "";
}
