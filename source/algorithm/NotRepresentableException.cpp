#include "../headers/geosAlgorithm.h"

NotRepresentableException::NotRepresentableException(){
	setName("NotRepresentableException");
	setMessage("Projective point not representable on the Cartesian plane.");
}
NotRepresentableException::NotRepresentableException(string msg){
	setName("NotRepresentableException");
	setMessage(msg);
}
NotRepresentableException::~NotRepresentableException(){}

