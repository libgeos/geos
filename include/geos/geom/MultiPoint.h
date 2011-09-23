/**********************************************************************
 * $Id: MultiPoint.h 3211 2011-02-13 19:54:01Z mloskot $
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2011 Sandro Santilli <strk@keybit.net>
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 * Copyright (C) 2005 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: geom/MultiPoint.java r320 (JTS-1.12)
 *
 **********************************************************************/

#ifndef GEOS_GEOS_MULTIPOINT_H
#define GEOS_GEOS_MULTIPOINT_H

#include <geos/export.h>
#include <geos/platform.h>
#include <geos/geom/GeometryCollection.h> // for inheritance
#include <geos/geom/Puntal.h> // for inheritance
#include <geos/geom/Dimension.h> // for Dimension::DimensionType

#include <geos/inline.h>

#include <string>
#include <vector>

namespace geos {
	namespace geom { // geos::geom
		class Coordinate;
		class CoordinateArraySequence;
	}
}

namespace geos {
namespace geom { // geos::geom

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4250) // T1 inherits T2 via dominance
#endif

/**
 * Models a collection of Point objects.
 *
 * Any collection of Points is a valid MultiPoint.
 */
class GEOS_DLL MultiPoint: public GeometryCollection, public Puntal
{

public:

	friend class GeometryFactory;

	virtual ~MultiPoint();

	/// Returns point dimension (0)
	Dimension::DimensionType getDimension() const;

	/// Returns Dimension::False (Point has no boundary)
	int getBoundaryDimension() const;

	/** \brief
	 * Gets the boundary of this geometry.
	 *
	 * Zero-dimensional geometries have no boundary by definition,
	 * so an empty GeometryCollection is returned.
	 *
	 * @return an empty GeometryCollection
	 * @see Geometry#getBoundary
	 */
	Geometry* getBoundary() const;

	std::string getGeometryType() const;

	virtual GeometryTypeId getGeometryTypeId() const;

	bool equalsExact(const Geometry *other, double tolerance=0) const;

	Geometry *clone() const { return new MultiPoint(*this); };

protected:

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

	MultiPoint(const MultiPoint &mp): Geometry(mp), GeometryCollection(mp) {}

	const Coordinate* getCoordinateN(int n) const;
};

#ifdef _MSC_VER
#pragma warning(pop)
#endif

} // namespace geos::geom
} // namespace geos

//#ifdef GEOS_INLINE
//# include "geos/geom/MultiPoint.inl"
//#endif

#endif // ndef GEOS_GEOS_MULTIPOINT_H

/**********************************************************************
 * $Log$
 * Revision 1.4  2006/05/04 15:49:39  strk
 * updated all Geometry::getDimension() methods to return Dimension::DimensionType (closes bug#93)
 *
 * Revision 1.3  2006/04/28 10:55:39  strk
 * Geometry constructors made protected, to ensure all constructions use GeometryFactory,
 * which has been made friend of all Geometry derivates. getNumPoints() changed to return
 * size_t.
 *
 * Revision 1.2  2006/03/24 09:52:41  strk
 * USE_INLINE => GEOS_INLINE
 *
 * Revision 1.1  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/
