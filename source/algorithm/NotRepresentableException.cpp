#include "geosAlgorithm.h"

NotRepresentableException::NotRepresentableException(): txt("Projective point not representable on the Cartesian plane."){}
NotRepresentableException::NotRepresentableException(string msg): txt(""){
	txt+="NonRepresentableException(";
	txt+=msg;
	txt+=")";
}
NotRepresentableException::~NotRepresentableException(){}
string NotRepresentableException::toString(){return txt;}

