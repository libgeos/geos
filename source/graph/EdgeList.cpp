/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 * $Log$
 * Revision 1.14  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/


#include "../headers/graph.h"

namespace geos {

EdgeList::EdgeList(){}

EdgeList::~EdgeList(){
//	for (int i=0; i<(int)this->size();i++) {
//		pop_back();
//	}
}
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
	for (int i=0; i<(int)this->size();i++) {
		if ( (*this)[i]->equals(e) )
            return i;
	}
	return -1;
}

string EdgeList::print() {
	string out="MULTILINESTRING ( ";
	for(unsigned int j=0; j<this->size();j++) {
        Edge *e=(*this)[j];
		if (j>0) out+=",";
		out+="(";
		const CoordinateList* pts=e->getCoordinates();
		for(int i=0; i<pts->getSize();i++) {
			if (i>0) out+=",";
			out+=pts->getAt(i).toString();
		}
		out+=")";
	}
	out+=")  ";
	return out;
}

}

