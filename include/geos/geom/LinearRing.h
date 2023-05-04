/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 * Copyright (C) 2005 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: geom/LinearRing.java r320 (JTS-1.12)
 *
 **********************************************************************/

#pragma once

#include <geos/export.h>
#include <string>
#include <vector>
#include <geos/geom/LineString.h>


// Forward declarations
namespace geos {
namespace geom { // geos::geom
class Coordinate;
}
}

namespace geos {
namespace geom { // geos::geom

/**
 * \brief
 * Models an OGC SFS LinearRing. A LinearRing is a LineString which is both
 * closed and simple.
 *
 * In other words, the first and last coordinate in the ring must be equal,
 * and the ring must not self-intersect.  Either orientation
 * of the ring is allowed.
 *
 * A ring must have either 0 or 3 or more points. The first and last points
 * must be equal (in 2D). If these conditions are not met, the constructors
 * throw an {@link geos::util::IllegalArgumentException}
 * A ring with 3 points is invalid, because it is collapsed
 * and thus has a self-intersection.  It is allowed to be constructed
 * so that it can be represented, and repaired if needed.
 */
class GEOS_DLL LinearRing : public LineString {

public:

    /** \brief
     * The minimum number of vertices allowed in a valid non-empty ring.
     * Empty rings with 0 vertices are also valid.
     */
    static const unsigned int MINIMUM_VALID_SIZE = 3;

    LinearRing(const LinearRing& lr);

    /**
     * \brief Constructs a LinearRing with the given points.
     *
     * @param  points  points forming a closed and simple linestring, or
     *      <code>null</code> or an empty array to create the empty
     *      geometry.
     *      This array must not contain <code>null</code> elements.
     *	If not null LinearRing will take ownership of points.
     *
     * @param newFactory the GeometryFactory used to create this geometry
     *
     */
    LinearRing(CoordinateSequence::Ptr && points,
            const GeometryFactory& newFactory);

    std::unique_ptr<LinearRing> clone() const
    {
        return std::unique_ptr<LinearRing>(cloneImpl());
    }

    ~LinearRing() override = default;

    /** \brief
     * Returns <code>Dimension.FALSE</code>, since by definition
     * LinearRings do not have a boundary.
     *
     * @return Dimension::False
     */
    int getBoundaryDimension() const override;

    bool isClosed() const override;

    std::string getGeometryType() const override;

    GeometryTypeId getGeometryTypeId() const override;

    void setPoints(const CoordinateSequence* cl);

    std::unique_ptr<LinearRing> reverse() const { return std::unique_ptr<LinearRing>(reverseImpl()); }

    void orient(bool isCW);

protected:

    int
    getSortIndex() const override
    {
        return SORTINDEX_LINEARRING;
    };

    LinearRing* cloneImpl() const override { return new LinearRing(*this); }

    LinearRing* reverseImpl() const override;

private:

    void validateConstruction();
};


} // namespace geos::geom
} // namespace geos

