/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: geom/prep/PreparedPolygonPredicate.java rev. 1.2 (2007-08-16)
 *
 **********************************************************************/

#include <geos/geom/prep/PreparedPolygonPredicate.h>
#include <geos/geom/prep/PreparedPolygon.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/util/ComponentCoordinateExtracter.h>
#include <geos/geom/Location.h>
#include <geos/algorithm/locate/PointOnGeometryLocator.h>
#include <geos/algorithm/locate/SimplePointInAreaLocator.h>
// std
#include <cstddef>

namespace geos {
namespace geom { // geos.geom
namespace prep { // geos.geom.prep
//
// private:
//

//
// protected:
//
bool 
PreparedPolygonPredicate::isAllTestComponentsInTargetArea(const geom::Geometry* testGeom)
{
    geom::Coordinate::ConstVect pts;
    geom::util::ComponentCoordinateExtracter::getCoordinates(*testGeom, pts);

    for (std::size_t i = 0, ni = pts.size(); i < ni; i++)
    {
        const geom::Coordinate* pt = pts[i];
        const int loc = prepPoly->getPointLocator()->locate(pt);
        if (geom::Location::EXTERIOR == loc)
        {
            return false;
        }
    }
    return true;
}

bool 
PreparedPolygonPredicate::isAllTestComponentsInTargetInterior(const geom::Geometry* testGeom)
{
    geom::Coordinate::ConstVect pts;
    geom::util::ComponentCoordinateExtracter::getCoordinates(*testGeom, pts);

    for (std::size_t i = 0, ni = pts.size(); i < ni; i++)
    {
        const geom::Coordinate * pt = pts[i];
        const int loc = prepPoly->getPointLocator()->locate(pt);
        if (geom::Location::INTERIOR != loc)
        {
            return false;
        }
    }
    return true;
}

bool 
PreparedPolygonPredicate::isAnyTestComponentInTargetArea(const geom::Geometry* testGeom)
{
    geom::Coordinate::ConstVect pts;
    geom::util::ComponentCoordinateExtracter::getCoordinates(*testGeom, pts);

    for (std::size_t i = 0, ni = pts.size(); i < ni; i++)
    {
        const Coordinate* pt = pts[i];
        const int loc = prepPoly->getPointLocator()->locate(pt);
        if (geom::Location::EXTERIOR != loc)
        {
            return true;
        }
    }
    return false;
}

bool 
PreparedPolygonPredicate::isAnyTestComponentInTargetInterior( const geom::Geometry * testGeom)
{
    geom::Coordinate::ConstVect pts;
    geom::util::ComponentCoordinateExtracter::getCoordinates(*testGeom, pts);

    for (std::size_t i = 0, ni = pts.size(); i < ni; i++)
    {
        const Coordinate * pt = pts[i];
        const int loc = prepPoly->getPointLocator()->locate(pt);
        if (geom::Location::INTERIOR == loc) 
        {
            return true;
        }
    }
    return false;
}

bool
PreparedPolygonPredicate::isAnyTargetComponentInTestArea(const geom::Geometry* testGeom,
                                                         const geom::Coordinate::ConstVect* targetRepPts)
{
    // TODO - mloskot: Replace manual delete with scoped_ptr
    algorithm::locate::PointOnGeometryLocator* piaLoc = 0;
    piaLoc = new algorithm::locate::SimplePointInAreaLocator(testGeom);

    for (std::size_t i = 0, ni = targetRepPts->size(); i < ni; i++)
    {
        const geom::Coordinate * pt = (*targetRepPts)[i];
        const int loc = piaLoc->locate(pt);
        if (geom::Location::EXTERIOR != loc)
        {
            delete piaLoc;
            return true;
        }
    }

    delete piaLoc;
    return false;
}

//
// public:
//

} // namespace geos.geom.prep
} // namespace geos.geom
} // namespace geos

/**********************************************************************
 * $Log$
 *
 **********************************************************************/
