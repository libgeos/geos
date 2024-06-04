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

class SimpleCurve;

class GEOS_DLL Curve : public Geometry {

public:
    using Geometry::apply_ro;
    using Geometry::apply_rw;

    void apply_ro(GeometryComponentFilter* filter) const override;

    void apply_ro(GeometryFilter* filter) const override;

    void apply_rw(GeometryComponentFilter* filter) override;

    void apply_rw(GeometryFilter* filter) override;

    /**
     * \brief
     * Returns Dimension::False for a closed Curve,
     * 0 otherwise (Curve boundary is a MultiPoint)
     */
    int
    getBoundaryDimension() const override
    {
        return isClosed() ? Dimension::False : 0;
    }

    /// Returns line dimension (1)
    Dimension::DimensionType getDimension() const override
    {
        return Dimension::L; // line
    }

    /// Returns true if the first and last coordinate in the Curve are the same
    virtual bool isClosed() const = 0;

    /// Returns true if the curve is closed and simple
    bool isRing() const;

    virtual std::size_t getNumCurves() const = 0;

    virtual const SimpleCurve* getCurveN(std::size_t) const = 0;

protected:
    Curve(const GeometryFactory& factory) : Geometry(&factory) {}

};

}
}
