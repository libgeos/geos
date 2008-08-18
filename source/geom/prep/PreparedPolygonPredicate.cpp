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
 **********************************************************************/


#include <geos/geom/prep/PreparedPolygonPredicate.h>
#include <geos/geom/prep/PreparedPolygon.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/util/ComponentCoordinateExtracter.h>
#include <geos/geom/Location.h>
#include <geos/algorithm/locate/PointOnGeometryLocator.h>
#include <geos/algorithm/locate/SimplePointInAreaLocator.h>

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
PreparedPolygonPredicate::isAllTestComponentsInTargetArea( const geom::Geometry * testGeom)
{
	geom::Coordinate::ConstVect pts;
	geom::util::ComponentCoordinateExtracter::getCoordinates( *testGeom, pts);

	for ( size_t i = 0, ni = pts.size(); i < ni; i++ )
	{
		const geom::Coordinate * pt = pts[ i ];
		int loc = prepPoly->getPointLocator()->locate( pt );
		if ( loc == geom::Location::EXTERIOR ) 
			return false;
	}
	return true;
}

bool 
PreparedPolygonPredicate::isAllTestComponentsInTargetInterior( const geom::Geometry * testGeom)
{
	geom::Coordinate::ConstVect pts;
	geom::util::ComponentCoordinateExtracter::getCoordinates( *testGeom, pts);

	for ( size_t i = 0, ni = pts.size(); i < ni; i++ )
	{
		const geom::Coordinate * pt = pts[ i ];
		int loc = prepPoly->getPointLocator()->locate( pt );
		if ( loc != geom::Location::INTERIOR ) 
			return false;
	}
	return true;
}

bool 
PreparedPolygonPredicate::isAnyTestComponentInTargetArea( const geom::Geometry * testGeom)
{
	geom::Coordinate::ConstVect pts;
	geom::util::ComponentCoordinateExtracter::getCoordinates( *testGeom, pts);

	for ( size_t i = 0, ni = pts.size(); i < ni; i++ )
	{
		const Coordinate * pt = pts[ i ];
		int loc = prepPoly->getPointLocator()->locate( pt );
		if ( loc != geom::Location::EXTERIOR ) 
			return true;
	}
	return false;
}

bool 
PreparedPolygonPredicate::isAnyTestComponentInTargetInterior( const geom::Geometry * testGeom)
{
	geom::Coordinate::ConstVect pts;
	geom::util::ComponentCoordinateExtracter::getCoordinates( *testGeom, pts);

	for ( size_t i = 0, ni = pts.size(); i < ni; i++ )
	{
		const Coordinate * pt = pts[ i ];
		int loc = prepPoly->getPointLocator()->locate( pt );
		if ( loc == geom::Location::INTERIOR) 
			return true;
	}
	return false;
}


bool
PreparedPolygonPredicate::isAnyTargetComponentInTestArea( const geom::Geometry * testGeom, const geom::Coordinate::ConstVect * targetRepPts)
{
	algorithm::locate::PointOnGeometryLocator * piaLoc;
	piaLoc = new algorithm::locate::SimplePointInAreaLocator( testGeom);

	for ( size_t i = 0, ni = targetRepPts->size(); i < ni; i++ )
	{
		const geom::Coordinate * pt = (*targetRepPts)[ i ];
		int loc = piaLoc->locate( pt);
		if ( loc != geom::Location::EXTERIOR ) 
			return true;
	}
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
