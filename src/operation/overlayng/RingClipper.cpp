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

#include <geos/operation/overlayng/RingClipper.h>
#include <geos/operation/valid/RepeatedPointRemover.h>
#include <geos/util.h>


namespace geos {      // geos
namespace operation { // geos.operation
namespace overlayng { // geos.operation.overlayng


/*public*/
std::unique_ptr<CoordinateArraySequence>
RingClipper::clip(const CoordinateSequence* cs) const
{
    std::vector<Coordinate> pts;
    pts.reserve(cs->size());
    cs->toVector(pts);
    for (int edgeIndex = 0; edgeIndex < 4; edgeIndex++) {
        bool closeRing = (edgeIndex == 3);
        if (!pts.empty()) {
            pts = clipToBoxEdge(pts, edgeIndex, closeRing);
        }
    }

    auto ret = detail::make_unique<CoordinateArraySequence>(std::move(pts));
    if (ret->hasRepeatedPoints()) {
        return operation::valid::RepeatedPointRemover::removeRepeatedPoints(ret.get());
    } else {
        return ret;
    }
}

/*private*/
std::vector<Coordinate>
RingClipper::clipToBoxEdge(const std::vector<Coordinate> & pts, int edgeIndex, bool closeRing) const
{
    // TODO: is it possible to avoid copying array 4 times?
    std::vector<Coordinate> ptsClip;
    ptsClip.reserve(pts.size());

    const Coordinate* p0 = &pts.back();
    for (std::size_t i = 0; i < pts.size(); i++) {
        const Coordinate* p1 = &pts[i];
        if (isInsideEdge(*p1, edgeIndex)) {
            if (!isInsideEdge(*p0, edgeIndex)) {
                ptsClip.emplace_back();
                intersection(*p0, *p1, edgeIndex, ptsClip.back());
            }
            // TODO: avoid copying so much?
            ptsClip.push_back(*p1);
        }
        else if (isInsideEdge(*p0, edgeIndex)) {
            ptsClip.emplace_back();
            intersection(*p0, *p1, edgeIndex, ptsClip.back());
        }

        // else p0-p1 is outside box, so it is dropped
        p0 = p1;
    }

    // add closing point if required
    if (closeRing && ptsClip.size() > 0) {
        if (!ptsClip.front().equals2D(ptsClip.back())) {
            ptsClip.push_back(ptsClip.front());
        }
    }
    return ptsClip;
}



} // namespace geos.operation.overlayng
} // namespace geos.operation
} // namespace geos
