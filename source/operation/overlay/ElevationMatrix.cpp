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

#include <sstream>
#include <geos/geom.h>
#include <geos/opOverlay.h>

#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif
#define PARANOIA_LEVEL 0

namespace geos {
namespace operation { // geos.operation
namespace overlay { // geos.operation.overlay

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

#if PARANOIA_LEVEL > 0
	Assert::isTrue(!avgElevationComputed, "Cannot add Geometries to an ElevationMatrix after it's average elevation has been computed");
#endif

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
	} catch (const IllegalArgumentException& exp) {
		// coordinate do not overlap matrix
		cerr<<"ElevationMatrix::add("<<c.toString()<<"): Coordinate does not overlap grid extent: "<<exp.toString()<<endl;
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
		throw  IllegalArgumentException(s.str());
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
 * Revision 1.6  2005/12/08 14:14:07  strk
 * ElevationMatrixFilter used for both elevation and Matrix fill,
 * thus removing CoordinateSequence copy in ElevetaionMatrix::add(Geometry *).
 * Changed CoordinateFilter::filter_rw to be a const method: updated
 * all apply_rw() methods to take a const CoordinateFilter.
 *
 * Revision 1.5  2004/12/08 13:54:44  strk
 * gcc warnings checked and fixed, general cleanups.
 *
 * Revision 1.4  2004/11/29 16:05:33  strk
 * Fixed a bug in LineIntersector::interpolateZ causing NaN values
 * to come out.
 * Handled dimensional collapses in ElevationMatrix.
 * Added ISNAN macro and changed ISNAN/FINITE macros to avoid
 * dispendious isnan() and finite() calls.
 *
 * Revision 1.3  2004/11/26 09:22:50  strk
 * Added FINITE(x) macro and its use.
 * Made input geoms average Z computation optional in OverlayOp.
 *
 * Revision 1.2  2004/11/24 12:29:36  strk
 * Handled boundary cases in ::getCell
 *
 * Revision 1.1  2004/11/23 16:22:49  strk
 * Added ElevationMatrix class and components to do post-processing draping of overlayed geometries.
 *
 **********************************************************************/
