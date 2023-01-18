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

#include <cassert>
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
using std::make_unique;

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

} // namespace detail
} // namespace geos

#endif // GEOS_UTIL_H
