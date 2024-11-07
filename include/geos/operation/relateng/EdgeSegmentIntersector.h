/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (c) 2024 Martin Davis
 * Copyright (C) 2024 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once


#include <geos/noding/SegmentIntersector.h>
#include <geos/algorithm/LineIntersector.h>
#include <geos/export.h>


// Forward declarations
namespace geos {
namespace noding {
    class SegmentString;
}
namespace operation {
namespace relateng {
    class RelateSegmentString;
    class TopologyComputer;
}
}
}

namespace geos {      // geos.
namespace operation { // geos.operation
namespace relateng { // geos.operation.relateng

class GEOS_DLL EdgeSegmentIntersector : public geos::noding::SegmentIntersector {
    using SegmentString = geos::noding::SegmentString;
    using LineIntersector = geos::algorithm::LineIntersector;

private:

    // Members
    LineIntersector li;
    TopologyComputer& topoComputer;

    // Methods


    void addIntersections(
        RelateSegmentString* ssA, std::size_t segIndexA,
        RelateSegmentString* ssB, std::size_t segIndexB);


public:

    EdgeSegmentIntersector(TopologyComputer& p_topoComputer)
        : topoComputer(p_topoComputer)
        {};

    void processIntersections(
        SegmentString* ss0, std::size_t segIndex0,
        SegmentString* ss1, std::size_t segIndex1) override;

    bool isDone() const override;

};

} // namespace geos.operation.relateng
} // namespace geos.operation
} // namespace geos

