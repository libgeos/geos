/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2021 Daniel Baston
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/operation/union/UnionStrategy.h>
#include <geos/geom/Geometry.h>

namespace geos {
namespace operation {
namespace geounion {

std::unique_ptr<geom::Geometry> UnionStrategy::Union(std::unique_ptr<geom::Geometry> && g0, std::unique_ptr<geom::Geometry> && g1) {
    // Default implementation just copies the inputs.
    return Union(g0.get(), g1.get());
}

}
}
}