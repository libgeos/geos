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
 **********************************************************************
 *
 * Utility header to retain a bit of backward compatibility.
 * Try to avoid including this header directly.
 *
 **********************************************************************/

#ifndef GEOS_UTIL_H
#define GEOS_UTIL_H

//#include <geos/util/AssertionFailedException.h>
#include <geos/util/GEOSException.h>
#include <geos/util/IllegalArgumentException.h>
#include <geos/util/TopologyException.h>
//#include <geos/util/UnsupportedOperationException.h>
//#include <geos/util/CoordinateArrayFilter.h>
//#include <geos/util/UniqueCoordinateArrayFilter.h>
#include <geos/util/GeometricShapeFactory.h>
//#include <geos/util/math.h>

#include <memory>

//
// Private macros definition
//

namespace geos {
template<class T>
void
ignore_unused_variable_warning(T const &) {}

namespace detail {
#if __cplusplus >= 201402L
using std::make_unique;
#else
// Backport of std::make_unique to C++11
// Source: https://stackoverflow.com/a/19472607
template<class T>
struct _Unique_if {
    typedef std::unique_ptr<T> _Single_object;
};

template<class T>
struct _Unique_if<T[]> {
    typedef std::unique_ptr<T[]> _Unknown_bound;
};

template<class T, size_t N>
struct _Unique_if<T[N]> {
    typedef void _Known_bound;
};

template<class T, class... Args>
typename _Unique_if<T>::_Single_object
make_unique(Args &&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

template<class T>
typename _Unique_if<T>::_Unknown_bound
make_unique(size_t n) {
    typedef typename std::remove_extent<T>::type U;
    return std::unique_ptr<T>(new U[n]());
}

template<class T, class... Args>
typename _Unique_if<T>::_Known_bound
make_unique(Args &&...) = delete;

#endif
}
}

#endif // GEOS_UTIL_H
