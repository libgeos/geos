/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (c) 2025 Martin Davis
 * Copyright (C) 2025 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/coverage/CoverageCleaner.h>

#include <geos/algorithm/construct/MaximumInscribedCircle.h>
#include <geos/algorithm/locate/SimplePointInAreaLocator.h>
#include <geos/dissolve/LineDissolver.h>
#include <geos/geom/Envelope.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/MultiPolygon.h>
#include <geos/geom/Point.h>
#include <geos/geom/Polygon.h>
#include <geos/noding/Noder.h>
#include <geos/noding/SegmentStringUtil.h>
#include <geos/noding/snap/SnappingNoder.h>
#include <geos/operation/polygonize/Polygonizer.h>
#include <geos/operation/relateng/RelateNG.h>
#include <geos/util/IllegalArgumentException.h>


using geos::algorithm::construct::MaximumInscribedCircle;
using geos::algorithm::locate::SimplePointInAreaLocator;
using geos::dissolve::LineDissolver;
using geos::geom::Envelope;
using geos::geom::Geometry;
using geos::geom::GeometryFactory;
using geos::geom::MultiPolygon;
using geos::geom::Point;
using geos::geom::Polygon;
using geos::noding::Noder;
using geos::noding::SegmentStringUtil;
using geos::noding::snap::SnappingNoder;
using geos::operation::polygonize::Polygonizer;
using geos::operation::relateng::RelateNG;


