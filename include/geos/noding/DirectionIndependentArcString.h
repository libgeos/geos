/**********************************************************************
*
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2026 ISciences, LLC
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <memory>
#include <geos/export.h>


namespace geos::noding {
class ArcString;
}

namespace geos::noding {

/// The DirectionIndependentArcString is a thin wrapper over an ArcString that provides specialized
/// equality-testing and hashing functions. DirectionIndependentArcStrings/ are considered equal
/// if they cover the same path, regardless of direction. The control points of arcs are not required
/// to be identical, but must describe the same arc.
class GEOS_DLL DirectionIndependentArcString {

public:

    explicit DirectionIndependentArcString(const ArcString& as) : m_as(as) {}

    const ArcString& getArcString() const
    {
        return m_as;
    }

    bool operator==(const DirectionIndependentArcString& other) const;

private:
    const ArcString& m_as;
};
}

template<>
struct GEOS_DLL std::hash<geos::noding::DirectionIndependentArcString> {
    std::size_t operator()(const geos::noding::DirectionIndependentArcString& as) const noexcept;
};
