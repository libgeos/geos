/**********************************************************************
 * $Id: CoordinateFilter.h 2556 2009-06-06 22:22:28Z strk $
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

#ifndef GEOS_GEOM_COORDINATEFILTER_H
#define GEOS_GEOM_COORDINATEFILTER_H

#include <geos/export.h>
#include <geos/inline.h>

#include <cassert>

namespace geos {
namespace geom { // geos::geom

class Coordinate;

/**
 * <code>Geometry</code> classes support the concept of applying a
 * coordinate filter to every coordinate in the <code>Geometry</code>.
 *
 * A  coordinate filter can either record information about each coordinate or
 * change the coordinate in some way. Coordinate filters implement the
 * interface <code>CoordinateFilter</code>. (<code>CoordinateFilter</code> is
 * an example of the Gang-of-Four Visitor pattern). Coordinate filters can be
 * used to implement such things as coordinate transformations, centroid and
 * envelope computation, and many other functions.
 *
 * TODO: provide geom::CoordinateInspector and geom::CoordinateMutator instead
 * of having the two versions of filter_rw and filter_ro
 *
 */
class GEOS_DLL CoordinateFilter {
public:
   virtual ~CoordinateFilter() {}

   /**
    * Performs an operation on <code>coord</code>.
    *
    * @param  coord  a <code>Coordinate</code> to which the filter is applied.
    */
   virtual void filter_rw(Coordinate* /*coord*/) const { assert(0); }

   /**
    * Performs an operation with <code>coord</code>.
    *
    * @param  coord  a <code>Coordinate</code> to which the filter is applied.
    */
   virtual void filter_ro(const Coordinate* /*coord*/) { assert(0); }
};

} // namespace geos::geom
} // namespace geos

//#ifdef GEOS_INLINE
//# include "geos/geom/CoordinateFilter.inl"
//#endif

#endif // ndef GEOS_GEOM_COORDINATEFILTER_H

/**********************************************************************
 * $Log$
 * Revision 1.4  2006/06/19 23:33:03  strk
 * Don't *require* CoordinateFilters to define both read-only and read-write methods.
 *
 * Revision 1.3  2006/03/24 09:52:41  strk
 * USE_INLINE => GEOS_INLINE
 *
 * Revision 1.2  2006/03/13 21:13:54  strk
 * Added comment about possible refactoring
 *
 * Revision 1.1  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/
