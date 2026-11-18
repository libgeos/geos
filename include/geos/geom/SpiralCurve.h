/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2026 Jeroen Bloemscheer
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/export.h>
#include <geos/geom/AffinePlacement.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/LineString.h>
#include <geos/geom/SimpleCurve.h>

#include <string>
#include <vector>

namespace geos {
namespace geom {

/// The curvature shape of an ST_SpiralCurve; see SpiralCurve.
enum class SpiralType : int {
    CLOTHOID,
    BLOSS,
    BIQUADRATIC,
    SINE,
    COSINE
};

/**
 * An ST_SpiralCurve (ISO/IEC 13249-3 7.9): a transition curve whose
 * curvature runs from a start to an end value along its length.
 *
 * Stored attributes are an affine reference location, the length L, the
 * start and end curvatures k0 and k1, the spiral type, and, when measured,
 * a STARTM/ENDM pair. In the placement's local frame the spiral starts at
 * the origin heading along +u, and at the fraction t = s / L of its length
 * its curvature is k0 + (k1 - k0) f(t). 7.9.1 Desc 19 leaves f to the
 * implementation; these are the usual transition-curve shapes:
 *
 * - CLOTHOID: f(t) = t
 * - BLOSS: f(t) = 3 t^2 - 2 t^3
 * - BIQUADRATIC: f(t) = 2 t^2 up to t = 1/2, then 1 - 2 (1 - t)^2
 * - SINE: f(t) = t - sin(2 pi t) / (2 pi)
 * - COSINE: f(t) = (1 - cos(pi t)) / 2
 *
 * Each f rises from 0 to 1 and satisfies f(1 - t) = 1 - f(t), so reversing
 * a spiral gives a spiral of the same type.
 *
 * L and the curvatures are taken in the local frame, like a clothoid's
 * constructive distances, so a placement that is not a unit similarity
 * scales the plan image. Construction requires finite attributes, a
 * positive length, and a turn L max(|k0|, |k1|) of at most 1024 radians.
 * Like Clothoid, the coordinate sequence holds only the evaluated start and
 * end points, so operations that read it as vertices refuse a SpiralCurve.
 */
class GEOS_DLL SpiralCurve : public SimpleCurve {

public:

    using Type = SpiralType;

    /// The spiral type's name as in WKT and WKB: "clothoid", "bloss",
    /// "biquadratic", "sine" or "cosine".
    static std::string typeName(Type type);

    /// The type with this name, ignoring case. Throws
    /// IllegalArgumentException for any other name.
    static Type typeFromName(const std::string& name);

    friend class GeometryFactory;
    friend class CompoundCurve;

    ~SpiralCurve() override = default;

    std::unique_ptr<SpiralCurve> clone() const;

    std::string getGeometryType() const override;

    GeometryTypeId getGeometryTypeId() const override;

    /// The plan length of the placed curve.
    double getLength() const override;

    const AffinePlacement& getReferenceLocation() const
    {
        return m_location;
    }

    /// The length attribute L, in the local frame.
    double getSpiralLength() const
    {
        return m_length;
    }

    double getStartCurvature() const
    {
        return m_k0;
    }

    double getEndCurvature() const
    {
        return m_k1;
    }

    Type getSpiralType() const
    {
        return m_type;
    }

    double getStartM() const
    {
        return m_startM;
    }

    double getEndM() const
    {
        return m_endM;
    }

    /// The point at the fraction t of the length, clamped to [0, 1]. Z is
    /// set when the curve has Z; M is NaN. Throws IllegalArgumentException
    /// when the curve is empty or t is NaN.
    CoordinateXYZM getPointAtFraction(double t) const;

    /// The image under (x, y) -> (m00 x + m01 y + m02, m10 x + m11 y + m12),
    /// with Z and M unchanged. The map is composed into the reference
    /// location, so the image is exact rather than linearized. The SRID is
    /// kept. Throws IllegalArgumentException when a coefficient is not finite.
    std::unique_ptr<SpiralCurve> affineTransform(double m00, double m01, double m02,
            double m10, double m11, double m12) const;

    bool hasCurvedComponents() const override;

    bool hasCurvedTypes() const override
    {
        return true;
    }

    bool isCurved() const override
    {
        return hasCurvedComponents();
    }

    void normalize() override;

    /// The same curve traced backwards: a spiral of the same type from the
    /// end point, with curvatures -k1 to -k0. Its placement is computed, so
    /// its end matches this curve's start to rounding.
    std::unique_ptr<SpiralCurve> reverse() const
    {
        return std::unique_ptr<SpiralCurve>(reverseImpl());
    }

    /// apply_rw throws; the reference frame is not a coordinate sequence.
    void apply_rw(const CoordinateFilter* filter) override;

    void addLinearizedPoints(CoordinateSequence& seq,
                             const algorithm::CurveToLineParams& params) const override;

    void apply_rw(CoordinateSequenceFilter& filter) override;

    bool equalsExact(const Geometry* other, double tolerance = 0) const override;

    bool equalsIdentical(const Geometry* other) const override;

protected:

    SpiralCurve(const AffinePlacement& location,
                double length, double startCurvature, double endCurvature,
                Type type,
                bool hasZ, bool hasM,
                double startM, double endM,
                const GeometryFactory& factory);

    SpiralCurve(bool hasZ, bool hasM, const GeometryFactory& factory);

    SpiralCurve(const SpiralCurve& other) = default;

    SpiralCurve* cloneImpl() const override
    {
        return new SpiralCurve(*this);
    }

    int compareToSameClass(const Geometry* geom) const override;

    void geometryChangedAction() override
    {
        envelope = computeSpiralEnvelope();
    }

    SpiralCurve* getCurvedImpl(const algorithm::LineToCurveParams&) const override
    {
        return cloneImpl();
    }

    LineString* getLinearizedImpl(const algorithm::CurveToLineParams& params) const override;

    int getSortIndex() const override
    {
        return SORTINDEX_SPIRALCURVE;
    }

    SpiralCurve* reverseImpl() const override;

private:

    void appendLinearizedPoints(CoordinateSequence& seq,
                                const algorithm::CurveToLineParams& params,
                                std::size_t first) const;

    Envelope computeSpiralEnvelope() const;

    void validateConstruction() const;

    /// Heading, in the local frame, at the fraction t.
    double heading(double t) const;

    /// Local position at the fraction t, from the panel table.
    CoordinateXY localPoint(double t) const;

    /// The local point mapped through the placement.
    Coordinate placedPoint(double t) const;

    /// Move stored endpoint 0 or 1 onto a neighbor's coordinate when they
    /// differ only by rounding in the integration.
    void snapEndpoint(std::size_t index, const CoordinateXY& target);

    AffinePlacement m_location;
    double m_length;
    double m_k0;
    double m_k1;
    Type m_type;
    double m_startM;
    double m_endM;
    /// Local positions at the panel boundaries t = i / n, integrated once.
    std::vector<CoordinateXY> m_panelPoints;
};

}
}
