/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2024 ISciences, LLC
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/geom/Geometry.h>

namespace geos {
namespace geom {

class Curve;

class GEOS_DLL Surface : public Geometry {

private:

protected:
    using Geometry::Geometry;

public:

    uint8_t getCoordinateDimension() const override;

    /// Returns the exterior ring (shell)
    virtual const Curve* getExteriorRing() const = 0;

    /// Returns number of interior rings (holes)
    virtual size_t getNumInteriorRing() const = 0;

    /// Get nth interior ring (hole)
    virtual const Curve* getInteriorRingN(std::size_t n) const = 0;

    size_t getNumPoints() const override;


    bool
    equalsExact(const Geometry* other, double tolerance) const override;

    bool
    equalsIdentical(const Geometry* other) const override;


    std::unique_ptr<Geometry> convexHull() const override;

    int
    getBoundaryDimension() const override
    {
        return 1;
    }

    const CoordinateXY* getCoordinate() const override;

    Dimension::DimensionType
    getDimension() const override
    {
        return Dimension::A; // area
    }

    const Envelope* getEnvelopeInternal() const override;

    bool hasM() const override;

    bool hasZ() const override;

    bool isEmpty() const override;

    void
    apply_ro(CoordinateFilter* filter) const override;

    void
    apply_rw(const CoordinateFilter* filter) override;

    void
    apply_rw(GeometryFilter* filter) override;

    void
    apply_ro(GeometryFilter* filter) const override;

    void
    apply_ro(GeometryComponentFilter* filter) const override;

    void
    apply_rw(GeometryComponentFilter* filter) override;

    void
    apply_rw(CoordinateSequenceFilter& filter) override;

    void
    apply_ro(CoordinateSequenceFilter& filter) const override;

protected:

    int
    compareToSameClass(const Geometry* g) const override;

    void geometryChangedAction() override {}

    static std::unique_ptr<Geometry> createEmptyRing(const GeometryFactory&);

    virtual Curve* getExteriorRing() = 0;

    virtual Curve* getInteriorRingN(std::size_t i) = 0;

};

}
}
