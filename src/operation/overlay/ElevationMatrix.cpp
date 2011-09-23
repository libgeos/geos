/**********************************************************************
 * $Id: ElevationMatrix.cpp 2873 2010-01-18 18:15:40Z mloskot $
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
 ***********************************************************************
 *
 * Last port: original (by strk)
 *
 **********************************************************************/

#include <geos/platform.h>
#include <geos/operation/overlay/ElevationMatrix.h>
#include <geos/util/IllegalArgumentException.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>

#include <cassert>
#include <cmath>
#include <iostream>
#include <sstream>
#include <string>

#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif
#define PARANOIA_LEVEL 0

#ifdef _MSC_VER
#pragma warning(disable:4355)
#endif

using namespace std;
using namespace geos::geom;

namespace geos {
namespace operation { // geos.operation
namespace overlay { // geos.operation.overlay

ElevationMatrixFilter::ElevationMatrixFilter(ElevationMatrix &newEm):
	em(newEm)
{ }

ElevationMatrixFilter::~ElevationMatrixFilter()
{ }

void
ElevationMatrixFilter::filter_rw(Coordinate *c) const
{
#if GEOS_DEBUG
	cerr<<"ElevationMatrixFilter::filter_rw("<<c->toString()<<") called"
		<<endl;
#endif

	// already has a Z value, nothing to do
	if ( ! ISNAN(c->z) ) return;

	double avgElevation = em.getAvgElevation();

	try {
		const ElevationMatrixCell &emc = em.getCell(*c);
		c->z = emc.getAvg();
		if ( ISNAN(c->z) ) c->z = avgElevation;
#if GEOS_DEBUG
		cerr<<"  z set to "<<c->z<<endl;
#endif
	} catch (const util::IllegalArgumentException& /* ex */) {
		c->z = avgElevation;
	}
}

void
ElevationMatrixFilter::filter_ro(const Coordinate *c)
{
#if GEOS_DEBUG
	cerr<<"ElevationMatrixFilter::filter_ro("<<c->toString()<<") called"
		<<endl;
#endif
	em.add(*c);
}


ElevationMatrix::ElevationMatrix(const Envelope &newEnv,
		unsigned int newRows, unsigned int newCols):
	filter(*this),
	env(newEnv), cols(newCols), rows(newRows),
	avgElevationComputed(false),
	avgElevation(DoubleNotANumber),
 	cells(newRows*newCols)
{
	cellwidth=env.getWidth()/cols;
	cellheight=env.getHeight()/rows;
	if ( ! cellwidth ) cols=1;
	if ( ! cellheight ) rows=1;
}

ElevationMatrix::~ElevationMatrix()
{
}

void
ElevationMatrix::add(const Geometry *geom)
{
#if GEOS_DEBUG
	cerr<<"ElevationMatrix::add(Geometry *) called"<<endl;
#endif // GEOS_DEBUG

	// Cannot add Geometries to an ElevationMatrix after it's average
	// elevation has been computed
	assert(!avgElevationComputed);

	//ElevationMatrixFilter filter(this);
	geom->apply_ro(&filter);

}

#if 0
void
ElevationMatrix::add(const CoordinateSequence *cs)
{
	unsigned int ncoords = cs->getSize();
	for (unsigned int i=0; i<ncoords; i++)
	{
		add(cs->getAt(i));
	}
}
#endif

void
ElevationMatrix::add(const Coordinate &c)
{
	if ( ISNAN(c.z) ) return;
	try {
		ElevationMatrixCell &emc = getCell(c);
		emc.add(c);
	} catch (const util::IllegalArgumentException& exp) {
		// coordinate do not overlap matrix
		cerr << "ElevationMatrix::add(" << c.toString()
		     << "): Coordinate does not overlap grid extent: "
		     << exp.what() << endl;
		return;
	}
}

ElevationMatrixCell &
ElevationMatrix::getCell(const Coordinate &c) 
{
	int col, row;

	if ( ! cellwidth ) col=0;
	else
	{
		double xoffset = c.x - env.getMinX();
		col = (int)(xoffset/cellwidth);
		if ( col == (int)cols ) col = cols-1;
	}
	if ( ! cellheight ) row=0;
	else
	{
		double yoffset = c.y - env.getMinY();
		row = (int)(yoffset/cellheight);
		if ( row == (int)rows ) row = rows-1;
	}
	int celloffset = (cols*row)+col;

	if  (celloffset<0 || celloffset >= (int)(cols*rows))
	{
		ostringstream s;
		s<<"ElevationMatrix::getCell got a Coordinate out of grid extent ("<<env.toString()<<") - cols:"<<cols<<" rows:"<<rows;
		throw util::IllegalArgumentException(s.str());
	}

	return cells[celloffset];
}

const ElevationMatrixCell &
ElevationMatrix::getCell(const Coordinate &c) const
{
	return (const ElevationMatrixCell &)
		((ElevationMatrix *)this)->getCell(c);
}

double
ElevationMatrix::getAvgElevation() const
{
	if ( avgElevationComputed ) return avgElevation;
	double ztot=0;
	int zvals=0;
	for (unsigned int r=0; r<rows; r++)
	{
		for(unsigned int c=0; c<cols; c++)
		{
			const ElevationMatrixCell &cell = cells[(r*cols)+c];
			double e = cell.getAvg();
			if ( !ISNAN(e) )
			{
				zvals++;
				ztot+=e;
			}
		}
	}
	if ( zvals ) avgElevation = ztot/zvals;
	else avgElevation = DoubleNotANumber;

	avgElevationComputed = true;

	return avgElevation;
}

string
ElevationMatrix::print() const
{
	ostringstream ret;
	ret << "Cols:"<<cols<<" Rows:"<<rows<<" AvgElevation:"<<getAvgElevation()<<endl;
	for (unsigned int r=0; r<rows; r++)
	{
		for (unsigned int c=0; c<cols; c++)
		{
			ret << cells[(r*cols)+c].print() << '\t';
		}
		ret << endl;
	}
	return ret.str();
}

void
ElevationMatrix::elevate(Geometry *g) const
{

	// Nothing to do if no elevation info in matrix
	if ( ISNAN(getAvgElevation()) ) return;

	g->apply_rw(&filter);
}

} // namespace geos.operation.overlay
} // namespace geos.operation
} // namespace geos;

/**********************************************************************
 * $Log$
 * Revision 1.15  2006/04/04 08:16:46  strk
 * Changed GEOSException hierarchy to be derived from std::runtime_exception.
 * Removed the GEOSException::toString redundant method (use ::what() instead)
 *
 * Revision 1.14  2006/03/17 13:24:59  strk
 * opOverlay.h header splitted. Reduced header inclusions in operation/overlay implementation files. ElevationMatrixFilter code moved from own file to ElevationMatrix.cpp (ideally a class-private).
 *
 * Revision 1.13  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 * Revision 1.12  2006/03/06 19:40:47  strk
 * geos::util namespace. New GeometryCollection::iterator interface, many cleanups.
 *
 * Revision 1.11  2006/03/03 10:46:22  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.10  2006/03/02 12:12:01  strk
 * Renamed DEBUG macros to GEOS_DEBUG, all wrapped in #ifndef block to allow global override (bug#43)
 *
 * Revision 1.9  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.8  2006/02/09 15:52:47  strk
 * GEOSException derived from std::exception; always thrown and cought by const ref.
 *
 * Revision 1.7  2005/12/11 10:41:56  strk
 * Fixed premature initialization of average Z value in ElevationMatrixFilter
 *
 **********************************************************************/
