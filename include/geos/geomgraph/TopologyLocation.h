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
 **********************************************************************
 *
 * Last port: geomgraph/TopologyLocation.java rev. 1.6 (JTS-1.10)
 *
 **********************************************************************/


#ifndef GEOS_GEOMGRAPH_TOPOLOGYLOCATION_H
#define GEOS_GEOMGRAPH_TOPOLOGYLOCATION_H

#include <geos/export.h>
#include <geos/inline.h>

#include <vector>
#include <string>

namespace geos {
namespace geomgraph { // geos.geomgraph

/** \brief
 * A TopologyLocation is the labelling of a
 * GraphComponent's topological relationship to a single Geometry.
 * 
 * If the parent component is an area edge, each side and the edge itself
 * have a topological location.  These locations are named
 * 
 *  - ON: on the edge
 *  - LEFT: left-hand side of the edge
 *  - RIGHT: right-hand side
 * 
 * If the parent component is a line edge or node, there is a single
 * topological relationship attribute, ON.
 *
 * The possible values of a topological location are
 * {Location::UNDEF, Location::EXTERIOR, Location::BOUNDARY, Location::INTERIOR}
 *
 * The labelling is stored in an array location[j] where
 * where j has the values ON, LEFT, RIGHT
 */
class GEOS_DLL TopologyLocation {

public:

	friend std::ostream& operator<< (std::ostream&, const TopologyLocation&);

	TopologyLocation();

	~TopologyLocation();

	TopologyLocation(const std::vector<int> &newLocation);

	/** \brief
	 * Constructs a TopologyLocation specifying how points on, to the
	 * left of, and to the right of some GraphComponent relate to some
	 * Geometry.
	 *
	 * Possible values for the
	 * parameters are Location::UNDEF, Location::EXTERIOR, Location::BOUNDARY, 
	 * and Location::INTERIOR.
	 *
	 * @see Location
	 */
	TopologyLocation(int on, int left, int right);

	TopologyLocation(int on);

	TopologyLocation(const TopologyLocation &gl);

	int get(size_t posIndex) const;

	/**
	 * @return true if all locations are Location::UNDEF
	 */
	bool isNull() const;

	/**
	 * @return true if any locations is Location::UNDEF
	 */
	bool isAnyNull() const;

	bool isEqualOnSide(const TopologyLocation &le, int locIndex) const;

	bool isArea() const;

	bool isLine() const;

	void flip();

	void setAllLocations(int locValue);

	void setAllLocationsIfNull(int locValue);

	void setLocation(size_t locIndex, int locValue);

	void setLocation(int locValue);

	/// Warning: returns reference to owned memory
	const std::vector<int> &getLocations() const;

	void setLocations(int on, int left, int right);

	bool allPositionsEqual(int loc) const;

	/** \brief
	 * merge updates only the UNDEF attributes of this object
	 * with the attributes of another.
	 */
	void merge(const TopologyLocation &gl);

	std::string toString() const;

private:

	std::vector<int> location;
};

std::ostream& operator<< (std::ostream&, const TopologyLocation&);

} // namespace geos.geomgraph
} // namespace geos

//#ifdef GEOS_INLINE
//# include "geos/geomgraph/TopologyLocation.inl"
//#endif

#endif // ifndef GEOS_GEOMGRAPH_TOPOLOGYLOCATION_H

/**********************************************************************
 * $Log$
 * Revision 1.4  2006/06/12 11:29:23  strk
 * unsigned int => size_t
 *
 * Revision 1.3  2006/04/06 09:01:11  strk
 * Doxygen comments, port info, operator<<, assertion checking
 *
 * Revision 1.2  2006/03/24 09:52:41  strk
 * USE_INLINE => GEOS_INLINE
 *
 * Revision 1.1  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/

