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
 * Revision 1.32  2004/07/27 16:35:46  strk
 * Geometry::getEnvelopeInternal() changed to return a const Envelope *.
 * This should reduce object copies as once computed the envelope of a
 * geometry remains the same.
 *
 * Revision 1.31  2004/07/02 13:28:27  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.30  2004/05/19 13:01:34  strk
 * avoided assignment operator calls for BufferBuilder
 *
 * Revision 1.29  2004/05/05 10:22:49  strk
 * Removed dynamic allocations.
 *
 * Revision 1.28  2004/05/05 10:03:49  strk
 * Reduced dynamic allocations in bufferOriginalPrecision and bufferFixedPrecision.
 *
 * Revision 1.27  2004/05/03 17:15:38  strk
 * leaks on exception fixed.
 *
 * Revision 1.26  2004/04/23 00:02:18  strk
 * const-correctness changes
 *
 * Revision 1.25  2004/04/20 13:24:15  strk
 * More leaks removed.
 *
 * Revision 1.24  2004/04/20 10:14:20  strk
 * Memory leaks removed.
 *
 * Revision 1.23  2004/04/19 15:14:46  strk
 * Added missing virtual destructor in SpatialIndex class.
 * Memory leaks fixes. Const and throw specifications added.
 *
 * Revision 1.22  2004/04/19 12:51:01  strk
 * Memory leaks fixes. Throw specifications added.
 *
 * Revision 1.21  2004/04/16 14:09:17  strk
 * Leaks fixes
 *
 * Revision 1.20  2004/04/16 11:04:24  strk
 * Memory leaks plugged on exception thrown
 *
 * Revision 1.19  2004/04/16 10:00:08  strk
 * Memory leak fixed.
 *
 * Revision 1.18  2004/04/14 09:11:57  strk
 * endCapStyle was never set in BufferOp contructor
 *
 * Revision 1.17  2004/04/14 08:38:31  strk
 * BufferOp constructor missed to set argGeom
 *
 * Revision 1.16  2004/04/10 22:41:25  ybychkov
 * "precision" upgraded to JTS 1.4
 *
 * Revision 1.15  2004/04/10 08:40:01  ybychkov
 * "operation/buffer" upgraded to JTS 1.4
 *
 *
 **********************************************************************/


#include <geos/opBuffer.h>
#include <geos/precision.h>

namespace geos {

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
* @return a scale factor that allows a reasonable amount of precision for the buffer computation
*/
double BufferOp::precisionScaleFactor(Geometry *g,double distance,int maxPrecisionDigits){
	const Envelope *env=g->getEnvelopeInternal();
	double envSize=max(env->getHeight(), env->getWidth());
	//delete env;
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
Geometry* BufferOp::bufferOp(Geometry *g, double distance){
	return BufferOp(g).getResultGeometry(distance);
}

/**
* Comutes the buffer for a geometry for a given buffer distance
* and accuracy of approximation->
*
* @param g the geometry to buffer
* @param distance the buffer distance
* @param quadrantSegments the number of segments used to approximate a quarter circle
* @return the buffer of the input geometry
*
*/
Geometry*
BufferOp::bufferOp(Geometry *g, double distance, int quadrantSegments)
{
	BufferOp bufOp=BufferOp(g);
	bufOp.setQuadrantSegments(quadrantSegments);
	return bufOp.getResultGeometry(distance);
}

int BufferOp::MAX_PRECISION_DIGITS=12;
/**
* Initializes a buffer computation for the given geometry
*
* @param g the geometry to buffer
*/
BufferOp::BufferOp(Geometry *g) {
	quadrantSegments=OffsetCurveBuilder::DEFAULT_QUADRANT_SEGMENTS;
	endCapStyle=BufferOp::CAP_ROUND;
	argGeom = g;
	resultGeometry=NULL;
	saveException=NULL;
}

/**
* Specifies the end cap style of the generated buffer->
* The styles supported are {@link CAP_ROUND}, {@link CAP_BUTT}, and {@link CAP_SQUARE}->
* The default is CAP_ROUND->
*
* @param endCapStyle the end cap style to specify
*/
void BufferOp::setEndCapStyle(int nEndCapStyle){
	endCapStyle=nEndCapStyle;
}

/**
* Specifies the end cap style of the generated buffer->
* The styles supported are {@link CAP_ROUND}, {@link CAP_BUTT}, and {@link CAP_SQUARE}->
* The default is CAP_ROUND->
*
* @param endCapStyle the end cap style to specify
*/
void BufferOp::setQuadrantSegments(int nQuadrantSegments){
	quadrantSegments=nQuadrantSegments;
}

/**
* Returns the buffer computed for a geometry for a given buffer distance->
*
* @param g the geometry to buffer
* @param distance the buffer distance
* @return the buffer of the input geometry
*/
Geometry* BufferOp::getResultGeometry(double nDistance){
	distance=nDistance;
	computeGeometry();
	return resultGeometry;
}

/**
* Comutes the buffer for a geometry for a given buffer distance
* and accuracy of approximation->
*
* @param g the geometry to buffer
* @param distance the buffer distance
* @param quadrantSegments the number of segments used to approximate a quarter circle
* @return the buffer of the input geometry
*
* @deprecated use setQuadrantSegments instead
*/
Geometry* BufferOp::getResultGeometry(double nDistance, int nQuadrantSegments){
	distance=nDistance;
	setQuadrantSegments(nQuadrantSegments);
	computeGeometry();
	return resultGeometry;
}

void
BufferOp::computeGeometry()
{
	bufferOriginalPrecision();
	if (resultGeometry!=NULL) return;
	// try and compute with decreasing precision
	for (int precDigits=MAX_PRECISION_DIGITS; precDigits >= 0; precDigits--) 	{
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

}
