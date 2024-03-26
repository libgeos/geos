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
#include <geos/util/UnsupportedOperationException.h>

namespace geos {
namespace geom {

class GEOS_DLL CircularString : public SimpleCurve {

public:
    using SimpleCurve::SimpleCurve;

    friend class GeometryFactory;

    ~CircularString() override;

    std::unique_ptr<CircularString> clone() const;

    std::string getGeometryType() const override;

    GeometryTypeId getGeometryTypeId() const override;

    CircularString* cloneImpl() const override
    {
        return new CircularString(*this);
    }

    CircularString* reverseImpl() const override;

    int
    getSortIndex() const override
    {
        return SORTINDEX_LINESTRING;
    };

    double getLength() const override {
        throw util::UnsupportedOperationException("Cannot calculate length of CircularString");
    }

    bool hasCurvedComponents() const override {
        return true;
    }

};


}
}
