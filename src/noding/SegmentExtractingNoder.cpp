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
#include <geos/noding/SegmentString.h>
#include <geos/noding/NodedSegmentString.h>
#include <geos/noding/SegmentExtractingNoder.h>

using geos::geom::Coordinate;
using geos::geom::CoordinateSequence;
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
    const CoordinateSequence* ss_seq = ss->getCoordinates();

    const NodedSegmentString* nss = dynamic_cast<const NodedSegmentString*>(ss);
    bool constructZ = nss ? nss->getNodeList().getConstructZ() : ss_seq->hasZ();
    bool constructM = nss ? nss->getNodeList().getConstructM() : ss_seq->hasM();

    for (std::size_t i = 0; i < ss_seq->getSize() - 1; i++) {
        auto cs = detail::make_unique<CoordinateSequence>(0, constructZ, constructM);
        cs->reserve(2);
        cs->add(*ss_seq, i, i + 1);
        std::unique_ptr<SegmentString> seg(new NodedSegmentString(cs.release(), constructZ, constructM, ss->getData()));
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
