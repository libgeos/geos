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

#include <geos/geom/Geometry.h>

namespace geos {
namespace geom {

class GEOS_DLL Curve : public Geometry {

public:
    /// Returns line dimension (1)
    Dimension::DimensionType getDimension() const override
    {
        return Dimension::L; // line
    }

    int
    getBoundaryDimension() const override
    {
        return isClosed() ? Dimension::False : 0;
    }

    virtual bool isClosed() const = 0;

protected:

    Curve(const GeometryFactory& factory) : Geometry(&factory) {}

};

}
}
