/**********************************************************************
 * $Id: OrientedCoordinateArray.h 2809 2009-12-06 01:05:24Z mloskot $
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2009    Sandro Santilli <strk@keybit.net>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: noding/OrientedCoordinateArray.java rev. 1.1 (JTS-1.9)
 *
 **********************************************************************/

#ifndef GEOS_NODING_ORIENTEDCOORDINATEARRAY_H
#define GEOS_NODING_ORIENTEDCOORDINATEARRAY_H

#include <geos/export.h>

//#include <vector>
//#include <iostream>

//#include <geos/inline.h>

// Forward declarations
namespace geos {
	namespace geom {
		class CoordinateSequence;
	}
	namespace noding {
		//class SegmentString;
	}
}

namespace geos {
namespace noding { // geos.noding

/** \brief
 * Allows comparing {@link geom::CoordinateSequence}s
 * in an orientation-independent way.
 */
class GEOS_DLL OrientedCoordinateArray
{
public:

	/**
	 * Creates a new {@link OrientedCoordinateArray}
	 * for the given {@link geom::CoordinateSequence}.
	 *
	 * @param pts the coordinates to orient
	 */
	OrientedCoordinateArray(const geom::CoordinateSequence& pts)
		:
		pts(pts),
		orientationVar(orientation(pts))
	{
	}

	/** \brief
	 * Compares two {@link OrientedCoordinateArray}s for their
	 * relative order
	 *
	 * @return -1 this one is smaller
	 * @return 0 the two objects are equal
	 * @return 1 this one is greater
	 *
	 * In JTS, this is used automatically by ordered lists.
	 * In C++, operator< would be used instead....
	 */
	int compareTo(const OrientedCoordinateArray& o1) const;


private:

	static int compareOriented(const geom::CoordinateSequence& pts1,
                                     bool orientation1,
                                     const geom::CoordinateSequence& pts2,
                                     bool orientation2);


	/**
	 * Computes the canonical orientation for a coordinate array.
	 *
	 * @param pts the array to test
	 * @return <code>true</code> if the points are oriented forwards
	 * @return <code>false</code if the points are oriented in reverse
	 */
	static bool orientation(const geom::CoordinateSequence& pts);

	/// Externally owned
	const geom::CoordinateSequence& pts;

	bool orientationVar;

    // Declare type as noncopyable
    OrientedCoordinateArray(const OrientedCoordinateArray& other);
    OrientedCoordinateArray& operator=(const OrientedCoordinateArray& rhs);
};

} // namespace geos.noding
} // namespace geos


#endif // GEOS_NODING_ORIENTEDCOORDINATEARRAY_H

