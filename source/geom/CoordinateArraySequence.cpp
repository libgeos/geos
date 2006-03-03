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

#include <geos/geom.h>
#include <geos/util.h>
#include <cassert>
#include <vector>

using namespace std;

namespace geos {

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

} //namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.3  2006/03/03 10:46:21  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.2  2006/02/28 14:34:03  strk
 * Added many assertions and debugging output hunting for a bug in BufferOp
 *
 * Revision 1.1  2006/01/31 19:07:33  strk
 * - Renamed DefaultCoordinateSequence to CoordinateArraySequence.
 * - Moved GetNumGeometries() and GetGeometryN() interfaces
 *   from GeometryCollection to Geometry class.
 * - Added getAt(int pos, Coordinate &to) funtion to CoordinateSequence class.
 * - Reworked automake scripts to produce a static lib for each subdir and
 *   then link all subsystem's libs togheter
 * - Moved C-API in it's own top-level dir capi/
 * - Moved source/bigtest and source/test to tests/bigtest and test/xmltester
 * - Fixed PointLocator handling of LinearRings
 * - Changed CoordinateArrayFilter to reduce memory copies
 * - Changed UniqueCoordinateArrayFilter to reduce memory copies
 * - Added CGAlgorithms::isPointInRing() version working with
 *   Coordinate::ConstVect type (faster!)
 * - Ported JTS-1.7 version of ConvexHull with big attention to
 *   memory usage optimizations.
 * - Improved XMLTester output and user interface
 * - geos::geom::util namespace used for geom/util stuff
 * - Improved memory use in geos::geom::util::PolygonExtractor
 * - New ShortCircuitedGeometryVisitor class
 * - New operation/predicate package
 *
 **********************************************************************/

