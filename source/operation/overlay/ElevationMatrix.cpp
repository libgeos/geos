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

#define DEBUG 0
#define ASSERT 1

namespace geos
{

ElevationMatrix::ElevationMatrix(const Envelope &newEnv,
	int newRows, int newCols): env(newEnv), rows(newRows),
	cols(newCols), cells(newRows*newCols),
	avgElevationComputed(false), avgElevation(DoubleNotANumber)
{
	cellwidth=env.getWidth()/cols;
	cellheight=env.getHeight()/rows;
}

ElevationMatrix::~ElevationMatrix()
{
}

void
ElevationMatrix::add(const Geometry *geom)
{
#if DEBUG
	cerr<<"ElevationMatrix::add(Geometry *) called"<<endl;
#endif // DEBUG

#if ASSERT
	Assert::isTrue(!avgElevationComputed, "Cannot add Geometries to an ElevationMatrix after it's average elevation has been computed");
#endif

	// Todo: optimize so to not require a CoordinateSequence copy
	CoordinateSequence *cs = geom->getCoordinates();
	add(cs);
	delete cs;
}

void
ElevationMatrix::add(const CoordinateSequence *cs)
{
	unsigned int ncoords = cs->getSize();
	for (unsigned int i=0; i<ncoords; i++)
	{
		add(cs->getAt(i));
	}
}

void
ElevationMatrix::add(const Coordinate &c)
{
	if ( !FINITE(c.z) ) return;
	try {
		ElevationMatrixCell &emc = getCell(c);
		emc.add(c);
	} catch (IllegalArgumentException *exp) {
		// coordinate do not overlap matrix
		cerr<<"ElevationMatrix::add("<<c.toString()<<"): Coordinate does not overlap grid extent: "<<exp->toString()<<endl;
		delete exp;
		return;
	}
}

ElevationMatrixCell &
ElevationMatrix::getCell(const Coordinate &c) 
{
	double xoffset = c.x - env.getMinX();
	double yoffset = c.y - env.getMinY();
	int col = (int)(xoffset/cellwidth);
	if ( col == cols ) col = cols-1;
	int row = (int)(yoffset/cellheight);
	if ( row == rows ) row = rows-1;
	int celloffset = (cols*row)+col;

	if  (celloffset<0 || celloffset >= cols*rows)
	{
		ostringstream s;
		s<<"ElevationMatrix::getCell got a Coordinate out of grid extent ("<<env.toString()<<")";
		throw new IllegalArgumentException(s.str());
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
			if ( FINITE(e) )
			{
				zvals++;
				ztot+=e;
			}
		}
	}
	if ( zvals ) avgElevation = ztot/zvals;
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
	ElevationMatrixFilter filter(this);
	g->apply_rw(&filter);
}

} // namespace geos;

/**********************************************************************
 * $Log$
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
