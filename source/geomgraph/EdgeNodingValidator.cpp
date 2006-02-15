/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2006      Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 **********************************************************************/

#include <geos/geomgraph.h>
#include <geos/geom.h>
#include <geos/geosAlgorithm.h>
#include <geos/noding.h>

namespace geos {

vector<SegmentString*>&
EdgeNodingValidator::toSegmentStrings(vector<Edge*> *edges)
{
	// convert Edges to SegmentStrings
	for(unsigned int i=0, n=edges->size(); i<n; i++) {
		Edge *e=(*edges)[i];
		segStr.push_back(new SegmentString(e->getCoordinates(),e));
	}
	return segStr;
}

EdgeNodingValidator::~EdgeNodingValidator()
{
	for (SegmentString::NonConstVect::iterator
			i=segStr.begin(), e=segStr.end();
			i != e;
			++i)
	{
		delete *i;
	}
}

} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.4  2006/02/15 17:19:18  strk
 * NodingValidator synced with JTS-1.7, added CoordinateSequence::operator[]
 * and size() to easy port maintainance.
 *
 * Revision 1.3  2004/07/02 13:28:26  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.2  2004/03/31 07:50:37  ybychkov
 * "geom" partially upgraded to JTS 1.4
 *
 * Revision 1.1  2004/03/19 09:48:45  ybychkov
 * "geomgraph" and "geomgraph/indexl" upgraded to JTS 1.4
 *
 *
 **********************************************************************/

