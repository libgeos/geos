/**********************************************************************
 * $Id: CoordinateArraySequence.cpp 3278 2011-04-13 10:44:04Z strk $
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
#include <geos/util/IllegalArgumentException.h>

#include <sstream>
#include <cassert>
#include <algorithm>
#include <vector>
#include <cmath>

using namespace std;

namespace geos {
namespace geom { // geos::geom

CoordinateArraySequence::CoordinateArraySequence():
	vect(new vector<Coordinate>()),
        dimension(3)
{
}

CoordinateArraySequence::CoordinateArraySequence(size_t n, 
                                                 size_t dimension_in ):
	vect(new vector<Coordinate>(n)),
        dimension(dimension_in)
{
}

CoordinateArraySequence::CoordinateArraySequence(
    vector<Coordinate> *coords, size_t dimension_in )
        : vect(coords), dimension(dimension_in)
{
	if ( ! vect ) vect = new vector<Coordinate>();
}

CoordinateArraySequence::CoordinateArraySequence(
    const CoordinateArraySequence &c )
	:
	CoordinateSequence(c),
	vect(new vector<Coordinate>(*(c.vect))),
        dimension(c.getDimension())
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

std::size_t 
CoordinateArraySequence::getDimension() const
{
    if( dimension != 0 )
        return dimension;

    if( vect->size() == 0 )
        return 3;

    if( ISNAN((*vect)[0].z) )
        dimension = 2;
    else
        dimension = 3;

    return dimension;
}

void
CoordinateArraySequence::toVector(vector<Coordinate>& out) const
{
	// TODO: can this be optimized ?
	out.insert(out.end(), vect->begin(), vect->end());
}

void
CoordinateArraySequence::add(const Coordinate& c)
{
	vect->push_back(c);
}

void
CoordinateArraySequence::add(const Coordinate& c, bool allowRepeated)
{
	if (!allowRepeated && ! vect->empty() )
	{
		const Coordinate& last=vect->back();
		if (last.equals2D(c)) return;
	}
	vect->push_back(c);
}

/*public*/
void
CoordinateArraySequence::add(size_t i, const Coordinate& coord,
                                       bool allowRepeated)
{
    // don't add duplicate coordinates
    if (! allowRepeated) {
      size_t sz = size();
      if (sz > 0) {
        if (i > 0) {
          const Coordinate& prev = getAt(i - 1);
          if (prev.equals2D(coord)) return;
        }
        if (i < sz) {
          const Coordinate& next = getAt(i);
          if (next.equals2D(coord)) return;
        }
      }
    }

    vect->insert(vect->begin()+i, coord);
}

size_t
CoordinateArraySequence::getSize() const
{
	return vect->size();
}

const Coordinate &
CoordinateArraySequence::getAt(size_t pos) const
{
	return (*vect)[pos];
}

void
CoordinateArraySequence::getAt(size_t pos, Coordinate &c) const
{
	c=(*vect)[pos];
}

void
CoordinateArraySequence::setAt(const Coordinate& c, size_t pos)
{
	(*vect)[pos]=c;
}

void
CoordinateArraySequence::deleteAt(size_t pos)
{
	vect->erase(vect->begin()+pos);
}

string
CoordinateArraySequence::toString() const
{
	string result("(");
	if (getSize()>0) {
		//char buffer[100];
		for (size_t i=0, n=vect->size(); i<n; i++)
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
	for (size_t i=0, n=vect->size(); i<n; ++i)
		env.expandToInclude((*vect)[i]);
}

double
CoordinateArraySequence::getOrdinate(size_t index, size_t ordinateIndex) const
{
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
CoordinateArraySequence::setOrdinate(size_t index, size_t ordinateIndex,
	double value)
{
	switch (ordinateIndex)
	{
		case CoordinateSequence::X:
			(*vect)[index].x = value;
			break;
		case CoordinateSequence::Y:
			(*vect)[index].y = value;
			break;
		case CoordinateSequence::Z:
			(*vect)[index].z = value;
			break;
		default:
		{
			std::stringstream ss;
			ss << "Unknown ordinate index " << index;
			throw util::IllegalArgumentException(ss.str());
			break;
		}
	}
}

void
CoordinateArraySequence::apply_rw(const CoordinateFilter *filter)
{
	for (vector<Coordinate>::iterator i=vect->begin(), e=vect->end(); i!=e; ++i)
	{
		filter->filter_rw(&(*i));
	}
	dimension = 0; // re-check (see http://trac.osgeo.org/geos/ticket/435)
}

void
CoordinateArraySequence::apply_ro(CoordinateFilter *filter) const
{
	for (vector<Coordinate>::const_iterator i=vect->begin(), e=vect->end(); i!=e; ++i)
	{
		filter->filter_ro(&(*i));
	}
}

CoordinateSequence&
CoordinateArraySequence::removeRepeatedPoints()
{
	// We use == operator, which is 2D only
	vector<Coordinate>::iterator new_end = \
		std::unique(vect->begin(), vect->end());
	
	vect->erase(new_end, vect->end());

	return *this;
}

} // namespace geos::geom
} //namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.10  2006/06/12 15:07:47  strk
 * explicitly invoked CoordinateSequence (copy) ctor - suggested by GCC warning.
 *
 * Revision 1.9  2006/06/12 10:10:39  strk
 * Fixed getGeometryN() to take size_t rather then int, changed unsigned int parameters to size_t.
 *
 * Revision 1.8  2006/05/03 08:58:34  strk
 * added new non-static CoordinateSequence::removeRepeatedPoints() mutator.
 *
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

