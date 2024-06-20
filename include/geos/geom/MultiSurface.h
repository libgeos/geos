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

#include <geos/geom/GeometryCollection.h>
#include <geos/geom/Surface.h>

namespace geos {
namespace geom {
class GEOS_DLL MultiSurface : public GeometryCollection {
    friend class GeometryFactory;

public:

    ~MultiSurface() override;

    std::unique_ptr<MultiSurface> clone() const
    {
        return std::unique_ptr<MultiSurface>(cloneImpl());
    };

    /** \brief
     * Computes the boundary of this geometry
     *
     * @return a lineal geometry (which may be empty)
     * @see Geometry#getBoundary
     */
    std::unique_ptr<Geometry> getBoundary() const override;

    /// Returns 1 (MultiSurface boundary is MultiCurve)
    int getBoundaryDimension() const override;

    /// Returns surface dimension (2)
    Dimension::DimensionType getDimension() const override;

    std::string getGeometryType() const override;

    GeometryTypeId getGeometryTypeId() const override;

    bool hasDimension(Dimension::DimensionType d) const override
    {
        return d == Dimension::A;
    }

    bool isDimensionStrict(Dimension::DimensionType d) const override
    {
        return d == Dimension::A;
    }

    std::unique_ptr<MultiSurface> reverse() const
    {
        return std::unique_ptr<MultiSurface>(reverseImpl());
    }

protected:

    MultiSurface(std::vector<std::unique_ptr<Geometry>>&& newPolys,
                 const GeometryFactory& newFactory);

    MultiSurface(std::vector<std::unique_ptr<Surface>>&& newPolys,
                 const GeometryFactory& newFactory);

    MultiSurface(const MultiSurface& mp)
        : GeometryCollection(mp)
    {};

    MultiSurface* cloneImpl() const override
    {
        return new MultiSurface(*this);
    }

    int
    getSortIndex() const override
    {
        return SORTINDEX_MULTISURFACE;
    };

    MultiSurface* reverseImpl() const override;

};
}
}
