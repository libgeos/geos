/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/geom.h>
#include <geos/util.h>

namespace geos {

#define PARANOIA_LEVEL 1

DefaultCoordinateSequence::DefaultCoordinateSequence():
	vect(new vector<Coordinate>())
{
}

DefaultCoordinateSequence::DefaultCoordinateSequence(unsigned int n):
	vect(new vector<Coordinate>(n))
{
}

DefaultCoordinateSequence::DefaultCoordinateSequence(
	vector<Coordinate> *coords): vect(coords)
{
	if ( ! vect ) vect = new vector<Coordinate>();
}

DefaultCoordinateSequence::DefaultCoordinateSequence(
	const DefaultCoordinateSequence &c):
		vect(new vector<Coordinate>(*(c.vect)))
{
}

CoordinateSequence *
DefaultCoordinateSequence::clone() const
{
	return new DefaultCoordinateSequence(*this);
}

void
DefaultCoordinateSequence::setPoints(const vector<Coordinate> &v)
{
	vect->assign(v.begin(), v.end());
}

const vector<Coordinate>*
DefaultCoordinateSequence::toVector() const
{
	return vect; //new vector<Coordinate>(vect->begin(),vect->end());
}

bool
DefaultCoordinateSequence::isEmpty() const
{
	return vect->empty();
}

void
DefaultCoordinateSequence::add(const Coordinate& c)
{
	vect->push_back(c);
}

unsigned int
DefaultCoordinateSequence::getSize() const
{
	return vect->size();
}

const Coordinate &
DefaultCoordinateSequence::getAt(unsigned int pos) const
{
#if PARANOIA_LEVEL > 0
	if (pos<0 || pos>=vect->size()) 
	throw IllegalArgumentException("Coordinate number out of range");
#endif
	return (*vect)[pos];
}

void
DefaultCoordinateSequence::setAt(const Coordinate& c, unsigned int pos)
{
#if PARANOIA_LEVEL > 0
	if (pos<0 || pos>=vect->size()) 
	throw IllegalArgumentException("Coordinate number out of range");
#endif
	(*vect)[pos]=c;
}

void
DefaultCoordinateSequence::deleteAt(unsigned int pos)
{
#if PARANOIA_LEVEL > 0
	if (pos<0 || pos>=vect->size()) 
	throw IllegalArgumentException("Coordinate number out of range");
#endif
	vect->erase(vect->begin()+pos);
}

string
DefaultCoordinateSequence::toString() const
{
	string result("");
	if (getSize()>0) {
		//char buffer[100];
		for (unsigned int i=0; i<vect->size(); i++) {
			Coordinate& c=(*vect)[i];
			//sprintf(buffer,"(%g,%g,%g) ",c.x,c.y,c.z);
			//result.append(buffer);
			result.append(c.toString());
		}
		result.append("");
	}
	return result;
}

DefaultCoordinateSequence::~DefaultCoordinateSequence()
{
	delete vect;
}

void
DefaultCoordinateSequence::expandEnvelope(Envelope &env) const
{
	unsigned int size = vect->size();
	for (unsigned int i=0; i<size; i++) env.expandToInclude((*vect)[i]);
}

double
DefaultCoordinateSequence::getOrdinate(unsigned int index, unsigned int ordinateIndex) const
{

#if PARANOIA_LEVEL > 0
	if ( index < 0 || index >= vect->size() ) 
	throw IllegalArgumentException("Coordinate number out of range");
#endif

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
DefaultCoordinateSequence::setOrdinate(unsigned int index, unsigned int ordinateIndex,
	double value)
{

#if PARANOIA_LEVEL > 0
	if ( index < 0 || index >= vect->size() ) 
	throw IllegalArgumentException("Coordinate number out of range");
#endif

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
DefaultCoordinateSequence::apply_rw(const CoordinateFilter *filter)
{
	for (vector<Coordinate>::iterator i=vect->begin(), e=vect->end(); i!=e; ++i)
	{
		filter->filter_rw(&(*i));
	}
}

void
DefaultCoordinateSequence::apply_ro(CoordinateFilter *filter) const
{
	for (vector<Coordinate>::const_iterator i=vect->begin(), e=vect->end(); i!=e; ++i)
	{
		filter->filter_ro(&(*i));
	}
}

} //namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.10  2005/12/08 14:14:07  strk
 * ElevationMatrixFilter used for both elevation and Matrix fill,
 * thus removing CoordinateSequence copy in ElevetaionMatrix::add(Geometry *).
 * Changed CoordinateFilter::filter_rw to be a const method: updated
 * all apply_rw() methods to take a const CoordinateFilter.
 *
 * Revision 1.9  2005/12/07 22:52:03  strk
 * Added CoordinateSequence::apply_rw(CoordinateFilter *) and
 * CoordinateSequence::apply_ro(CoordinateFilter *) const
 * to reduce coordinate copies on read-write CoordinateFilter
 * applications (previously required getAt()/setAt() calls).
 * Undefined PROFILE_COORDINATE_COPIES (erroneously left defined by previous commit)
 *
 * Revision 1.8  2005/11/29 14:39:08  strk
 * More signed/unsigned fixes
 *
 * Revision 1.7  2005/11/24 23:09:15  strk
 * CoordinateSequence indexes switched from int to the more
 * the correct unsigned int. Optimizations here and there
 * to avoid calling getSize() in loops.
 * Update of all callers is not complete yet.
 *
 * Revision 1.6  2005/11/08 12:32:41  strk
 * Cleanups, ::setPoint small improvement
 *
 * Revision 1.5  2005/04/29 11:52:40  strk
 * Added new JTS interfaces for CoordinateSequence and factories,
 * removed example implementations to reduce maintainance costs.
 * Added first implementation of WKBWriter, made ByteOrderDataInStream
 * a template class.
 *
 * Revision 1.4  2005/01/28 07:58:04  strk
 * removed sprintf usage, ad ::toString call Coordinate::toString
 *
 * Revision 1.3  2004/12/03 22:52:56  strk
 * enforced const return of CoordinateSequence::toVector() method to derivate classes.
 *
 * Revision 1.2  2004/11/23 16:22:49  strk
 * Added ElevationMatrix class and components to do post-processing draping of overlayed geometries.
 *
 * Revision 1.1  2004/07/08 19:38:56  strk
 * renamed from *List* equivalents
 *
 **********************************************************************/

