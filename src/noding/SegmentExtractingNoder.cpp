/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2021 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/CoordinateArraySequence.h>
#include <geos/noding/SegmentString.h>
#include <geos/noding/NodedSegmentString.h>
#include <geos/noding/SegmentExtractingNoder.h>

using geos::geom::Coordinate;
using geos::geom::CoordinateSequence;
using geos::geom::CoordinateArraySequence;
using geos::noding::SegmentString;

namespace geos {
namespace noding {  // geos.noding


/* public */
void
SegmentExtractingNoder::computeNodes(
    std::vector<SegmentString*>* segStrings)
{
    segList = new std::vector<SegmentString*>;
    extractSegments(*segStrings, *segList);
}


/* private */
void
SegmentExtractingNoder::extractSegments(
    const std::vector<SegmentString*>& inputSegs,
    std::vector<SegmentString*>& outputSegs)
{
    for (SegmentString* ss : inputSegs) {
        extractSegments(ss, outputSegs);
    }
    return;
}


/* private */
void
SegmentExtractingNoder::extractSegments(
        const SegmentString* ss,
        std::vector<SegmentString*>& outputSegs)
{
    std::size_t ssSize = ss->size() - 1;
    for (std::size_t i = 0; i < ssSize; i++) {
        std::vector<Coordinate> coords(2);
        coords[0] = ss->getCoordinate(i);
        coords[1] = ss->getCoordinate(i + 1);
        std::unique_ptr<CoordinateSequence> cs(new CoordinateArraySequence(std::move(coords)));
        std::unique_ptr<SegmentString> seg(new NodedSegmentString(cs.release(), ss->getData()));
        outputSegs.push_back(seg.release());
    }
}


std::vector<SegmentString*>*
SegmentExtractingNoder::getNodedSubstrings() const
{
    return segList;
}


} // namespace geos.noding
} // namespace geos
