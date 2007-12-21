/**********************************************************************
 * $Id
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
 *
 **********************************************************************/

#ifndef GEOS_GEOM_PREP_PREPAREDLINESTRING_H
#define GEOS_GEOM_PREP_PREPAREDLINESTRING_H

#include <geos/geom/prep/BasicPreparedGeometry.h> // for inheritance

namespace geos {
	namespace noding {
		class FastSegmentSetIntersectionFinder;
	}
}

namespace geos {
namespace geom { // geos::geom
namespace prep { // geos::geom::prep

/**
 * A prepared version for {@link Lineal} geometries.
 * 
 * @author mbdavis
 *
 */
class PreparedLineString : public BasicPreparedGeometry 
{
private:
	noding::FastSegmentSetIntersectionFinder * segIntFinder;

protected:
public:
	PreparedLineString(const Geometry * geom) 
		: 
		BasicPreparedGeometry( geom),
		segIntFinder( NULL)
	{ }

	noding::FastSegmentSetIntersectionFinder * getIntersectionFinder();

	bool intersects(const geom::Geometry * g) const;

};

} // namespace geos::geom::prep
} // namespace geos::geom
} // namespace geos

#endif // GEOS_GEOM_PREP_PREPAREDLINESTRING_H
/**********************************************************************
 * $Log$
 **********************************************************************/

