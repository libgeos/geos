#include "../headers/util.h"

AssertionFailedException::AssertionFailedException(): txt("AssertionFaied: unknown"){}
AssertionFailedException::AssertionFailedException(string msg): txt(""){
	txt+="AssertionFailedException(";
	txt+=msg;
	txt+=")";
}
AssertionFailedException::~AssertionFailedException(){}
string AssertionFailedException::toString(){return txt;}
