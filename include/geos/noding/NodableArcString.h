/**********************************************************************
*
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2025 ISciences, LLC
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/export.h>
#include <geos/noding/ArcString.h>
#include <geos/noding/NodablePath.h>

namespace geos::noding {

class GEOS_DLL NodableArcString : public ArcString, public NodablePath {

public:
    using ArcString::ArcString;

    void addIntersection(geom::CoordinateXYZM intPt, size_t segmentIndex) override {
        m_adds[segmentIndex].push_back(intPt);
    }

    static double pseudoAngleDiffCCW(double paStart, double pa) {
        double diff = pa - paStart;

        if (diff < 0) {
            diff += 4;
        }

        return diff;
    }

    std::unique_ptr<ArcString> getNoded() {
        if (m_adds.empty()) {
            // use std::move ?
            return std::make_unique<ArcString>(*this);
        }

        std::vector<geom::CircularArc> arcs;
        for (size_t i = 0; i < m_arcs.size(); i++) {
            auto it = m_adds.find(i);
            if (it == m_adds.end()) {
                arcs.push_back(m_arcs[i]);
            } else {
                std::vector<geom::CoordinateXYZM>& splitPoints = it->second;

                // TODO check split point actually inside arc
                // TODO ignore duplicate splitpoints

                geom::CircularArc remainder = m_arcs[i];
                const bool isCCW = remainder.getOrientation() == algorithm::Orientation::COUNTERCLOCKWISE;
                const double paStart = geom::Quadrant::pseudoAngle(remainder.getCenter(), remainder.p0);
                //double paStart = isCCW ? geom::Quadrant::pseudoAngle(remainder.getCenter(), remainder.p0) :
                //                               geom::Quadrant::pseudoAngle(remainder.getCenter(), remainder.p2);

                // Need a function for lengthFraction (pt);
                std::sort(splitPoints.begin(), splitPoints.end(), [&remainder, paStart, isCCW](const auto& p0, const auto& p1) {
                    double pa0 = geom::Quadrant::pseudoAngle(remainder.getCenter(), p0);
                    double pa1 = geom::Quadrant::pseudoAngle(remainder.getCenter(), p1);

                    // FIXME check this comparator...
                    if (isCCW) {
                        return pseudoAngleDiffCCW(paStart, pa0) < pseudoAngleDiffCCW(paStart, pa1);
                    } else {
                        return pseudoAngleDiffCCW(paStart, pa0) > pseudoAngleDiffCCW(paStart, pa1);
                    }
                });

                std::cout << "arc " << remainder.toString() << " " << (isCCW ? "CCW" : "CW") << std::endl;
                std::cout << "  paStart " << paStart << std::endl;
                for (const auto& pt : splitPoints) {
                    const double pa = geom::Quadrant::pseudoAngle(remainder.getCenter(), pt);
                    std::cout << "  " << pt << " pa " << pa << " diff " << pseudoAngleDiffCCW(paStart, pa) << std::endl;
                }

                for (const auto& splitPoint : splitPoints) {
                    if (!arcs.empty() && splitPoint.equals2D(arcs.back().p2)) {
                        continue;
                    }
                    auto [a, b] = remainder.splitAtPoint(splitPoint);
                    arcs.push_back(a);
                    remainder = b;
                }
                arcs.push_back(remainder);
            }
        }

        return std::make_unique<ArcString>(std::move(arcs));
    }

private:
    std::map<size_t, std::vector<geom::CoordinateXYZM>> m_adds;
};

}