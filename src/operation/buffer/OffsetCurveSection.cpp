/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (c) 2021 Martin Davis
 * Copyright (C) 2021 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/operation/buffer/OffsetCurveSection.h>
#include <geos/operation/buffer/OffsetCurve.h>

#include <geos/geom/Coordinate.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/LineString.h>

using geos::geom::Geometry;
using geos::geom::Coordinate;
using geos::geom::CoordinateSequence;
using geos::geom::LineString;


namespace geos {      // geos
namespace operation { // geos.operation
namespace buffer {    // geos.operation.buffer


/*public*/
const CoordinateSequence*
OffsetCurveSection::getCoordinates() const
{
    return sectionPts.get();
}

std::unique_ptr<CoordinateSequence>
OffsetCurveSection::releaseCoordinates()
{
    return std::move(sectionPts);
}


/* private */
bool
OffsetCurveSection::isEndInSameSegment(double nextLoc) const
{
    long segIndex = std::lround(std::floor(locLast));
    long nextIndex = std::lround(std::floor(nextLoc));
    // long segIndex = static_cast<long>(locLast);
    // long nextIndex = static_cast<long>(nextLoc);
    return segIndex == nextIndex;
}

bool
OffsetCurveSection::OffsetCurveSectionComparator(
    const std::unique_ptr<OffsetCurveSection>& a,
    const std::unique_ptr<OffsetCurveSection>& b)
{
    if (a->getLocation() < b->getLocation())
        return true;
    else
        return false;
}

/* public static */
std::unique_ptr<Geometry>
OffsetCurveSection::toGeometry(
    std::vector<std::unique_ptr<OffsetCurveSection>>& sections,
    const GeometryFactory* geomFactory)
{
    if (sections.size() == 0)
        return geomFactory->createLineString();
    if (sections.size() == 1) {
        auto cs = sections[0]->releaseCoordinates();
        return geomFactory->createLineString(std::move(cs));
    }

    //-- sort sections in order along the offset curve

    std::sort(sections.begin(), sections.end(), OffsetCurveSectionComparator);
    std::vector<std::unique_ptr<LineString>> lines;
    for (auto& section : sections) {
        auto cs = section->releaseCoordinates();
        auto ls = geomFactory->createLineString(std::move(cs));
        lines.emplace_back(ls.release());
    }
    return geomFactory->createMultiLineString(std::move(lines));
}

/**
* Joins section coordinates into a LineString.
* Join vertices which lie in the same raw curve segment
* are removed, to simplify the result linework.
*
* @param sections the sections to join
* @param geomFactory the geometry factory to use
* @return the simplified linestring for the joined sections
*/
/* public static */
std::unique_ptr<Geometry>
OffsetCurveSection::toLine(
    std::vector<std::unique_ptr<OffsetCurveSection>>& sections,
    const GeometryFactory* geomFactory)
{
    if (sections.size() == 0)
        return geomFactory->createLineString();
    if (sections.size() == 1) {
        auto cs = sections[0]->releaseCoordinates();
        return geomFactory->createLineString(std::move(cs));
    }

    //-- sort sections in order along the offset curve
    std::sort(sections.begin(), sections.end(), OffsetCurveSectionComparator);

    std::unique_ptr<CoordinateSequence> pts(new CoordinateSequence());

    bool removeStartPt = false;
    for (std::size_t i = 0; i < sections.size(); i++) {
        auto& section = sections[i];
        bool removeEndPt = false;
        if (i < sections.size() - 1) {
            double nextStartLoc = sections[i+1]->getLocation();
            removeEndPt = section->isEndInSameSegment(nextStartLoc);
        }
        const CoordinateSequence* secPts = section->getCoordinates();
        for (std::size_t j = 0; j < secPts->size(); j++) {
            if ((removeStartPt && j == 0) || (removeEndPt && j == secPts->size()-1))
                continue;
            pts->add(secPts->getAt(j), false);
        }
        removeStartPt = removeEndPt;
    }
    return geomFactory->createLineString(std::move(pts));
}

/* public static */
std::unique_ptr<OffsetCurveSection>
OffsetCurveSection::create(
    const CoordinateSequence* srcPts,
    std::size_t start, std::size_t end,
    double loc, double locLast)
{
    std::size_t len;
    if (end <= start)
        len = srcPts->size() - start + end;
    else
        len = end - start + 1;

    std::unique_ptr<CoordinateSequence> secPts(new CoordinateSequence());
    for (std::size_t i = 0; i < len; i++) {
        std::size_t index = (start + i) % (srcPts->size() - 1);
        secPts->add(srcPts->getAt(index));
    }
    std::unique_ptr<OffsetCurveSection> ocs(new OffsetCurveSection(std::move(secPts), loc, locLast));
    return ocs;
}



} // namespace geos.operation.buffer
} // namespace geos.operation
} // namespace geos
