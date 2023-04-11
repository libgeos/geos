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


#include <geos/coverage/Corner.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/Envelope.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Triangle.h>
#include <geos/io/WKTWriter.h>
#include <geos/simplify/LinkedLine.h>
#include <geos/constants.h>

using geos::coverage::Corner;
using geos::geom::Coordinate;
using geos::geom::Envelope;
using geos::geom::GeometryFactory;
using geos::geom::Triangle;
using geos::io::WKTWriter;

namespace geos {
namespace coverage { // geos.coverage


Corner::Corner(const LinkedLine* edge, std::size_t i)
    : m_edge(edge)
    , m_index(i)
    , m_prev(edge->prev(i))
    , m_next(edge->next(i))
    , m_area(area(*edge, i))
    {}

/* public */
bool
Corner::isVertex(std::size_t index) const
{
    return index == m_index
        || index == m_prev
        || index == m_next;
}

/* public */
const Coordinate&
Corner::prev() const
{
    return m_edge->getCoordinate(m_prev);
}

/* public */
const Coordinate&
Corner::next() const
{
    return m_edge->getCoordinate(m_next);
}

/* private static */
double
Corner::area(const LinkedLine& edge, std::size_t index)
{
    const Coordinate& pp = edge.prevCoordinate(index);
    const Coordinate& p = edge.getCoordinate(index);
    const Coordinate& pn = edge.nextCoordinate(index);
    return Triangle::area(pp, p, pn);
}


/* public */
Envelope
Corner::envelope() const
{
    const Coordinate& pp = m_edge->getCoordinate(m_prev);
    const Coordinate& p = m_edge->getCoordinate(m_index);
    const Coordinate& pn = m_edge->getCoordinate(m_next);
    Envelope env(pp, pn);
    env.expandToInclude(p);
    return env;
}

/* public */
bool
Corner::isVertex(const Coordinate& v) const
{
    if (v.equals2D(m_edge->getCoordinate(m_prev))) return true;
    if (v.equals2D(m_edge->getCoordinate(m_index))) return true;
    if (v.equals2D(m_edge->getCoordinate(m_next))) return true;
    return false;
}

/* public */
bool
Corner::isBaseline(const Coordinate& p0, const Coordinate& p1) const
{
    const Coordinate& l_prev = prev();
    const Coordinate& l_next = next();
    if (l_prev.equals2D( p0 ) && l_next.equals2D( p1 ))
        return true;
    if (l_prev.equals2D( p1 ) && l_next.equals2D( p0 ))
        return true;
    return false;
}

/* public */
bool
Corner::intersects(const Coordinate& v) const
{
    const Coordinate& pp = m_edge->getCoordinate(m_prev);
    const Coordinate& p = m_edge->getCoordinate(m_index);
    const Coordinate& pn = m_edge->getCoordinate(m_next);
    return Triangle::intersects(pp, p, pn, v);
}

/* public */
bool
Corner::isRemoved() const
{
    return m_edge->prev(m_index) != m_prev
        || m_edge->next(m_index) != m_next;
}

/* public */
std::unique_ptr<LineString>
Corner::toLineString() const
{
    Coordinate pp = m_edge->getCoordinate(m_prev);
    Coordinate p = m_edge->getCoordinate(m_index);
    Coordinate pn = m_edge->getCoordinate(m_next);

    /* safeCoord replacement */
    if (pp.isNull()) pp.x = pp.y = DoubleNotANumber;
    if (p.isNull()) p.x = p.y = DoubleNotANumber;
    if (pn.isNull()) pn.x = pn.y = DoubleNotANumber;

    CoordinateSequence cs;
    cs.add(pp); cs.add(p); cs.add(pn);

    auto gf = GeometryFactory::create();
    return gf->createLineString(std::move(cs));
}


std::ostream&
operator<< (std::ostream& os, const Corner& corner)
{
    WKTWriter writer;
    auto ls = corner.toLineString();
    os << writer.write(*ls);
    return os;
}


} // namespace geos.coverage
} // namespace geos
