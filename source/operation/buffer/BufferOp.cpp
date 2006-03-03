/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2005-2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: operation/buffer/BufferOp.java rev. 1.31 (JTS-1.7)
 *
 **********************************************************************/

#include <geos/opBuffer.h>
#include <geos/noding.h>
#include <geos/nodingSnapround.h>
#include <geos/precision.h>
#include <geos/profiler.h>
#include <algorithm>

#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif

//#define PROFILE 1

using namespace std;
using namespace geos::noding;
using namespace geos::noding::snapround;

namespace geos {
namespace operation { // geos.operation
namespace buffer { // geos.operation.buffer

#if PROFILE
static Profiler *profiler = Profiler::instance();
#endif

int BufferOp::MAX_PRECISION_DIGITS=12;


/*private*/
double
BufferOp::precisionScaleFactor(const Geometry *g, double distance,
	int maxPrecisionDigits)
{
	const Envelope *env=g->getEnvelopeInternal();
	double envSize=max(env->getHeight(), env->getWidth());
	double expandByDistance=distance > 0.0 ? distance : 0.0;
	double bufEnvSize=envSize + 2 * expandByDistance;
	// the smallest power of 10 greater than the buffer envelope
	int bufEnvLog10=(int) (log(bufEnvSize) / log(10.0) + 1.0);
	int minUnitLog10=bufEnvLog10 - maxPrecisionDigits;
	// scale factor is inverse of min Unit size, so flip sign of exponent
	double scaleFactor=pow(10.0,-minUnitLog10);
	return scaleFactor;
}

/*public static*/
Geometry*
BufferOp::bufferOp(const Geometry *g, double distance,
		int quadrantSegments,
		int nEndCapStyle)
{
	BufferOp bufOp(g);
	bufOp.setQuadrantSegments(quadrantSegments);
	bufOp.setEndCapStyle(nEndCapStyle);
	return bufOp.getResultGeometry(distance);
}

/*public*/
Geometry*
BufferOp::getResultGeometry(double nDistance)
{
	distance=nDistance;
	computeGeometry();
	return resultGeometry;
}

/*public*/
Geometry*
BufferOp::getResultGeometry(double nDistance, int nQuadrantSegments)
{
	distance=nDistance;
	setQuadrantSegments(nQuadrantSegments);
	computeGeometry();
	return resultGeometry;
}

/*private*/
void
BufferOp::computeGeometry()
{
#if GEOS_DEBUG
	cerr<<"BufferOp::computeGeometry: trying with original precision"<<endl;
#endif

	bufferOriginalPrecision();

	if (resultGeometry!=NULL) return;

	const PrecisionModel& argPM = *(argGeom->getFactory()->getPrecisionModel());
	if ( argPM.getType() == PrecisionModel::FIXED )
		bufferFixedPrecision(argPM);
	else
		bufferReducedPrecision();
}

/*private*/
void
BufferOp::bufferReducedPrecision()
{

	// try and compute with decreasing precision
	for (int precDigits=MAX_PRECISION_DIGITS; precDigits >= 0; precDigits--)
	{
#if GEOS_DEBUG
		cerr<<"BufferOp::computeGeometry: trying with precDigits "<<precDigits<<endl;
#endif
		try {
			bufferFixedPrecision(precDigits);
		} catch (const TopologyException& ex) {
			saveException=ex;
			// don't propagate the exception - it will be detected by fact that resultGeometry is null
		} 

		if (resultGeometry!=NULL) {
			// debug
			//if ( saveException ) cerr<<saveException->toString()<<endl;
			return;
		}
	}
	// tried everything - have to bail
	throw saveException;
}

/*private*/
void
BufferOp::bufferOriginalPrecision()
{
	BufferBuilder bufBuilder;
	bufBuilder.setQuadrantSegments(quadrantSegments);
	bufBuilder.setEndCapStyle(endCapStyle);

	//cerr<<"computing with original precision"<<endl;
	try
	{
		resultGeometry=bufBuilder.buffer(argGeom, distance);
	}
	catch (const TopologyException& ex)
	{
		// don't propagate the exception - it will be detected by
		// fact that resultGeometry is null
		saveException=ex;

		//cerr<<ex->toString()<<endl;
	} 
	//cerr<<"done"<<endl;
}

void
BufferOp::bufferReducedPrecision(int precisionDigits)
{
	double sizeBasedScaleFactor=precisionScaleFactor(argGeom, distance, precisionDigits);
	PrecisionModel fixedPM(sizeBasedScaleFactor);
	bufferFixedPrecision(fixedPM);
}

/*private*/
void
BufferOp::bufferFixedPrecision(const PrecisionModel& fixedPM)
{
	PrecisionModel pm(1.0); // fixed as well
	MCIndexSnapRounder snapRounder(pm);
	ScaledNoder noder(snapRounder, fixedPM.getScale());

	BufferBuilder bufBuilder;
	bufBuilder.setWorkingPrecisionModel(&fixedPM);

	bufBuilder.setNoder(&noder);

	bufBuilder.setQuadrantSegments(quadrantSegments);
	bufBuilder.setEndCapStyle(endCapStyle);

	// this may throw an exception, if robustness errors are encountered
	resultGeometry=bufBuilder.buffer(argGeom, distance);
}

} // namespace geos.operation.buffer
} // namespace geos.operation
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.44  2006/03/03 10:46:21  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.43  2006/03/02 12:12:01  strk
 * Renamed DEBUG macros to GEOS_DEBUG, all wrapped in #ifndef block to allow global override (bug#43)
 *
 * Revision 1.42  2006/02/23 20:05:21  strk
 * Fixed bug in MCIndexNoder constructor making memory checker go crazy, more
 * doxygen-friendly comments, miscellaneous cleanups
 *
 * Revision 1.41  2006/02/23 11:54:20  strk
 * - MCIndexPointSnapper
 * - MCIndexSnapRounder
 * - SnapRounding BufferOp
 * - ScaledNoder
 * - GEOSException hierarchy cleanups
 * - SpatialIndex memory-friendly query interface
 * - GeometryGraph::getBoundaryNodes memory-friendly
 * - NodeMap::getBoundaryNodes memory-friendly
 * - Cleanups in geomgraph::Edge
 * - Added an XML test for snaprounding buffer (shows leaks, working on it)
 *
 * Revision 1.40  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.39  2006/02/09 15:52:47  strk
 * GEOSException derived from std::exception; always thrown and cought by const ref.
 *
 * Revision 1.38  2006/02/08 17:18:28  strk
 * - New WKTWriter::toLineString and ::toPoint convenience methods
 * - New IsValidOp::setSelfTouchingRingFormingHoleValid method
 * - New Envelope::centre()
 * - New Envelope::intersection(Envelope)
 * - New Envelope::expandBy(distance, [ydistance])
 * - New LineString::reverse()
 * - New MultiLineString::reverse()
 * - New Geometry::buffer(distance, quadSeg, endCapStyle)
 * - Obsoleted toInternalGeometry/fromInternalGeometry
 * - More const-correctness in Buffer "package"
 *
 * Revision 1.37  2005/07/11 10:27:14  strk
 * Fixed initializzazion lists
 *
 * Revision 1.36  2005/05/19 10:29:28  strk
 * Removed some CGAlgorithms instances substituting them with direct calls
 * to the static functions. Interfaces accepting CGAlgorithms pointers kept
 * for backward compatibility but modified to make the argument optional.
 * Fixed a small memory leak in OffsetCurveBuilder::getRingCurve.
 * Inlined some smaller functions encountered during bug hunting.
 * Updated Copyright notices in the touched files.
 *
 * Revision 1.35  2004/11/08 15:58:13  strk
 * More performance tuning.
 *
 * Revision 1.34  2004/11/04 19:08:07  strk
 * Cleanups, initializers list, profiling.
 *
 * Revision 1.33  2004/11/01 16:43:04  strk
 * Added Profiler code.
 * Temporarly patched a bug in DoubleBits (must check drawbacks).
 * Various cleanups and speedups.
 *
 **********************************************************************/