namespace geos {     // geos
namespace coverage { // geos.coverage


/* public static */
std::vector<std::unique_ptr<Geometry>>
CoverageCleaner::clean(std::vector<const Geometry*>& p_coverage,
    double p_snappingDistance,
    int p_overlapMergeStrategy,
    double p_maxGapWidth)
{
    CoverageCleaner cc(p_coverage);
    cc.setSnappingDistance(p_snappingDistance);
    cc.setGapMaximumWidth(p_maxGapWidth);
    cc.setOverlapMergeStrategy(p_overlapMergeStrategy);
    cc.clean();
    return cc.getResult();
}


/* public static */
std::vector<std::unique_ptr<Geometry>>
CoverageCleaner::clean(std::vector<const Geometry*>& p_coverage,
    double p_snappingDistance,
    double p_maxGapWidth)
{
    CoverageCleaner cc(p_coverage);
    cc.setSnappingDistance(p_snappingDistance);
    cc.setGapMaximumWidth(p_maxGapWidth);
    cc.clean();
    return cc.getResult();
}


/* public static */
std::vector<std::unique_ptr<Geometry>>
CoverageCleaner::cleanOverlapGap(std::vector<const Geometry*>& p_coverage,
      int p_overlapMergeStrategy,
      double p_maxGapWidth)
{
    return clean(p_coverage, -1, p_overlapMergeStrategy, p_maxGapWidth);
}


/* public static */
std::vector<std::unique_ptr<Geometry>>
CoverageCleaner::cleanGapWidth(std::vector<const Geometry*>& p_coverage,
    double p_maxGapWidth)
{
    return clean(p_coverage, -1, p_maxGapWidth);
}


/* public */
CoverageCleaner::CoverageCleaner(std::vector<const Geometry*>& p_coverage)
    : coverage(p_coverage)
    , geomFactory(p_coverage.empty() ? nullptr : coverage[0]->getFactory())
    , snappingDistance(computeDefaultSnappingDistance(p_coverage))
{}


/* public */
void
CoverageCleaner::setSnappingDistance(double p_snappingDistance)
{
    //-- use default distance if invalid argument
    if (p_snappingDistance < 0)
        return;
    snappingDistance = p_snappingDistance;
}


/* public */
void
CoverageCleaner::setOverlapMergeStrategy(int mergeStrategy)
{
    if (mergeStrategy < MERGE_LONGEST_BORDER ||
        mergeStrategy > MERGE_MIN_INDEX)
        throw util::IllegalArgumentException("Invalid merge strategy code");

    overlapMergeStrategy = mergeStrategy;
}


/* public */
void
CoverageCleaner::setGapMaximumWidth(double maxWidth)
{
    if (maxWidth < 0)
        return;
    gapMaximumWidth = maxWidth;
}


/* public */
void
CoverageCleaner::clean()
{
    computeResultants(snappingDistance);
    //System.out.format("Overlaps: %d    Gaps: %d\n", overlaps.size(), mergableGaps.size());

    //Stopwatch sw = new Stopwatch();
    mergeOverlaps(overlapParentMap);
    //System.out.println("Merge Overlaps: " + sw.getTimeString());
    //sw.reset();
    cleanCov->mergeGaps(mergableGaps);
    //System.out.println("Merge Gaps: " + sw.getTimeString());
}


/* public */
std::vector<std::unique_ptr<Geometry>>
CoverageCleaner::getResult()
{
    return cleanCov->toCoverage(geomFactory);
}


/* public */
std::vector<const Polygon*>
CoverageCleaner::getOverlaps()
{
    return overlaps;
}


/* public */
std::vector<const Polygon*>
CoverageCleaner::getMergedGaps()
{
    return mergableGaps;
}

//-------------------------------------------------

/* private static */
double
CoverageCleaner::computeDefaultSnappingDistance(std::vector<const Geometry*>& geoms)
{
    double diameter = extent(geoms).getDiameter();
    return diameter / DEFAULT_SNAPPING_FACTOR;
}


/* private static */
Envelope
CoverageCleaner::extent(std::vector<const Geometry*>& geoms)
{
    Envelope env;
    for (const Geometry* geom : geoms) {
        env.expandToInclude(geom->getEnvelopeInternal());
    }
    return env;
}


/* private */
void
CoverageCleaner::mergeOverlaps(
    std::map<std::size_t, std::vector<std::size_t>>& overlapParentMap_p)
{
    for (const auto& [resIndex, _] : overlapParentMap_p) {
        auto ms = mergeStrategy(overlapMergeStrategy);
        cleanCov->mergeOverlap(
            resultants[resIndex].get(),
            *ms,
            overlapParentMap_p[resIndex]);
    }
}


/* private */
std::unique_ptr<CleanCoverage::MergeStrategy>
CoverageCleaner::mergeStrategy(int mergeStrategyId)
{
    switch (mergeStrategyId) {
        case MERGE_LONGEST_BORDER:
            return std::make_unique<CleanCoverage::BorderMergeStrategy>();
        case MERGE_MAX_AREA:
            return std::make_unique<CleanCoverage::AreaMergeStrategy>(true);
        case MERGE_MIN_AREA:
            return std::make_unique<CleanCoverage::AreaMergeStrategy>(false);
        case MERGE_MIN_INDEX:
            return std::make_unique<CleanCoverage::IndexMergeStrategy>(false);
    }
    throw util::IllegalArgumentException("CoverageCleaner::mergeStrategy - Unknown merge strategy");
}


/* private */
void
CoverageCleaner::computeResultants(double tolerance)
{
    //System.out.println("Coverage Cleaner ===> polygons: " + coverage.length);
    //System.out.format("Snapping distance: %f\n", snappingDistance);
    //Stopwatch sw = new Stopwatch();
    //sw.start();

    std::unique_ptr<Geometry> nodedEdges = node(coverage, tolerance);
    //System.out.println("Noding: " + sw.getTimeString());

    //sw.reset();
    std::unique_ptr<Geometry> cleanEdges = LineDissolver::dissolve(nodedEdges.get());
    //System.out.println("Dissolve: " + sw.getTimeString());

    //sw.reset();
    resultants = polygonize(cleanEdges.get());
    //System.out.println("Polygonize: " + sw.getTimeString());

    cleanCov = std::make_unique<CleanCoverage>(coverage.size());

    //sw.reset();
    createCoverageIndex();
    classifyResult(resultants);
    //System.out.println("Classify: " + sw.getTimeString());

    mergableGaps = findMergableGaps(gaps);
 }


/* private */
void
CoverageCleaner::createCoverageIndex()
{
    covIndex = std::make_unique<index::strtree::TemplateSTRtree<std::size_t>>();
    for (std::size_t i = 0; i < coverage.size(); i++) {
        covIndex->insert(*(coverage[i]->getEnvelopeInternal()), i);
    }
}


/* private */
void
CoverageCleaner::classifyResult(std::vector<std::unique_ptr<Polygon>>& rs)
{
    for (std::size_t i = 0; i < rs.size(); i++) {
        classifyResultant(i, rs[i].get());
    }
}


/* private */
void
CoverageCleaner::classifyResultant(std::size_t resultIndex, const Polygon* resPoly)
{
    std::unique_ptr<Point> intPt = resPoly->getInteriorPoint();
    std::size_t parentIndex = INDEX_UNKNOWN;
    std::vector<std::size_t> overlapIndexes;

    std::vector<std::size_t> candidateParentIndex;
    covIndex->query(*(intPt->getEnvelopeInternal()), candidateParentIndex);

    for (std::size_t i : candidateParentIndex) {
        const Geometry* parent = coverage[i];
        if (covers(parent, intPt.get())) {
            //-- found first parent
            if (parentIndex == INDEX_UNKNOWN) {
                parentIndex = i;
            }
            else {
                //-- more than one parent - record them all
                overlapIndexes.push_back(parentIndex);
                overlapIndexes.push_back(i);
            }
        }
    }
    /**
     * Classify resultant based on # of parents:
     * 0 - gap
     * 1 - single polygon face
     * >1 - overlap
     */
    if (parentIndex == INDEX_UNKNOWN) {
        gaps.push_back(resPoly);
    }
    else if (!overlapIndexes.empty()) {
        overlapParentMap[resultIndex] = overlapIndexes;
        overlaps.push_back(resPoly);
    }
    else {
        cleanCov->add(parentIndex, resPoly);
    }
}


/* private static */
bool
CoverageCleaner::covers(const Geometry* poly, const Point* intPt)
{
    return SimplePointInAreaLocator::isContained(
        *(intPt->getCoordinate()),
        poly);
}


/* private */
std::vector<const Polygon*>
CoverageCleaner::findMergableGaps(std::vector<const Polygon*> p_gaps)
{
    std::vector<const Polygon*> filtered;

    std::copy_if(p_gaps.begin(), p_gaps.end(),
                 std::back_inserter(filtered),
                 [this](const Polygon* gap) { return isMergableGap(gap); }
                 );

    return filtered;

    // return gaps.stream().filter(gap -> isMergableGap(gap)).collect(Collectors.toList());
}


/* private */
bool
CoverageCleaner::isMergableGap(const Polygon* gap)
{
    if (gapMaximumWidth <= 0) {
        return false;
    }
    return MaximumInscribedCircle::isRadiusWithin(gap, gapMaximumWidth / 2.0);
}


/* private static */
std::vector<std::unique_ptr<geom::Polygon>>
CoverageCleaner::polygonize(const Geometry* cleanEdges)
{
    Polygonizer polygonizer;
    polygonizer.add(cleanEdges);
    return polygonizer.getPolygons();
}


/* public static */
std::unique_ptr<Geometry>
CoverageCleaner::toGeometry(
    std::vector<SegmentString*>& segStrings,
    const GeometryFactory* geomFact)
{
    std::vector<std::unique_ptr<LineString>> lines;
    for (SegmentString* ss : segStrings) {
        auto cs = ss->getCoordinates()->clone();
        std::unique_ptr<LineString> line = geomFact->createLineString(std::move(cs));
        lines.emplace_back(line.release());
    }
    if (lines.size() == 1) return lines[0]->clone();
    return geomFact->createMultiLineString(std::move(lines));
}


/* public static */
std::unique_ptr<Geometry>
CoverageCleaner::node(std::vector<const Geometry*>& p_coverage, double p_snapDistance)
{
    std::vector<const SegmentString*> csegs;

    for (const Geometry* geom : p_coverage) {
        //-- skip non-polygonal and empty elements
        if (! isPolygonal(geom))
            continue;
        if (geom->isEmpty())
            continue;
        SegmentStringUtil::extractSegmentStrings(geom, csegs);
    }

    std::vector<SegmentString*> segs;
    for (auto* css : csegs) {
        segs.push_back(const_cast<SegmentString*>(css));
    }

    SnappingNoder noder(p_snapDistance);
    noder.computeNodes(segs);
    auto nodedSegStrings= noder.getNodedSubstrings();
    for (auto* ss : segs) {
        delete ss;
    }

    auto result = toGeometry(nodedSegStrings, geomFactory);
    for (SegmentString* ss : nodedSegStrings) {
        delete ss;
    }

    return result;
}

/* private static */
bool
CoverageCleaner::isPolygonal(const Geometry* geom)
{
    return geom->getGeometryTypeId() == geom::GEOS_POLYGON ||
           geom->getGeometryTypeId() == geom::GEOS_MULTIPOLYGON;
}


/* private static */
std::vector<const Polygon*>
CoverageCleaner::toPolygonArray(const Geometry* geom)
{
    std::size_t sz = geom->getNumGeometries();
    std::vector<const Polygon*> geoms;
    geoms.resize(sz);
    for (std::size_t i = 0; i < sz; i++) {
        const Geometry* subgeom = geom->getGeometryN(i);
        geoms.push_back(static_cast<const Polygon*>(subgeom));
    }
    return geoms;
}


} // namespace geos.coverage
} // namespace geos


