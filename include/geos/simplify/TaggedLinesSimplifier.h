/**********************************************************************
 * $Id: TaggedLinesSimplifier.h 2958 2010-03-29 11:29:40Z mloskot $
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
 *        TaggedLineStringSimplifier member and making
 *        simplify(collection) method become a templated
 *        function.
 *
 **********************************************************************/

#ifndef GEOS_SIMPLIFY_TAGGEDLINESSIMPLIFIER_H
#define GEOS_SIMPLIFY_TAGGEDLINESSIMPLIFIER_H

#include <geos/export.h>
#include <vector>
#include <memory>
#include <cassert>

#include <geos/simplify/LineSegmentIndex.h> // for templated function body
#include <geos/simplify/TaggedLineStringSimplifier.h>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251) // warning C4251: needs to have dll-interface to be used by clients of class
#endif

// Forward declarations
namespace geos {
	namespace simplify {
		class TaggedLineString;
	}
}

namespace geos {
namespace simplify { // geos::simplify

/** \brief
 * Simplifies a collection of TaggedLineStrings, preserving topology
 * (in the sense that no new intersections are introduced).
 */
class GEOS_DLL TaggedLinesSimplifier {

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
	 * Simplify a set of {@link TaggedLineString}s
	 *
	 * @param iterator_type
	 * 	an iterator, must support assignment, increment,
	 *      inequality and dereference operators.
	 *	Dereference operator must return a TaggedLineString*
	 *
	 * @param begin iterator to the first element 
	 *              to be simplified.
	 * @param end an iterator to one-past-last element
	 *            to be simplified.
	 */
	template <class iterator_type>
	void simplify(
		iterator_type begin,
		iterator_type end)
	{
		// add lines to the index
		for (iterator_type it=begin; it != end; ++it) {
			assert(*it);
			inputIndex->add(*(*it));
		}

		// Simplify lines
		for (iterator_type it=begin; it != end; ++it) {
			assert(*it);
			simplify(*(*it));
		}
	}


private:

	void simplify(TaggedLineString& line);

	std::auto_ptr<LineSegmentIndex> inputIndex;

	std::auto_ptr<LineSegmentIndex> outputIndex;

	std::auto_ptr<TaggedLineStringSimplifier> taggedlineSimplifier;
};

} // namespace geos::simplify
} // namespace geos

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif // GEOS_SIMPLIFY_TAGGEDLINESSIMPLIFIER_H

/**********************************************************************
 * $Log$
 * Revision 1.4  2006/05/24 15:32:11  strk
 * * source/headers/geos/simplify/TaggedLinesSimplifier.h: added LineSegmentIndex.h include so that every use of the templated simplify() function get all the required definitions.
 *
 * Revision 1.3  2006/05/24 11:41:23  strk
 *         * source/headers/geos/simplify/TaggedLinesSimplifier.h,
 *         source/simplify/TaggedLinesSimplifier.cpp,
 *         source/simplify/TopologyPreservingSimplifier.cpp:
 *         fixed bug in TopologyPreservingSimplifier failing to
 *         detect intersections, refactored TaggedLinesSimplifier
 *         class to more closely match JTS and use templated
 *         functions.
 *
 * Revision 1.2  2006/04/13 14:25:17  strk
 * TopologyPreservingSimplifier initial port
 *
 * Revision 1.1  2006/04/13 10:39:12  strk
 * Initial implementation of TaggedLinesSimplifier class
 *
 **********************************************************************/
