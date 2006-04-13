/**********************************************************************
 * $Id$
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
 * Last port: simplify/TaggedLinesSimplifier.java rev. 1.4 (JTS-1.7.1)
 *
 **********************************************************************
 *
 * NOTES: changed from JTS design adding a private
 *        TaggedLineStringSimplifier member and a
 *        public simplifyLine(TaggedLineString*) method.
 *        This reduced overhead in usage from
 *        TopologyPreservingSimplifier class.
 *
 **********************************************************************/

#ifndef _GEOS_SIMPLIFY_TAGGEDLINESSIMPLIFIER_H_
#define _GEOS_SIMPLIFY_TAGGEDLINESSIMPLIFIER_H_ 

#include <vector>
#include <memory>

// Forward declarations
namespace geos {
	namespace simplify {
		class LineSegmentIndex;
		class TaggedLineString;
		class TaggedLineStringSimplifier;
	}
}

namespace geos {
namespace simplify { // geos::simplify

/** \brief
 * Simplifies a collection of TaggedLineStrings, preserving topology
 * (in the sense that no new intersections are introduced).
 */
class TaggedLinesSimplifier {

public:

	TaggedLinesSimplifier();

	/** \brief
	 * Sets the distance tolerance for the simplification.
	 *
	 * All vertices in the simplified geometry will be within this
	 * distance of the original geometry.
	 *
	 * @param tolerance the approximation tolerance to use
	 */
	void setDistanceTolerance(double tolerance);

	/**
	 * Simplify a vector of {@link TaggedLineString}s
	 *
	 * @param begin iterator to the first element 
	 *              in the vector to be simplified.
	 * @param end an iterator to one-past-last element
	 *            in the vector to be simplified.
	 */
	void simplify(
		std::vector<TaggedLineString*>::iterator begin,
		std::vector<TaggedLineString*>::iterator end);

	void simplifyLine(TaggedLineString* line);

private:

	std::auto_ptr<LineSegmentIndex> inputIndex;

	std::auto_ptr<LineSegmentIndex> outputIndex;

	std::auto_ptr<TaggedLineStringSimplifier> taggedlineSimplifier;

};


} // namespace geos::simplify
} // namespace geos

#endif // _GEOS_SIMPLIFY_TAGGEDLINESSIMPLIFIER_H_ 

/**********************************************************************
 * $Log$
 * Revision 1.2  2006/04/13 14:25:17  strk
 * TopologyPreservingSimplifier initial port
 *
 * Revision 1.1  2006/04/13 10:39:12  strk
 * Initial implementation of TaggedLinesSimplifier class
 *
 **********************************************************************/
