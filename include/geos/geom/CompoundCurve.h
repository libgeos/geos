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

#include <geos/geom/SimpleCurve.h>
#include <geos/util.h>
#include <vector>

namespace geos {
namespace geom {

class GEOS_DLL CompoundCurve : public Curve {
    friend class GeometryFactory;

public:
    using Curve::apply_ro;
    using Curve::apply_rw;

    void apply_ro(CoordinateFilter* filter) const override;

    void apply_ro(CoordinateSequenceFilter& filter) const override;

    void apply_rw(CoordinateSequenceFilter& filter) override;

    void apply_rw(const CoordinateFilter* filter) override;

    int compareToSameClass(const Geometry* geom) const override;

    std::unique_ptr<CompoundCurve> clone() const;

    bool equalsExact(const Geometry* other, double tolerance = 0)
    const override;

    bool equalsIdentical(const Geometry* other) const override;

    std::unique_ptr<Geometry> getBoundary() const override;

    const CoordinateXY* getCoordinate() const override;

    uint8_t getCoordinateDimension() const override;

    std::unique_ptr<CoordinateSequence> getCoordinates() const override;

    /// Returns the nth section of the CompoundCurve
    const SimpleCurve* getCurveN(std::size_t) const override;

    const Envelope* getEnvelopeInternal() const override
    {
        return &envelope;
    }

    std::string getGeometryType() const override;

    GeometryTypeId getGeometryTypeId() const override;

    double getLength() const override;

    /// Returns the number of sections in the CompoundCurve
    std::size_t getNumCurves() const override;

    std::size_t getNumPoints() const override;

    bool hasCurvedComponents() const override;

    bool hasM() const override;

    bool hasZ() const override;

    bool isClosed() const override;

    bool isEmpty() const override;

    void normalize() override;

    std::unique_ptr<CompoundCurve> reverse() const;

    void validateConstruction() const;

protected:
    /// Construct a CompoundCurve, taking ownership of the
    /// provided CoordinateSequence
    CompoundCurve(std::vector<std::unique_ptr<SimpleCurve>>&&,
                  const GeometryFactory&);

    CompoundCurve(const CompoundCurve&);

    CompoundCurve& operator=(const CompoundCurve&);

    CompoundCurve* cloneImpl() const override;

    Envelope computeEnvelopeInternal() const;

    void geometryChangedAction() override
    {
        envelope = computeEnvelopeInternal();
    }

    int getSortIndex() const override
    {
        return SORTINDEX_COMPOUNDCURVE;
    }

    CompoundCurve* reverseImpl() const override;

private:
    std::vector<std::unique_ptr<SimpleCurve>> curves;
    Envelope envelope;
};

}
}
