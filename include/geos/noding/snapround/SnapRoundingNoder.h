/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2006      Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: noding/snapround/SnapRoundingNoder.java r320 (JTS-1.12)
 *
 **********************************************************************/

#pragma once

#include <geos/export.h>

#include <geos/geom/Coordinate.h>
#include <geos/geom/Envelope.h>
#include <geos/noding/snapround/HotPixelIndex.h>
#include <geos/noding/Noder.h>


// Forward declarations
namespace geos {
namespace geom {
class Envelope;
class PrecisionModel;
}
namespace noding {
class NodedSegmentString;
}
}

namespace geos {
namespace noding { // geos::noding
namespace snapround { // geos::noding::snapround

class GEOS_DLL SnapRoundingNoder : public Noder {

private:

    // Members
    const geom::PrecisionModel* pm;
    noding::snapround::HotPixelIndex pixelIndex;
    std::vector<SegmentString*> snappedResult;

    // Methods
    void snapRound(const std::vector<SegmentString*>& inputSegStrings, std::vector<SegmentString*>& resultNodedSegments);

    static void createNodedStrings(const std::vector<SegmentString*>& segStrings,
        std::vector<SegmentString*>& nodedStrings);

    void addVertexPixels(const std::vector<SegmentString*>& segStrings);

    void round(const geom::Coordinate& pt, geom::Coordinate& ptOut);

    /**
    * Gets a list of the rounded coordinates.
    * Duplicate (collapsed) coordinates are removed.
    *
    * @param pts the coordinates to round
    * @return array of rounded coordinates
    */
    std::unique_ptr<std::vector<geom::Coordinate>> round(const std::vector<geom::Coordinate>& pts);

    /**
    * Computes all interior intersections in the collection of {@link SegmentString}s,
    * and returns their {@link Coordinate}s.
    *
    * Also adds the intersection nodes to the segments.
    *
    * @return a list of Coordinates for the intersections
    */
    std::unique_ptr<std::vector<geom::Coordinate>> findInteriorIntersections(std::vector<SegmentString*>& inputSS);

    /**
    * Computes new segment strings which are rounded and contain
    * any intersections added as a result of snapping segments to snap points (hot pixels).
    *
    * @param segStrings segments to snap
    * @return the snapped segment strings
    */
    void computeSnaps(const std::vector<SegmentString*>& segStrings, std::vector<SegmentString*>& snapped);
    NodedSegmentString* computeSnaps(NodedSegmentString* ss);

    /**
    * Snaps a segment in a segmentString to HotPixels that it intersects.
    *
    * @param p0 the segment start coordinate
    * @param p1 the segment end coordinate
    * @param ss the segment string to add intersections to
    * @param segIndex the index of the segment
    */
    void snapSegment(geom::Coordinate& p0, geom::Coordinate& p1, NodedSegmentString* ss, size_t segIndex);


public:

    SnapRoundingNoder(const geom::PrecisionModel* p_pm)
        : pm(p_pm)
        , pixelIndex(p_pm)
        {}

    /**
    * @return a Collection of NodedSegmentStrings representing the substrings
    */
    std::vector<SegmentString*>* getNodedSubstrings() const override;

    void computeNodes(std::vector<SegmentString*>* inputSegStrings) override; //override

};


} // namespace geos::noding::snapround
} // namespace geos::noding
} // namespace geos



