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
#include <memory>
#include <vector>

namespace geos::geom {
    class CoordinateSequence;
}

namespace geos::noding {

/// A PathString represents a contiguous line/arc to be used as an input or output
/// of a noding process.
class GEOS_DLL PathString {
public:
    virtual ~PathString() = default;

    virtual std::size_t getSize() const = 0;

    virtual double getLength() const = 0;

    /// \brief
    /// Return a pointer to the CoordinateSequence associated
    /// with this PathString.
    virtual const std::shared_ptr<const geom::CoordinateSequence>& getCoordinates() const = 0;

    std::vector<PathString*>
    static toRawPointerVector(const std::vector<std::unique_ptr<PathString>> & segStrings);
};

}
