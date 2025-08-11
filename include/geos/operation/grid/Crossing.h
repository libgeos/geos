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

#pragma once

#include <geos/geom/Coordinate.h>
#include <geos/operation/grid/Side.h>

namespace geos::operation::grid {

class Crossing
{
  public:
    Crossing(Side s, double x, double y)
      : m_side{ s }
      , m_coord{ x, y }
    {
    }

    Crossing(Side s, const geom::CoordinateXY& c)
      : m_side{ s }
      , m_coord{ c }
    {
    }

    const Side& getSide() const
    {
        return m_side;
    }

    const geom::CoordinateXY& getCoord() const
    {
        return m_coord;
    }

  private:
    Side m_side;
    geom::CoordinateXY m_coord;
};

}
