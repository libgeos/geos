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

#ifndef GEOS_GEOS_GEOMETRYCOLLECTION_H
#define GEOS_GEOS_GEOMETRYCOLLECTION_H

#include <geos/geom/Geometry.h> // for inheritance
//#include <geos/platform.h>

#include <string>
#include <vector>

#include <geos/inline.h>

// Forward declarations
namespace geos {
	namespace geom { // geos::geom
		class Coordinate;
		class CoordinateArraySequence;
	}
}

namespace geos {
namespace geom { // geos::geom

/**
 * \class GeometryCollection geom.h geos.h
 *
 * \brief Represents a collection of heterogeneous Geometry objects.
 *
 * Collections of Geometry of the same type are 
 * represented by GeometryCollection subclasses MultiPoint,
 * MultiLineString, MultiPolygon.
 */
class GeometryCollection : public Geometry {

public:

	typedef std::vector<Geometry *>::const_iterator const_iterator;
	typedef std::vector<Geometry *>::iterator iterator;

	const_iterator begin() const;
	const_iterator end() const;

	GeometryCollection(const GeometryCollection &gc);

	/** \brief
	 * Construct a GeometryCollection with the given GeometryFactory.
	 * Will keep a reference to the factory, so don't
	 * delete it until al Geometry objects referring to
	 * it are deleted.
	 * Will take ownership of the Geometry vector.
	 *
	 * @param newGeoms
	 *	The <code>Geometry</code>s for this
	 *	<code>GeometryCollection</code>,
	 *	or <code>null</code> or an empty array to
	 *	create the empty geometry.
	 *	Elements may be empty <code>Geometry</code>s,
	 *	but not <code>null</code>s.
	 *
	 *	If construction succeed the created object will take
	 *	ownership of newGeoms vector and elements.
	 *
	 *	If construction	fails "IllegalArgumentException *"
	 *	is thrown and it is your responsibility to delete newGeoms
	 *	vector and content.
	 *
	 * @param newFactory the GeometryFactory used to create this geometry
	 */
	GeometryCollection(std::vector<Geometry *> *newGeoms, const GeometryFactory *newFactory);

	virtual Geometry *clone() const {
		return new GeometryCollection(*this);
	}

	virtual ~GeometryCollection();

	/**
	 * \brief
	 * Collects all coordinates of all subgeometries into a
	 * CoordinateSequence.
	 * 
	 * Note that the returned coordinates are copies, so
	 * you want be able to use them to modify the geometries
	 * in place. Also you'll need to delete the CoordinateSequence
	 * when finished using it.
	 * 
	 * @return the collected coordinates
	 *
	 */
	virtual CoordinateSequence* getCoordinates() const;

	virtual bool isEmpty() const;

	/**
	 * \brief
	 * Returns the maximum dimension of geometries in this collection
	 * (0=point, 1=line, 2=surface)
	 */
	virtual int getDimension() const;

	virtual Geometry* getBoundary() const;

	/**
	 * \brief
	 * Returns the maximum boundary dimension of geometries in
	 * this collection.
	 */
	virtual int getBoundaryDimension() const;

	virtual int getNumPoints() const;
	virtual std::string getGeometryType() const;
	virtual GeometryTypeId getGeometryTypeId() const;
	virtual bool isSimple() const;
	virtual bool equalsExact(const Geometry *other, double tolerance=0) const;

	virtual void apply_ro(CoordinateFilter *filter) const;
	virtual void apply_rw(const CoordinateFilter *filter);
	virtual void apply_ro(GeometryFilter *filter) const;
	virtual void apply_rw(GeometryFilter *filter);
	virtual void apply_ro(GeometryComponentFilter *filter) const;
	virtual void apply_rw(GeometryComponentFilter *filter);

	virtual void normalize();
	virtual const Coordinate* getCoordinate() const;
	/// Returns the total area of this collection
	virtual double getArea() const;
	/// Returns the total length of this collection
	virtual double getLength() const;
	/// Returns the number of geometries in this collection
	virtual int getNumGeometries() const;
	/// Returns a pointer to the nth Geometry int this collection
	virtual const Geometry* getGeometryN(int n) const;

protected:
	std::vector<Geometry *>* geometries;
	virtual Envelope* computeEnvelopeInternal() const;
	virtual int compareToSameClass(const Geometry *gc) const;

};

} // namespace geos::geom
} // namespace geos

#ifdef USE_INLINE
# include "geos/geom/GeometryCollection.inl"
#endif

#endif // ndef GEOS_GEOS_GEOMETRYCOLLECTION_H

/**********************************************************************
 * $Log$
 * Revision 1.2  2006/03/23 12:12:01  strk
 * Fixes to allow build with -DUSE_INLINE
 *
 * Revision 1.1  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/
