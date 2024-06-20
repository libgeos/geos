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
 * Last port: geom/LineString.java r320 (JTS-1.12)
 *
 **********************************************************************/

#pragma once

#include <geos/export.h>
#include <geos/geom/Geometry.h> // for inheritance
#include <geos/geom/CoordinateSequence.h> // for proper use of unique_ptr<>
#include <geos/geom/Envelope.h> // for proper use of unique_ptr<>
#include <geos/geom/Dimension.h> // for Dimension::DimensionType
#include <geos/geom/SimpleCurve.h>

#include <string>
#include <vector>
#include <memory> // for unique_ptr


#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251) // warning C4251: needs to have dll-interface to be used by clients of class
#endif

namespace geos {
namespace geom {
class Coordinate;
class CoordinateSequenceFilter;
}
}

namespace geos {
namespace geom { // geos::geom

/**
 *  Models an OGC-style <code>LineString</code>.
 *
 *  A LineString consists of a sequence of two or more vertices,
 *  along with all points along the linearly-interpolated curves
 *  (line segments) between each
 *  pair of consecutive vertices.
 *  Consecutive vertices may be equal.
 *  The line segments in the line may intersect each other (in other words,
 *  the linestring may "curl back" in itself and self-intersect).
 *  Linestrings with exactly two identical points are invalid.
 *
 *  A linestring must have either 0 or 2 or more points.
 *  If these conditions are not met, the constructors throw
 *  an {@link util::IllegalArgumentException}.
 */
class GEOS_DLL LineString: public SimpleCurve {

public:

    friend class GeometryFactory;

    /// A vector of const LineString pointers
    typedef std::vector<const LineString*> ConstVect;

    ~LineString() override;

    /**
     * \brief
     * Creates and returns a full copy of this {@link LineString} object
     * (including all coordinates contained by it)
     *
     * @return A clone of this instance
     */
    std::unique_ptr<LineString> clone() const
    {
        return std::unique_ptr<LineString>(cloneImpl());
    }

    std::string getGeometryType() const override;

    GeometryTypeId getGeometryTypeId() const override;

    double getLength() const override;

    bool isCurved() const override {
        return false;
    }

    /**
     * Creates a LineString whose coordinates are in the reverse
     * order of this object's
     *
     * @return a LineString with coordinates in the reverse order
     */
    std::unique_ptr<LineString> reverse() const { return std::unique_ptr<LineString>(reverseImpl()); }

protected:

    LineString(const LineString& ls);

    /// \brief
    /// Constructs a LineString taking ownership the
    /// given CoordinateSequence.
    LineString(CoordinateSequence::Ptr && pts,
               const GeometryFactory& newFactory);

    LineString* cloneImpl() const override { return new LineString(*this); }

    LineString* reverseImpl() const override;

    int
    getSortIndex() const override
    {
        return SORTINDEX_LINESTRING;
    };

    void geometryChangedAction() override
    {
        envelope = computeEnvelopeInternal(true);
    }

private:

    void validateConstruction();
};

struct GEOS_DLL  LineStringLT {
    bool
    operator()(const LineString* ls1, const LineString* ls2) const
    {
        return ls1->compareTo(ls2) < 0;
    }
};

} // namespace geos::geom
} // namespace geos

#ifdef _MSC_VER
#pragma warning(pop)
#endif

