/**********************************************************************
 * $Id: bigtest.h 1861 2006-10-12 15:33:40Z strk $
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
 * Revision 1.1  2006/01/31 19:07:35  strk
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
 * Revision 1.1  2004/07/19 10:35:23  strk
 * bigtest.h moved to local dir
 *
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

using namespace std;
using namespace geos;

// Forward declaration
namespace geos {
	namespace geom {
		class Polygon;
		class CoordinateSequence;
		class GeometryFactory;
	}
}

class GeometryTestFactory {
public:
	static geom::Polygon* createBox(geom::GeometryFactory *fact,double minx,double miny,int nSide,double segLen);
	static geom::CoordinateSequence* createBox(double minx,double miny,int nSide,double segLen);
	static geom::CoordinateSequence* createCircle(double basex,double basey,double size,int nPts);
	static geom::Polygon* createCircle(geom::GeometryFactory *fact,double basex,double basey,double size,int nPts);
	static geom::CoordinateSequence* createSineStar(double basex,double basey,double size,double armLen,int nArms,int nPts);
	static geom::Polygon* createSineStar(geom::GeometryFactory *fact,double basex,double basey,double size,double armLen,int nArms,int nPts);
};

#endif
