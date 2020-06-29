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

#include <geos/operation/overlayng/OverlayNoder.h>

using geos::operation::valid::RepeatedPointRemover;

namespace geos {      // geos
namespace operation { // geos.operation
namespace overlayng { // geos.operation.overlayng

/*private*/
Noder*
OverlayNoder::getNoder()
{
    if (customNoder != nullptr) {
        return customNoder;
    }

    if (OverlayUtil::isFloating(pm)) {
        internalNoder = createFloatingPrecisionNoder(IS_NODING_VALIDATED);
    }
    else {
        internalNoder = createFixedPrecisionNoder(pm);
    }
    return internalNoder.get();
}

/*private*/
std::unique_ptr<Noder>
OverlayNoder::createFixedPrecisionNoder(const PrecisionModel* pm)
{
    std::unique_ptr<SnapRoundingNoder> srNoder(new SnapRoundingNoder(pm));
    return srNoder;
}

/*private*/
std::unique_ptr<Noder>
OverlayNoder::createFloatingPrecisionNoder(bool doValidation)
{
    std::unique_ptr<MCIndexNoder> mcNoder(new MCIndexNoder());
    mcNoder->setSegmentIntersector(&intAdder);

    if (doValidation) {
        spareInternalNoder = std::move(mcNoder);
        Noder* noder = spareInternalNoder.get();
        std::unique_ptr<ValidatingNoder> validNoder(new ValidatingNoder(*noder));
        return validNoder;
    }

    return mcNoder;
}

/*public*/
void
OverlayNoder::setNoder(Noder* noder)
{
    customNoder = noder;
}

/*public*/
void
OverlayNoder::setClipEnvelope(const Envelope* p_clipEnv)
{
    clipEnv = p_clipEnv;
    clipper.reset(new RingClipper(p_clipEnv));
    limiter.reset(new LineLimiter(p_clipEnv));
}

/*public*/
std::vector<SegmentString*>*
OverlayNoder::node()
{
    Noder* noder = getNoder();
    noder->computeNodes(segStrings);

    std::vector<SegmentString*>* nodedSS = noder->getNodedSubstrings();
    scanForEdges(nodedSS);

    return nodedSS;
}

/*private*/
void
OverlayNoder::scanForEdges(std::vector<SegmentString*>* segStrings)
{
    for (SegmentString* ss : *segStrings) {
        const EdgeSourceInfo* info = static_cast<const EdgeSourceInfo*>(ss->getData());
        int geomIndex = info->getIndex();
        if (geomIndex == 0) {
            hasEdgesA = true;
        }
        else if (geomIndex == 1) {
            hasEdgesB = true;
        }
        // short-circuit if both have been found
        if (hasEdgesA && hasEdgesB) return;
    }
}

/*public*/
bool
OverlayNoder::hasEdgesFor(int geomIndex )
{
    if (geomIndex == 0) return hasEdgesA;
    return hasEdgesB;
}

/*public*/
void
OverlayNoder::add(const Geometry* g, int geomIndex)
{
    if (g == nullptr || g->isEmpty())
        return;

    if (isClippedCompletely(g->getEnvelopeInternal()))
        return;

    switch (g->getGeometryTypeId())
    {
        case GEOS_POLYGON:
            return addPolygon(static_cast<const Polygon*>(g), geomIndex);
        case GEOS_LINESTRING:
            return addLine(static_cast<const LineString*>(g), geomIndex);
        case GEOS_MULTILINESTRING:
        case GEOS_MULTIPOLYGON:
        case GEOS_GEOMETRYCOLLECTION:
            return addCollection(static_cast<const GeometryCollection*>(g), geomIndex);
        case GEOS_POINT:
        case GEOS_LINEARRING:
        case GEOS_MULTIPOINT:
            return; // do nothing
        default:
            return; // do nothing
    }
}

/*private*/
void
OverlayNoder::addCollection(const GeometryCollection* gc, int geomIndex)
{
    for (std::size_t i = 0; i < gc->getNumGeometries(); i++) {
        const Geometry* g = gc->getGeometryN(i);
        add(g, geomIndex);
    }
}

/*private*/
void
OverlayNoder::addPolygon(const Polygon* poly, int geomIndex)
{
    const LinearRing* shell = poly->getExteriorRing();
    addPolygonRing(shell, false, geomIndex);

    for (std::size_t i = 0; i < poly->getNumInteriorRing(); i++) {
        const LinearRing* hole = poly->getInteriorRingN(i);

        // Holes are topologically labelled opposite to the shell, since
        // the interior of the polygon lies on their opposite side
        // (on the left, if the hole is oriented CW)
        addPolygonRing(hole, true, geomIndex);
    }
}

/*private*/
void
OverlayNoder::addPolygonRing(const LinearRing* ring, bool isHole, int index)
  {
    // don't add empty rings
    if (ring->isEmpty()) return;

    if (isClippedCompletely(ring->getEnvelopeInternal()))
      return;

    std::unique_ptr<geom::CoordinateArraySequence> pts = clip(ring);

    /**
    * Don't add edges that collapse to a point
    */
    if (pts->size() < 2) {
        return;
    }

    int depthDelta = computeDepthDelta(ring, isHole);
    addEdge(pts, createEdgeSourceInfo(index, depthDelta, isHole));
}

/*private*/
const EdgeSourceInfo*
OverlayNoder::createEdgeSourceInfo(int index)
{
    // Concentrate small memory allocations via std::deque and
    // retain ownership of the EdgeSourceInfo* in the OverlayNoder
    edgeSourceInfoQue.emplace_back(index);
    return &(edgeSourceInfoQue.back());
}

/*private*/
const EdgeSourceInfo*
OverlayNoder::createEdgeSourceInfo(int index, int depthDelta, bool isHole)
{
    // Concentrate small memory allocations via std::deque and
    // retain ownership of the EdgeSourceInfo* in the OverlayNoder
    edgeSourceInfoQue.emplace_back(index, depthDelta, isHole);
    return &(edgeSourceInfoQue.back());
}

/*private*/
void
OverlayNoder::addEdge(std::unique_ptr<CoordinateArraySequence>& cas, const EdgeSourceInfo* info)
{
    // TODO: manage these internally to OverlayNoder in a std::deque,
    // since they do not have a life span longer than the OverlayNoder
    // in OverlayNG::buildGraph()
    NodedSegmentString* ss = new NodedSegmentString(cas.release(), reinterpret_cast<const void*>(info));
    segStrings->push_back(ss);
}

/*private*/
void
OverlayNoder::addEdge(std::unique_ptr<std::vector<Coordinate>> pts, const EdgeSourceInfo* info)
{
    CoordinateArraySequence* cas = new CoordinateArraySequence(pts.release());
    NodedSegmentString* ss = new NodedSegmentString(cas, reinterpret_cast<const void*>(info));
    segStrings->push_back(ss);
}

/*private*/
bool
OverlayNoder::isClippedCompletely(const Envelope* env)
{
    if (clipEnv == nullptr) return false;
    return clipEnv->disjoint(env);
}

/* private */
std::unique_ptr<geom::CoordinateArraySequence>
OverlayNoder::clip(const LinearRing* ring)
{
    const Envelope* env = ring->getEnvelopeInternal();

    /**
     * If no clipper or ring is completely contained then no need to clip.
     * But repeated points must be removed to ensure correct noding.
     */
    if (clipper == nullptr || clipEnv->covers(env)) {
        return removeRepeatedPoints(ring);
    }

    return clipper->clip(ring->getCoordinatesRO());
}

/*private*/
std::unique_ptr<CoordinateArraySequence>
OverlayNoder::removeRepeatedPoints(const LineString* line)
{
    const CoordinateSequence* pts = line->getCoordinatesRO();
    return RepeatedPointRemover::removeRepeatedPoints(pts);
}

/*private*/
int
OverlayNoder::computeDepthDelta(const LinearRing* ring, bool isHole)
{
    /**
     * Compute the orientation of the ring, to
     * allow assigning side interior/exterior labels correctly.
     * JTS canonical orientation is that shells are CW, holes are CCW.
     *
     * It is important to compute orientation on the original ring,
     * since topology collapse can make the orientation computation give the wrong answer.
     */
    bool isCCW = algorithm::Orientation::isCCW(ring->getCoordinatesRO());

    /**
     * Compute whether ring is in canonical orientation or not.
     * Canonical orientation for the overlay process is
     * Shells : CW, Holes: CCW
     */
    bool isOriented = true;
    if (!isHole) {
      isOriented = !isCCW;
    }
    else {
      isOriented = isCCW;
    }
    /**
     * Depth delta can now be computed.
     * Canonical depth delta is 1 (Exterior on L, Interior on R).
     * It is flipped to -1 if the ring is oppositely oriented.
     */
    int depthDelta = isOriented ? 1 : -1;
    return depthDelta;
}

/*private*/
void
OverlayNoder::addLine(const LineString* line, int geomIndex)
{
    // don't add empty lines
    if (line->isEmpty()) return;

    if (isClippedCompletely(line->getEnvelopeInternal()))
        return;

    if (isToBeLimited(line)) {
        std::vector<std::unique_ptr<CoordinateArraySequence>>& sections = limit(line);
        for (auto& pts : sections) {
            addLine(pts, geomIndex);
        }
    }
    else {
        std::unique_ptr<CoordinateArraySequence> ptsNoRepeat = removeRepeatedPoints(line);
        addLine(ptsNoRepeat, geomIndex);
    }
}

/*private*/
void
OverlayNoder::addLine(std::unique_ptr<CoordinateArraySequence>& pts, int geomIndex)
{
    /**
     * Don't add edges that collapse to a point
     */
    if (pts->size() < 2) {
        return;
    }

    addEdge(pts, createEdgeSourceInfo(geomIndex));
}

/*private*/
bool
OverlayNoder::isToBeLimited(const LineString* line)
{
    const CoordinateSequence* pts = line->getCoordinatesRO();
    if (limiter == nullptr || pts->size() <= MIN_LIMIT_PTS) {
        return false;
    }
    const Envelope* env = line->getEnvelopeInternal();
    /**
     * If line is completely contained then no need to limit
     */
    if (clipEnv->covers(env)) {
        return false;
    }
    return true;
}

/*private*/
std::vector<std::unique_ptr<CoordinateArraySequence>>&
OverlayNoder::limit(const LineString* line)
{
    const CoordinateSequence* pts = line->getCoordinatesRO();
    return limiter->limit(pts);
}



} // namespace geos.operation.overlayng
} // namespace geos.operation
} // namespace geos
