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
    double x;
    double y;
    double z;
    double ux;
    double uy;
    double uz;
    double vx;
    double vy;
    double vz;
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

    ~Clothoid() override = default;

    std::unique_ptr<Clothoid> clone() const;

    std::string getGeometryType() const override;

    GeometryTypeId getGeometryTypeId() const override;

    double getLength() const override;

    double getStartTangent() const;

    double getStartCurvature() const;

    double getEndCurvature() const;

    double getEndTangent() const;

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

    std::unique_ptr<Geometry> convexHull() const override;

    void apply_rw(const CoordinateFilter* filter) override;

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

    void loadConstructive(algorithm::clothoid::Parameters& curve) const;

    void pointOnCurve(double travel, double& x, double& y, double& z, double& m) const;

    Envelope computeClothoidEnvelope() const;

    void validateConstruction() const;

    double travelHeading(double constructiveDistance) const;

    double travelCurvature(double constructiveDistance) const;

    double m_locX;
    double m_locY;
    double m_locZ;
    double m_ux;
    double m_uy;
    double m_uz;
    double m_vx;
    double m_vy;
    double m_vz;
    double m_scale;
    double m_t0;
    double m_t1;
    double m_startM;
    double m_endM;
};

}
}
