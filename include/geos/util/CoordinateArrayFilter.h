/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
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

#pragma once

#include <geos/export.h>
#include <cassert>
#include <set>
#include <vector>

#include <geos/geom/CoordinateFilter.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/Coordinate.h>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251) // warning C4251: needs to have dll-interface to be used by clients of class
#endif

namespace geos {
namespace util { // geos::util

/*
 *  A CoordinateFilter that fills a vector of Coordinate const pointers.
 */
class GEOS_DLL CoordinateArrayFilter : public geom::CoordinateInspector<CoordinateArrayFilter> {
public:

    CoordinateArrayFilter(std::vector<const geom::Coordinate*>& target)
        : pts(target)
    {}

    /**
     * Destructor.
     * Virtual dctor promises appropriate behaviour when someone will
     * delete a derived-class object via a base-class pointer.
     * http://www.parashift.com/c++-faq-lite/virtual-functions.html#faq-20.7
     */
    ~CoordinateArrayFilter() override {}

    template<typename CoordType>
    void filter(const CoordType* coord)
    {
        pts.push_back(coord);
    }

    void filter(const geom::CoordinateXY*) {
        assert(0); // not supported
    }

    void filter(const geom::CoordinateXYM*) {
        assert(0); // not supported
    }

private:
    std::vector<const geom::Coordinate*>& pts;  // target set reference

    // Declare type as noncopyable
    CoordinateArrayFilter(const CoordinateArrayFilter& other) = delete;
    CoordinateArrayFilter& operator=(const CoordinateArrayFilter& rhs) = delete;
};





} // namespace geos::util
} // namespace geos

#ifdef _MSC_VER
#pragma warning(pop)
#endif

