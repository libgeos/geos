#include "graph.h"

EdgeList::EdgeList(){}

/**
 * Insert an edge unless it is already in the list
 */
void EdgeList::insert(Edge *e) {
	push_back(e);
}

/**
 * If the edge e is already in the list, return its index.
 * @return  index, if e is already in the list
 *          -1 otherwise
 */
int EdgeList::findEdgeIndex(Edge *e) {
	for (int i=0; i<this->size();i++) {
		if ( (*this)[i]->equals(e) )
            return i;
	}
	return -1;
}

string EdgeList::print() {
	string out="MULTILINESTRING ( ";
	for(unsigned int j=0; j<this->size();j++) {
//		Edge *e=at(j);
        Edge *e=(*this)[j];
		if (j>0) out+=",";
		out+="(";
		CoordinateList *pts=e->getCoordinates();
		for(int i=0; i<pts->getSize();i++) {
			if (i>0) out+=",";
			out+=pts->getAt(i).toString();
		}
		out+=")";
	}
	out+=")  ";
	return out;
}