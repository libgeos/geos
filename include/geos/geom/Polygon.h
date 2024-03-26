/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2011 Sandro Santilli <strk@kbt.io>
 * Copyright (C) 2005 2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: geom/Polygon.java r320 (JTS-1.12)
 *
 **********************************************************************/

#pragma once

#include <geos/export.h>
#include <string>
#include <vector>
#include <geos/geom/Geometry.h> // for inheritance
#include <geos/geom/Envelope.h> // for proper use of unique_ptr<>
#include <geos/geom/LinearRing.h>
#include <geos/geom/Dimension.h> // for Dimension::DimensionType
#include <geos/geom/SurfaceImpl.h>

#include <memory> // for unique_ptr

// Forward declarations
namespace geos {
namespace geom { // geos::geom
class Coordinate;
class CoordinateSequenceFilter;
class LineString;
}
}

namespace geos {
namespace geom { // geos::geom

/**
 * \class Polygon geom.h geos.h
 *
 * \brief Represents a linear polygon, which may include holes.
 *
 * The shell and holes of the polygon are represented by {@link LinearRing}s.
 * In a valid polygon, holes may touch the shell or other holes at a single point.
 * However, no sequence of touching holes may split the polygon into two pieces.
 * The orientation of the rings in the polygon does not matter.
 * <p>
 *  The shell and holes must conform to the assertions specified in the <A
 *  HREF="http://www.opengis.org/techno/specs.htm">OpenGIS Simple Features
 *  Specification for SQL</A> .
 *
 */
class GEOS_DLL Polygon: public SurfaceImpl<LinearRing> {

public:

    friend class GeometryFactory;

    /// A vector of const Polygon pointers
    typedef std::vector<const Polygon*> ConstVect;

    ~Polygon() override = default;

    std::unique_ptr<CoordinateSequence>
    getCoordinates() const override;

    /**
     * Creates and returns a full copy of this {@link Polygon} object.
     * (including all coordinates contained by it).
     *
     * @return a clone of this instance
     */
    std::unique_ptr<Polygon> clone() const
    {
        return std::unique_ptr<Polygon>(cloneImpl());
    }

    /** \brief
     * Computes the boundary of this geometry
     *
     * @return a lineal geometry (which may be empty)
     * @see Geometry#getBoundary
     */
    std::unique_ptr<Geometry> getBoundary() const override;

    std::string getGeometryType() const override;
    GeometryTypeId getGeometryTypeId() const override;

    void normalize() override;

    std::unique_ptr<Polygon> reverse() const { return std::unique_ptr<Polygon>(reverseImpl()); }

    double getArea() const override;

    bool isRectangle() const override;

    /**
    * \brief
    * Apply a ring ordering convention to this polygon, with
    * interior rings having an opposite orientation to the
    * specified exterior orientation.
    *
    * \param exteriorCW should exterior ring be clockwise?
    */
    void orientRings(bool exteriorCW);

protected:

    using SurfaceImpl::SurfaceImpl;

    Polygon* cloneImpl() const override { return new Polygon(*this); }

    Polygon* reverseImpl() const override;

    int
    getSortIndex() const override
    {
        return SORTINDEX_POLYGON;
    };


private:

    void normalize(LinearRing* ring, bool clockwise);

};

} // namespace geos::geom
} // namespace geos

