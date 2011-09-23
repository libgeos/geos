/**********************************************************************
 * $Id: BufferOp.cpp 3319 2011-04-28 15:09:04Z strk $
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2009-2011 Sandro Santilli <strk@keybit.net>
 * Copyright (C) 2005-2007 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: operation/buffer/BufferOp.java r378 (JTS-1.12)
 *
 **********************************************************************/

#include <algorithm>
#include <cmath>

#include <geos/profiler.h>
#include <geos/operation/buffer/BufferOp.h>
#include <geos/operation/buffer/BufferBuilder.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/PrecisionModel.h>

#include <geos/noding/ScaledNoder.h>

#include <geos/noding/snapround/MCIndexSnapRounder.h>
#include <geos/noding/snapround/MCIndexPointSnapper.h>

//FIXME: for temporary use, see other FIXME in file
#include <geos/algorithm/LineIntersector.h>
#include <geos/noding/MCIndexNoder.h>
#include <geos/noding/IntersectionAdder.h>
#include <geos/noding/snapround/SimpleSnapRounder.h>




#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif

//#define PROFILE 1

using namespace geos::noding;
using namespace geos::geom;

namespace geos {
namespace operation { // geos.operation
namespace buffer { // geos.operation.buffer

#if PROFILE
static Profiler *profiler = Profiler::instance();
#endif

/*private*/
double
BufferOp::precisionScaleFactor(const Geometry *g,
	double distance,
	int maxPrecisionDigits)
{
	const Envelope *env=g->getEnvelopeInternal();
	double envSize=(std::max)(env->getHeight(), env->getWidth());
	double expandByDistance=distance > 0.0 ? distance : 0.0;
	double bufEnvSize=envSize + 2 * expandByDistance;
	// the smallest power of 10 greater than the buffer envelope
	int bufEnvLog10=(int) (std::log(bufEnvSize) / std::log(10.0) + 1.0);
	int minUnitLog10=bufEnvLog10 - maxPrecisionDigits;
	// scale factor is inverse of min Unit size, so flip sign of exponent
	double scaleFactor=std::pow(10.0,-minUnitLog10);
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

/*private*/
void
BufferOp::computeGeometry()
{
#if GEOS_DEBUG
	std::cerr<<"BufferOp::computeGeometry: trying with original precision"<<std::endl;
#endif

	//bufferReducedPrecision(); return; // FIXME: remove this code
	bufferOriginalPrecision();

	if (resultGeometry!=NULL) return;

#if GEOS_DEBUG
	std::cerr << "bufferOriginalPrecision failed (" << saveException.what() << "), trying with reduced precision"
	          << std::endl;
#endif

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
		std::cerr<<"BufferOp::computeGeometry: trying with precDigits "<<precDigits<<std::endl;
#endif
		try {
			bufferReducedPrecision(precDigits);
		} catch (const util::TopologyException& ex) {
			saveException=ex;
			// don't propagate the exception - it will be detected by fact that resultGeometry is null
		} 

		if (resultGeometry!=NULL) {
			// debug
			//if ( saveException ) std::cerr<<saveException->toString()<<std::endl;
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
	BufferBuilder bufBuilder(bufParams);

	//std::cerr<<"computing with original precision"<<std::endl;
	try
	{
		resultGeometry=bufBuilder.buffer(argGeom, distance);
	}
	catch (const util::TopologyException& ex)
	{
		// don't propagate the exception - it will be detected by
		// fact that resultGeometry is null
		saveException=ex;

		//std::cerr<<ex->toString()<<std::endl;
	} 
	//std::cerr<<"done"<<std::endl;
}

void
BufferOp::bufferReducedPrecision(int precisionDigits)
{
	double sizeBasedScaleFactor=precisionScaleFactor(argGeom, distance, precisionDigits);

#if GEOS_DEBUG
	std::cerr << "recomputing with precision scale factor = "
		<< sizeBasedScaleFactor
		<< std::endl;
#endif

	assert(sizeBasedScaleFactor>0);
	PrecisionModel fixedPM(sizeBasedScaleFactor);
	bufferFixedPrecision(fixedPM);
}

/*private*/
void
BufferOp::bufferFixedPrecision(const PrecisionModel& fixedPM)
{


	PrecisionModel pm(1.0); // fixed as well
	snapround::MCIndexSnapRounder inoder(pm); 

	ScaledNoder noder(inoder, fixedPM.getScale());

	BufferBuilder bufBuilder(bufParams);
	bufBuilder.setWorkingPrecisionModel(&fixedPM);

	bufBuilder.setNoder(&noder);

	// this may throw an exception, if robustness errors are encountered
	resultGeometry=bufBuilder.buffer(argGeom, distance);
}

} // namespace geos.operation.buffer
} // namespace geos.operation
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.52  2006/05/04 15:49:39  strk
 * updated all Geometry::getDimension() methods to return Dimension::DimensionType (closes bug#93)
 *
 * Revision 1.51  2006/05/03 10:26:56  strk
 * Fixed misuse of precision model in noder (bufferFixedPrecision)
 *
 * Revision 1.50  2006/03/23 09:17:19  strk
 * precision.h header split, minor optimizations
 *
 * Revision 1.49  2006/03/15 18:56:30  strk
 * Temporary hack to avoid snapround:: Noders (still using ScaledNoder wrapper)
 * to allow for buffer_snapround.xml test to succeed
 **********************************************************************/

