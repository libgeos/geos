/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 * Copyright (C) 2005 2006 Refractions Research Inc.
 * Copyright (C) 2024 ISciences, LLC
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/geom/GeometryFactory.h>
#include <geos/geom/MultiCurve.h>
#include <geos/geom/MultiSurface.h>

namespace geos {
namespace geom {

MultiSurface::MultiSurface(std::vector<std::unique_ptr<Geometry>>&& newPolys, const GeometryFactory& factory)
    : GeometryCollection(std::move(newPolys), factory)
{
    for (const auto& geom : geometries) {
        if (!dynamic_cast<const Surface*>(geom.get())) {
            throw util::IllegalArgumentException("All elements of MultiSurface must be a Surface");
        }
    }
}

MultiSurface::MultiSurface(std::vector<std::unique_ptr<Surface>>&& newPolys, const GeometryFactory& factory)
    : GeometryCollection(std::move(newPolys), factory)
{
}

MultiSurface::~MultiSurface() {}

std::unique_ptr<Geometry>
MultiSurface::getBoundary() const
{
    if (isEmpty()) {
        return std::unique_ptr<Geometry>(getFactory()->createMultiCurve());
    }

    std::vector<std::unique_ptr<Geometry>> allRings;
    for (const auto& pg : geometries) {
        auto g = pg->getBoundary();

        if (g->getNumGeometries() == 1) {
            allRings.push_back(std::move(g));
        }
        else {
            for (auto& gi : (static_cast<GeometryCollection&>(*g)).releaseGeometries()) {
                allRings.push_back(std::move(gi));
            }
        }
    }

    return getFactory()->createMultiCurve(std::move(allRings));
}

int
MultiSurface::getBoundaryDimension() const
{
    return 1;
}

Dimension::DimensionType
MultiSurface::getDimension() const
{
    return Dimension::A; // area
}

std::string
MultiSurface::getGeometryType() const
{
    return "MultiSurface";
}

GeometryTypeId
MultiSurface::getGeometryTypeId() const
{
    return GEOS_MULTISURFACE;
}

MultiSurface*
MultiSurface::reverseImpl() const
{
    if (isEmpty()) {
        return clone().release();
    }

    std::vector<std::unique_ptr<Geometry>> reversed(geometries.size());

    std::transform(geometries.begin(),
                   geometries.end(),
                   reversed.begin(),
    [](const std::unique_ptr<Geometry>& g) {
        return g->reverse();
    });

    return getFactory()->createMultiSurface(std::move(reversed)).release();
}

}
}
