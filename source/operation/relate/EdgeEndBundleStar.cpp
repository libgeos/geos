#include "../../headers/opRelate.h"
#include "stdio.h"

namespace geos {

EdgeEndBundleStar::EdgeEndBundleStar(){}

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