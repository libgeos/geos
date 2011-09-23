/**********************************************************************
 * $Id: CoordinateArraySequenceFactory.inl 2990 2010-05-18 19:18:54Z warmerdam $
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2005-2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#ifndef GEOS_GEOM_COORDINATEARRAYSEQUENCEFACTORY_INL
#define GEOS_GEOM_COORDINATEARRAYSEQUENCEFACTORY_INL

#include <cassert>
#include <geos/geom/CoordinateArraySequenceFactory.h>
#include <geos/geom/CoordinateArraySequence.h>

namespace geos {
namespace geom { // geos::geom

INLINE CoordinateSequence*
CoordinateArraySequenceFactory::create(std::vector<Coordinate> *coords) const
{
	return new CoordinateArraySequence(coords,3);
}

INLINE CoordinateSequence *
CoordinateArraySequenceFactory::create(std::vector<Coordinate> *coords,
		size_t dimension ) const
{
	return new CoordinateArraySequence(coords,dimension);
}

INLINE CoordinateSequence *
CoordinateArraySequenceFactory::create(std::size_t size, std::size_t dimension)
		const
{
	return new CoordinateArraySequence(size,dimension);
}


} // namespace geos::geom
} // namespace geos

#endif // GEOS_GEOM_COORDINATEARRAYSEQUENCEFACTORY_INL

