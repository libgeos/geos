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
 * Revision 1.4  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/


#ifndef GEOS_INDEX_H
#define GEOS_INDEX_H

#include <memory>
#include "platform.h"
#include "geom.h"

using namespace std;

namespace geos {

class SpatialIndex {
public:
	/**
	* Adds a spatial item to the index with the given envelope
	*/
	virtual void insert(Envelope *itemEnv,void *item)=0;
	/**
	* Queries the index for all items whose envelopes intersect the given search envelope
	*
	* @param searchEnv the envelope to query for
	* @return a list of the items found by the query
	*/
	virtual vector<void*>* query(Envelope *searchEnv)=0;
};
}

#endif

