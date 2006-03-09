/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2005-2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#ifndef GEOS_OPERATION_ENDPOINTINFO_H
#define GEOS_OPERATION_ENDPOINTINFO_H

#include <geos/geom/Coordinate.h> // for composition

namespace geos {
namespace operation { // geos.operation


class EndpointInfo{
public:
	geom::Coordinate pt;
	bool isClosed;
	int degree;
    	EndpointInfo(const geom::Coordinate& newPt);
	void addEndpoint(bool newIsClosed);
};

} // namespace geos.operation
} // namespace geos

#endif

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/

