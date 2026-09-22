/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2026 Martin Davis
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: operation/distance/CoordinateSequenceLocation.java
 * (locationtech/jts DirectedHausdorffDistance)
 *
 **********************************************************************/

#pragma once

#include <geos/export.h>
#include <geos/geom/Coordinate.h>

#include <cstddef>

namespace geos {
namespace geom {
class CoordinateSequence;
}
}

namespace geos {
namespace operation {
namespace distance {

/**
 * A location on a {@link FacetSequence}.
 *
 * Location indexes are always the index of a sequence segment.
 * Thus they are always less than the number of vertices
 * in the sequence. The endpoint in a sequence
 * has the index of the final segment in the sequence.
 * If the sequence is a ring, the index of the final endpoint is
 * normalized to 0.
 *
 * \author Martin Davis
 */
class GEOS_DLL CoordinateSequenceLocation {
public:

    CoordinateSequenceLocation(const geom::CoordinateSequence* seq,
                               std::size_t index,
                               const geom::Coordinate& pt);

    const geom::Coordinate& getCoordinate() const;

    std::size_t getIndex() const;

    bool isSameSegment(const CoordinateSequenceLocation& f) const;

    geom::Coordinate getEndPoint(int i) const;

    std::size_t normalize(std::size_t index) const;

private:

    const geom::CoordinateSequence* seq;
    std::size_t index;
    geom::Coordinate pt;

    bool isNext(std::size_t index, std::size_t index1) const;
};

} // namespace distance
} // namespace operation
} // namespace geos
