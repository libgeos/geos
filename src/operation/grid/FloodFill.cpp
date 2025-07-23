/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2018-2025 ISciences, LLC
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/operation/grid/FloodFill.h>
#include <geos/operation/grid/Grid.h>
#include <geos/algorithm/locate/IndexedPointInAreaLocator.h>
#include <geos/geom/Location.h>

using geos::geom::Geometry;

namespace geos::operation::grid {

FloodFill::FloodFill(const Geometry& g, const Grid<bounded_extent>& extent)
  : m_extent{ extent }
  , m_g{ g }
  , m_loc{ nullptr }
{
  m_loc = std::make_unique<algorithm::locate::IndexedPointInAreaLocator>(m_g);
}

FloodFill::~FloodFill() = default;

bool
FloodFill::cellIsInside(size_t i, size_t j) const
{
    geom::CoordinateXY p(  m_extent.getColX(j), m_extent.getRowY(i) );

    return m_loc->locate(&p) == geom::Location::INTERIOR;
}

}
