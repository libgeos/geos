/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2006      Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#ifndef GEOS_NODING_SCALEDNODER_H
#define GEOS_NODING_SCALEDNODER_H

#include <cassert>
#include <vector>

#include <geos/inline.h>

#include <geos/noding/Noder.h> // for inheritance
#include <geos/geom/CoordinateFilter.h> // for inheritance

// Forward declarations
namespace geos {
	namespace geom {
		class Coordinate;
	}
	namespace noding {
		class SegmentString;
	}
}

namespace geos {
namespace noding { // geos.noding

/**
 * Wraps a {@link Noder} and transforms its input
 * into the integer domain.
 * This is intended for use with Snap-Rounding noders,
 * which typically are only intended to work in the integer domain.
 * Offsets can be provided to increase the number of digits of
 * available precision.
 *
 * Last port: noding/ScaledNoder.java rev. 1.2 (JTS-1.7)
 */
class ScaledNoder : public Noder, public geom::CoordinateFilter { // implements Noder

private:

	Noder& noder;
	double scaleFactor;
	double offsetX;
	double offsetY;
	bool isScaled;

	void rescale(std::vector<SegmentString*>& segStrings) const;

public:

	bool isIntegerPrecision() { return (scaleFactor == 1.0); }

	ScaledNoder(Noder& n, double nScaleFactor,
			double nOffsetX=0.0, double nOffsetY=0.0)
		:
		noder(n),
		scaleFactor(nScaleFactor),
		offsetX(nOffsetX),
		offsetY(nOffsetY),
		isScaled(nScaleFactor!=1.0)
	{}

	std::vector<SegmentString*>* getNodedSubstrings() const;

	void computeNodes(std::vector<SegmentString*>* inputSegStr);

	//void filter(Coordinate& c);

	void filter_ro(const geom::Coordinate* c) { assert(0); }
	void filter_rw(geom::Coordinate* c) const;

};

} // namespace geos.noding
} // namespace geos


//#ifdef USE_INLINE
//# include "geos/noding/ScaledNoder.inl"
//#endif

#endif // GEOS_NODING_SCALEDNODER_H

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/

