/**********************************************************************
 * $Id: GeometryFilter.h 2556 2009-06-06 22:22:28Z strk $
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

//#include <geos/platform.h>
#include <geos/export.h>
#include <geos/inline.h>

#include <string>
#include <vector>
#include <cassert>

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
class GEOS_DLL GeometryFilter {
public:
	/*
	 * Performs an operation with or on <code>geom</code>.
	 *
	 * @param  geom  a <code>Geometry</code> to which the filter
	 *         is applied.
	 *
	 * NOTE: this are not pure abstract to allow read-only
	 * or read-write-only filters to avoid defining a fake
	 * version of the not-implemented kind.
	 */
	virtual void filter_ro(const Geometry * /*geom*/) { assert(0); }
	virtual void filter_rw(Geometry * /*geom*/) { assert(0); }

	virtual ~GeometryFilter() {}
};

} // namespace geos::geom
} // namespace geos

//#ifdef GEOS_INLINE
//# include "geos/geom/GeometryFilter.inl"
//#endif

#endif // ndef GEOS_GEOM_GEOMETRYFILTER_H

/**********************************************************************
 * $Log$
 * Revision 1.4  2006/06/12 17:14:11  strk
 * added assert(0) version of filter_ro() and filter_rw() to allow implementations to only defined the required one.
 *
 * Revision 1.3  2006/06/08 11:20:24  strk
 * Added missing virtual destructor to abstract classes.
 *
 * Revision 1.2  2006/03/24 09:52:41  strk
 * USE_INLINE => GEOS_INLINE
 *
 * Revision 1.1  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/
