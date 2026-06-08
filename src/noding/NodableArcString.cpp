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
#include <iomanip>

using geos::geom::CoordinateXYZM;
using geos::geom::CircularArc;

#define DEBUG_NODABLE_ARC_STRING 0

namespace geos::noding {

static double
pseudoAngleDiffCCW(double paStart, double pa) {
    double diff = pa - paStart;

    if (diff < 0) {
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

struct SplitPoints {

    std::vector<CoordinateXYZM> points{};
    bool splitStart{false};
    bool splitEnd{false};
};

static SplitPoints
prepareArcPoints(const CircularArc& arc, std::vector<CoordinateXYZM> splitPoints)
{
    const bool isCCW = arc.getOrientation() == algorithm::Orientation::COUNTERCLOCKWISE;
    const geom::CoordinateXY& center = arc.getCenter();

    bool splitStart = false;
    bool splitEnd = false;

    // Some potential split points may be skipped, for example, if they would create an arc section that is
    // too short to have a constructed midpoint. Because the results of this logic could depend on the
    // direction in which the arc is processed, we reverse clockwise arcs and then reverse the list of
    // retained points.
    const double paStart = geom::Quadrant::pseudoAngle(center, isCCW ? arc.p0() : arc.p2());

    std::vector<CoordinateXYZM> retained;
    // Add starting point of input arc
    {
        CoordinateXYZM p0;
        arc.getCoordinateSequence()->getAt(arc.getCoordinatePosition() + (isCCW ? 0 : 2), p0);
        retained.push_back(p0);
    }

    std::sort(splitPoints.begin(), splitPoints.end(), [&center, paStart](const auto& p0, const auto& p1) {
        double pa0 = geom::Quadrant::pseudoAngle(center, p0);
        double pa1 = geom::Quadrant::pseudoAngle(center, p1);

        double diff0 = pseudoAngleDiffCCW(paStart, pa0);
        double diff1 = pseudoAngleDiffCCW(paStart, pa1);

        if (diff0 < diff1) {
            return true;
        }
        if (diff0 > diff1) {
            return false;
        }

        return p0 < p1;
    });

    // Add ending point of input arc
    {
        CoordinateXYZM p2;
        arc.getCoordinateSequence()->getAt(arc.getCoordinatePosition() + (isCCW ? 2 : 0), p2);
        splitPoints.push_back(p2);
    }

#if DEBUG_NODABLE_ARC_STRING
    std::cout << std::setprecision(17);
    std::cout << std::endl;
    std::cout << "Edge from " << arc.p0() << " to " << arc.p2() << " has " << splitPoints.size() << " potential split points:" << std::endl;
    std::cout << "Edge is CIRCULARSTRING " << *arc.getCoordinateSequence() << std::endl;
    std::cout << "Start point is " << retained.back() << " pa " << paStart << std::endl;
    std::cout << "Potential split points:" << std::endl;
    for (std::size_t i = 0; i < splitPoints.size(); i++) {
        std::cout << "  " << splitPoints[i] << " paDiff " << pseudoAngleDiffCCW(paStart, geom::Quadrant::pseudoAngle(center, splitPoints[i]));
        if (i > 0) {
            std::cout << "     " << splitPoints[i].distance(splitPoints[i-1]);
        }
        std::cout << std::endl;
    }
#endif

    for (const auto& p2 : splitPoints) {
        auto& p0 = retained.back();

#if DEBUG_NODABLE_ARC_STRING
        std::cout << "p0 = " << p0 << std::endl;
#endif

        if (p2.equals2D(p0)) {
#if DEBUG_NODABLE_ARC_STRING
            std::cout << "Split point " << p2 << " equal to start point " << p0 << std::endl;
#endif
            if (std::isnan(p0.z) && !std::isnan(p2.z)) {
                p0.z = p2.z;
            }
            if (std::isnan(p0.m) && !std::isnan(p2.m)) {
                p0.m = p2.m;
            }

            if (retained.size() == 1) {
                splitStart = true;
            }
            continue;
        }

        if (!arc.containsPointOnCircle(p2)) {
#if DEBUG_NODABLE_ARC_STRING
            std::cout << "Skipping split point " << p2 << " because it is outside of the arc" << std::endl;
#endif
            continue;
        }

        // Calculate the midpoint of an arc between p0 and p2.
        // We don't actually use the calculated point here, we just want to make sure that
        // the arc from p0 to p2 is long enough to contain a midpoint.
        const geom::CoordinateXY p1 = algorithm::CircularArcs::getMidpoint(p0, p2, center, arc.getRadius(), true);

        if (p1.equals2D(p0) || p1.equals2D(p2)) {
#if DEBUG_NODABLE_ARC_STRING
            std::cout << "Skipping split point " << p2 << " because the calculated arc midpoint " << p1 << " equals one of the endpoints" << std::endl;
#endif
            if (retained.size() == 1) {
                splitStart = true;
            }
            continue;
        }

        if (algorithm::Orientation::index(p1, p0, p2) == algorithm::Orientation::COLLINEAR) {
#if DEBUG_NODABLE_ARC_STRING
            std::cout << "Skipping split point " << p2 << " because the calculated arc midpoint " << p1 << " is collinear with the endpoints" << std::endl;
#endif
            if (retained.size() == 1) {
                splitStart = true;
            }
            continue;
        }

        // Reject split point where computed doesn't fall between endpoints
        const double t0 = algorithm::Angle::normalizePositive(isCCW ? arc.theta0() : arc.theta2());
        const double t1 = algorithm::Angle::normalizePositive(algorithm::CircularArcs::getAngle(p1, center));
        const double t2 = algorithm::Angle::normalizePositive(isCCW ? arc.theta2() : arc.theta0());

        if (!algorithm::Angle::isWithinCCW(t1, t0, t2)) { // != isCCW) {
#if DEBUG_NODABLE_ARC_STRING
            std::cout << "Skipping split point " << p2 << " because the calculated arc midpoint " << p1 << " does not fall within the arc from " << p0 << " to " << p2 << std::endl;
#endif
            if (retained.size() == 1) {
                splitStart = true;
            }
            continue;
        }

#if DEBUG_NODABLE_ARC_STRING
        std::cout << "Keeping split point " << p2 << std::endl;
#endif
        retained.push_back(p2);
    }

    // Make sure that endpoint of split arc is unchanged
    {
        CoordinateXYZM p2;
        arc.getCoordinateSequence()->getAt(arc.getCoordinatePosition() + (isCCW ? 2 : 0), p2);
        CoordinateXYZM& back = retained.back();

        if (!back.equals2D(p2)) {
            splitEnd = true;
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

    if (!isCCW) {
        std::reverse(retained.begin(), retained.end());
        std::swap(splitStart, splitEnd);
    }

#if DEBUG_NODABLE_ARC_STRING
    std::cout << "Retained split points:" << std::endl;
    for (std::size_t i = 0; i < retained.size(); i++) {
        std::cout << "  " << retained[i] << " paDiff " << pseudoAngleDiffCCW(paStart, geom::Quadrant::pseudoAngle(center, retained[i]));
        if (i > 0) {
            std::cout << "     " << retained[i].distance(retained[i-1]);
        }
        std::cout << std::endl;
    }
#endif

    return SplitPoints{std::move(retained), splitStart, splitEnd};
}

class ArcBuilder {

public:

    ArcBuilder(std::vector<std::unique_ptr<ArcString>>& splitArcs, const void* data, bool constructZ, bool constructM) :
        m_data(data),
        m_splitArcs(splitArcs),
        m_constructZ(constructZ),
        m_constructM(constructM) {}

    void addArc(const CoordinateXYZM& p0, const CoordinateXYZM& p1, const CoordinateXYZM& p2, const geom::CoordinateXY& center, double radius, int orientation) {
        if (!m_coords) {
            m_coords = std::make_unique<geom::CoordinateSequence>(0, m_constructZ, m_constructM);
        }

        m_coords->add(p0, false);
        m_coords->add(p1, false);
        m_coords->add(p2, false);

        m_arcs.emplace_back(*m_coords, m_coords->getSize() - 3, center, radius, orientation);
    }

    void addArc(const geom::CoordinateSequence& seq, std::size_t pos, const geom::CoordinateXY& center, double radius, int orientation) {
        if (!m_coords) {
            m_coords = std::make_unique<geom::CoordinateSequence>(0, m_constructZ, m_constructM);
        }

        m_coords->add(seq, pos, pos + 2, false);
        m_arcs.emplace_back(*m_coords, m_coords->getSize() - 3, center, radius, orientation);
    }

    void finish() {
        if (m_arcs.empty()) {
            return;
        }

        m_splitArcs.push_back(std::make_unique<NodableArcString>(std::move(m_arcs), std::move(m_coords), m_constructZ, m_constructM, m_data));
        m_arcs = std::vector<CircularArc>();
    }

private:
    const void* m_data;
    std::vector<std::unique_ptr<ArcString>>& m_splitArcs;
    std::vector<CircularArc> m_arcs;
    std::unique_ptr<geom::CoordinateSequence> m_coords;
    const bool m_constructZ;
    const bool m_constructM;
};

void
NodableArcString::getNoded(std::vector<std::unique_ptr<ArcString>>& splitArcs) {
    ArcBuilder builder(splitArcs, getData(), m_constructZ, m_constructM);

    for (size_t arcIndex = 0; arcIndex < m_arcs.size(); arcIndex++) {
        const CircularArc& toSplit = m_arcs[arcIndex];
        const geom::CoordinateXY& center = toSplit.getCenter();
        const double radius = toSplit.getRadius();
        const int orientation = toSplit.getOrientation();

        SplitPoints split;

        const bool preserveControlPoint = true;

        const auto it = m_adds.find(arcIndex);
        if (it != m_adds.end()) {
            split = prepareArcPoints(toSplit, it->second);
        }

        if (split.splitStart) {
            builder.finish();
        }

        if (preserveControlPoint && split.points.size() <= 2) {
            // No nodes added, just copy the coordinates into the sequence.
            const geom::CoordinateSequence* srcSeq = m_arcs[arcIndex].getCoordinateSequence();
            std::size_t srcPos = m_arcs[arcIndex].getCoordinatePosition();

            builder.addArc(*srcSeq, srcPos, center, radius, orientation);

            if (split.splitEnd) {
                builder.finish();
            }

            continue;
        }

        const bool isCCW = orientation == algorithm::Orientation::COUNTERCLOCKWISE;

        for (std::size_t i = 1; i < split.points.size(); i++) {
            const CoordinateXYZM& p0 = split.points[i - 1];
            const CoordinateXYZM& p2 = split.points[i];

            // TODO: Check if control point of original arc falls into this section,
            // and use it instead of calculating a midpoint here?
            CoordinateXYZM p1(algorithm::CircularArcs::getMidpoint(p0, p2, center, radius, isCCW));
            p1.z = (p0.z + p2.z) / 2;
            p1.m = (p0.m + p2.m) / 2;

            builder.addArc(p0, p1, p2, center, radius, orientation);

            // Finish the ArcString, start a new one.
            const bool isSplitPoint = (i < split.points.size() - 1) || split.splitEnd;
            if (isSplitPoint) {
                builder.finish();

            }
        }
    }

    builder.finish();
}

}