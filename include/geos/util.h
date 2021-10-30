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

#include <geos/util/GEOSException.h>
#include <geos/util/IllegalArgumentException.h>
#include <geos/util/TopologyException.h>
#include <geos/util/GeometricShapeFactory.h>

#include <memory>
#include <type_traits>

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

template<class T, std::size_t N>
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
make_unique(std::size_t n) {
    typedef typename std::remove_extent<T>::type U;
    return std::unique_ptr<T>(new U[n]());
}

template<class T, class... Args>
typename _Unique_if<T>::_Known_bound
make_unique(Args &&...) = delete;

#endif

/** Use detail::down_cast<Derived*>(pointer_to_base) as equivalent of
 * static_cast<Derived*>(pointer_to_base) with safe checking in debug
 * mode.
 *
 * Only works if no virtual inheritance is involved.
 *
 * @param f pointer to a base class
 * @return pointer to a derived class
 */
template<typename To, typename From> inline To down_cast(From* f)
{
    static_assert(
        (std::is_base_of<From,
                        typename std::remove_pointer<To>::type>::value),
        "target type not derived from source type");
#if GEOS_DEBUG
    assert(f == nullptr || dynamic_cast<To>(f) != nullptr);
#endif
    return static_cast<To>(f);
}

// Avoid "redundant move" warning when calling std::move() to return
// unique_ptr<Derived> from a function with return type unique_ptr<Base>
// The std::move is required for the gcc 4.9 series, which has not addressed
// CWG defect 1579 ("return by converting move constructor")
// http://www.open-std.org/jtc1/sc22/wg21/docs/cwg_defects.html#1579
#if __GNUC__ > 0 && __GNUC__ < 5
#define RETURN_UNIQUE_PTR(x) (std::move(x))
#else
#define RETURN_UNIQUE_PTR(x) (x)
#endif

} // namespace detail
} // namespace geos

#endif // GEOS_UTIL_H
