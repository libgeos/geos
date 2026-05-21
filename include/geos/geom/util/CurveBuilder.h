/**********************************************************************
*
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2026 ISciences LLC
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundations.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/export.h>

#include <memory>
#include <vector>

namespace geos::geom {
class CoordinateSequence;
class Curve;
class GeometryFactory;
class SimpleCurve;
}

namespace geos::geom::util {

/// The CurveBuilder is a utility class to assist in construction of simple or
/// compound curves, such as when combining coordinates from a list of Edges.
class GEOS_DLL CurveBuilder {
public:
    CurveBuilder(const GeometryFactory& gfact, bool hasZ, bool hasM);

    // Add all coordinates in the provided geometry
    void add(const Curve& geom);

    // Add all coordinates in the provided sequence
    void add(const CoordinateSequence& seq, bool isCurved);

    // Close the ring, if necessary, using a linear segment
    void closeRing();

    // Get the result geometry
    std::unique_ptr<Curve> getGeometry();

    // Get a reference to the CoordinateSequence to which
    // coordinates are currently being added.
    CoordinateSequence& getSeq(bool isCurved);

    bool hasZ() const {
        return m_hasZ;
    }

    bool hasM() const {
        return m_hasM;
    }

    bool hasActiveSequence() const {
        return m_pts != nullptr;
    }

    bool isCurved() const {
        return m_isCurved;
    }

    void setOutputLinearRing(bool outputLinearRing) {
        m_outputLinearRing = outputLinearRing;
    }

private:
    void finishCurve();
    void finishLine();

    std::vector<std::unique_ptr<SimpleCurve>> m_curves;
    std::unique_ptr<CoordinateSequence> m_pts{nullptr};
    const GeometryFactory& m_gfact;
    const bool m_hasZ;
    const bool m_hasM;
    bool m_outputLinearRing{true};
    bool m_isCurved{false};
};

}