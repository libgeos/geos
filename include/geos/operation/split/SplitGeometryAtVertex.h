/**********************************************************************
*
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2026 ISciences, LLC
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/export.h>
#include <memory>
#include <utility>

// Forward declarations
namespace geos::geom {
class CircularString;
class LineString;
class SimpleCurve;
}

namespace geos::operation::split {

class GEOS_DLL SplitGeometryAtVertex {

public:
    static std::pair<std::unique_ptr<geom::SimpleCurve>, std::unique_ptr<geom::SimpleCurve>>
            splitSimpleCurveAtVertex(const geom::SimpleCurve& sc, std::size_t i);

    static std::pair<std::unique_ptr<geom::LineString>, std::unique_ptr<geom::LineString>>
            splitLineStringAtVertex(const geom::LineString& ls, std::size_t i);

    static std::pair<std::unique_ptr<geom::CircularString>, std::unique_ptr<geom::CircularString>>
            splitCircularStringAtVertex(const geom::CircularString& cs, std::size_t i);
};

}
