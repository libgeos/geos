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
 * Revision 1.8  2004/07/02 13:28:29  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.7  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/


#include <geos/opRelate.h>
#include <stdio.h>

namespace geos {

EdgeEndBundleStar::EdgeEndBundleStar(){}
EdgeEndBundleStar::~EdgeEndBundleStar(){
	map<EdgeEnd*,void*,EdgeEndLT>::iterator	it=edgeMap->begin();
	for (;it!=edgeMap->end();it++) {
		EdgeEndBundle *eeb=(EdgeEndBundle*) it->second;
//		void *ee= it->second;
		delete eeb;
	}
}

/**
* Insert a EdgeEnd in order in the list.
* If there is an existing EdgeStubBundle which is parallel, the EdgeEnd is
* added to the bundle.  Otherwise, a new EdgeEndBundle is created
* to contain the EdgeEnd.
* <br>
*/
void EdgeEndBundleStar::insert(EdgeEnd *e) {
	EdgeEndBundle *eb;
	map<EdgeEnd*,void*,EdgeEndLT>::iterator i=edgeMap->find(e);
	if (i==edgeMap->end())
		eb=NULL;
	else
		eb=(EdgeEndBundle*) (i->second);
	if (eb==NULL) {
		eb=new EdgeEndBundle(e);
		insertEdgeEnd(e,eb);
	} else {
		eb->insert(e);
	}
}

/**
* Update the IM with the contribution for the EdgeStubs around the node.
*/
void EdgeEndBundleStar::updateIM(IntersectionMatrix *im) {
	vector<EdgeEnd*> *edges=getEdges();
	for (vector<EdgeEnd*>::iterator it=edges->begin();it<edges->end();it++) {
		EdgeEndBundle *esb=(EdgeEndBundle*) *it;
		esb->updateIM(im);
	}
}
}

