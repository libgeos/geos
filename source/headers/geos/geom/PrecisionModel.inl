/**********************************************************************
 * $Id$
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

#ifndef GEOS_GEOM_PRECISIONMODEL_INL
#define GEOS_GEOM_PRECISIONMODEL_INL

#include <geos/geom/PrecisionModel.h>
#include <cassert>

namespace geos {
namespace geom { // geos::geom

INLINE 
PrecisionModel::~PrecisionModel(void)
{
}

INLINE void
PrecisionModel::makePrecise(Coordinate* coord) const
{
	assert(coord);
	return makePrecise(*coord);
}

} // namespace geos::geom
} // namespace geos

#endif // GEOS_GEOM_PRECISIONMODEL_INL

