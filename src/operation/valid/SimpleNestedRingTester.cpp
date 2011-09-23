/**********************************************************************
 * $Id: SimpleNestedRingTester.cpp 2572 2009-06-08 22:10:55Z strk $
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
 **********************************************************************
 *
 * Last port: operation/valid/SimpleNestedRingTester.java rev. 1.14 (JTS-1.10)
 *
 **********************************************************************/

#include <geos/operation/valid/SimpleNestedRingTester.h>
#include <geos/operation/valid/IsValidOp.h>
#include <geos/algorithm/CGAlgorithms.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/Envelope.h>

#include <cassert>

using namespace geos::algorithm;
using namespace geos::geom;

namespace geos {
namespace operation { // geos.operation
namespace valid { // geos.operation.valid

bool
SimpleNestedRingTester::isNonNested()
{
	for(size_t i=0, ni=rings.size(); i<ni; i++) {
		LinearRing* innerRing=rings[i];
		CoordinateSequence *innerRingPts=innerRing->getCoordinates();
		for(size_t j=0, nj=rings.size(); j<nj; j++) {
			LinearRing* searchRing=rings[j];
			CoordinateSequence *searchRingPts=searchRing->getCoordinates();
			if (innerRing==searchRing)
				continue;
			if (!innerRing->getEnvelopeInternal()->intersects(searchRing->getEnvelopeInternal()))
				continue;
			const Coordinate *innerRingPt=IsValidOp::findPtNotNode(innerRingPts,searchRing,graph);
			// Unable to find a ring point not a node of the search ring
			assert(innerRingPt!=NULL);

			bool isInside=CGAlgorithms::isPointInRing(*innerRingPt,searchRingPts);
			if (isInside) {
				/*
				 * innerRingPt is const just because the input
				 * CoordinateSequence is const. If the input
				 * Polygon survives lifetime of this object
				 * we are safe.
				 */
				nestedPt=const_cast<Coordinate *>(innerRingPt);
				return false;
			}
		}
	}
	return true;
}

} // namespace geos.operation.valid
} // namespace geos.operation
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.16  2006/06/12 11:29:24  strk
 * unsigned int => size_t
 *
 * Revision 1.15  2006/03/20 16:57:44  strk
 * spatialindex.h and opValid.h headers split
 *
 * Revision 1.14  2006/03/17 16:48:56  strk
 * LineIntersector and PointLocator made complete components of RelateComputer
 * (were statics const pointers before). Reduced inclusions from opRelate.h
 * and opValid.h, updated .cpp files to allow build.
 *
 * Revision 1.13  2006/03/06 19:40:47  strk
 * geos::util namespace. New GeometryCollection::iterator interface, many cleanups.
 *
 * Revision 1.12  2006/02/19 19:46:50  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.11  2006/01/31 19:07:34  strk
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
 * Revision 1.10  2005/11/04 11:04:09  strk
 * Ported revision 1.38 of IsValidOp.java (adding closed Ring checks).
 * Changed NestedRingTester classes to use Coorinate pointers
 * rather then actual objects, to speedup NULL tests.
 * Added JTS port revision when applicable.
 *
 * Revision 1.9  2004/07/08 19:34:50  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added CoordinateArraySequenceFactory::instance() function.
 *
 * Revision 1.8  2004/07/02 13:28:29  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.7  2004/03/29 06:59:25  ybychkov
 * "noding/snapround" package ported (JTS 1.4);
 * "operation", "operation/valid", "operation/relate" and "operation/overlay" upgraded to JTS 1.4;
 * "geom" partially upgraded.
 *
 * Revision 1.6  2003/11/07 01:23:43  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/

