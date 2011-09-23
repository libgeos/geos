/**********************************************************************
 * $Id: GeometryCollection.inl 2435 2009-04-30 11:15:12Z strk $
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: geom/GeometryCollection.java rev. 1.41
 *
 **********************************************************************/

#ifndef GEOS_GEOMETRYCOLLECTION_INL
#define GEOS_GEOMETRYCOLLECTION_INL

#include <geos/geom/GeometryCollection.h>

#include <vector>

namespace geos {
namespace geom { // geos::geom

INLINE GeometryCollection::const_iterator
GeometryCollection::begin() const
{
	return geometries->begin();
}

INLINE GeometryCollection::const_iterator
GeometryCollection::end() const
{
	return geometries->end();
}


} // namespace geos::geom
} // namespace geos

#endif // GEOS_GEOMETRYCOLLECTION_INL
