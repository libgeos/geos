/**********************************************************************
*
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2026 ISciences LLC
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/geom/util/CurveBuilder.h>

#include <geos/geom/CircularString.h>
#include <geos/geom/CompoundCurve.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/LineString.h>
#include <geos/util/GEOSException.h>

#include <cmath>

namespace geos::geom::util {

CurveBuilder::CurveBuilder(const GeometryFactory& gfact, bool hasZ, bool hasM)
    : m_gfact(gfact),
      m_hasZ(hasZ),
      m_hasM(hasM)
{}

void
CurveBuilder::add(const Curve& geom)
{
    if (geom.getGeometryTypeId() == GEOS_COMPOUNDCURVE) {
        const CompoundCurve& cc = static_cast<const CompoundCurve&>(geom);
        for (std::size_t i = 0; i < cc.getNumCurves(); i++) {
            add(*cc.getCurveN(i));
        }
    } else {
        const CoordinateSequence* coords = static_cast<const SimpleCurve&>(geom).getCoordinatesRO();
        const bool isCurved =  geom.getGeometryTypeId() == GEOS_CIRCULARSTRING;
        add(*coords, isCurved);
    }
}

void
CurveBuilder::add(const CoordinateSequence& coords, bool isCurved)
{
    add(coords, isCurved, true);
}

void
CurveBuilder::add(const CoordinateSequence& coords, bool isCurved, bool isForward)
{
    if (coords.isEmpty()) {
        return;
    }

    if (isForward) {
        add(coords, 0, coords.size() - 1, isCurved);
    } else {
        add(coords, coords.size() - 1, 0, isCurved);
    }
}

void
CurveBuilder::add(const CoordinateSequence& src, std::size_t from, std::size_t to, bool isCurved)
{
    CoordinateSequence& dst = getSeq(isCurved);
    const std::size_t insertionPos = dst.size();

    if (from > to) {
        for(std::size_t i = from + 1; i > to; --i) {
            src.applyAt(i-1, [&dst](const auto& coord) {
                dst.add(coord, false);
            });
        }
    } else {
        dst.add(src, from, to, false);
    }

    // When adding a sequence whose initial point is the same as the current final point, we may need to
    // copy Z/M values from the initial point to the final point, or vice-versa.

    if (!m_hasZ && !m_hasM) {
        return;
    }

    if (insertionPos == 0 && m_pts.size() == 1) {
        return;
    }

    CoordinateSequence& prev = insertionPos == 0 ? *m_pts[m_pts.size() - 2].first : dst;
    std::size_t prevPos = insertionPos == 0 ? prev.size() - 1 : insertionPos - 1;
    std::size_t currPos = insertionPos == 0 ? 0 : insertionPos - 1;

    const bool firstPointEqualsLast = prev.getAt<CoordinateXY>(prevPos).equals2D(src.getAt<CoordinateXY>(from));

    if (!firstPointEqualsLast) {
        return;
    }

    if (m_hasZ) {
        const double currZ = src.getZ(from);
        const double prevZ = prev.getZ(prevPos);

        if (std::isnan(prevZ) && !std::isnan(currZ)) {
            prev.setZ(prevPos, currZ);
        }
        if (std::isnan(currZ) && !std::isnan(prevZ)) {
            dst.setZ(currPos, prevZ);
        }
    }

    if (m_hasM) {
        const double currM = src.getM(from);
        const double prevM = prev.getM(prevPos);

        if (std::isnan(prevM) && !std::isnan(currM)) {
            prev.setM(prevPos, currM);
        }
        if (std::isnan(currM) && !std::isnan(prevM)) {
            dst.setM(currPos, prevM);
        }
    }
}

void
CurveBuilder::matchRingZM()
{
    if (m_pts.empty()) {
        return;
    }

    CoordinateSequence& firstSeq = *m_pts.front().first;
    CoordinateSequence& lastSeq = *m_pts.back().first;

    if (!firstSeq.front<CoordinateXY>().equals2D(lastSeq.back<CoordinateXY>())) {
        throw geos::util::GEOSException("CurveBuilder::matchRingZM called on non-ring");
    }

    if (m_hasZ) {
        const double firstZ = firstSeq.getZ(0);
        const double lastZ = lastSeq.getZ(lastSeq.size() - 1);

        if (std::isnan(firstZ) && !std::isnan(lastZ)) {
            firstSeq.setZ(0, lastZ);
        }
        if (std::isnan(lastZ) && !std::isnan(firstZ)) {
            lastSeq.setZ(lastSeq.size() - 1, firstZ);
        }
    }

    if (m_hasM) {
        const double firstM = firstSeq.getM(0);
        const double lastM = lastSeq.getM(lastSeq.size() - 1);

        if (std::isnan(firstM) && !std::isnan(lastM)) {
            firstSeq.setM(0, lastM);
        }
        if (std::isnan(lastM) && !std::isnan(firstM)) {
            lastSeq.setM(lastSeq.size() - 1, firstM);
        }
    }
}

void
CurveBuilder::closeRing()
{
    if (m_pts.empty()) {
        return;
    }

    CoordinateXYZM first;
    m_pts.front().first->getAt(0, first);

    CoordinateXYZM last;
    m_pts.back().first->getAt(m_pts.back().first->size() - 1, last);

    if (first.equals2D(last)) {
        matchRingZM();
        return;
    }

    if (isCurved()) {
        getSeq(false).add(last);
    }
    getSeq(false).add(first);
}

std::unique_ptr<Curve>
CurveBuilder::getGeometry()
{
    if (m_pts.empty()) {
        return m_gfact.createLineString(std::make_unique<CoordinateSequence>(0, m_hasZ, m_hasM));
    }

    if (m_pts.size() == 1) {
        auto& [pts, isCurve] = m_pts.front();
        if (isCurve) {
            return m_gfact.createCircularString(std::move(pts));
        }
        if (m_outputLinearRing && pts->isRing()) {
            return m_gfact.createLinearRing(std::move(pts));
        }
        return m_gfact.createLineString(std::move(pts));
    }

    std::vector<std::unique_ptr<SimpleCurve>> curves;
    for (auto& [pts, isCurve] : m_pts) {
        if (isCurve) {
            curves.push_back(m_gfact.createCircularString(std::move(pts)));
        } else {
            curves.push_back(m_gfact.createLineString(std::move(pts)));
        }
    }

    return m_gfact.createCompoundCurve(std::move(curves));
}

CoordinateSequence&
CurveBuilder::getSeq(bool isCurved)
{
    if (m_pts.empty() || m_pts.back().second != isCurved) {
        m_pts.emplace_back(std::make_unique<CoordinateSequence>(0, m_hasZ, m_hasM), isCurved);
    }

    return *m_pts.back().first;
}

}
