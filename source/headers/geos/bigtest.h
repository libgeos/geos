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
 **********************************************************************
 * $Log$
 * Revision 1.2  2004/07/08 19:34:49  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added DefaultCoordinateSequenceFactory::instance() function.
 *
 * Revision 1.1  2004/07/02 13:20:42  strk
 * Header files moved under geos/ dir.
 *
 * Revision 1.4  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/


#ifndef GEOS_BIGTEST_H
#define GEOS_BIGTEST_H

#include <memory>
#include <geos/geom.h>

using namespace std;
using namespace geos;

class GeometryTestFactory {
public:
	static Polygon* createBox(GeometryFactory *fact,double minx,double miny,int nSide,double segLen);
	static CoordinateSequence* createBox(double minx,double miny,int nSide,double segLen);
	static CoordinateSequence* createCircle(double basex,double basey,double size,int nPts);
	static Polygon* createCircle(GeometryFactory *fact,double basex,double basey,double size,int nPts);
	static CoordinateSequence* createSineStar(double basex,double basey,double size,double armLen,int nArms,int nPts);
	static Polygon* createSineStar(GeometryFactory *fact,double basex,double basey,double size,double armLen,int nArms,int nPts);
};

#endif
