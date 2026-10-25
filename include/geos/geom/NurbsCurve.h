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

#include <geos/constants.h>
#include <geos/export.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/LineString.h>
#include <geos/geom/SimpleCurve.h>

#include <cstddef>
#include <vector>

namespace geos {
namespace geom {

/// An ST_NURBSPoint: a control point already multiplied by its weight.
///
/// The weighted point holds (w x, w y, w z), not the cartesian point. A NaN
/// weight is absent, and the point is then cartesian with weight 1.
struct GEOS_DLL NurbsPoint {
    Coordinate weightedPoint;
    double weight = DoubleNotANumber;

    bool hasWeight() const;

    /// The weight, or 1 when it is absent.
    double effectiveWeight() const;
};

/// An ST_Knot: a distinct knot value and the number of times it occurs.
struct GEOS_DLL NurbsKnot {
    double value;
    std::size_t multiplicity;
};

/**
 * An ST_NURBSCurve (ISO/IEC 13249-3 7.7): a single non-uniform rational
 * B-spline segment.
 *
 * Stored attributes are the degree, the weighted control points, the knots
 * as distinct values with multiplicities, and, when measured, a STARTM/ENDM
 * pair. The knots need not be clamped. The curve is defined on the knot
 * domain [u_p, u_n] of the expanded knot vector u_0 .. u_{n+p}, for n
 * control points of degree p.
 *
 * Either every control point has a weight or none has; the curve is
 * rational in the first case. Weights must be positive, so the curve lies in
 * the convex hull of its cartesian control points.
 *
 * Construction also requires a degree from 1 to MAX_DEGREE, at least
 * degree + 1 control points, finite strictly increasing knot values, a total
 * multiplicity of control points + degree + 1, no multiplicity above
 * degree inside the domain (which would break the curve, 7.7.1 Desc 21) and
 * none above degree + 1 at all.
 *
 * The coordinate sequence holds only the evaluated start and
 * end points, so operations that read it as vertices refuse a NurbsCurve.
 */
class GEOS_DLL NurbsCurve : public SimpleCurve {

public:

    /// Largest supported degree. A clamped end has multiplicity degree + 1,
    /// and WKB writes a multiplicity in one byte.
    static constexpr int MAX_DEGREE = 254;

    friend class GeometryFactory;
    friend class CompoundCurve;

    ~NurbsCurve() override = default;

    std::unique_ptr<NurbsCurve> clone() const;

    std::string getGeometryType() const override;

    GeometryTypeId getGeometryTypeId() const override;

    double getLength() const override;

    int getDegree() const
    {
        return m_degree;
    }

    const std::vector<NurbsPoint>& getControlPoints() const
    {
        return m_controlPoints;
    }

    const std::vector<NurbsKnot>& getKnots() const
    {
        return m_knots;
    }

    /// True when the control points carry weights.
    bool isRational() const;

    double getStartM() const
    {
        return m_startM;
    }

    double getEndM() const
    {
        return m_endM;
    }

    /// Start of the knot domain, u_p.
    double getStartParameter() const;

    /// End of the knot domain, u_n.
    double getEndParameter() const;

    /// The point at knot parameter u, clamped to the knot domain. Z is set
    /// when the curve has Z. M is NaN, since a NURBS measure is only given
    /// at its ends. Throws IllegalArgumentException when the curve is empty
    /// or u is NaN.
    CoordinateXYZM getPointAtParameter(double u) const;

    /// The image under (x, y) -> (m00 x + m01 y + m02, m10 x + m11 y + m12),
    /// with Z and M unchanged. NURBS are affine invariant, so the map is
    /// applied to the control points and the image is exact rather than
    /// linearized. The SRID is kept. Throws IllegalArgumentException when a
    /// coefficient is not finite.
    std::unique_ptr<NurbsCurve> affineTransform(double m00, double m01, double m02,
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

    std::unique_ptr<NurbsCurve> reverse() const
    {
        return std::unique_ptr<NurbsCurve>(reverseImpl());
    }

    /// apply_rw throws; the control points are not a coordinate sequence.
    void apply_rw(const CoordinateFilter* filter) override;

    void addLinearizedPoints(CoordinateSequence& seq,
                             const algorithm::CurveToLineParams& params) const override;

    void apply_rw(CoordinateSequenceFilter& filter) override;

    bool equalsExact(const Geometry* other, double tolerance = 0) const override;

    bool equalsIdentical(const Geometry* other) const override;

protected:

    NurbsCurve(int degree,
               std::vector<NurbsPoint> controlPoints,
               std::vector<NurbsKnot> knots,
               bool hasZ, bool hasM,
               double startM, double endM,
               const GeometryFactory& factory);

    NurbsCurve(bool hasZ, bool hasM, const GeometryFactory& factory);

    NurbsCurve(const NurbsCurve& other) = default;

    NurbsCurve* cloneImpl() const override
    {
        return new NurbsCurve(*this);
    }

    int compareToSameClass(const Geometry* geom) const override;

    void geometryChangedAction() override
    {
        envelope = computeNurbsEnvelope();
    }

    NurbsCurve* getCurvedImpl(const algorithm::LineToCurveParams&) const override
    {
        return cloneImpl();
    }

    LineString* getLinearizedImpl(const algorithm::CurveToLineParams& params) const override;

    int getSortIndex() const override
    {
        return SORTINDEX_NURBSCURVE;
    }

    NurbsCurve* reverseImpl() const override;

private:

    void appendLinearizedPoints(CoordinateSequence& seq,
                                const algorithm::CurveToLineParams& params,
                                std::size_t first) const;

    Envelope computeNurbsEnvelope() const;

    std::unique_ptr<CoordinateSequence> evaluateEndpoints() const;

    void validateConstruction() const;

    /// Move stored endpoint 0 or 1 onto a neighbor's coordinate when they
    /// differ only by rounding in the division by the weight.
    void snapEndpoint(std::size_t index, const CoordinateXY& target);

    int m_degree;
    std::vector<NurbsPoint> m_controlPoints;
    std::vector<NurbsKnot> m_knots;
    /// The knot vector u_0 .. u_{n+p}, with each value repeated.
    std::vector<double> m_expandedKnots;
    double m_startM;
    double m_endM;
};

}
}
