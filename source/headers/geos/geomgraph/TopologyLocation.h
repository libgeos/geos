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


#ifndef GEOS_GEOMGRAPH_TOPOLOGYLOCATION_H
#define GEOS_GEOMGRAPH_TOPOLOGYLOCATION_H

#include <vector>
#include <string>

#include <geos/inline.h>

namespace geos {
namespace geomgraph { // geos.geomgraph

class TopologyLocation {
public:
	TopologyLocation();
	~TopologyLocation();
	TopologyLocation(const std::vector<int> &newLocation);

	/**
	 * Constructs a TopologyLocation specifying how points on,
	 * to the left of, and to the right of some GraphComponent
	 * relate to some Geometry. Possible values for the
	 * parameters are Location.NULL, Location.EXTERIOR, Location.BOUNDARY, 
	 * and Location.INTERIOR.
	 * @see Location
	 */
	TopologyLocation(int on, int left, int right);
	TopologyLocation(int on);
	TopologyLocation(const TopologyLocation &gl);
	int get(unsigned int posIndex) const;
	bool isNull() const;
	bool isAnyNull() const;
	bool isEqualOnSide(const TopologyLocation &le, int locIndex) const;
	bool isArea() const;
	bool isLine() const;
	void flip();
	void setAllLocations(int locValue);
	void setAllLocationsIfNull(int locValue);
	void setLocation(unsigned int locIndex, int locValue);
	void setLocation(int locValue);
	const std::vector<int> &getLocations() const;
	void setLocations(int on, int left, int right);
	bool allPositionsEqual(int loc) const;
	void merge(const TopologyLocation &gl);
	std::string toString() const;
private:
	std::vector<int> location;
};


} // namespace geos.geomgraph
} // namespace geos

//#ifdef GEOS_INLINE
//# include "geos/geomgraph/TopologyLocation.inl"
//#endif

#endif // ifndef GEOS_GEOMGRAPH_TOPOLOGYLOCATION_H

/**********************************************************************
 * $Log$
 * Revision 1.2  2006/03/24 09:52:41  strk
 * USE_INLINE => GEOS_INLINE
 *
 * Revision 1.1  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/

