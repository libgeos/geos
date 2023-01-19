/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2021 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/export.h>

#include <geos/geom/Coordinate.h>
#include <geos/geom/Envelope.h>
#include <geos/noding/Noder.h>
#include <geos/noding/snap/SnappingPointIndex.h>


// Forward declarations
namespace geos {
namespace geom {
class Envelope;
class PrecisionModel;
}
namespace noding {
class SegmentString;
}
}

namespace geos {
namespace noding { // geos::noding

/**
 * A noder which extracts all line segments
 * as SegmentString.
 * This enables fast overlay of geometries which are known to be already fully noded.
 * In particular, it provides fast union of polygonal and linear coverages.
 * Unioning a noded set of lines is an effective way
 * to perform line merging and line dissolving.
 *
 * No precision reduction is carried out.
 * If that is required, another noder must be used (such as a snap-rounding noder),
 * or the input must be precision-reduced beforehand.
 *
 */
class GEOS_DLL SegmentExtractingNoder : public Noder {

private:

    std::vector<SegmentString*>* segList;

    void extractSegments(
        const std::vector<SegmentString*>& inputSegs,
        std::vector<SegmentString*>& outputSegs);

    void extractSegments(
        const SegmentString* ss,
        std::vector<SegmentString*>& outputSegs);


public:

    /**
    * Creates a new segment-extracting noder.
    */
    SegmentExtractingNoder() : segList(nullptr)
        {};

    void computeNodes(std::vector<SegmentString*>* segStrings) override;

    /**
    * @return a Collection of SegmentString representing the
    * substrings. Caller takes ownership over vector and contents.
    */
    std::vector<SegmentString*>* getNodedSubstrings() const override;


};


} // namespace geos::noding
} // namespace geos
