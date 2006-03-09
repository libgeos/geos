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

#ifndef GEOS_GEOS_MULTILINESTRING_H
#define GEOS_GEOS_MULTILINESTRING_H

#include <string>
#include <vector>

#include <geos/platform.h>
#include <geos/geom/GeometryCollection.h>

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

/// Basic implementation of MultiLineString objects.
class MultiLineString: public GeometryCollection {
public:

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

	virtual ~MultiLineString();

	/// Returns line dimension (1)
	int getDimension() const;

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
	bool isSimple() const;
	bool equalsExact(const Geometry *other, double tolerance=0) const;

	MultiLineString(const MultiLineString &mp);

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

};

} // namespace geos::geom
} // namespace geos

#ifdef USE_INLINE
# include "geos/geom/MultiLineString.inl"
#endif

#endif // ndef GEOS_GEOS_MULTILINESTRING_H

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/
