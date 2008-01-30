/**********************************************************************
 * $Id:
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


#include <geos/geom/Polygon.h>

#include <geos/geom/prep/PreparedPolygon.h>
#include <geos/geom/prep/PreparedPolygonContains.h>
#include <geos/geom/prep/PreparedPolygonContainsProperly.h>
#include <geos/geom/prep/PreparedPolygonCovers.h>
#include <geos/geom/prep/PreparedPolygonIntersects.h>
#include <geos/geom/prep/PreparedPolygonPredicate.h>

#include <geos/noding/FastSegmentSetIntersectionFinder.h>
#include <geos/noding/SegmentStringUtil.h>
#include <geos/operation/predicate/RectangleContains.h>
#include <geos/operation/predicate/RectangleIntersects.h>
#include <geos/algorithm/locate/PointOnGeometryLocator.h>
#include <geos/algorithm/locate/IndexedPointInAreaLocator.h>

namespace geos {
namespace geom { // geos.geom
namespace prep { // geos.geom.prep
//
// public:
//
PreparedPolygon::
PreparedPolygon( const geom::Geometry * geom) 
:	BasicPreparedGeometry( geom),
	segIntFinder( NULL),
	ptOnGeomLoc( NULL)
{
	isRectangle = getGeometry().isRectangle();
}

PreparedPolygon::
~PreparedPolygon( )
{
	delete segIntFinder;
	delete ptOnGeomLoc;

	for ( size_t i = 0, ni = segStrings.size(); i < ni; i++ )
	{
		delete segStrings[ i ]->getCoordinates();
		delete segStrings[ i ];
	}
}


noding::FastSegmentSetIntersectionFinder * 
PreparedPolygon::
getIntersectionFinder() const
{
	if (! segIntFinder)
	{
		noding::SegmentStringUtil::extractSegmentStrings( &getGeometry(), segStrings );
		segIntFinder = new noding::FastSegmentSetIntersectionFinder( &segStrings );
	}
	return segIntFinder;
}

algorithm::locate::PointOnGeometryLocator * 
PreparedPolygon::
getPointLocator() const
{
	if (! ptOnGeomLoc) 
		ptOnGeomLoc = new algorithm::locate::IndexedPointInAreaLocator( getGeometry() );

	return ptOnGeomLoc;
}

bool 
PreparedPolygon::
contains( const geom::Geometry * g) const
{
    // short-circuit test
    if ( !envelopeCovers( g) ) 
		return false;
  	
    // optimization for rectangles
    if ( isRectangle )
		return operation::predicate::RectangleContains::contains( *((geom::Polygon *)&getGeometry()), *g);

	return PreparedPolygonContains::contains( this, g);
}

bool 
PreparedPolygon::
containsProperly( const geom::Geometry* g) const
{
    // short-circuit test
    if ( !envelopeCovers( g) ) 
		return false;

	return PreparedPolygonContainsProperly::containsProperly( this, g);
}

bool 
PreparedPolygon::
covers( const geom::Geometry* g) const
{
    // short-circuit test
    if ( !envelopeCovers( g) ) 
		return false;

    // optimization for rectangle arguments
    if ( isRectangle) 
		return true;

	return PreparedPolygonCovers::covers( this, g);
}

bool 
PreparedPolygon::
intersects( const geom::Geometry* g) const
{
  	// envelope test
  	if ( !envelopesIntersect( g) ) 
		return false;
  	
    // optimization for rectangles
    if ( isRectangle )
		return operation::predicate::RectangleIntersects::intersects( *((geom::Polygon *)&getGeometry()), *g);
    
	return PreparedPolygonIntersects::intersects( this, g);
}

} // namespace geos.geom.prep
} // namespace geos.geom
} // namespace geos

/**********************************************************************
 * $Log$
 *
 **********************************************************************/
