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

#include <vector>

#include <geos/geom/Coordinate.h>
#include <geos/operation/grid/Side.h>

namespace geos::operation::grid {

/**
 * @brief The Traversal class records the coordinates of a line that are
 *        within a grid cell, as well as the `Side` from which the line
 *        entered and exited the cell.
 */
class Traversal
{
  public:
    Traversal()
      : m_entry{ Side::NONE }
      , m_exit{ Side::NONE }
    {
    }

    bool isClosedRing() const;

    bool isEmpty() const;

    bool isEntered() const;

    bool isExited() const;

    bool isTraversed() const;

    bool hasMultipleUniqueCoordinates() const;

    /// Begin a Traversal on the specified `Side`
    void enter(const geom::CoordinateXY& c, Side s);

    /// Complete a Traversal on the specified `Side`
    void exit(const geom::CoordinateXY& c, Side s);

    Side getEntrySide() const { return m_entry; }

    Side getExitSide() const { return m_exit; }

    const geom::CoordinateXY& getLastCoordinate() const;

    const geom::CoordinateXY& getExitCoordinate() const;

    void add(const geom::CoordinateXY& c);

    void forceExit(Side s) { m_exit = s; }

    const std::vector<geom::CoordinateXY>& getCoordinates() const { return m_coords; }

  private:
    std::vector<geom::CoordinateXY> m_coords;
    Side m_entry;
    Side m_exit;
};

}
