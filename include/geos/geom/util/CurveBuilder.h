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

    // Add all coordinates in the provided sequence (optionally reversed)
    void add(const CoordinateSequence& seq, bool isCurved, bool isForward);

    // Add coordinates between the specified indices (inclusive)
    void add(const CoordinateSequence& seq, std::size_t from, std::size_t to, bool isCurved);

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
        return !m_pts.empty();
    }

    bool isCurved() const {
        return hasActiveSequence() && m_pts.back().second;
    }

    void setOutputLinearRing(bool outputLinearRing) {
        m_outputLinearRing = outputLinearRing;
    }

    void matchRingZM();

    // Declare type as noncopyable
    CurveBuilder(const CurveBuilder& other) = delete;
    CurveBuilder& operator=(const CurveBuilder& rhs) = delete;

private:
    std::vector<std::pair<std::unique_ptr<CoordinateSequence>, bool>> m_pts;
    const GeometryFactory& m_gfact;
    const bool m_hasZ;
    const bool m_hasM;
    bool m_outputLinearRing{true};
};

}