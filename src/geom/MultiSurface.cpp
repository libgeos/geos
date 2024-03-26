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

#include <geos/geom/MultiSurface.h>
#include <geos/geom/MultiCurve.h>
#include <geos/geom/GeometryFactory.h>

namespace geos {
namespace geom {

/*protected*/
MultiSurface::MultiSurface(std::vector<std::unique_ptr<Geometry>>&& newPolys, const GeometryFactory& factory)
    : GeometryCollection(std::move(newPolys), factory)
{
    // FIXME check that all elements are in fact surfaces
}

MultiSurface::MultiSurface(std::vector<std::unique_ptr<Surface>>&& newPolys, const GeometryFactory& factory)
    : GeometryCollection(std::move(newPolys), factory)
{
}

MultiSurface::~MultiSurface() {}

Dimension::DimensionType
MultiSurface::getDimension() const
{
    return Dimension::A; // area
}

int
MultiSurface::getBoundaryDimension() const
{
    return 1;
}

std::string
MultiSurface::getGeometryType() const
{
    return "MultiSurface";
}

std::unique_ptr<Geometry>
MultiSurface::getBoundary() const
{
    // FIXME implement
#if 0
    if (isEmpty()) {
        return std::unique_ptr<Geometry>(getFactory()->createMultiCurve());
    }
#endif

    std::vector<std::unique_ptr<Geometry>> allRings;
    for (const auto& pg : geometries) {
        auto g = pg->getBoundary();

        if (g->getNumGeometries() == 1) {
            allRings.push_back(std::move(g));
        }
        else {
            for (std::size_t i = 0; i < g->getNumGeometries(); ++i) {
                // TODO avoid this clone
                allRings.push_back(g->getGeometryN(i)->clone());
            }
        }
    }

    return getFactory()->createMultiCurve(std::move(allRings));
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
