#include "../headers/io.h"

namespace geos {

Writer::Writer(){
	str="";
}

Writer::~Writer(){}

void Writer::write(string txt){
	str+=txt;
}

string Writer::toString() {
	return str;
}

}