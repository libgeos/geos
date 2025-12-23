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

#include <geos/geom/SurfaceImpl.h>
#include <geos/geom/Polygon.h>

namespace geos {
namespace geom {

class GEOS_DLL CurvePolygon : public SurfaceImpl<Curve> {
    friend class GeometryFactory;

public:
    ~CurvePolygon() override = default;

    double getArea() const override;

    std::unique_ptr<Geometry> getBoundary() const override;

    std::unique_ptr<CoordinateSequence> getCoordinates() const override;

    std::string getGeometryType() const override;

    GeometryTypeId getGeometryTypeId() const override;

    std::unique_ptr<CurvePolygon> getCurved(double) {
        return std::unique_ptr<CurvePolygon>(cloneImpl());
    }

    std::unique_ptr<Polygon> getLinearized(double degreeSpacing) const;

    bool hasCurvedComponents() const override;

    void normalize() override;

protected:
    using SurfaceImpl::SurfaceImpl;

    CurvePolygon* cloneImpl() const override;

    Polygon* getLinearizedImpl(double degreeSpacing) const override;

    CurvePolygon* getCurvedImpl(double) const override { return cloneImpl(); }

    int
    getSortIndex() const override
    {
        return SORTINDEX_CURVEPOLYGON;
    }

    Geometry* reverseImpl() const override;
};


}
}
