/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 * Copyright (C) 2005 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#ifndef GEOS_GEOM_GEOMETRYFILTER_H
#define GEOS_GEOM_GEOMETRYFILTER_H

#include <string>
#include <vector>

//#include <geos/platform.h>

#include <geos/inline.h>

namespace geos {
	namespace geom { // geos::geom
		class Geometry;
	}
}

namespace geos {
namespace geom { // geos::geom


/** \brief
 * Geometry classes support the concept of applying a Geometry
 * filter to the Geometry.
 *
 * In the case of GeometryCollection
 * subclasses, the filter is applied to every element Geometry.
 * A Geometry filter can either record information about the Geometry
 * or change the Geometry in some way.
 * Geometry filters implement the interface GeometryFilter.
 * (GeometryFilter is an example of the Gang-of-Four Visitor pattern).
 */
class GeometryFilter {
public:
	/*
	 * Performs an operation with or on <code>geom</code>.
	 *
	 * @param  geom  a <code>Geometry</code> to which the filter
	 *         is applied.
	 */
	virtual void filter_ro(const Geometry *geom)=0;
	virtual void filter_rw(Geometry *geom)=0;
};

} // namespace geos::geom
} // namespace geos

//#ifdef GEOS_INLINE
//# include "geos/geom/GeometryFilter.inl"
//#endif

#endif // ndef GEOS_GEOM_GEOMETRYFILTER_H

/**********************************************************************
 * $Log$
 * Revision 1.2  2006/03/24 09:52:41  strk
 * USE_INLINE => GEOS_INLINE
 *
 * Revision 1.1  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/
