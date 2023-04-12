/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2011 Sandro Santilli <strk@kbt.io>
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
 * Last port: geom/Point.java r320 (JTS-1.12)
 *
 **********************************************************************/

#pragma once

#include <geos/export.h>
#include <geos/geom/Geometry.h> // for inheritance
#include <geos/geom/CoordinateSequence.h> // for proper use of unique_ptr<>
#include <geos/geom/Envelope.h> // for proper use of unique_ptr<>
#include <geos/geom/Dimension.h> // for Dimension::DimensionType

#include <string>
#include <vector>
#include <memory> // for unique_ptr

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251) // warning C4251: needs to have dll-interface to be used by clients of class
#endif

// Forward declarations
namespace geos {
namespace geom { // geos::geom
class Coordinate;
class CoordinateFilter;
class CoordinateSequenceFilter;
class GeometryComponentFilter;
class GeometryFilter;
}
}

namespace geos {
namespace geom { // geos::geom

/**
 * Implementation of Point.
 *
 * A Point is valid iff:
 *
 * - the coordinate which defines it is a valid coordinate
 *   (i.e does not have an NaN X or Y ordinate)
 *
 */
class GEOS_DLL Point : public Geometry {

public:

    friend class GeometryFactory;

    /// A vector of const Point pointers
    typedef std::vector<const Point*> ConstVect;

    ~Point() override = default;

    /**
     * Creates and returns a full copy of this {@link Point} object.
     * (including all coordinates contained by it).
     *
     * @return a clone of this instance
     */
    std::unique_ptr<Point> clone() const
    {
        return std::unique_ptr<Point>(cloneImpl());
    }

    std::unique_ptr<CoordinateSequence> getCoordinates(void) const override;

    const CoordinateSequence* getCoordinatesRO() const;

    std::size_t getNumPoints() const override;
    bool isEmpty() const override;
    bool isSimple() const override;

    /// Returns point dimension (0)
    Dimension::DimensionType getDimension() const override;

    /// Returns coordinate dimension.
    uint8_t getCoordinateDimension() const override;

    bool hasM() const override;

    bool hasZ() const override;

    /// Returns Dimension::False (Point has no boundary)
    int getBoundaryDimension() const override;

    /**
     * Gets the boundary of this geometry.
     * Zero-dimensional geometries have no boundary by definition,
     * so an empty GeometryCollection is returned.
     *
     * @return an empty GeometryCollection
     * @see Geometry::getBoundary
     */
    std::unique_ptr<Geometry> getBoundary() const override;

    void setXY(double x, double y) {
        if (isEmpty()) {
            coordinates.add(x, y);
        } else {
            CoordinateXY& prev = coordinates.front<CoordinateXY>();
            prev.x = x;
            prev.y = y;
        }
        geometryChangedAction();
    }

    const CoordinateXY* getCoordinate() const override {
        return isEmpty() ? nullptr : &coordinates.getAt<CoordinateXY>(0);
    }

    double getX() const;
    double getY() const;
    double getZ() const;
    double getM() const;

    std::string getGeometryType() const override;
    GeometryTypeId getGeometryTypeId() const override;
    void apply_ro(CoordinateFilter* filter) const override;
    void apply_rw(const CoordinateFilter* filter) override;
    void apply_ro(GeometryFilter* filter) const override;
    void apply_rw(GeometryFilter* filter) override;
    void apply_rw(GeometryComponentFilter* filter) override;
    void apply_ro(GeometryComponentFilter* filter) const override;
    void apply_rw(CoordinateSequenceFilter& filter) override;
    void apply_ro(CoordinateSequenceFilter& filter) const override;

    bool equalsExact(const Geometry* other, double tolerance = 0) const override;

    bool equalsIdentical(const Geometry* other) const override;

    void
    normalize(void) override
    {
        // a Point is always in normalized form
    }

    std::unique_ptr<Point> reverse() const
    {
        return std::unique_ptr<Point>(reverseImpl());
    }

    const Envelope* getEnvelopeInternal() const override {
        return &envelope;
    }

protected:

    /**
     * \brief
     * Creates a Point taking ownership of the given CoordinateSequence
     * (must have 1 element)
     *
     * @param  newCoords
     *	contains the single coordinate on which to base this
     *	<code>Point</code> or <code>null</code> to create
     *	the empty geometry.
     *
     * @param newFactory the GeometryFactory used to create this geometry
     */
    Point(CoordinateSequence&& newCoords, const GeometryFactory* newFactory);

    Point(const Coordinate& c, const GeometryFactory* newFactory);

    Point(const CoordinateXY& c, const GeometryFactory* newFactory);

    Point(const CoordinateXYM& c, const GeometryFactory* newFactory);

    Point(const CoordinateXYZM& c, const GeometryFactory* newFactory);

    Point(const Point& p);

    Point* cloneImpl() const override { return new Point(*this); }

    Point* reverseImpl() const override { return new Point(*this); }

    Envelope computeEnvelopeInternal() const;

    int compareToSameClass(const Geometry* p) const override;

    int
    getSortIndex() const override
    {
        return SORTINDEX_POINT;
    };

    void geometryChangedAction() override {
        envelope = computeEnvelopeInternal();
    }

private:

    CoordinateSequence coordinates;
    Envelope envelope;
};

} // namespace geos::geom
} // namespace geos


#ifdef _MSC_VER
#pragma warning(pop)
#endif

