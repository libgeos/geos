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

#include <geos/geom/Curve.h>
#include <geos/geom/GeometryCollection.h>

namespace geos {
namespace geom {

class GEOS_DLL MultiCurve : public GeometryCollection {
    friend class GeometryFactory;

public:
    ~MultiCurve() override = default;

    std::unique_ptr<MultiCurve> clone() const
    {
        return std::unique_ptr<MultiCurve>(cloneImpl());
    };

    /// Returns a (possibly empty) [MultiPoint](@ref geom::MultiPoint)
    std::unique_ptr<Geometry> getBoundary() const override;

    /**
     * \brief
     * Returns Dimension::False if all [Curves](@ref geom::Curve) in the collection
     * are closed, 0 otherwise.
     */
    int getBoundaryDimension() const override;

    /// Returns line dimension (1)
    Dimension::DimensionType getDimension() const override;

    const Curve* getGeometryN(std::size_t n) const override;

    std::string getGeometryType() const override;

    GeometryTypeId getGeometryTypeId() const override;

    bool hasDimension(Dimension::DimensionType d) const override
    {
        return d == Dimension::L;
    }

    /// Returns true if the MultiCurve is not empty, and every included
    /// Curve is also closed.
    bool isClosed() const;

    bool isDimensionStrict(Dimension::DimensionType d) const override
    {
        return d == Dimension::L;
    }

    /**
     * Creates a MultiCurve in the reverse
     * order to this object.
     * Both the order of the component Curves
     * and the order of their coordinate sequences
     * are reversed.
     *
     * @return a MultiCurve in the reverse order
     */
    std::unique_ptr<MultiCurve> reverse() const
    {
        return std::unique_ptr<MultiCurve>(reverseImpl());
    }

protected:

    /**
     * \brief Constructs a MultiCurve.
     *
     * @param  newLines The [Curves](@ref geom::Curve) for this
     *                  MultiCurve, or `null`
     *                  or an empty array to create the empty geometry.
     *                  Elements may be empty Curve,
     *                  but not `null`s.
     *
     * @param newFactory The GeometryFactory used to create this geometry.
     *                   Caller must keep the factory alive for the life-time
     *                   of the constructed MultiCurve.
     *
     * @note Constructed object will take ownership of
     *       the vector and its elements.
     *
     */
    MultiCurve(std::vector<std::unique_ptr<Curve>>&& newLines,
               const GeometryFactory& newFactory);

    MultiCurve(std::vector<std::unique_ptr<Geometry>>&& newLines,
               const GeometryFactory& newFactory);

    MultiCurve(const MultiCurve& mp)
        : GeometryCollection(mp)
    {}

    MultiCurve* cloneImpl() const override
    {
        return new MultiCurve(*this);
    }

    MultiCurve* reverseImpl() const override;

    int
    getSortIndex() const override
    {
        return SORTINDEX_MULTICURVE;
    };

};

}
}
