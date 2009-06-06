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
 * Last port: simplify/LineSegmentIndex.java rev. 1.1 (JTS-1.7.1)
 *
 **********************************************************************
 *
 * NOTES
 *
 **********************************************************************/

#ifndef _GEOS_SIMPLIFY_LINESEGMENTINDEX_H_
#define _GEOS_SIMPLIFY_LINESEGMENTINDEX_H_ 

#include <geos/export.h>
#include <vector>
#include <memory> // for auto_ptr

// Forward declarations
namespace geos {
	namespace geom {
		class Envelope;
		class LineSegment;
	}
	namespace simplify {
		class TaggedLineString;
	}
	namespace index {
		namespace quadtree {
			class Quadtree;
		}
	}
}

namespace geos {
namespace simplify { // geos::simplify

class GEOS_DLL LineSegmentIndex {

public:

	LineSegmentIndex();

	~LineSegmentIndex();

	void add(const TaggedLineString& line);

	void add(const geom::LineSegment* seg);

	void remove(const geom::LineSegment* seg);

	std::auto_ptr< std::vector<geom::LineSegment*> >
			query(const geom::LineSegment* seg) const;

private:

	std::auto_ptr<index::quadtree::Quadtree> index;

	std::vector<geom::Envelope*> newEnvelopes;
	
	// Copying is turned off
	LineSegmentIndex(const LineSegmentIndex&);
	LineSegmentIndex& operator=(const LineSegmentIndex&);
};

} // namespace geos::simplify
} // namespace geos

#endif // _GEOS_SIMPLIFY_LINESEGMENTINDEX_H_ 

/**********************************************************************
 * $Log$
 * Revision 1.2  2006/04/13 09:28:09  mloskot
 * Removed definition of copy ctor and assignment operator for LineSegmentString class.
 *
 * Revision 1.1  2006/04/12 15:20:37  strk
 * LineSegmentIndex class
 *
 **********************************************************************/
