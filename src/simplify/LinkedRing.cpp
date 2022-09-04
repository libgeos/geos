/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2022 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/simplify/LinkedRing.h>

#include <geos/geom/CoordinateSequence.h>
#include <cassert>


using geos::geom::Coordinate;


namespace geos {
namespace simplify { // geos.simplify


/* private static */
std::vector<std::size_t>
LinkedRing::createNextLinks(std::size_t p_size)
{
    std::vector<std::size_t> next(p_size);
    for (std::size_t i = 0; i < p_size; i++) {
        next[i] = i + 1;
    }
    next[p_size - 1] = 0;
    return next;
}

/* private static */
std::vector<std::size_t>
LinkedRing::createPrevLinks(std::size_t p_size)
{
    assert(p_size>0);
    std::vector<std::size_t> prev(p_size);
    prev[0] = p_size - 1;
    for (std::size_t i = 1; i < p_size; i++) {
        prev[i] = i - 1;
    }
    return prev;
}

/* public */
std::size_t
LinkedRing::size() const
{
    return m_size;
}

/* public */
std::size_t
LinkedRing::next(std::size_t i) const
{
    return m_next[i];
}

/* public */
std::size_t
LinkedRing::prev(std::size_t i) const
{
    return m_prev[i];
}

/* public */
const Coordinate&
LinkedRing::getCoordinate(std::size_t index) const
{
    return m_coord[index];
}

/* public */
const Coordinate&
LinkedRing::prevCoordinate(std::size_t index) const
{
    return m_coord[prev(index)];
}

/* public */
const Coordinate&
LinkedRing::nextCoordinate(std::size_t index) const
{
    return m_coord[next(index)];
}

/* public */
bool
LinkedRing::hasCoordinate(std::size_t index) const
{
    return index < m_prev.size()
        && m_prev[index] != NO_COORD_INDEX;
}

/* public */
void
LinkedRing::remove(std::size_t index)
{
    std::size_t iprev = m_prev[index];
    std::size_t inext = m_next[index];
    m_next[iprev] = inext;
    m_prev[inext] = iprev;
    m_prev[index] = NO_COORD_INDEX;
    m_next[index] = NO_COORD_INDEX;
    m_size--;
}

/* public */
std::unique_ptr<CoordinateSequence>
LinkedRing::getCoordinates() const
{
    std::unique_ptr<CoordinateSequence> coords(new CoordinateSequence());
    for (std::size_t i = 0; i < m_coord.size() - 1; i++) {
        if (m_prev[i] != NO_COORD_INDEX) {
            coords->add(m_coord[i], false);
        }
    }
    coords->closeRing();
    return coords;
}


} // namespace geos.simplify
} // namespace geos

