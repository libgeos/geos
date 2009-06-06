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
 **********************************************************************
 *
 * Last port: noding/SegmentString.java rev. 1.6 (JTS-1.9)
 *
 **********************************************************************/

#ifndef GEOS_NODING_SEGMENTSTRING_H
#define GEOS_NODING_SEGMENTSTRING_H

#include <geos/export.h>

#include <geos/noding/SegmentNodeList.h>

#include <geos/inline.h>

#include <vector>

// Forward declarations
namespace geos {
	namespace algorithm {
		class LineIntersector;
	}
}

namespace geos {
namespace noding { // geos.noding

/** \brief
 * An interface for classes which represent a sequence of contiguous
 * line segments.
 *
 * SegmentStrings can carry a context object, which is useful
 * for preserving topological or parentage information.
 */
class GEOS_DLL SegmentString {
public:
	typedef std::vector<const SegmentString*> ConstVect;
	typedef std::vector<SegmentString *> NonConstVect;

	friend std::ostream& operator<< (std::ostream& os,
			const SegmentString& ss);

	/// Construct a SegmentString.
	//
	/// @param newContext the context associated to this SegmentString
	///
	SegmentString(const void* newContext)
		:
		context(newContext)
	{}

	virtual ~SegmentString() {}

	/**
	 * Gets the user-defined data for this segment string.
	 *
	 * @return the user-defined data
	 */
	const void* getData() const { return context; }

	/**
	 * Sets the user-defined data for this segment string.
	 *
	 * @param data an Object containing user-defined data
	 */
	void setData(const void* data) { context=data; }


	virtual unsigned int size() const=0;

	virtual const geom::Coordinate& getCoordinate(unsigned int i) const=0;

	/// \brief
	/// Return a pointer to the CoordinateSequence associated
	/// with this SegmentString.
	//
	/// Note that the CoordinateSequence is not owned by
	/// this SegmentString!
	///
	virtual geom::CoordinateSequence* getCoordinates() const=0;

	// Return a read-only pointer to this SegmentString CoordinateSequence
	//const CoordinateSequence* getCoordinatesRO() const { return pts; }

	virtual bool isClosed() const=0;

	virtual std::ostream& print(std::ostream& os) const;

private:

	const void* context;

};

std::ostream& operator<< (std::ostream& os, const SegmentString& ss);

} // namespace geos.noding
} // namespace geos

#ifdef GEOS_INLINE
# include "geos/noding/SegmentString.inl"
#endif

#endif

/**********************************************************************
 * $Log$
 * Revision 1.10  2006/05/05 14:25:05  strk
 * moved getSegmentOctant out of .inl into .cpp, renamed private eiList to nodeList as in JTS, added more assertion checking and fixed doxygen comments
 *
 * Revision 1.9  2006/05/05 10:19:06  strk
 * droppped SegmentString::getContext(), new name is getData() to reflect change in JTS
 *
 * Revision 1.8  2006/05/04 08:29:07  strk
 * * source/noding/ScaledNoder.cpp: removed use of SegmentString::setCoordinates().
 * * source/headers/geos/noding/SegmentStrign.{h,inl}: removed new setCoordinates() interface.
 *
 * Revision 1.7  2006/05/04 07:43:44  strk
 * output operator for SegmentString class
 *
 * Revision 1.6  2006/05/03 18:04:49  strk
 * added SegmentString::setCoordinates() interface
 *
 * Revision 1.5  2006/05/03 16:19:39  strk
 * fit in 80 columns
 *
 * Revision 1.4  2006/05/03 15:26:02  strk
 * testInvariant made public and always inlined
 *
 * Revision 1.3  2006/03/24 09:52:41  strk
 * USE_INLINE => GEOS_INLINE
 *
 * Revision 1.2  2006/03/13 21:14:24  strk
 * Added missing forward declarations
 *
 * Revision 1.1  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/

