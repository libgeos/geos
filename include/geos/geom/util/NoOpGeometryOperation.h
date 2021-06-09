/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2020 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/export.h>
#include <geos/geom/util/GeometryEditorOperation.h> // for inheritance

// Forward declarations
namespace geos {
namespace geom {
class Geometry;
class CoordinateSequence;
class GeometryFactory;
}
}

namespace geos {
namespace geom { // geos.geom
namespace util { // geos.geom.util


class GEOS_DLL NoOpGeometryOperation: public GeometryEditorOperation {

public:

    /**
     * Return a newly created geometry, ownership to caller
     */
    std::unique_ptr<Geometry> edit(const Geometry* geometry,
                                   const GeometryFactory* factory) override;

    ~NoOpGeometryOperation() override = default;
};



} // namespace geos.geom.util
} // namespace geos.geom
} // namespace geos

