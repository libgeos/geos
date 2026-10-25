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

namespace geos {
namespace geom {

/**
 * An ST_EllipticalCurve (ISO/IEC 13249-3 7.6): an arc of an ellipse.
 *
 * Stored attributes are an affine reference location, the u and v axis
 * lengths a and b, and the start and end parameter angles in radians.
 * Measure, when present, is a STARTM/ENDM pair.
 *
 * The point at parameter angle t is origin + a cos(t) u + b sin(t) v. The
 * arc runs over all angles numerically between the start and end angle:
 * counterclockwise in the (u, v) frame when the end angle is larger, and
 * clockwise when it is smaller. Angles may be negative or beyond 2 pi; an
 * arc whose angles differ by 2 pi is a complete, closed ellipse.
 *
 * At an angle within a few ulps of a multiple of pi / 2, cos and sin are
 * taken as exactly 0 or +-1, so an arc given in whole quadrants, such as
 * 0 to 360 degrees, starts and ends exactly on its axes.
 *
 * Construction requires finite attributes, positive axis lengths and a
 * sweep of at most one turn. The coordinate sequence holds
 * only the evaluated start and end points, so operations that read it as
 * vertices refuse an EllipticalCurve.
 */
class GEOS_DLL EllipticalCurve : public SimpleCurve {

public:

    friend class GeometryFactory;
    friend class CompoundCurve;

    ~EllipticalCurve() override = default;

    std::unique_ptr<EllipticalCurve> clone() const;

    std::string getGeometryType() const override;

    GeometryTypeId getGeometryTypeId() const override;

    double getLength() const override;

    const AffinePlacement& getReferenceLocation() const
    {
        return m_location;
    }

    double getUAxisLength() const
    {
        return m_uAxis;
    }

    double getVAxisLength() const
    {
        return m_vAxis;
    }

    /// Start parameter angle, in radians.
    double getStartAngle() const
    {
        return m_startAngle;
    }

    /// End parameter angle, in radians.
    double getEndAngle() const
    {
        return m_endAngle;
    }

    double getStartM() const
    {
        return m_startM;
    }

    double getEndM() const
    {
        return m_endM;
    }

    /// True when the start and end angles differ by a full turn.
    bool isFullEllipse() const;

    /// The point at parameter angle t, in radians. The angle is not limited
    /// to the arc. Z is set when the curve has Z; M is NaN. Throws
    /// IllegalArgumentException when the curve is empty or t is not finite.
    CoordinateXYZM getPointAtAngle(double t) const;

    /// The image under (x, y) -> (m00 x + m01 y + m02, m10 x + m11 y + m12),
    /// with Z and M unchanged. The map is composed into the reference
    /// location, so the image is exact rather than linearized. The SRID is
    /// kept. Throws IllegalArgumentException when a coefficient is not finite.
    std::unique_ptr<EllipticalCurve> affineTransform(double m00, double m01, double m02,
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

    std::unique_ptr<EllipticalCurve> reverse() const
    {
        return std::unique_ptr<EllipticalCurve>(reverseImpl());
    }

    /// apply_rw throws; the reference frame is not a coordinate sequence.
    void apply_rw(const CoordinateFilter* filter) override;

    void addLinearizedPoints(CoordinateSequence& seq,
                             const algorithm::CurveToLineParams& params) const override;

    void apply_rw(CoordinateSequenceFilter& filter) override;

    bool equalsExact(const Geometry* other, double tolerance = 0) const override;

    bool equalsIdentical(const Geometry* other) const override;

protected:

    EllipticalCurve(const AffinePlacement& location,
                    double uAxisLength, double vAxisLength,
                    double startAngle, double endAngle,
                    bool hasZ, bool hasM,
                    double startM, double endM,
                    const GeometryFactory& factory);

    EllipticalCurve(bool hasZ, bool hasM, const GeometryFactory& factory);

    EllipticalCurve(const EllipticalCurve& other) = default;

    EllipticalCurve* cloneImpl() const override
    {
        return new EllipticalCurve(*this);
    }

    int compareToSameClass(const Geometry* geom) const override;

    void geometryChangedAction() override
    {
        envelope = computeEllipticalEnvelope();
    }

    EllipticalCurve* getCurvedImpl(const algorithm::LineToCurveParams&) const override
    {
        return cloneImpl();
    }

    LineString* getLinearizedImpl(const algorithm::CurveToLineParams& params) const override;

    int getSortIndex() const override
    {
        return SORTINDEX_ELLIPTICALCURVE;
    }

    EllipticalCurve* reverseImpl() const override;

private:

    void appendLinearizedPoints(CoordinateSequence& seq,
                                const algorithm::CurveToLineParams& params,
                                std::size_t first) const;

    Envelope computeEllipticalEnvelope() const;

    std::unique_ptr<CoordinateSequence> evaluateEndpoints() const;

    /// True when the angles differ by a full turn. Unlike isFullEllipse(),
    /// it does not look at the endpoints, which construction computes from it.
    bool sweepsFullTurn() const;

    void validateConstruction() const;

    /// Move stored endpoint 0 or 1 onto a neighbor's coordinate when they
    /// differ only by rounding in cos and sin.
    void snapEndpoint(std::size_t index, const CoordinateXY& target);

    AffinePlacement m_location;
    double m_uAxis;
    double m_vAxis;
    double m_startAngle;
    double m_endAngle;
    double m_startM;
    double m_endM;
};

}
}
