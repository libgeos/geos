/**********************************************************************
 * $Id: ScaledNoder.h 3255 2011-03-01 17:56:10Z mloskot $
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
 **********************************************************************
 *
 * Last port: noding/ScaledNoder.java rev. 1.3 (JTS-1.7.1)
 *
 **********************************************************************/

#ifndef GEOS_NODING_SCALEDNODER_H
#define GEOS_NODING_SCALEDNODER_H

#include <geos/export.h>

#include <cassert>
#include <vector>

#include <geos/inline.h>
#include <geos/noding/Noder.h> // for inheritance
//#include <geos/geom/CoordinateFilter.h> // for inheritance
#include <geos/util.h>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251) // warning C4251: needs to have dll-interface to be used by clients of class
#endif

// Forward declarations
namespace geos {
	namespace geom {
		class Coordinate;
		class CoordinateSequence;
	}
	namespace noding {
		class SegmentString;
	}
}

namespace geos {
namespace noding { // geos.noding

/** \brief
 * Wraps a {@link Noder} and transforms its input
 * into the integer domain.
 *
 * This is intended for use with Snap-Rounding noders,
 * which typically are only intended to work in the integer domain.
 * Offsets can be provided to increase the number of digits of
 * available precision.
 *
 */
class GEOS_DLL ScaledNoder : public Noder { // , public geom::CoordinateFilter { // implements Noder

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

	~ScaledNoder();

	std::vector<SegmentString*>* getNodedSubstrings() const;

	void computeNodes(std::vector<SegmentString*>* inputSegStr);

	//void filter(Coordinate& c);

	void filter_ro(const geom::Coordinate* c)
    {
        ::geos::ignore_unused_variable_warning(c);
        assert(0);
    }

	void filter_rw(geom::Coordinate* c) const;

private:

	Noder& noder;

	double scaleFactor;

	double offsetX;

	double offsetY;

	bool isScaled;

	void rescale(std::vector<SegmentString*>& segStrings) const;

	void scale(std::vector<SegmentString*>& segStrings) const;

	class Scaler;

	class ReScaler;

	friend class ScaledNoder::Scaler;

	friend class ScaledNoder::ReScaler;

    mutable std::vector<geom::CoordinateSequence*> newCoordSeq;

    // Declare type as noncopyable
    ScaledNoder(const ScaledNoder& other);
    ScaledNoder& operator=(const ScaledNoder& rhs);
};

} // namespace geos.noding
} // namespace geos

#ifdef _MSC_VER
#pragma warning(pop)
#endif

//#ifdef GEOS_INLINE
//# include "geos/noding/ScaledNoder.inl"
//#endif

#endif // GEOS_NODING_SCALEDNODER_H

/**********************************************************************
 * $Log$
 * Revision 1.8  2006/06/19 23:33:03  strk
 * Don't *require* CoordinateFilters to define both read-only and read-write methods.
 *
 * Revision 1.7  2006/05/04 06:36:00  strk
 * Extended definition to take new CoordinateSequence tracking into account
 *
 * Revision 1.6  2006/05/03 15:04:33  strk
 * removed reduntant port info
 *
 * Revision 1.5  2006/05/03 15:00:42  strk
 * Fixed scale() function to remove repeated points *after* rounding. Added brief doxygen class description.
 *
 * Revision 1.4  2006/05/03 09:14:22  strk
 * * source/operation/buffer/OffsetCurveSetBuilder.cpp: used auto_ptr to protect leaks of CoordinateSequence
 * * source/noding/ScaledNoder.cpp, source/headers/geos/noding/ScaledNoder.h: ported JTS bugfix in scale method.
 *
 * Revision 1.3  2006/03/24 09:52:41  strk
 * USE_INLINE => GEOS_INLINE
 *
 * Revision 1.2  2006/03/13 21:19:51  strk
 * Fixed bug in ScaledNoder scaling mechanism (hugly code, due to CoordinateSequence visitor pattern design). Tests are still failing so this possibly needs some other fix. Streamlined includes by implementation file.
 *
 * Revision 1.1  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/

