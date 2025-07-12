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

#include <cstddef>
#include <stdexcept>

#include <geos/operation/grid/Traversal.h>

using geos::geom::CoordinateXY;

namespace geos::operation::grid {

void
Traversal::add(const CoordinateXY& c)
{
    m_coords.push_back(c);
}

bool
Traversal::isEmpty() const
{
    return m_coords.empty();
}

void
Traversal::enter(const CoordinateXY& c, Side s)
{
    if (!m_coords.empty()) {
        throw std::runtime_error("Traversal already started");
    }

    add(c);
    m_entry = s;
}

void
Traversal::exit(const CoordinateXY& c, Side s)
{
    add(c);
    m_exit = s;
}

bool
Traversal::isClosedRing() const
{
    return m_coords.size() >= 3 && m_coords[0] == m_coords[m_coords.size() - 1];
}

bool
Traversal::isEntered() const
{
    return m_entry != Side::NONE;
}

bool
Traversal::isExited() const
{
    return m_exit != Side::NONE;
}

bool
Traversal::hasMultipleUniqueCoordinates() const
{
    for (size_t i = 1; i < m_coords.size(); i++) {
        if (m_coords[0] != m_coords[i]) {
            return true;
        }
    }

    return false;
}

bool
Traversal::isTraversed() const
{
    return isEntered() && isExited();
}

const CoordinateXY&
Traversal::getLastCoordinate() const
{
    return m_coords.at(m_coords.size() - 1);
}

const CoordinateXY&
Traversal::getExitCoordinate() const
{
    if (!isExited()) {
        throw std::runtime_error("Can't get exit coordinate from incomplete traversal.");
    }

    return getLastCoordinate();
}

}