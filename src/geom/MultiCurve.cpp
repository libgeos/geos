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
#include <geos/operation/BoundaryOp.h>
#include <geos/util.h>

namespace geos {
namespace geom {

MultiCurve::MultiCurve(std::vector<std::unique_ptr<Geometry>>&& newLines,
                       const GeometryFactory& factory)
    : GeometryCollection(std::move(newLines), factory)
{
    for (const auto& geom : geometries) {
        if (!dynamic_cast<const Curve*>(geom.get())) {
            throw util::IllegalArgumentException("All elements of MultiCurve must be a Curve");
        }
    }
}

MultiCurve::MultiCurve(std::vector<std::unique_ptr<Curve>>&& newLines,
                       const GeometryFactory& factory)
    : GeometryCollection(std::move(newLines), factory)
{}

std::unique_ptr<Geometry>
MultiCurve::getBoundary() const
{
    operation::BoundaryOp bop(*this);
    return bop.getBoundary();
}

int
MultiCurve::getBoundaryDimension() const
{
    if (isClosed()) {
        return Dimension::False;
    }
    return 0;
}

Dimension::DimensionType
MultiCurve::getDimension() const
{
    return Dimension::L; // line
}

const Curve*
MultiCurve::getGeometryN(std::size_t i) const
{
    return static_cast<const Curve*>(geometries[i].get());
}

std::string
MultiCurve::getGeometryType() const
{
    return "MultiCurve";
}

GeometryTypeId
MultiCurve::getGeometryTypeId() const
{
    return GEOS_MULTICURVE;
}

MultiLineString*
MultiCurve::getLinearizedImpl(double stepSizeDegrees) const
{
    std::vector<std::unique_ptr<Geometry>> lines(geometries.size());

    for (std::size_t i = 0; i < geometries.size(); i++) {
        lines[i] = geometries[i]->getLinearized(stepSizeDegrees);
    }

    return getFactory()->createMultiLineString(std::move(lines)).release();
}

bool
MultiCurve::isClosed() const
{
    if (isEmpty()) {
        return false;
    }
    for (const auto& g : geometries) {
        const Curve* ls = detail::down_cast<const Curve*>(g.get());
        if (! ls->isClosed()) {
            return false;
        }
    }
    return true;
}

MultiCurve*
MultiCurve::reverseImpl() const
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

    return getFactory()->createMultiCurve(std::move(reversed)).release();
}

}
}
