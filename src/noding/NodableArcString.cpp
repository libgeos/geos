/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2025-2026 ISciences, LLC
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/noding/NodableArcString.h>
#include <geos/algorithm/Angle.h>

#include <algorithm>

using geos::geom::CoordinateXYZM;
using geos::geom::CircularArc;

namespace geos::noding {

static double
pseudoAngleDiffCCW(double paStart, double pa) {
    double diff = pa - paStart;

    if (diff <= 0) {
        diff += 4;
    }

    return diff;
}

NodableArcString::NodableArcString(std::vector<geom::CircularArc> arcs, const std::shared_ptr<const geom::CoordinateSequence>& coords, bool constructZ, bool constructM, const void* p_context) :
    ArcString(std::move(arcs), coords, p_context),
    m_constructZ(constructZ),
    m_constructM(constructM)
{
}

static
std::vector<CoordinateXYZM> prepareArcPoints(const CircularArc& arc, std::vector<CoordinateXYZM> splitPoints)
{
    const bool isCCW = arc.getOrientation() == algorithm::Orientation::COUNTERCLOCKWISE;
    const geom::CoordinateXY& center = arc.getCenter();
    const double paStart = geom::Quadrant::pseudoAngle(center, arc.p0());

    std::vector<CoordinateXYZM> retained;
    // Add starting point of input arc
    {
        CoordinateXYZM p0;
        arc.getCoordinateSequence()->getAt(arc.getCoordinatePosition(), p0);
        retained.push_back(p0);
    }

    std::sort(splitPoints.begin(), splitPoints.end(), [&center, paStart, isCCW](const auto& p0, const auto& p1) {
        double pa0 = geom::Quadrant::pseudoAngle(center, p0);
        double pa1 = geom::Quadrant::pseudoAngle(center, p1);

        if (isCCW) {
            return pseudoAngleDiffCCW(paStart, pa0) < pseudoAngleDiffCCW(paStart, pa1);
        } else {
            return pseudoAngleDiffCCW(paStart, pa0) > pseudoAngleDiffCCW(paStart, pa1);
        }
    });

    // Add ending point of input arc
    {
        CoordinateXYZM p2;
        arc.getCoordinateSequence()->getAt(arc.getCoordinatePosition() + 2, p2);
        splitPoints.push_back(p2);
    }

    for (const auto& p2 : splitPoints) {
        auto& p0 = retained.back();

        if (p2.equals2D(p0)) {
            if (std::isnan(p0.z) && !std::isnan(p2.z)) {
                p0.z = p2.z;
            }
            if (std::isnan(p0.m) && !std::isnan(p2.m)) {
                p0.m = p2.m;
            }
            continue;
        }

        if (!arc.containsPointOnCircle(p2)) {
            continue;
        }

        const auto p1 = algorithm::CircularArcs::getMidpoint(p0, p2, center, arc.getRadius(), isCCW);

        if (p1.equals2D(p0) || p1.equals2D(p2)) {
            continue;
        }

        // Reject split point where sub-arc midpoint doesn't fall inside arc
        if (!arc.containsPointOnCircle(p1)) {
            continue;
        }

        // Reject split point where computed doesn't fall between endpoints
        const double t0 = algorithm::Angle::normalizePositive(arc.theta0());
        const double t1 = algorithm::Angle::normalizePositive(algorithm::CircularArcs::getAngle(p1, center));
        const double t2 = algorithm::Angle::normalizePositive(algorithm::CircularArcs::getAngle(p2, center));

        if (algorithm::Angle::isWithinCCW(t1, t0, t2) != isCCW) {
            continue;
        }

        retained.push_back(p2);
    }

    // Make sure that endpoint of split arc is unchanged
    {
        CoordinateXYZM p2;
        arc.getCoordinateSequence()->getAt(arc.getCoordinatePosition() + 2, p2);
        CoordinateXYZM& back = retained.back();

        if (!back.equals2D(p2)) {
            back.x = p2.x;
            back.y = p2.y;
        }
        if (std::isnan(back.z) && !std::isnan(p2.z)) {
            back.z = p2.z;
        }
        if (std::isnan(back.m) && !std::isnan(p2.m)) {
            back.m = p2.m;
        }
    }

    return retained;
}

void
NodableArcString::getNoded(std::vector<std::unique_ptr<ArcString>>& splitArcs) {
    auto dstSeq = std::make_unique<geom::CoordinateSequence>(0, m_constructZ, m_constructM);
    std::vector<geom::CircularArc> arcs;

    for (size_t arcIndex = 0; arcIndex < m_arcs.size(); arcIndex++) {
        const CircularArc& toSplit = m_arcs[arcIndex];
        const geom::CoordinateXY& center = toSplit.getCenter();
        const double radius = toSplit.getRadius();
        const int orientation = toSplit.getOrientation();

        bool arcIsSplit = true;
        const bool preserveControlPoint = true;
        std::vector<CoordinateXYZM> arcPoints;
        const auto it = m_adds.find(arcIndex);
        if (it == m_adds.end()) {
            arcIsSplit = false;
        } else {
            arcPoints = prepareArcPoints(toSplit, it->second);

            if (arcPoints.size() == 2) {
                // All added nodes collapsed
                arcIsSplit = false;
            }
        }

        if (preserveControlPoint && !arcIsSplit) {
            // No nodes added, just copy the coordinates into the sequence.
            const geom::CoordinateSequence* srcSeq = m_arcs[arcIndex].getCoordinateSequence();
            std::size_t srcPos = m_arcs[arcIndex].getCoordinatePosition();
            dstSeq->add(*srcSeq, srcPos, srcPos + 2, false);
            std::size_t dstPos = dstSeq->getSize() - 3;
            arcs.emplace_back(*dstSeq, dstPos, center, radius, orientation);

            continue;
        }

        const bool isCCW = orientation == algorithm::Orientation::COUNTERCLOCKWISE;

        for (std::size_t i = 1; i < arcPoints.size(); i++) {
            const CoordinateXYZM& p0 = arcPoints[i - 1];
            const CoordinateXYZM& p2 = arcPoints[i];

            // TODO: Check if control point of original arc falls into this section,
            // and use it instead of calculating a midpoint here?
            CoordinateXYZM p1(algorithm::CircularArcs::getMidpoint(p0, p2, center, radius, isCCW));
            p1.z = (p0.z + p2.z) / 2;
            p1.m = (p0.m + p2.m) / 2;

            if (dstSeq->isEmpty()) {
                dstSeq->add(p0);
            }
            dstSeq->add(p1);
            dstSeq->add(p2);

            const std::size_t dstPos = dstSeq->getSize() - 3;
            arcs.emplace_back(*dstSeq, dstPos, center, radius, orientation);

            // Finish the ArcString, start a new one.
            const bool isSplitPoint = i != arcPoints.size() - 1;
            if (isSplitPoint) {
                splitArcs.push_back(std::make_unique<NodableArcString>(std::move(arcs), std::move(dstSeq), m_constructZ, m_constructM, getData()));
                dstSeq = std::make_unique<geom::CoordinateSequence>(0, m_constructZ, m_constructM);
                arcs.clear();
            }
        }
    }

    if (!arcs.empty()) {
        splitArcs.push_back(std::make_unique<NodableArcString>(std::move(arcs), std::move(dstSeq), m_constructZ, m_constructM, getData()));
    }
}

}