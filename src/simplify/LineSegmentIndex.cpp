/**********************************************************************
 * $Id: LineSegmentIndex.cpp 1820 2006-09-06 16:54:23Z mloskot $
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
 **********************************************************************/

#include <geos/simplify/LineSegmentIndex.h>
#include <geos/simplify/TaggedLineSegment.h>
#include <geos/simplify/TaggedLineString.h>
#include <geos/index/quadtree/Quadtree.h> 
#include <geos/index/ItemVisitor.h> 
#include <geos/geom/LineSegment.h>
#include <geos/geom/Envelope.h>

#include <vector>
#include <memory> // for auto_ptr
#include <cassert>

#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif

#ifdef GEOS_DEBUG
#include <iostream>
#endif

using namespace std;
using namespace geos::geom;
using namespace geos::index::quadtree;

namespace geos {
namespace simplify { // geos::simplify

/**
 * ItemVisitor subclass to reduce volume of query results.
 */
class LineSegmentVisitor: public index::ItemVisitor {

// MD - only seems to make about a 10% difference in overall time.

private:

	const LineSegment* querySeg;

	auto_ptr< vector<LineSegment*> > items;

public:

	LineSegmentVisitor(const LineSegment* s)
		:
		ItemVisitor(),
		querySeg(s),
		items(new vector<LineSegment*>())
	{}

	virtual ~LineSegmentVisitor()
	{
		// nothing to do, LineSegments are not owned by us
	}

	LineSegmentVisitor(const LineSegmentVisitor& o)
		:
		ItemVisitor(),
		querySeg(o.querySeg),
		items(new vector<LineSegment*>(*(o.items.get())))
	{
	}

	LineSegmentVisitor& operator=(const LineSegmentVisitor& o)
	{
		if ( this == &o ) return *this;
		querySeg = o.querySeg;
		items.reset(new vector<LineSegment*>(*(o.items.get())));
	}

	void visitItem(void* item)
	{
		LineSegment* seg = (LineSegment*) item;
		if ( Envelope::intersects(seg->p0, seg->p1,
				querySeg->p0, querySeg->p1) )
		{
			items->push_back(seg);
		}
	}

	auto_ptr< vector<LineSegment*> > getItems()
	{
		return items;
	}


};

/*public*/
LineSegmentIndex::LineSegmentIndex()
	:
	index(new Quadtree())
{
}

/*public*/
LineSegmentIndex::~LineSegmentIndex()
{
	for (size_t i=0, n=newEnvelopes.size(); i<n; ++i)
	{
		delete newEnvelopes[i];
	}
}

/*public*/
void
LineSegmentIndex::add(const TaggedLineString& line)
{
	const vector<TaggedLineSegment*>& segs = line.getSegments();
	for (size_t i=0, n=segs.size(); i<n; ++i)
	{
		const LineSegment* seg = segs[i];
		add(seg);
	}
}

/*public*/
void
LineSegmentIndex::add(const LineSegment* seg)
{
	Envelope* env = new Envelope(seg->p0, seg->p1);
	newEnvelopes.push_back(env);

	// We need a cast because index wants a non-const,
	// altought it won't change the argument
	index->insert(env, (LineSegment*)seg);
}

/*public*/
void
LineSegmentIndex::remove(const LineSegment* seg)
{
	Envelope env(seg->p0, seg->p1);

	// We need a cast because index wants a non-const
	// altought it won't change the argument
	index->remove(&env, (LineSegment*)seg);
}

/*public*/
auto_ptr< vector<LineSegment*> > 
LineSegmentIndex::query(const LineSegment* querySeg) const
{
	Envelope env(querySeg->p0, querySeg->p1);

	LineSegmentVisitor visitor(querySeg);
	index->query(&env, visitor);

	auto_ptr< vector<LineSegment*> > itemsFound = visitor.getItems();

	return itemsFound;
}

} // namespace geos::simplify
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.4  2006/06/12 17:18:56  strk
 * LineSegmentIndex: explicitly initialized parent class in constructors.
 *
 * Revision 1.3  2006/06/12 11:29:24  strk
 * unsigned int => size_t
 *
 * Revision 1.2  2006/04/13 09:28:09  mloskot
 * Removed definition of copy ctor and assignment operator for LineSegmentString class.
 *
 * Revision 1.1  2006/04/12 15:20:37  strk
 * LineSegmentIndex class
 *
 **********************************************************************/
