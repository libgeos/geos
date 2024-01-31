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

#include <geos/geom/Curve.h>
#include <geos/geom/CurvePolygon.h>
#include <geos/geom/CoordinateSequence.h>

namespace geos {
namespace geom {

    std::unique_ptr<CoordinateSequence>
    CurvePolygon::getCoordinates() const
    {
        auto coordinates = shell->getCoordinates();
        for (const auto& hole : holes) {
            // FIXME remove unncessary copy
            coordinates->add(*hole->getCoordinates());
        }
        return coordinates;
    }


    std::string CurvePolygon::getGeometryType() const {
        return "CurvePolygon";
    }

    GeometryTypeId CurvePolygon::getGeometryTypeId() const {
        return GEOS_CURVEPOLYGON;
    }

    std::unique_ptr<Geometry>
    CurvePolygon::getBoundary() const {
        throw std::runtime_error("Not implemented.");
    }

    void
    CurvePolygon::normalize() {
        throw std::runtime_error("Not implemented.");
    }

    Geometry*
    CurvePolygon::cloneImpl() const {
        throw std::runtime_error("Not implemented.");
    }

    Geometry*
    CurvePolygon::reverseImpl() const {
        throw std::runtime_error("Not implemented.");
    }

}
}
