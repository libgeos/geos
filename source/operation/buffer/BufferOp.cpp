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

#include <geos/opBuffer.h>
#include <geos/precision.h>
#include <geos/profiler.h>

#ifndef DEBUG
#define DEBUG 0
#endif

//#define PROFILE 1

namespace geos {

#if PROFILE
static Profiler *profiler = Profiler::instance();
#endif

int BufferOp::MAX_PRECISION_DIGITS=12;


/**
 * Compute a reasonable scale factor to limit the precision of
 * a given combination of Geometry and buffer distance->
 * The scale factor is based on a heuristic->
 *
 * @param g the Geometry being buffered
 * @param distance the buffer distance
 * @param maxPrecisionDigits the mzx # of digits that should be allowed by
 *          the precision determined by the computed scale factor
 *
 * @return a scale factor that allows a reasonable amount of precision
 *         for the buffer computation
 */
double
BufferOp::precisionScaleFactor(Geometry *g, double distance,
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

/**
 * Computes the buffer of a geometry for a given buffer distance->
 *
 * @param g the geometry to buffer
 * @param distance the buffer distance
 * @return the buffer of the input geometry
 */
Geometry*
BufferOp::bufferOp(Geometry *g, double distance)
{
	Geometry *ret = BufferOp(g).getResultGeometry(distance);
#if PROFILE
	cerr<<*profiler<<endl;
#endif
	return ret;
}

/**
 * Comutes the buffer for a geometry for a given buffer distance
 * and accuracy of approximation->
 *
 * @param g the geometry to buffer
 * @param distance the buffer distance
 * @param quadrantSegments the number of segments used to approximate a
 *        quarter circle
 * @return the buffer of the input geometry
 *
 */
Geometry*
BufferOp::bufferOp(Geometry *g, double distance, int quadrantSegments)
{
	BufferOp bufOp(g);
	bufOp.setQuadrantSegments(quadrantSegments);
	return bufOp.getResultGeometry(distance);
}

/**
 * Initializes a buffer computation for the given geometry
 *
 * @param g the geometry to buffer
 */
BufferOp::BufferOp(Geometry *g):
	argGeom(g),
	saveException(NULL),
	quadrantSegments(OffsetCurveBuilder::DEFAULT_QUADRANT_SEGMENTS),
	endCapStyle(BufferOp::CAP_ROUND),
	resultGeometry(NULL)
{
	//quadrantSegments=OffsetCurveBuilder::DEFAULT_QUADRANT_SEGMENTS;
	//endCapStyle=BufferOp::CAP_ROUND;
	//argGeom = g;
	//resultGeometry=NULL;
	//saveException=NULL;
}

/**
 * Returns the buffer computed for a geometry for a given buffer distance->
 *
 * @param g the geometry to buffer
 * @param distance the buffer distance
 * @return the buffer of the input geometry
 */
Geometry*
BufferOp::getResultGeometry(double nDistance)
{
	distance=nDistance;
	computeGeometry();
	return resultGeometry;
}

/*
 * Computes the buffer for a geometry for a given buffer distance
 * and accuracy of approximation
 *
 * @param g the geometry to buffer
 * @param distance the buffer distance
 * @param quadrantSegments the number of segments used to approximate
 *        a quarter circle
 * @return the buffer of the input geometry
 *
 * @deprecated use setQuadrantSegments instead
 */
Geometry*
BufferOp::getResultGeometry(double nDistance, int nQuadrantSegments)
{
	distance=nDistance;
	setQuadrantSegments(nQuadrantSegments);
	computeGeometry();
	return resultGeometry;
}

void
BufferOp::computeGeometry()
{
#if DEBUG
	cerr<<"BufferOp::computeGeometry: trying with original precision"<<endl;
#endif
#if PROFILE
	profiler->start("BufferOp::bufferOriginalPrecision()");
#endif
	bufferOriginalPrecision();
#if PROFILE
	profiler->stop("BufferOp::bufferOriginalPrecision()");
#endif
	if (resultGeometry!=NULL) return;

	// try and compute with decreasing precision
	for (int precDigits=MAX_PRECISION_DIGITS; precDigits >= 0; precDigits--) 	{
#if DEBUG
		cerr<<"BufferOp::computeGeometry: trying with precDigits "<<precDigits<<endl;
#endif
		try {
			bufferFixedPrecision(precDigits);
		} catch (TopologyException *ex) {
			delete saveException;
			saveException=ex;
			// don't propagate the exception - it will be detected by fact that resultGeometry is null
		} 

		if (resultGeometry!=NULL)
		{
			// debug
			//if ( saveException ) cerr<<saveException->toString()<<endl;
			delete saveException;
			return;
		}
	}
	// tried everything - have to bail
	throw saveException;
	//return resultGeometry;
}

void
BufferOp::bufferOriginalPrecision()
{
	BufferBuilder bufBuilder;
	bufBuilder.setQuadrantSegments(quadrantSegments);
	bufBuilder.setEndCapStyle(endCapStyle);

	//cerr<<"computing with original precision"<<endl;
	try {
		resultGeometry=bufBuilder.buffer(argGeom, distance);
	} catch (TopologyException *ex) {
		//cerr<<ex->toString()<<endl;
		delete saveException;
		saveException=ex;
		return;
	} 
	//cerr<<"done"<<endl;
}

void
BufferOp::bufferFixedPrecision(int precisionDigits)
{
	double sizeBasedScaleFactor=precisionScaleFactor(argGeom, distance, precisionDigits);
	PrecisionModel fixedPM=PrecisionModel(sizeBasedScaleFactor);
	// don't change the precision model of the Geometry, just reduce the precision
	SimpleGeometryPrecisionReducer reducer=SimpleGeometryPrecisionReducer(&fixedPM);
	Geometry* reducedGeom=reducer.reduce(argGeom);
	//cerr<<"recomputing with precision scale factor="<<sizeBasedScaleFactor<<" (precision digits "<<precisionDigits<<")"<<endl;

	BufferBuilder bufBuilder;
	bufBuilder.setWorkingPrecisionModel(&fixedPM);
	bufBuilder.setQuadrantSegments(quadrantSegments);

	// this may throw an exception, if robustness errors are encountered
	try {
		resultGeometry=bufBuilder.buffer(reducedGeom, distance);
	} catch (...) {
		delete reducedGeom;
		throw;
	}
	delete reducedGeom;
}

} // namespace geos

/**********************************************************************
 * $Log$
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

