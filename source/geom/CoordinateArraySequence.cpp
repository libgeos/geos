/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 * Copyright (C) 2005 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/geom/Envelope.h>
#include <geos/geom/CoordinateArraySequence.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateFilter.h>

#include <cassert>
#include <algorithm>
#include <vector>

using namespace std;

namespace geos {
namespace geom { // geos::geom

CoordinateArraySequence::CoordinateArraySequence():
	vect(new vector<Coordinate>())
{
}

CoordinateArraySequence::CoordinateArraySequence(unsigned int n):
	vect(new vector<Coordinate>(n))
{
}

CoordinateArraySequence::CoordinateArraySequence(
	vector<Coordinate> *coords): vect(coords)
{
	if ( ! vect ) vect = new vector<Coordinate>();
}

CoordinateArraySequence::CoordinateArraySequence(
	const CoordinateArraySequence &c):
		vect(new vector<Coordinate>(*(c.vect)))
{
}

CoordinateSequence *
CoordinateArraySequence::clone() const
{
	return new CoordinateArraySequence(*this);
}

void
CoordinateArraySequence::setPoints(const vector<Coordinate> &v)
{
	vect->assign(v.begin(), v.end());
}

const vector<Coordinate>*
CoordinateArraySequence::toVector() const
{
	return vect; //new vector<Coordinate>(vect->begin(),vect->end());
}

bool
CoordinateArraySequence::isEmpty() const
{
	return vect->empty();
}

void
CoordinateArraySequence::add(const Coordinate& c)
{
	vect->push_back(c);
}

unsigned int
CoordinateArraySequence::getSize() const
{
	return vect->size();
}

const Coordinate &
CoordinateArraySequence::getAt(unsigned int pos) const
{
	assert(pos<vect->size());
	return (*vect)[pos];
}

void
CoordinateArraySequence::getAt(unsigned int pos, Coordinate &c) const
{
	assert(pos<vect->size());
	c=(*vect)[pos];
}

void
CoordinateArraySequence::setAt(const Coordinate& c, unsigned int pos)
{
	assert(pos<vect->size());
	(*vect)[pos]=c;
}

void
CoordinateArraySequence::deleteAt(unsigned int pos)
{
	assert(pos<vect->size());
	vect->erase(vect->begin()+pos);
}

string
CoordinateArraySequence::toString() const
{
	string result("(");
	if (getSize()>0) {
		//char buffer[100];
		for (unsigned int i=0, n=vect->size(); i<n; i++)
		{
			Coordinate& c=(*vect)[i];
			if ( i ) result.append(", ");
			result.append(c.toString());
		}
	}
	result.append(")");

	return result;
}

CoordinateArraySequence::~CoordinateArraySequence()
{
	delete vect;
}

void
CoordinateArraySequence::expandEnvelope(Envelope &env) const
{
	unsigned int size = vect->size();
	for (unsigned int i=0; i<size; i++) env.expandToInclude((*vect)[i]);
}

double
CoordinateArraySequence::getOrdinate(unsigned int index, unsigned int ordinateIndex) const
{

	assert(index<vect->size());

	switch (ordinateIndex)
	{
		case CoordinateSequence::X:
			return (*vect)[index].x;
		case CoordinateSequence::Y:
			return (*vect)[index].y;
		case CoordinateSequence::Z:
			return (*vect)[index].z;
		default:
			return DoubleNotANumber;
	}
}

void
CoordinateArraySequence::setOrdinate(unsigned int index, unsigned int ordinateIndex,
	double value)
{

	assert(index<vect->size());

	switch (ordinateIndex)
	{
		case CoordinateSequence::X:
			(*vect)[index].x = value;
		case CoordinateSequence::Y:
			(*vect)[index].y = value;
		case CoordinateSequence::Z:
			(*vect)[index].z = value;
		default:
			return;
	}
}

void
CoordinateArraySequence::apply_rw(const CoordinateFilter *filter)
{
	for (vector<Coordinate>::iterator i=vect->begin(), e=vect->end(); i!=e; ++i)
	{
		filter->filter_rw(&(*i));
	}
}

void
CoordinateArraySequence::apply_ro(CoordinateFilter *filter) const
{
	for (vector<Coordinate>::const_iterator i=vect->begin(), e=vect->end(); i!=e; ++i)
	{
		filter->filter_ro(&(*i));
	}
}

} // namespace geos::geom
} //namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.7  2006/03/27 09:00:50  strk
 * Bug #79 - Small fix in CoordinateArraySequence::toString()
 *
 * Revision 1.6  2006/03/22 16:58:34  strk
 * Removed (almost) all inclusions of geom.h.
 * Removed obsoleted .cpp files.
 * Fixed a bug in WKTReader not using the provided CoordinateSequence
 * implementation, optimized out some memory allocations.
 *
 **********************************************************************/

