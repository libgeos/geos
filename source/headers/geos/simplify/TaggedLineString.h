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
 * Last port: simplify/TaggedLineString.java rev. 1.2 (JTS-1.7.1)
 *
 **********************************************************************
 *
 * NOTES: 
 *
 **********************************************************************/

#ifndef _GEOS_SIMPLIFY_TAGGEDLINESTRING_H_
#define _GEOS_SIMPLIFY_TAGGEDLINESTRING_H_ 

#include <vector>
#include <memory>

// Forward declarations
namespace geos {
	namespace geom {
		class Coordinate;
		class CoordinateSequence;
		class Geometry;
		class LineString;
		class LinearRing;
	}
	namespace simplify {
		class TaggedLineSegment;
	}
}

namespace geos {
namespace simplify { // geos::simplify


class TaggedLineString {

public:

	typedef std::vector<geom::Coordinate> CoordVect;
	
	typedef std::auto_ptr<CoordVect> CoordVectPtr;

	typedef geom::CoordinateSequence CoordSeq;

	typedef std::auto_ptr<geom::CoordinateSequence> CoordSeqPtr;

	TaggedLineString(const geom::LineString* nParentLine,
			unsigned int minimumSize=2);

	~TaggedLineString();

	unsigned int getMinimumSize() const;

	const geom::LineString* getParent() const;

	const CoordSeq* getParentCoordinates() const;

	CoordSeqPtr getResultCoordinates() const;

	unsigned int getResultSize() const;

	TaggedLineSegment* getSegment(unsigned int);

	std::vector<TaggedLineSegment*>& getSegments();

	const std::vector<TaggedLineSegment*>& getSegments() const;

	void addToResult(std::auto_ptr<TaggedLineSegment> seg);

	std::auto_ptr<geom::Geometry> asLineString() const;

	std::auto_ptr<geom::Geometry> asLinearRing() const;

private:

	const geom::LineString* parentLine;

	// TaggedLineSegments owned by this object
	std::vector<TaggedLineSegment*> segs;

	// TaggedLineSegments owned by this object
	std::vector<TaggedLineSegment*> resultSegs;

	unsigned int minimumSize;

	void init();

	static CoordVectPtr extractCoordinates(
			const std::vector<TaggedLineSegment*>& segs);

	// This object is uncopiable 
	TaggedLineString(const TaggedLineString&);

	// This object is uncopiable 
	TaggedLineString& operator= (const TaggedLineString&);

};

} // namespace geos::simplify
} // namespace geos

#endif // _GEOS_SIMPLIFY_TAGGEDLINESTRING_H_ 

/**********************************************************************
 * $Log$
 * Revision 1.2  2006/04/12 15:20:37  strk
 * LineSegmentIndex class
 *
 * Revision 1.1  2006/04/12 14:22:12  strk
 * Initial implementation of TaggedLineSegment and TaggedLineString classes
 *
 **********************************************************************/
