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

class GEOS_DLL CircularString : public SimpleCurve {

public:
    using SimpleCurve::SimpleCurve;

    friend class GeometryFactory;

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

    bool isCurved() const override {
        return true;
    }

    std::unique_ptr<CircularString> reverse() const
    {
        return std::unique_ptr<CircularString>(reverseImpl());
    }

    std::unique_ptr<Curve> getCurved(double distanceTolerance) const;

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

    CircularString* getCurvedImpl(double) const override {
        return cloneImpl();
    }

    LineString* getLinearizedImpl(double) const override;

    int
    getSortIndex() const override
    {
        return SORTINDEX_LINESTRING;
    };

    CircularString* reverseImpl() const override;

    void validateConstruction();

private:
    void createArcs() const;

    mutable std::vector<CircularArc> arcs;

};


}
}
