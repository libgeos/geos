/**********************************************************************
 * $Id: MultiLineString.h 3211 2011-02-13 19:54:01Z mloskot $
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
 * Last port: geom/MultiLineString.java r320 (JTS-1.12)
 *
 **********************************************************************/

#ifndef GEOS_GEOS_MULTILINESTRING_H
#define GEOS_GEOS_MULTILINESTRING_H

#include <geos/export.h>
#include <geos/geom/GeometryCollection.h> // for inheritance
#include <geos/geom/Lineal.h> // for inheritance
#include <geos/geom/Dimension.h>

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

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4250) // T1 inherits T2 via dominance
#endif

/// Models a collection of (@link LineString}s.
class GEOS_DLL MultiLineString: public GeometryCollection, public Lineal {

public:

	friend class GeometryFactory;

	virtual ~MultiLineString();

	/// Returns line dimension (1)
	Dimension::DimensionType getDimension() const;

	/**
	 * \brief
	 * Returns Dimension::False if all LineStrings in the collection
	 * are closed, 0 otherwise.
	 */
	int getBoundaryDimension() const;

	/// Returns a (possibly empty) MultiPoint 
	Geometry* getBoundary() const;

	std::string getGeometryType() const;

	virtual GeometryTypeId getGeometryTypeId() const;

	bool isClosed() const;

	bool equalsExact(const Geometry *other, double tolerance=0) const;

	Geometry *clone() const;

	/**
	 * Creates a MultiLineString in the reverse
	 * order to this object.
	 * Both the order of the component LineStrings
	 * and the order of their coordinate sequences
	 * are reversed.
	 *
	 * @return a MultiLineString in the reverse order
	 */
	MultiLineString* reverse() const;

protected:

	/**
	 * \brief Constructs a <code>MultiLineString</code>.
	 *
	 * @param  newLines
	 *	The <code>LineStrings</code>s for this
	 *	<code>MultiLineString</code>, or <code>null</code>
	 *	or an empty array to create the empty geometry.
	 *	Elements may be empty <code>LineString</code>s,
	 *	but not <code>null</code>s.
	 *
	 *	Constructed object will take ownership of
	 *	the vector and its elements.
	 *
	 * @param newFactory
	 * 	The GeometryFactory used to create this geometry.
	 *	Caller must keep the factory alive for the life-time
	 *	of the constructed MultiLineString.
	 * 	
	 */
	MultiLineString(std::vector<Geometry *> *newLines,
			const GeometryFactory *newFactory);

	MultiLineString(const MultiLineString &mp);
};

#ifdef _MSC_VER
#pragma warning(pop)
#endif

} // namespace geos::geom
} // namespace geos

#ifdef GEOS_INLINE
# include "geos/geom/MultiLineString.inl"
#endif

#endif // ndef GEOS_GEOS_MULTILINESTRING_H

/**********************************************************************
 * $Log$
 * Revision 1.5  2006/05/04 15:49:39  strk
 * updated all Geometry::getDimension() methods to return Dimension::DimensionType (closes bug#93)
 *
 * Revision 1.4  2006/04/28 10:55:39  strk
 * Geometry constructors made protected, to ensure all constructions use GeometryFactory,
 * which has been made friend of all Geometry derivates. getNumPoints() changed to return
 * size_t.
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
