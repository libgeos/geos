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
#include <geos/index/strtree/STRtree.h>
#include <geos/noding/NodedSegmentString.h>
#include <geos/noding/Noder.h>
#include <geos/noding/SegmentStringUtil.h>
#include <geos/noding/snap/SnappingNoder.h>
#include <geos/operation/polygonize/Polygonizer.h>
#include <geos/operation/relateng/RelateNG.h>
#include <geos/operation/relateng/RelatePredicate.h>

using geos::algorithm::construct::MaximumInscribedCircle;
using geos::algorithm::locate::SimplePointInAreaLocator;
using geos::dissolve::LineDissolver;
using geos::geom::Envelope;
using geos::geom::Geometry;
using geos::geom::GeometryFactory;
using geos::geom::MultiPolygon;
using geos::geom::Point;
using geos::geom::Polygon;
using geos::index::strtree::STRtree;
using geos::noding::NodedSegmentString;
using geos::noding::Noder;
using geos::noding::SegmentStringUtil;
using geos::noding::snap::SnappingNoder;
using geos::operation::polygonize::Polygonizer;
using geos::operation::relateng::RelateNG;
using geos::operation::relateng::RelatePredicate;



namespace geos {     // geos
namespace coverage { // geos.coverage


/**
 * Create a new cleaner instance for a set of polygonal geometries.
 *
 * @param coverage an array of polygonal geometries to clean
 */
/* public */
CoverageCleaner::CoverageCleaner(std::vector<const Geometry*>& p_coverage)
    : coverage(p_coverage)
    , geomFactory(p_coverage.empty() ? nullptr : coverage[0]->getFactory())
    , computeDefaultSnappingDistance(p_coverage)
{}

/**
 * Sets the snapping distance tolerance.
 * The default is to use a small fraction of the input extent diameter.
 * A distance of zero prevents snapping from being used.
 *
 * @param snappingDistance the snapping distance tolerance
 */
/* public */
void
CoverageCleaner::setSnappingDistance(double p_snappingDistance)
{
    //-- use default distance if invalid argument
    if (p_snappingDistance < 0)
        return;
    snappingDistance = p_snappingDistance;
}

/**
 * Sets the overlap merge strategy to use.
 * The default is {@link #MERGE_LONGEST_BORDER}.
 *
 * @param mergeStrategy the merge strategy code
 */
/* public */
void
CoverageCleaner::setOverlapMergeStrategy(int mergeStrategy)
{
    if (mergeStrategy < MERGE_LONGEST_BORDER ||
        mergeStrategy > MERGE_MIN_INDEX)
        throw IllegalArgumentException("Invalid merge strategy code");

    overlapMergeStrategy = mergeStrategy;
}

/**
 * Sets the maximum width of the gaps that will be filled and merged.
 * The width of a gap is twice the radius of the Maximum Inscribed Circle in the gap polygon,
 * A width of zero prevents gaps from being merged.
 *
 * @param maxWidth the maximum gap width to merge
 */
/* public */
void
CoverageCleaner::setGapMaximumWidth(double maxWidth)
{
    if (maxWidth < 0)
        return;
    gapMaximumWidth = maxWidth;
}

//TODO: support snap-rounding noder for precision reduction
//TODO: add merge gaps by: area?

/**
 * Cleans the coverage.
 *
 */
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

/**
 * Gets the cleaned coverage.
 *
 * @return the clean coverage
 */
/* public */
std::vector<std::unique_ptr<Geometry>>
CoverageCleaner::getResult()
{
    return cleanCov->toCoverage(geomFactory);
}

/**
 * Gets polygons representing the overlaps in the input,
 * which have been merged.
 *
 * @return a list of overlap polygons
 */
/* public */
std::vector<const Polygon*>
CoverageCleaner::getOverlaps()
{
    return overlaps;
}

/**
 * Gets polygons representing the gaps in the input
 * which have been merged.
 *
 * @return a list of gap polygons
 */
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
    std::map<std::size_t, std::vector<std::size_t>>& overlapParentMap)
{
    for (const auto& [resIndex, _] : overlapParentMap) {
        auto ms = mergeStrategy(overlapMergeStrategy);
        cleanCov->mergeOverlap(
            resultants[resIndex].get(),
            *ms,
            overlapParentMap[resIndex]);
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
    throw IllegalArgumentException("Unknown merge strategy: " + mergeStrategyId);
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
    std::unique_ptr<Geometry> cleanEdges = LineDissolver::dissolve(nodedEdges);
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
    covIndex = std::make_unique<STRtree>();
    for (std::size_t i = 0; i < coverage.size(); i++) {
        covIndex->insert(coverage[i]->getEnvelopeInternal(), i);
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
    covIndex->query(intPt->getEnvelopeInternal(), candidateParentIndex);

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
CoverageCleaner::findMergableGaps(std::vector<const Polygon*> gaps)
{
    std::vector<const Polygon*> filtered;

    std::copy_if(gaps.begin(), gaps.end(),
                 std::back_inserter(filtered),
                 [](const Polygon* xgap) { return isMergableGap(gap); }
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
    if (lines.size() == 1) return lines[0];
    return geomFact->createMultiLineString(std::move(lines));
}


/* public static */
std::unique_ptr<Geometry>
CoverageCleaner::node(std::vector<const Geometry*>& coverage, double snapDistance)
{
    std::vector<SegmentString*> segs;

    for (const Geometry* geom : coverage) {
        //-- skip non-polygonal and empty elements
        if (! isPolygonal(geom))
            continue;
        if (geom->isEmpty())
            continue;
        SegmentStringUtil::extractSegmentStrings(geom, segs);
    }

    SnappingNoder noder(snapDistance);
    noder.computeNodes(&segs);
    std::unique_ptr<std::vector<SegmentString*>> nodedSegStrings(noder.getNodedSubstrings());
    for (SegmentString* ss : *segs) {
        delete ss;
    }

    auto result = toGeometry(*nodedSegStrings, geomFactory);
    for (SegmentString* ss : *nodedSegStrings) {
        delete ss;
    }

    return result;
}

/* private static */
bool
CoverageCleaner::isPolygonal(const Geometry* geom)
{
    return geom->getGeometryTypeId() == GEOS_POLYGON ||
           geom->getGeometryTypeId() == GEOS_MULTIPOLYGON;
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


