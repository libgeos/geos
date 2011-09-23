/**********************************************************************
 * $Id: GeometryComponentFilter.h 2556 2009-06-06 22:22:28Z strk $
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2005-2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#ifndef GEOS_GEOM_GEOMETRYCOMPONENTFILTER_H
#define GEOS_GEOM_GEOMETRYCOMPONENTFILTER_H

#include <geos/export.h>
#include <geos/inline.h>

namespace geos {
	namespace geom { // geos::geom
		class Geometry;
	}
}

namespace geos {
namespace geom { // geos::geom

/**
 *  <code>Geometry</code> classes support the concept of applying
 *  a <code>GeometryComponentFilter</code>
 *  filter to the <code>Geometry</code>.
 *  The filter is applied to every component of the <code>Geometry</code>
 *  which is itself a <code>Geometry</code>.
 *  A <code>GeometryComponentFilter</code> filter can either
 *  record information about the <code>Geometry</code>
 *  or change the <code>Geometry</code> in some way.
 *  <code>GeometryComponentFilter</code>
 *  is an example of the Gang-of-Four Visitor pattern.
 *
 */
class GEOS_DLL GeometryComponentFilter {
public:

	/**
	 *  Performs an operation with or on <code>geom</code>.
	 *
	 * @param  geom  a <code>Geometry</code> to which the filter
	 * is applied.
	 */
	virtual void filter_rw(Geometry *geom);
	virtual void filter_ro(const Geometry *geom);

	virtual ~GeometryComponentFilter() {}
};

} // namespace geos::geom
} // namespace geos

//#ifdef GEOS_INLINE
//# include "geos/geom/GeometryComponentFilter.inl"
//#endif

#endif // ndef GEOS_GEOM_GEOMETRYCOMPONENTFILTER_H

/**********************************************************************
 * $Log$
 * Revision 1.4  2006/06/08 11:20:24  strk
 * Added missing virtual destructor to abstract classes.
 *
 * Revision 1.3  2006/04/13 14:25:17  strk
 * TopologyPreservingSimplifier initial port
 *
 * Revision 1.2  2006/03/24 09:52:41  strk
 * USE_INLINE => GEOS_INLINE
 *
 * Revision 1.1  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/
