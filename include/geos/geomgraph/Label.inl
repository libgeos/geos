/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2005-2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: geomgraph/Label.java r428 (JTS-1.12+)
 *
 **********************************************************************/

#include <geos/geomgraph/Label.h>
#include <geos/geomgraph/TopologyLocation.h>
#include <geos/geomgraph/Position.h>
#include <geos/geom/Location.h>

#include <cassert>

namespace geos {
namespace geomgraph { // geos.geomgraph

/*public static*/
INLINE Label
Label::toLineLabel(const Label& label)
{
    Label lineLabel(geom::Location::UNDEF);
    for(int i = 0; i < 2; i++) {
        lineLabel.setLocation(i, label.getLocation(i));
    }
    return lineLabel;
}

/*public*/
INLINE
Label::Label(geom::Location onLoc) :
    elt{TopologyLocation(onLoc), TopologyLocation(onLoc)}
{
}

/*public*/
INLINE
Label::Label(int geomIndex, geom::Location onLoc) :
    elt{TopologyLocation(geom::Location::UNDEF), TopologyLocation(geom::Location::UNDEF)}
{
    assert(geomIndex >= 0 && geomIndex < 2);
    elt[geomIndex].setLocation(onLoc);
}

/*public*/
INLINE
Label::Label(geom::Location onLoc, geom::Location leftLoc, geom::Location rightLoc)
    : elt {TopologyLocation(onLoc, leftLoc, rightLoc), TopologyLocation(onLoc, leftLoc, rightLoc)}
{
}

/*public*/
INLINE
Label::Label()
    : elt{TopologyLocation(geom::Location::UNDEF), TopologyLocation(geom::Location::UNDEF)}
{
}

/*public*/
INLINE
Label::Label(const Label& l) :
    elt{TopologyLocation(l.elt[0]), TopologyLocation(l.elt[1])}
{
}

/*public*/
INLINE Label&
Label::operator=(const Label& l)
{
    elt[0] = TopologyLocation(l.elt[0]);
    elt[1] = TopologyLocation(l.elt[1]);
    return *this;
}

/*public*/
INLINE
Label::Label(int geomIndex, geom::Location onLoc, geom::Location leftLoc, geom::Location rightLoc)
{
    elt[0] = TopologyLocation(geom::Location::UNDEF, geom::Location::UNDEF, geom::Location::UNDEF);
    elt[1] = TopologyLocation(geom::Location::UNDEF, geom::Location::UNDEF, geom::Location::UNDEF);
    elt[geomIndex].setLocations(onLoc, leftLoc, rightLoc);
}

/*public*/
INLINE void
Label::flip()
{
    elt[0].flip();
    elt[1].flip();
}

/*public*/
INLINE geom::Location
Label::getLocation(int geomIndex, int posIndex) const
{
    assert(geomIndex >= 0 && geomIndex < 2);
    return elt[geomIndex].get(posIndex);
}

/*public*/
INLINE geom::Location
Label::getLocation(int geomIndex) const
{
    assert(geomIndex >= 0 && geomIndex < 2);
    return elt[geomIndex].get(Position::ON);
}

/*public*/
INLINE void
Label::setLocation(int geomIndex, int posIndex, geom::Location location)
{
    assert(geomIndex >= 0 && geomIndex < 2);
    elt[geomIndex].setLocation(posIndex, location);
}

/*public*/
INLINE void
Label::setLocation(int geomIndex, geom::Location location)
{
    assert(geomIndex >= 0 && geomIndex < 2);
    elt[geomIndex].setLocation(Position::ON, location);
}

/*public*/
INLINE void
Label::setAllLocations(int geomIndex, geom::Location location)
{
    assert(geomIndex >= 0 && geomIndex < 2);
    elt[geomIndex].setAllLocations(location);
}

/*public*/
INLINE void
Label::setAllLocationsIfNull(int geomIndex, geom::Location location)
{
    assert(geomIndex >= 0 && geomIndex < 2);
    elt[geomIndex].setAllLocationsIfNull(location);
}

/*public*/
INLINE void
Label::setAllLocationsIfNull(geom::Location location)
{
    setAllLocationsIfNull(0, location);
    setAllLocationsIfNull(1, location);
}

/*public*/
INLINE void
Label::merge(const Label& lbl)
{
    for(int i = 0; i < 2; i++) {
        elt[i].merge(lbl.elt[i]);
    }
}

/*public*/
INLINE int
Label::getGeometryCount() const
{
    int count = 0;
    if(!elt[0].isNull()) {
        count++;
    }
    if(!elt[1].isNull()) {
        count++;
    }
    return count;
}

/*public*/
INLINE bool
Label::isNull(int geomIndex) const
{
    assert(geomIndex >= 0 && geomIndex < 2);
    return elt[geomIndex].isNull();
}

/*public*/
INLINE bool
Label::isNull() const
{
    return elt[0].isNull() && elt[1].isNull();
}

/*public*/
INLINE bool
Label::isAnyNull(int geomIndex) const
{
    assert(geomIndex >= 0 && geomIndex < 2);
    return elt[geomIndex].isAnyNull();
}

/*public*/
INLINE bool
Label::isArea() const
{
    return elt[0].isArea() || elt[1].isArea();
}

/*public*/
INLINE bool
Label::isArea(int geomIndex) const
{
    assert(geomIndex >= 0 && geomIndex < 2);
    return elt[geomIndex].isArea();
}

/*public*/
INLINE bool
Label::isLine(int geomIndex) const
{
    assert(geomIndex >= 0 && geomIndex < 2);
    return elt[geomIndex].isLine();
}

/*public*/
INLINE bool
Label::isEqualOnSide(const Label& lbl, int side) const
{
    return elt[0].isEqualOnSide(lbl.elt[0], side)
            && elt[1].isEqualOnSide(lbl.elt[1], side);
}

/*public*/
INLINE bool
Label::allPositionsEqual(int geomIndex, geom::Location loc) const
{
    assert(geomIndex >= 0 && geomIndex < 2);
    return elt[geomIndex].allPositionsEqual(loc);
}

/*public*/
INLINE void
Label::toLine(int geomIndex)
{
    assert(geomIndex >= 0 && geomIndex < 2);
    if(elt[geomIndex].isArea()) {
        elt[geomIndex] = TopologyLocation(elt[geomIndex].getLocations()[0]);
    }
}

} // namespace geos.geomgraph
} // namespace geos
