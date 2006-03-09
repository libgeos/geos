/**********************************************************************
 * $Id$
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

#ifndef GEOS_GEOM_COORDINATESEQUENCEFACTORY_H
#define GEOS_GEOM_COORDINATESEQUENCEFACTORY_H


#include <vector>

//#include <geos/geom/Coordinate.h>
#include <geos/inline.h>

// Forward declarations
namespace geos {
	namespace geom { 
		class CoordinateSequence;
		class Coordinate;
	}
}

namespace geos {
namespace geom { // geos::geom

/**
 * \brief
 * An object that knows how to build a particular implementation of
 * CoordinateSequence from an array of Coordinates.
 */
class CoordinateSequenceFactory {
public:

	/** \brief
	 * Returns a CoordinateSequence based on the given array.
	 * Whether or not the vector is copied is implementation-dependent,
	 * for this reason caller does give up ownership of it.
	 * Implementations that will not copy it will need take care
	 * of deleting it.
	 * Note that a NULL value is allowed as coordinates, and will
	 * create an empty CoordinateSequence.
	 */
	virtual CoordinateSequence *create(std::vector<Coordinate> *coordinates) const=0;

	/** \brief
	 * Creates a CoordinateSequence of the specified size and dimension.
	 * For this to be useful, the CoordinateSequence implementation must
	 * be mutable.
	 *
	 * @param size the number of coordinates in the sequence
	 * @param dimension the dimension of the coordinates in the sequence
	 * 	(if user-specifiable, otherwise ignored)
	 */
	virtual CoordinateSequence *create(unsigned int size, unsigned int dimension)
		const=0;
};

} // namespace geos::geom
} // namespace geos

//#ifdef USE_INLINE
//# include "geos/geom/CoordinateSequenceFactory.inl"
//#endif

#endif // ndef GEOS_GEOM_COORDINATESEQUENCEFACTORY_H

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/
