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
 * Revision 1.2  2004/03/31 07:50:37  ybychkov
 * "geom" partially upgraded to JTS 1.4
 *
 * Revision 1.1  2004/03/19 09:48:45  ybychkov
 * "geomgraph" and "geomgraph/indexl" upgraded to JTS 1.4
 *
 *
 **********************************************************************/


#include "../headers/geomgraph.h"
#include "../headers/geom.h"
#include "../headers/geosAlgorithm.h"
#include "../headers/noding.h"
namespace geos {
/**
 * Validates that a collection of SegmentStrings is correctly noded.
 * Throws an appropriate exception if an noding error is found.
 *
 * @version 1.4
 */
vector<SegmentString*>* EdgeNodingValidator::toSegmentStrings(vector<Edge*> *edges) {
	// convert Edges to SegmentStrings
    vector<SegmentString*> *segStrings=new vector<SegmentString*>();
	for(int i=0;i<(int)edges->size();i++) {
		Edge *e=(*edges)[i];
		segStrings->push_back(new SegmentString(e->getCoordinates(),e));
    }
    return segStrings;
}

EdgeNodingValidator::EdgeNodingValidator(vector<Edge*> *edges) {
	nv=new NodingValidator(toSegmentStrings(edges));
}

EdgeNodingValidator::~EdgeNodingValidator() {
	delete nv;
}

void EdgeNodingValidator::checkValid() {
	nv->checkValid();
}

}
