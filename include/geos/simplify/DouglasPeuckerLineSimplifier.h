/**********************************************************************
 * $Id: DouglasPeuckerLineSimplifier.h 3255 2011-03-01 17:56:10Z mloskot $
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: simplify/DouglasPeuckerLineSimplifier.java rev. 1.4
 *
 **********************************************************************/

#ifndef GEOS_SIMPLIFY_DOUBGLASPEUCKERLINESIMPLIFIER_H
#define GEOS_SIMPLIFY_DOUBGLASPEUCKERLINESIMPLIFIER_H

#include <geos/export.h>
#include <vector>
#include <memory> // for auto_ptr

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251) // warning C4251: needs to have dll-interface to be used by clients of class
#endif

// Forward declarations
namespace geos {
	namespace geom {
		class Coordinate;
	}
}

namespace geos {
namespace simplify { // geos::simplify

/** \brief
 * Simplifies a linestring (sequence of points) using
 * the standard Douglas-Peucker algorithm.
 */
class GEOS_DLL DouglasPeuckerLineSimplifier {

public:

	typedef std::vector<short int> BoolVect;
	typedef std::auto_ptr<BoolVect> BoolVectAutoPtr;

	typedef std::vector<geom::Coordinate> CoordsVect;
	typedef std::auto_ptr<CoordsVect> CoordsVectAutoPtr;


	/** \brief
	 * Returns a newly allocated Coordinate vector, wrapped
	 * into an auto_ptr
	 */
	static CoordsVectAutoPtr simplify(
			const CoordsVect& nPts,
			double distanceTolerance);

	DouglasPeuckerLineSimplifier(const CoordsVect& nPts);

	/** \brief
	 * Sets the distance tolerance for the simplification.
	 *
	 * All vertices in the simplified linestring will be within this
	 * distance of the original linestring.
	 *
	 * @param nDistanceTolerance the approximation tolerance to use
	 */
	void setDistanceTolerance(double nDistanceTolerance);

	/** \brief
	 * Returns a newly allocated Coordinate vector, wrapped
	 * into an auto_ptr
	 */
	CoordsVectAutoPtr simplify();

private:

	const CoordsVect& pts;
	BoolVectAutoPtr usePt;
	double distanceTolerance;

	void simplifySection(std::size_t i, std::size_t j);

    // Declare type as noncopyable
    DouglasPeuckerLineSimplifier(const DouglasPeuckerLineSimplifier& other);
    DouglasPeuckerLineSimplifier& operator=(const DouglasPeuckerLineSimplifier& rhs);
};

} // namespace geos::simplify
} // namespace geos

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif // GEOS_SIMPLIFY_DOUBGLASPEUCKERLINESIMPLIFIER_H

/**********************************************************************
 * $Log$
 * Revision 1.4  2006/07/10 10:16:06  strk
 * changed vector<bool> to vector<short int> (see bug#101)
 *
 * Revision 1.3  2006/06/12 11:29:23  strk
 * unsigned int => size_t
 *
 * Revision 1.2  2006/04/13 10:39:12  strk
 * Initial implementation of TaggedLinesSimplifier class
 *
 * Revision 1.1  2006/04/03 10:16:11  strk
 * DouglasPeuckerLineSimplifier class port
 *
 **********************************************************************/
