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

#include <geos/geom/CircularArc.h>
#include <geos/geom/LineString.h>
#include <geos/geom/SimpleCurve.h>

namespace geos {
namespace geom {

/// A curve of circular arcs encoded as control points.
///
/// Empty CircularStrings are valid. A non-empty string is valid when it
/// has an odd control count of at least 3, or when it is the closed
/// four-control form CIRCULARSTRING(A, B, C, A) that defines a circumcircle.
class GEOS_DLL CircularString : public SimpleCurve {

public:
    using SimpleCurve::SimpleCurve;

    friend class GeometryFactory;

    /// Empty, odd count >= 3, or closed four-control circumcircle.
    static bool isValidControlCount(const CoordinateSequence& seq);

    ~CircularString() override;

    std::unique_ptr<CircularString> clone() const;

    const std::vector<CircularArc>& getArcs() const;

    std::string getGeometryType() const override;

    GeometryTypeId getGeometryTypeId() const override;

    double getLength() const override;

    bool hasCurvedComponents() const override
    {
        return true;
    }

    bool hasCurvedTypes() const override
    {
        return true;
    }

    bool isCurved() const override {
        return true;
    }

    void normalize() override;

    std::unique_ptr<CircularString> reverse() const
    {
        return std::unique_ptr<CircularString>(reverseImpl());
    }

    std::unique_ptr<Curve> getCurved(const algorithm::LineToCurveParams&) const;

protected:

    /// \brief
    /// Constructs a CircularString taking ownership the
    /// given CoordinateSequence.
    CircularString(std::unique_ptr<CoordinateSequence>&& pts,
                   const GeometryFactory& newFactory);

    CircularString(const std::shared_ptr<const CoordinateSequence>& pts,
                   const GeometryFactory& newFactory);

    CircularString* cloneImpl() const override
    {
        return new CircularString(*this);
    }

    void geometryChangedAction() override
    {
        envelope = computeEnvelopeInternal(false);
    }

    CircularString* getCurvedImpl(const algorithm::LineToCurveParams&) const override {
        return cloneImpl();
    }

    LineString* getLinearizedImpl(const algorithm::CurveToLineParams&) const override;

    int
    getSortIndex() const override
    {
        return SORTINDEX_CIRCULARSTRING;
    };

    CircularString* reverseImpl() const override;

    void validateConstruction();

private:
    void createArcs() const;

    void normalizeClosed();

    mutable std::vector<CircularArc> arcs;

};


}
}
