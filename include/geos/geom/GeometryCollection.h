/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
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
 * Last port: geom/GeometryCollection.java rev. 1.41
 *
 **********************************************************************/

#ifndef GEOS_GEOS_GEOMETRYCOLLECTION_H
#define GEOS_GEOS_GEOMETRYCOLLECTION_H

#include <geos/export.h>
#include <geos/geom/Geometry.h> // for inheritance
//#include <geos/platform.h>
#include <geos/geom/Envelope.h> // for proper use of auto_ptr<>
#include <geos/geom/Dimension.h> // for Dimension::DimensionType

#include <geos/inline.h>

#include <string>
#include <vector>
#include <memory> // for auto_ptr

// Forward declarations
namespace geos {
	namespace geom { // geos::geom
		class Coordinate;
		class CoordinateArraySequence;
		class CoordinateSequenceFilter;
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
class GEOS_DLL GeometryCollection : public virtual Geometry {

public:
	friend class GeometryFactory;

	typedef std::vector<Geometry *>::const_iterator const_iterator;

	typedef std::vector<Geometry *>::iterator iterator;

	const_iterator begin() const;

	const_iterator end() const;

	/**
	 * Creates and returns a full copy of this GeometryCollection object.
	 * (including all coordinates contained by it).
	 *
	 * @return a clone of this instance
	 */
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
	 *
	 * @see Dimension::DimensionType
	 */
	virtual Dimension::DimensionType getDimension() const;

	/// Returns coordinate dimension.
	virtual int getCoordinateDimension() const;

	virtual Geometry* getBoundary() const;

	/**
	 * \brief
	 * Returns the maximum boundary dimension of geometries in
	 * this collection.
	 */
	virtual int getBoundaryDimension() const;

	virtual std::size_t getNumPoints() const;

	virtual std::string getGeometryType() const;

	virtual GeometryTypeId getGeometryTypeId() const;

	virtual bool equalsExact(const Geometry *other,
			double tolerance=0) const;

	virtual void apply_ro(CoordinateFilter *filter) const;

	virtual void apply_rw(const CoordinateFilter *filter);

	virtual void apply_ro(GeometryFilter *filter) const;

	virtual void apply_rw(GeometryFilter *filter);

	virtual void apply_ro(GeometryComponentFilter *filter) const;

	virtual void apply_rw(GeometryComponentFilter *filter);

	virtual void apply_rw(CoordinateSequenceFilter& filter);

	virtual void apply_ro(CoordinateSequenceFilter& filter) const;

	virtual void normalize();

	virtual const Coordinate* getCoordinate() const;

	/// Returns the total area of this collection
	virtual double getArea() const;

	/// Returns the total length of this collection
	virtual double getLength() const;

	/// Returns the number of geometries in this collection
	virtual std::size_t getNumGeometries() const;

	/// Returns a pointer to the nth Geometry int this collection
	virtual const Geometry* getGeometryN(std::size_t n) const;

protected:

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


	std::vector<Geometry *>* geometries;

	Envelope::AutoPtr computeEnvelopeInternal() const;

	int compareToSameClass(const Geometry *gc) const;

};

} // namespace geos::geom
} // namespace geos

#ifdef GEOS_INLINE
# include "geos/geom/GeometryCollection.inl"
#endif

#endif // ndef GEOS_GEOS_GEOMETRYCOLLECTION_H

/**********************************************************************
 * $Log$
 * Revision 1.8  2006/06/12 10:10:39  strk
 * Fixed getGeometryN() to take size_t rather then int, changed unsigned int parameters to size_t.
 *
 * Revision 1.7  2006/05/04 15:49:39  strk
 * updated all Geometry::getDimension() methods to return Dimension::DimensionType (closes bug#93)
 *
 * Revision 1.6  2006/04/28 10:55:39  strk
 * Geometry constructors made protected, to ensure all constructions use GeometryFactory,
 * which has been made friend of all Geometry derivates. getNumPoints() changed to return
 * size_t.
 *
 * Revision 1.5  2006/04/10 18:15:09  strk
 * Changed Geometry::envelope member to be of type auto_ptr<Envelope>.
 * Changed computeEnvelopeInternal() signater to return auto_ptr<Envelope>
 *
 * Revision 1.4  2006/04/07 09:54:30  strk
 * Geometry::getNumGeometries() changed to return 'unsigned int'
 * rather then 'int'
 *
 * Revision 1.3  2006/03/24 09:52:41  strk
 * USE_INLINE => GEOS_INLINE
 *
 * Revision 1.2  2006/03/23 12:12:01  strk
 * Fixes to allow build with -DGEOS_INLINE
 *
 * Revision 1.1  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/
