/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2022 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/coverage/CoverageRing.h>

#include <geos/algorithm/Orientation.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/LineString.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/util/PolygonExtracter.h>
#include <geos/util/IllegalStateException.h>

using geos::geom::Coordinate;
using geos::geom::CoordinateSequence;
using geos::geom::Geometry;
using geos::geom::GeometryFactory;
using geos::geom::LineString;
using geos::geom::LinearRing;
using geos::geom::Polygon;


namespace geos {     // geos
namespace coverage { // geos.coverage


/* public static */
bool
CoverageRing::isKnown(std::vector<CoverageRing*>& rings)
{
    for (auto* ring : rings) {
        if (! ring->isKnown())
            return false;
    }
    return true;
}


/* public */
CoverageRing::CoverageRing(CoordinateSequence* inPts, bool interiorOnRight)
    : noding::BasicSegmentString(inPts, nullptr)
    , m_isInteriorOnRight(interiorOnRight)
{
    m_isInvalid.resize(size() - 1, false);
    m_isMatched.resize(size() - 1, false);
}


/* public */
CoverageRing::CoverageRing(const LinearRing* ring, bool isShell)
    : CoverageRing(
        // This is bad. The ownership rules of SegmentStrings need
        // to be carefully considered. Most noders don't even touch
        // them so a const CoordinateSequence makes sense. Some add
        // things, like the NodedSegmentString, but do so out-of-line.
        // Some noders (just ScalingNoder?) completely transform the
        // inputs. Could maybe do bulk copying for that case?
        const_cast<CoordinateSequence*>(ring->getCoordinatesRO()),
        algorithm::Orientation::isCCW(ring->getCoordinatesRO()) != isShell)
{}


/* public */
bool
CoverageRing::isInteriorOnRight() const
{
    return m_isInteriorOnRight;
}


/* public */
void
CoverageRing::markInvalid(std::size_t index)
{
    m_isInvalid[index] = true;
}


/* public */
void
CoverageRing::markMatched(std::size_t index)
{
    m_isMatched[index] = true;
}


/* public */
bool
CoverageRing::isKnown() const
{
    for (size_t i = 0; i < m_isMatched.size(); i++ ) {
        if (!(m_isMatched[i] && m_isInvalid[i]))
            return false;
    }
    return true;
}

/* public */
bool
CoverageRing::isInvalid(std::size_t i) const
{
    return m_isInvalid[i];
}

/* public */
bool
CoverageRing::isInvalid() const
{
    for (bool b: m_isInvalid) {
        if (!b)
            return false;
    }
    return true;
}


/* public */
bool
CoverageRing::hasInvalid() const
{
    for (bool b: m_isInvalid) {
        if (b)
            return true;
    }
    return false;
}


/* public */
bool
CoverageRing::isKnown(std::size_t i) const
{
    return m_isMatched[i] || m_isInvalid[i];
}


/* public */
const Coordinate&
CoverageRing::findVertexPrev(std::size_t index, const Coordinate& pt) const
{
    std::size_t iPrev = index;
    const Coordinate* cPrev = &getCoordinate(iPrev);
    while (pt.equals2D(*cPrev)) {
        iPrev = prev(iPrev);
        cPrev = &getCoordinate(iPrev);
    }
    return *cPrev;
}


/* public */
const Coordinate&
CoverageRing::findVertexNext(std::size_t index, const Coordinate& pt) const
{
    //-- safe, since index is always the start of a segment
    std::size_t iNext = index + 1;
    const Coordinate* cNext = &getCoordinate(iNext);
    while (pt.equals2D(*cNext)) {
        iNext = next(iNext);
        cNext = &getCoordinate(iNext);
    }
    return *cNext;
}


/* public */
std::size_t
CoverageRing::prev(std::size_t index) const
{
    if (index == 0)
        return size() - 2;
    return index - 1;
}


/* public */
std::size_t
CoverageRing::next(std::size_t index) const
{
    if (index < size() - 2)
        return index + 1;
    return 0;
}


/* public */
void
CoverageRing::createInvalidLines(
    const GeometryFactory* geomFactory,
    std::vector<std::unique_ptr<LineString>>& lines)
{
    //-- empty case
    if (! hasInvalid()) {
        return;
    }
    //-- entire ring case
    if (isInvalid()) {
        std::unique_ptr<LineString> line = createLine(0, size() - 1, geomFactory);
        lines.push_back(std::move(line));
        return;
    }

    //-- find first end after index 0, to allow wrap-around
    std::size_t startIndex = findInvalidStart(0);
    std::size_t firstEndIndex = findInvalidEnd(startIndex);
    std::size_t endIndex = firstEndIndex;
    while (true) {
        startIndex = findInvalidStart(endIndex);
        endIndex = findInvalidEnd(startIndex);
        std::unique_ptr<LineString> line = createLine(startIndex, endIndex, geomFactory);
        lines.push_back(std::move(line));
        if (endIndex == firstEndIndex)
            break;
    }
}


/* private */
std::size_t
CoverageRing::findInvalidStart(std::size_t index)
{
    while (! isInvalid(index)) {
        index = nextMarkIndex(index);
    }
    return index;
}


/* private */
std::size_t
CoverageRing::findInvalidEnd(std::size_t index)
{
    index = nextMarkIndex(index);
    while (isInvalid(index)) {
        index = nextMarkIndex(index);
    }
    return index;
}


/* private */
std::size_t
CoverageRing::nextMarkIndex(std::size_t index)
{
    if (index >= m_isInvalid.size() - 1) {
        return 0;
    }
    return index + 1;
}


/* private */
std::unique_ptr<LineString>
CoverageRing::createLine(
    std::size_t startIndex,
    std::size_t endIndex,
    const GeometryFactory* geomFactory)
{
    std::unique_ptr<CoordinateSequence> linePts = endIndex < startIndex
        ? extractSectionWrap(startIndex, endIndex)
        : extractSection(startIndex, endIndex);
    return geomFactory->createLineString(std::move(linePts));
}


/* private */
std::unique_ptr<CoordinateSequence>
CoverageRing::extractSection(std::size_t startIndex, std::size_t endIndex)
{
    // std::size_t sz = endIndex - startIndex + 1;
    std::unique_ptr<CoordinateSequence> linePts(new CoordinateSequence());
    for (std::size_t i = startIndex; i <= endIndex; i++) {
        linePts->add(getCoordinate(i));
    }

    return linePts;
}


/* private */
std::unique_ptr<CoordinateSequence>
CoverageRing::extractSectionWrap(std::size_t startIndex, std::size_t endIndex)
{
    std::size_t sz = endIndex + (size() - startIndex);
    std::unique_ptr<CoordinateSequence> linePts(new CoordinateSequence);
    std::size_t index = startIndex;
    for (std::size_t i = 0; i < sz; i++) {
        linePts->add(getCoordinate(index));
        index = nextMarkIndex(index);
    }

    return linePts;
}


} // namespace geos.coverage
} // namespace geos


