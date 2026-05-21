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
    getSeq(isCurved).add(coords, false);
}

void
CurveBuilder::closeRing()
{
    if (m_curves.empty() && (m_pts == nullptr || m_pts->isEmpty())) {
        return;
    }

    CoordinateXYZM first;
    if (!m_curves.empty()) {
        m_curves.front()->getCoordinatesRO()->getAt(0, first);
    } else {
        m_pts->getAt(0, first);
    }

    CoordinateXYZM last;
    if (m_pts && !m_pts->isEmpty()) {
        m_pts->getAt(m_pts->size() - 1, last);
    } else {
        const auto* seq = m_curves.back()->getCoordinatesRO();
        seq->getAt(seq->size() - 1, last);
    }

    if (first.equals2D(last)) {
        return;
    }

    if (m_pts && m_isCurved) {
        finishCurve();
        getSeq(false).add(last);
    }

    getSeq(false).add(first);
}

void
CurveBuilder::finishCurve()
{
    m_curves.push_back(m_gfact.createCircularString(std::move(m_pts)));
    m_pts = nullptr;
}

void
CurveBuilder::finishLine()
{
    m_curves.push_back(m_gfact.createLineString(std::move(m_pts)));
    m_pts = nullptr;
}

std::unique_ptr<Curve>
CurveBuilder::getGeometry()
{
    if (m_pts) {
        if (m_isCurved) {
            finishCurve();
        } else {

            if (m_outputLinearRing && m_curves.empty() && m_pts->isRing()) {
                m_curves.push_back(m_gfact.createLinearRing(std::move(m_pts)));
            } else {
                finishLine();
            }
        }
    }

    if (m_curves.empty()) {
        auto seq = std::make_unique<CoordinateSequence>(0, m_hasZ, m_hasM);
        return m_gfact.createLineString(std::move(seq));
    }

    if (m_curves.size() == 1) {
        return std::move(m_curves[0]);
    }

    return m_gfact.createCompoundCurve(std::move(m_curves));
}

CoordinateSequence&
CurveBuilder::getSeq(bool isCurved)
{
    if (m_pts) {
        if (m_isCurved && !isCurved) {
            finishCurve();
        } else if (isCurved && !m_isCurved) {
            finishLine();
        }
    }

    if (!m_pts) {
        m_pts = std::make_unique<CoordinateSequence>(0, m_hasZ, m_hasM);
        m_isCurved = isCurved;
    }

    return *m_pts;
}

}
