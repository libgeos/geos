/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2022 Paul Ramsey <pramsey@cleverelephant.ca>
 * Copyright (c) 2022 Martin Davis.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/simplify/LinkedLine.h>

#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/io/WKTWriter.h>
#include <geos/constants.h>

using geos::geom::Coordinate;
using geos::geom::CoordinateSequence;
using geos::io::WKTWriter;


namespace geos {
namespace simplify { // geos.simplify


LinkedLine::LinkedLine(const CoordinateSequence& pts)
    : m_coord(pts)
    , m_isRing(pts.isRing())
    , m_size(pts.isRing() ? pts.size() - 1 : pts.size())
{
    createNextLinks(m_size);
    createPrevLinks(m_size);
}


/* public */
bool
LinkedLine::isRing() const
{
    return m_isRing;
}

/* public */
bool
LinkedLine::isCorner(std::size_t i) const
{
    if (! isRing()
        && (i == 0 || i == m_coord.size() - 1))
    {
        return false;
    }
    return true;
}

/* private */
void
LinkedLine::createNextLinks(std::size_t size)
{
    m_next.resize(size);
    for (std::size_t i = 0; i < size; i++) {
      m_next[i] = i + 1;
    }
    m_next[size - 1] = m_isRing ? 0 : NO_COORD_INDEX;
    return;
}

/* private */
void
LinkedLine::createPrevLinks(std::size_t size)
{
    m_prev.resize(size);
    for (std::size_t i = 1; i < size; i++) {
      m_prev[i] = i - 1;
    }
    m_prev[0] = m_isRing ? size - 1 : NO_COORD_INDEX;
    return;
}

/* public */
std::size_t
LinkedLine::size() const
{
    return m_size;
}

/* public */
std::size_t
LinkedLine::next(std::size_t i) const
{
    return m_next[i];
}

/* public */
std::size_t
LinkedLine::prev(std::size_t i) const
{
    return m_prev[i];
}

/* public */
const Coordinate&
LinkedLine::getCoordinate(std::size_t index) const
{
    return m_coord.getAt(index);
}

/* public */
const Coordinate&
LinkedLine::prevCoordinate(std::size_t index) const
{
    return m_coord.getAt(prev(index));
}

/* public */
const Coordinate&
LinkedLine::nextCoordinate(std::size_t index) const
{
    return m_coord.getAt(next(index));
}

/* public */
bool
LinkedLine::hasCoordinate(std::size_t index) const
{
    //-- if not a ring, endpoints are alway present
    if (! m_isRing && (index == 0 || index == m_coord.size() - 1))
        return true;

    return index != NO_COORD_INDEX
        && index < m_prev.size()
        && m_prev[index] != NO_COORD_INDEX;
}

/* public */
void
LinkedLine::remove(std::size_t index)
{
    std::size_t iprev = m_prev[index];
    std::size_t inext = m_next[index];
    if (iprev != NO_COORD_INDEX)
        m_next[iprev] = inext;
    if (inext != NO_COORD_INDEX)
        m_prev[inext] = iprev;
    m_prev[index] = NO_COORD_INDEX;
    m_next[index] = NO_COORD_INDEX;
    m_size = m_size > 0 ? m_size - 1 : m_size;
}

/* public */
std::unique_ptr<CoordinateSequence>
LinkedLine::getCoordinates() const
{
    std::unique_ptr<CoordinateSequence> coords(new CoordinateSequence());
    std::size_t len = m_isRing ? m_coord.size() - 1 : m_coord.size();
    for (std::size_t i = 0; i < len; i++) {
        if (hasCoordinate(i)) {
            coords->add(m_coord.getAt(i), false);
        }
    }
    if (m_isRing) {
        coords->closeRing();
    }
    return coords;
}

std::ostream&
operator<< (std::ostream& os, const LinkedLine& ll)
{
    auto cs = ll.getCoordinates();
    os << WKTWriter::toLineString(*cs);
    return os;
}



} // namespace geos.simplify
} // namespace geos
