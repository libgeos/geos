/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2005-2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/export.h>
#include <geos/geom/Coordinate.h>

#include <cassert>

namespace geos {
namespace geom { // geos::geom

/** \brief
 * Geometry classes support the concept of applying a
 * coordinate filter to every coordinate in the Geometry.
 *
 * A  coordinate filter can either record information about each coordinate or
 * change the coordinate in some way. Coordinate filters implement the
 * interface <code>CoordinateFilter</code>. (<code>CoordinateFilter</code> is
 * an example of the Gang-of-Four Visitor pattern). Coordinate filters can be
 * used to implement such things as coordinate transformations, centroid and
 * envelope computation, and many other functions.
 *
 * A CoordinateFilter should be able to process a CoordinateXY object and can
 * optionally provide specialized implementations for higher-dimensionality
 * Coordinates. If the behavior can be templated on coordinate type, then
 * a filter may inherit from CoordinateInspector or CoordinateMutator to
 * generate these implementations from a template.
 *
 */
class GEOS_DLL CoordinateFilter {
public:
    virtual
    ~CoordinateFilter() {}

    /** \brief
     * Performs an operation on `coord`.
     *
     * **param** `coord` a Coordinate to which the filter is applied.
     */
    virtual void
    filter_rw(CoordinateXY* /*coord*/) const
    {
        assert(0);
    }

    /** \brief
     * Performs an operation with `coord`.
     *
     * **param** `coord`  a Coordinate to which the filter is applied.
     */
    virtual void
    filter_ro(const CoordinateXY* /*coord*/)
    {
        assert(0);
    }

    virtual void
    filter_rw(Coordinate* c) const
    {
        filter_rw(static_cast<CoordinateXY*>(c));
    }

    virtual void
    filter_ro(const Coordinate* c)
    {
        filter_ro(static_cast<const CoordinateXY*>(c));
    }

    virtual void
    filter_rw(CoordinateXYM* c) const
    {
        filter_rw(static_cast<CoordinateXY*>(c));
    }

    virtual void
    filter_ro(const CoordinateXYM* c)
    {
        filter_ro(static_cast<const CoordinateXY*>(c));
    }

    virtual void
    filter_rw(CoordinateXYZM* c) const
    {
        filter_rw(static_cast<Coordinate*>(c));
    }

    virtual void
    filter_ro(const CoordinateXYZM* c)
    {
        filter_ro(static_cast<const Coordinate*>(c));
    }
};

template<class Derived>
class CoordinateInspector : public CoordinateFilter
{
public:
    virtual void filter_ro(const CoordinateXY* c) override { static_cast<Derived*>(this)->filter(c); }
    virtual void filter_ro(const Coordinate* c) override { static_cast<Derived*>(this)->filter(c); }
    virtual void filter_ro(const CoordinateXYM* c) override { static_cast<Derived*>(this)->filter(c); }
    virtual void filter_ro(const CoordinateXYZM* c) override { static_cast<Derived*>(this)->filter(c); }
};

template<class Derived>
class CoordinateMutator : public CoordinateFilter
{
public:
    virtual void filter_rw(CoordinateXY* c) const override { static_cast<const Derived*>(this)->filter(c); }
    virtual void filter_rw(Coordinate* c) const override { static_cast<const Derived*>(this)->filter(c); }
    virtual void filter_rw(CoordinateXYM* c) const override { static_cast<const Derived*>(this)->filter(c); }
    virtual void filter_rw(CoordinateXYZM* c) const override { static_cast<const Derived*>(this)->filter(c); }
};

} // namespace geos::geom
} // namespace geos

