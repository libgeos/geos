/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2020 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/geom/CoordinateSequence.h>

#include <geos/export.h>
#include <array>
#include <memory>
#include <vector>

// Forward declarations
namespace geos {
namespace geom {
class Envelope;
class Coordinate;
class CoordinateSequence;
}
}

namespace geos {      // geos.
namespace operation { // geos.operation
namespace overlayng { // geos.operation.overlayng

/**
 * Limits the segments in a list of segments
 * to those which intersect an envelope.
 * This creates zero or more sections of the input segment sequences,
 * containing only line segments which intersect the limit envelope.
 * Segments are not clipped, since that can move
 * line segments enough to alter topology,
 * and it happens in the overlay in any case.
 * This can substantially reduce the number of vertices which need to be
 * processed during overlay.
 *
 * This optimization is only applicable to Line geometries,
 * since it does not maintain the closed topology of rings.
 * Polygonal geometries are optimized using the {@link RingClipper}.
 *
 * @author Martin Davis
 */
class GEOS_DLL LineLimiter {
    using Envelope = geos::geom::Envelope;
    using Coordinate = geos::geom::Coordinate;
    using CoordinateSequence = geos::geom::CoordinateSequence;

private:

    // Members
    const Envelope* limitEnv;
    std::unique_ptr<geom::CoordinateSequence> ptList;
    const Coordinate* lastOutside;
    std::vector<std::unique_ptr<CoordinateSequence>> sections;

    // Methods
    void addPoint(const Coordinate* p);
    void addOutside(const Coordinate* p);
    bool isLastSegmentIntersecting(const Coordinate* p);
    bool isSectionOpen();
    void startSection();
    void finishSection();


public:

    LineLimiter(const Envelope* env)
        : limitEnv(env)
        , ptList(nullptr)
        , lastOutside(nullptr)
        {};

    std::vector<std::unique_ptr<CoordinateSequence>>& limit(const CoordinateSequence *pts);

};


} // namespace geos.operation.overlayng
} // namespace geos.operation
} // namespace geos

