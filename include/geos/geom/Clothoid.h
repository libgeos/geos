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
#include <geos/geom/Coordinate.h>
#include <geos/geom/LineString.h>
#include <geos/geom/SimpleCurve.h>

namespace geos {
namespace algorithm {
namespace clothoid {
struct Parameters;
}
}
namespace geom {

/// Affine reference location: origin and the images of the constructive axes.
struct GEOS_DLL ClothoidReferenceLocation {
    Coordinate origin;
    Coordinate u;
    Coordinate v;
};

/**
 * An ST_Clothoid (ISO/IEC 13249-3).
 *
 * Stored attributes are an affine reference location, a scale factor, and
 * the start and end constructive distances from the inflection. Measure,
 * when present, is a STARTM/ENDM pair.
 *
 * In the constructive frame, theta(t) = t^2 / (2 A^2) and kappa(t) = t / A^2.
 * A negative scale factor is stored and evaluated as |A|.
 */
class GEOS_DLL Clothoid : public SimpleCurve {

public:

    friend class GeometryFactory;
    friend class CompoundCurve;

    ~Clothoid() override = default;

    std::unique_ptr<Clothoid> clone() const;

    std::string getGeometryType() const override;

    GeometryTypeId getGeometryTypeId() const override;

    double getLength() const override;

    double getStartHeading() const;

    double getStartCurvature() const;

    double getEndCurvature() const;

    double getEndHeading() const;

    double getScaleFactor() const
    {
        return m_scale;
    }

    double getStartDistance() const
    {
        return m_t0;
    }

    double getEndDistance() const
    {
        return m_t1;
    }

    double getStartM() const
    {
        return m_startM;
    }

    double getEndM() const
    {
        return m_endM;
    }

    ClothoidReferenceLocation getReferenceLocation() const;

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

    std::unique_ptr<Clothoid> reverse() const
    {
        return std::unique_ptr<Clothoid>(reverseImpl());
    }

    /// apply_rw throws; the reference frame is not a coordinate sequence.
    void apply_rw(const CoordinateFilter* filter) override;

    void addLinearizedPoints(CoordinateSequence& seq,
                             const algorithm::CurveToLineParams& params) const override;

    void apply_rw(CoordinateSequenceFilter& filter) override;

    bool equalsExact(const Geometry* other, double tolerance = 0) const override;

    bool equalsIdentical(const Geometry* other) const override;

protected:

    Clothoid(const ClothoidReferenceLocation& location,
             double scaleFactor,
             double startDistance,
             double endDistance,
             bool hasZ, bool hasM,
             double startM, double endM,
             const GeometryFactory& factory);

    Clothoid(bool hasZ, bool hasM, const GeometryFactory& factory);

    Clothoid(const Clothoid& other) = default;

    Clothoid* cloneImpl() const override
    {
        return new Clothoid(*this);
    }

    int compareToSameClass(const Geometry* geom) const override;

    void geometryChangedAction() override
    {
        envelope = computeClothoidEnvelope();
    }

    Clothoid* getCurvedImpl(const algorithm::LineToCurveParams&) const override
    {
        return cloneImpl();
    }

    LineString* getLinearizedImpl(const algorithm::CurveToLineParams& params) const override;

    int getSortIndex() const override
    {
        return SORTINDEX_CLOTHOID;
    }

    Clothoid* reverseImpl() const override;

private:

    void appendLinearizedPoints(CoordinateSequence& seq,
                                const algorithm::CurveToLineParams& params,
                                std::size_t first) const;

    Envelope computeClothoidEnvelope() const;

    void validateConstruction() const;

    double travelHeading(double constructiveDistance) const;

    double travelCurvature(double constructiveDistance) const;

    /// Move stored endpoint 0 or 1 onto a neighbor's coordinate when they
    /// differ only by rounding in the evaluation of the reference location.
    void snapEndpoint(std::size_t index, const CoordinateXY& target);

    Coordinate m_origin;
    Coordinate m_u;
    Coordinate m_v;
    double m_scale;
    double m_t0;
    double m_t1;
    double m_startM;
    double m_endM;
};

}
}
