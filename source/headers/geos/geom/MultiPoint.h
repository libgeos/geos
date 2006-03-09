/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 * Copyright (C) 2005 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#ifndef GEOS_GEOS_MULTIPOINT_H
#define GEOS_GEOS_MULTIPOINT_H

#include <string>
#include <vector>
#include <geos/platform.h>
#include <geos/geom/GeometryCollection.h>

#include <geos/inline.h>

namespace geos {
	namespace geom { // geos::geom
		class Coordinate;
		class CoordinateArraySequence;
	}
}

namespace geos {
namespace geom { // geos::geom

/**
 * \class MultiPoint geom.h geos.h
 * \brief  Models a collection of Point objects.
 */
class MultiPoint: public GeometryCollection {
public:

	/**
	 * \brief Constructs a <code>MultiPoint</code>.
	 *
	 * @param  newPoints
	 *	the <code>Point</code>s for this <code>MultiPoint</code>,
	 *	or <code>null</code> or an empty array to create the empty
	 * 	geometry.
	 *	Elements may be empty <code>Point</code>s,
	 *	but not <code>null</code>s.
	 *
	 *	Constructed object will take ownership of
	 *	the vector and its elements.
	 *
	 * @param newFactory
	 * 	The GeometryFactory used to create this geometry
	 *	Caller must keep the factory alive for the life-time
	 *	of the constructed MultiPoint.
	 */
	MultiPoint(std::vector<Geometry *> *newPoints, const GeometryFactory *newFactory);

	virtual ~MultiPoint();

	/// Returns point dimension (0)
	int getDimension() const;

	/// Returns Dimension::False (Point has no boundary)
	int getBoundaryDimension() const;

	/// Returns an EMPTY Geometry
	Geometry* getBoundary() const;

	std::string getGeometryType() const;
	virtual GeometryTypeId getGeometryTypeId() const;
	//bool isValid() const;
	bool isSimple() const;
	bool equalsExact(const Geometry *other, double tolerance=0) const;

	MultiPoint(const MultiPoint &mp): GeometryCollection(mp) {}
	Geometry *clone() const { return new MultiPoint(*this); };

protected:
	const Coordinate* getCoordinateN(int n) const;
};

} // namespace geos::geom
} // namespace geos

//#ifdef USE_INLINE
//# include "geos/geom/MultiPoint.inl"
//#endif

#endif // ndef GEOS_GEOS_MULTIPOINT_H

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/
