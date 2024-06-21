/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (c) 2024 Martin Davis
 * Copyright (C) 2024 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/operation/relateng/NodeSection.h>
#include <geos/io/WKTWriter.h>
#include <geos/geom/Dimension.h>
#include <sstream>

namespace geos {      // geos
namespace operation { // geos.operation
namespace relateng {  // geos.operation.relateng

using geos::geom::Coordinate;
using geos::geom::CoordinateXY;
using geos::geom::Geometry;
using geos::geom::Dimension;
using geos::io::WKTWriter;


/* public */
const CoordinateXY *
NodeSection::getVertex(int i) const
{
    return i == 0 ? m_v0 : m_v1;
}


/* public */
const CoordinateXY *
NodeSection::nodePt() const
{
    return m_nodePt;
}


/* public */
int
NodeSection::dimension() const
{
    return m_dim;
}


/* public */
int
NodeSection::id() const
{
    return m_id;
}


/* public */
int
NodeSection::ringId() const
{
    return m_ringId;
}


/* public */
const Geometry *
NodeSection::getPolygonal() const
{
    return m_poly;
}


/* public */
bool
NodeSection::isShell() const
{
    return m_ringId == 0;
}


/* public */
bool
NodeSection::isArea() const
{
    return m_dim == Dimension::A;
}


/* public */
bool
NodeSection::isA() const
{
     return m_isA;
}


/* public */
bool
NodeSection::isSameGeometry(const NodeSection& ns) const
{
    return isA() == ns.isA();
}


/* public */
bool
NodeSection::isSamePolygon(const NodeSection& ns) const
{
    return isA() == ns.isA() && id() == ns.id();
}


/* public */
bool
NodeSection::isNodeAtVertex() const
{
    return m_isNodeAtVertex;
}


/* public */
bool
NodeSection::isProper() const
{
    return ! m_isNodeAtVertex;
}


/* public static */
bool
NodeSection::isProper(const NodeSection& a, const NodeSection& b)
{
    return a.isProper() && b.isProper();
}


/* public static */
std::string
NodeSection::edgeRep(const CoordinateXY* p0, const CoordinateXY* p1)
{
    if (p0 == nullptr || p1 == nullptr)
        return "null";
    return WKTWriter::toLineString(*p0, *p1);
}


/* private */
int
NodeSection::compare(int a, int b)
{
    if (a < b) return -1;
    if (a > b) return 1;
    return 0;
}


/* public */
int
NodeSection::compareTo(const NodeSection& o) const
{
    // Assert: nodePt.equals2D(o.nodePt())

    // sort A before B
    if (m_isA != o.m_isA) {
        if (m_isA) return -1;
        return 1;
    }
    //-- sort on dimensions
    int compDim = compare(m_dim,  o.m_dim);
    if (compDim != 0) return compDim;

    //-- sort on id and ring id
    int compId = compare(m_id, o.m_id);
    if (compId != 0) return compId;

    int compRingId = compare(m_ringId, o.m_ringId);
    if (compRingId != 0) return compRingId;

    //-- sort on edge coordinates
    int compV0 = compareWithNull(m_v0, o.m_v0);
    if (compV0 != 0) return compV0;

    return compareWithNull(m_v1, o.m_v1);
}


/* private static */
int
NodeSection::compareWithNull(const CoordinateXY* v0, const CoordinateXY* v1)
{
    if (v0 == nullptr) {
        if (v1 == nullptr)
            return 0;
        //-- null is lower than non-null
        return -1;
    }
    // v0 is non-null
    if (v1 == nullptr)
        return 1;
    return v0->compareTo(*v1);
}


/* public */
std::string
NodeSection::toString() const
{
    // TODO, port RelateGeometry
    // os << RelateGeometry::name(m_isA);
    std::stringstream ss;
    ss << m_dim;
    if (m_id >= 0) {
        ss << "[" << m_id << ":" << m_ringId << "]";
    }
    ss << ": " << edgeRep(m_v0, m_nodePt);
    ss << (m_isNodeAtVertex ? "-V-" : "---");
    ss << " " << edgeRep(m_nodePt, m_v1);
    return ss.str();
}

/* public friend */
std::ostream&
operator<<(std::ostream& os, const NodeSection& ns)
{
    os << ns.toString();
    return os;
}


} // namespace geos.operation.overlayng
} // namespace geos.operation
} // namespace geos


