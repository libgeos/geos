/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#ifndef GEOS_UTIL_TOPOLOGYEXCEPTION_H
#define GEOS_UTIL_TOPOLOGYEXCEPTION_H

#include <string>

#include <geos/util/GEOSException.h>
#include <geos/geom/Coordinate.h> // to be removed when .inl is available

namespace geos {
namespace util { // geos.util

/**
 * \class TopologyException util.h geos.h
 *
 * \brief
 * Indicates an invalid or inconsistent topological situation encountered
 * during processing
 */
class TopologyException: public GEOSException {
public:
	TopologyException()
		:
		GEOSException("TopologyException", "")
	{}

	TopologyException(const std::string& msg)
		:
		GEOSException("TopologyException", msg)
	{}

	TopologyException(const std::string& msg, const geom::Coordinate *newPt)
		: GEOSException("TopologyException", msg+" "+newPt->toString()),
		pt(*newPt)
	{}

	~TopologyException() throw() {}
	geom::Coordinate* getCoordinate() { return &pt; }
private:
	geom::Coordinate pt;
};

} // namespace geos::util
} // namespace geos


#endif // GEOS_UTIL_TOPOLOGYEXCEPTION_H

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/
