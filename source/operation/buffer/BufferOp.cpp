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
 * Revision 1.16  2004/04/10 22:41:25  ybychkov
 * "precision" upgraded to JTS 1.4
 *
 * Revision 1.15  2004/04/10 08:40:01  ybychkov
 * "operation/buffer" upgraded to JTS 1.4
 *
 *
 **********************************************************************/


#include "../../headers/opBuffer.h"
#include "../../headers/precision.h"

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
	Envelope *env=g->getEnvelopeInternal();
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
Geometry* BufferOp::bufferOp(Geometry *g, double distance){
	BufferOp *gBuf=new BufferOp(g);
	Geometry* geomBuf=gBuf->getResultGeometry(distance);
	return geomBuf;
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
Geometry* BufferOp::bufferOp(Geometry *g, double distance, int quadrantSegments){
	BufferOp *bufOp=new BufferOp(g);
	bufOp->setQuadrantSegments(quadrantSegments);
	Geometry *geomBuf=bufOp->getResultGeometry(distance);
	return geomBuf;
}

int BufferOp::MAX_PRECISION_DIGITS=12;
/**
* Initializes a buffer computation for the given geometry
*
* @param g the geometry to buffer
*/
BufferOp::BufferOp(Geometry *g) {
	quadrantSegments=OffsetCurveBuilder::DEFAULT_QUADRANT_SEGMENTS;
	int endCapStyle=BufferOp::CAP_ROUND;
	resultGeometry=NULL;
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

void BufferOp::computeGeometry(){
	bufferOriginalPrecision();
	if (resultGeometry!=NULL) return;
	// try and compute with decreasing precision
	for (int precDigits=MAX_PRECISION_DIGITS; precDigits >= 0; precDigits--) {
	try {
		bufferFixedPrecision(precDigits);
	} catch (TopologyException *ex) {
		saveException=ex;
		// don't propagate the exception - it will be detected by fact that resultGeometry is null
	}
	if (resultGeometry!=NULL) return;
	}
	// tried everything - have to bail
	throw saveException;
	//return resultGeometry;
}

void BufferOp::bufferOriginalPrecision() {
	try {
		BufferBuilder *bufBuilder=new BufferBuilder();
		bufBuilder->setQuadrantSegments(quadrantSegments);
		bufBuilder->setEndCapStyle(endCapStyle);
		resultGeometry=bufBuilder->buffer(argGeom, distance);
	} catch (TopologyException *ex) {
		saveException=ex;
		// don't propagate the exception - it will be detected by fact that resultGeometry is null
	}
}

void BufferOp::bufferFixedPrecision(int precisionDigits) {
	double sizeBasedScaleFactor=precisionScaleFactor(argGeom, distance, precisionDigits);
	PrecisionModel *fixedPM=new PrecisionModel(sizeBasedScaleFactor);
	// don't change the precision model of the Geometry, just reduce the precision
	SimpleGeometryPrecisionReducer *reducer=new SimpleGeometryPrecisionReducer(fixedPM);
	Geometry* reducedGeom=reducer->reduce(argGeom);
	//System->out->println("recomputing with precision scale factor=" + sizeBasedScaleFactor);
	BufferBuilder *bufBuilder=new BufferBuilder();
	bufBuilder->setWorkingPrecisionModel(fixedPM);
	bufBuilder->setQuadrantSegments(quadrantSegments);
	// this may throw an exception, if robustness errors are encountered
	resultGeometry=bufBuilder->buffer(reducedGeom, distance);
}

}
