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
class GEOS_DLL Polygon: public Geometry {

public:

    friend class GeometryFactory;

    /// A vector of const Polygon pointers
    typedef std::vector<const Polygon*> ConstVect;

    ~Polygon() override = default;

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

    std::unique_ptr<CoordinateSequence> getCoordinates() const override;

    std::size_t getNumPoints() const override;

    /// Returns surface dimension (2)
    Dimension::DimensionType getDimension() const override;

    /// Returns coordinate dimension.
    uint8_t getCoordinateDimension() const override;

    bool hasM() const override;

    bool hasZ() const override;

    /// Returns 1 (Polygon boundary is a MultiLineString)
    int getBoundaryDimension() const override;

    /** \brief
     * Computes the boundary of this geometry
     *
     * @return a lineal geometry (which may be empty)
     * @see Geometry#getBoundary
     */
    std::unique_ptr<Geometry> getBoundary() const override;

    bool isEmpty() const override;

    /// Returns the exterior ring (shell)
    const LinearRing* getExteriorRing() const;

    /**
     * \brief
     * Take ownership of this Polygon's exterior ring.
     * After releasing the exterior ring, the Polygon should be
     * considered in a moved-from state and should not be accessed,
     * except to release the interior rings (if desired.)
     * @return exterior ring
     */
    std::unique_ptr<LinearRing> releaseExteriorRing();

    /// Returns number of interior rings (hole)
    std::size_t getNumInteriorRing() const;

    /// Get nth interior ring (hole)
    const LinearRing* getInteriorRingN(std::size_t n) const;

    /**
     * \brief
     * Take ownership of this Polygon's interior rings.
     * After releasing the rings, the Polygon should be
     * considered in a moved-from state and should not be accessed,
     * except to release the exterior ring (if desired.)
     * @return vector of rings (may be empty)
     */
    std::vector<std::unique_ptr<LinearRing>> releaseInteriorRings();

    std::string getGeometryType() const override;
    GeometryTypeId getGeometryTypeId() const override;
    bool equalsExact(const Geometry* other, double tolerance = 0) const override;
    bool equalsIdentical(const Geometry* other) const override;
    void apply_rw(const CoordinateFilter* filter) override;
    void apply_ro(CoordinateFilter* filter) const override;
    void apply_rw(GeometryFilter* filter) override;
    void apply_ro(GeometryFilter* filter) const override;
    void apply_rw(CoordinateSequenceFilter& filter) override;
    void apply_ro(CoordinateSequenceFilter& filter) const override;
    void apply_rw(GeometryComponentFilter* filter) override;
    void apply_ro(GeometryComponentFilter* filter) const override;

    std::unique_ptr<Geometry> convexHull() const override;

    void normalize() override;

    /**
     * \brief
     * Apply a ring ordering convention to this polygon, with
     * interior rings having an opposite orientation to the
     * specified exterior orientation.
     *
     * \param exteriorCW should exterior ring be clockwise?
     */
    void orientRings(bool exteriorCW);

    std::unique_ptr<Polygon> reverse() const { return std::unique_ptr<Polygon>(reverseImpl()); }

    const CoordinateXY* getCoordinate() const override;

    double getArea() const override;

    /// Returns the perimeter of this <code>Polygon</code>
    double getLength() const override;

    bool isRectangle() const override;

    const Envelope* getEnvelopeInternal() const override {
        return shell->getEnvelopeInternal();
    }

protected:


    Polygon(const Polygon& p);

    int compareToSameClass(const Geometry* p) const override;

    /**
     * Constructs a <code>Polygon</code> with the given exterior
     * and interior boundaries.
     *
     * @param  newShell  the outer boundary of the new Polygon,
     *                   or <code>null</code> or an empty
     *		     LinearRing if the empty geometry
     *                   is to be created.
     *
     * @param  newHoles  the LinearRings defining the inner
     *                   boundaries of the new Polygon, or
     *                   null or empty LinearRing
     *                   if the empty  geometry is to be created.
     *
     * @param newFactory the GeometryFactory used to create this geometry
     *
     * Polygon will take ownership of Shell and Holes LinearRings
     */
    Polygon(std::unique_ptr<LinearRing> && newShell,
            std::vector<std::unique_ptr<LinearRing>> && newHoles,
            const GeometryFactory& newFactory);

    Polygon(std::unique_ptr<LinearRing> && newShell,
            const GeometryFactory& newFactory);

    Polygon* cloneImpl() const override { return new Polygon(*this); }

    Polygon* reverseImpl() const override;

    std::unique_ptr<LinearRing> shell;

    std::vector<std::unique_ptr<LinearRing>> holes;

    void geometryChangedAction() override {}

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

