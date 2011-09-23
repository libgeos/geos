/**********************************************************************
 * $Id: CoordinateArraySequenceFactory.h 2998 2010-05-31 16:25:57Z warmerdam $
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
 **********************************************************************/

#ifndef GEOS_GEOM_COORDINATEARRAYSEQUENCEFACTORY_H
#define GEOS_GEOM_COORDINATEARRAYSEQUENCEFACTORY_H


#include <geos/export.h>
#include <vector>

#include <geos/geom/CoordinateSequenceFactory.h> // for inheritance

#include <geos/inline.h>

// Forward declarations
namespace geos {
	namespace geom { 
		class Coordinate;
	}
}

namespace geos {
namespace geom { // geos::geom

/**
 * \class CoordinateArraySequenceFactory geom.h geos.h
 *
 * \brief
 * Creates CoordinateSequences internally represented as an array of
 * Coordinates.
 */
class GEOS_DLL CoordinateArraySequenceFactory: public CoordinateSequenceFactory {

public:

	/** \brief
	 * Returns a CoordinateArraySequence based on the given vector
	 * (the vector is not copied - callers give up ownership).
	 */
	CoordinateSequence *create(std::vector<Coordinate> *coords) const;

	CoordinateSequence *create(std::vector<Coordinate> *coords, std::size_t dims) const;

   	/** @see CoordinateSequenceFactory::create(std::size_t, int) */
	CoordinateSequence *create(std::size_t size, std::size_t dimension=0) const;

	/** \brief
	 * Returns the singleton instance of CoordinateArraySequenceFactory
	 */
	static const CoordinateSequenceFactory *instance();
};

/// This is for backward API compatibility
typedef CoordinateArraySequenceFactory DefaultCoordinateSequenceFactory;

} // namespace geos::geom
} // namespace geos

#ifdef GEOS_INLINE
# include "geos/geom/CoordinateArraySequenceFactory.inl"
#endif

#endif // ndef GEOS_GEOM_COORDINATEARRAYSEQUENCEFACTORY_H

/**********************************************************************
 * $Log$
 * Revision 1.4  2006/06/12 11:29:23  strk
 * unsigned int => size_t
 *
 * Revision 1.3  2006/06/12 10:10:39  strk
 * Fixed getGeometryN() to take size_t rather then int, changed unsigned int parameters to size_t.
 *
 * Revision 1.2  2006/03/24 09:52:41  strk
 * USE_INLINE => GEOS_INLINE
 *
 * Revision 1.1  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/
