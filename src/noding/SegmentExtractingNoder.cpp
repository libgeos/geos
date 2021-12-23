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
#include <geos/noding/BasicSegmentString.h>
#include <geos/noding/SegmentExtractingNoder.h>

using geos::geom::Coordinate;
using geos::geom::CoordinateSequence;
using geos::geom::CoordinateArraySequence;
using geos::noding::SegmentString;

namespace geos {
namespace noding {  // geos.noding


/* public */
void
SegmentExtractingNoder::computeNodes(std::vector<SegmentString*>* segStrings) {
    segList = extractSegments(segStrings);
}


/* private static */
std::vector<SegmentString*>*
SegmentExtractingNoder::extractSegments(std::vector<SegmentString*>* segStrings)
{
    std::unique_ptr<std::vector<SegmentString*>> segList(new std::vector<SegmentString*>());
    for (SegmentString* ss : *segStrings) {
        extractSegments(ss, segList.get());
    }
    return segList.release();
}


/* private static  */
void
SegmentExtractingNoder::extractSegments(SegmentString* ss, std::vector<SegmentString*>* segList)
{
    for (std::size_t i = 0; i < ss->size() - 1; i++) {
        std::vector<Coordinate> coords(2);
        coords[0] = ss->getCoordinate(i);
        coords[1] = ss->getCoordinate(i + 1);
        std::unique_ptr<CoordinateSequence> cs(new CoordinateArraySequence(std::move(coords)));
        std::unique_ptr<SegmentString> seg(new BasicSegmentString(cs.release(), nullptr));
        segList->push_back(seg.release());
    }
}


std::vector<SegmentString*>*
SegmentExtractingNoder::getNodedSubstrings() const
{
    return segList;
}


} // namespace geos.noding
} // namespace geos
