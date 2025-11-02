/**********************************************************************
*
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2025 ISciences, LLC
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/export.h>

#include <cstddef>

namespace geos::noding {

/// A PathString represents a contiguous line/arc to the used as an input for
/// noding. To access the coordinates, it is necessary to know whether they
/// represent a set of line segments (SegmentString) or circular arcs (ArcString).
class GEOS_DLL PathString {
public:
    virtual ~PathString() = default;

    virtual std::size_t getSize() const = 0;

    virtual double getLength() const = 0;
};

}
