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
 * Revision 1.2  2004/05/03 17:15:38  strk
 * leaks on exception fixed.
 *
 * Revision 1.1  2004/04/10 22:41:25  ybychkov
 * "precision" upgraded to JTS 1.4
 *
 *
 **********************************************************************/


#include "../headers/precision.h"

namespace geos {
/**
* Creates a new instance of class, which reshifts result {@link Geometry}s.
*/
CommonBitsOp::CommonBitsOp(){
	returnToOriginalPrecision=true;
}

/**
* Creates a new instance of class, specifying whether
* the result {@link Geometry}s should be reshifted.
*
* @param returnToOriginalPrecision
*/
CommonBitsOp::CommonBitsOp(bool nReturnToOriginalPrecision){
	returnToOriginalPrecision=nReturnToOriginalPrecision;
}

/**
* Computes the set-theoretic intersection of two {@link Geometry}s, using enhanced precision.
* @param geom0 the first Geometry
* @param geom1 the second Geometry
* @return the Geometry representing the set-theoretic intersection of the input Geometries.
*/
Geometry* CommonBitsOp::intersection(Geometry* geom0, Geometry* geom1){
	vector<Geometry*> *geom=removeCommonBits(geom0, geom1);
	return computeResultPrecision(((*geom)[0])->intersection((*geom)[1]));
}

/**
* Computes the set-theoretic union of two {@link Geometry}s, using enhanced precision.
* @param geom0 the first Geometry
* @param geom1 the second Geometry
* @return the Geometry* representing the set-theoretic union of the input Geometries.
*/
Geometry* CommonBitsOp::Union(Geometry* geom0, Geometry* geom1){
	vector<Geometry*> *geom=removeCommonBits(geom0, geom1);
	return computeResultPrecision(((*geom)[0])->Union((*geom)[1]));
}

/**
* Computes the set-theoretic difference of two {@link Geometry}s, using enhanced precision.
* @param geom0 the first Geometry
* @param geom1 the second Geometry, to be subtracted from the first
* @return the Geometry* representing the set-theoretic difference of the input Geometries.
*/
Geometry* CommonBitsOp::difference(Geometry* geom0, Geometry* geom1){
	vector<Geometry*> *geom=removeCommonBits(geom0, geom1);
	return computeResultPrecision(((*geom)[0])->difference((*geom)[1]));
}

/**
* Computes the set-theoretic symmetric difference of two geometries,
* using enhanced precision.
* @param geom0 the first Geometry
* @param geom1 the second Geometry
* @return the Geometry* representing the set-theoretic symmetric difference of the input Geometries.
*/
Geometry* CommonBitsOp::symDifference(Geometry* geom0, Geometry* geom1){
	vector<Geometry*> *geom=removeCommonBits(geom0, geom1);
	return computeResultPrecision(((*geom)[0])->symDifference((*geom)[1]));
}

/**
* Computes the buffer a geometry,
* using enhanced precision.
* @param geom0 the Geometry* to buffer
* @param distance the buffer distance
* @return the Geometry* representing the buffer of the input Geometry.
*/
Geometry* CommonBitsOp::buffer(Geometry* geom0, double distance){
	Geometry *geom=removeCommonBits(geom0);
	return computeResultPrecision(geom->buffer(distance));
}

/**
* If required, returning the result to the orginal precision if required.
* <p>
* In this current implementation, no rounding is performed on the
* reshifted result geometry, which means that it is possible
* that the returned Geometry* is invalid.
*
* @param result the result Geometry* to modify
* @return the result Geometry* with the required precision
*/
Geometry* CommonBitsOp::computeResultPrecision(Geometry* result){
	if (returnToOriginalPrecision)
		cbr->addCommonBits(result);
	return result;
}

/**
* Computes a copy of the input {@link Geometry} with the calculated common bits
* removed from each coordinate.
* @param geom0 the Geometry* to remove common bits from
* @return a copy of the input Geometry* with common bits removed
*/
Geometry* CommonBitsOp::removeCommonBits(Geometry* geom0){
	cbr=new CommonBitsRemover();
	cbr->add(geom0);
	Geometry* geom=cbr->removeCommonBits(geom0->clone());
	delete cbr;
	return geom;
}

/**
* Computes a copy of each input {@link Geometry}s with the calculated common bits
* removed from each coordinate.
* @param geom0 a Geometry* to remove common bits from
* @param geom1 a Geometry* to remove common bits from
* @return an array containing copies
* of the input Geometry's with common bits removed
*/
vector<Geometry*>* CommonBitsOp::removeCommonBits(Geometry* geom0, Geometry* geom1){
	cbr=new CommonBitsRemover();
	cbr->add(geom0);
	cbr->add(geom1);
	vector<Geometry*> *gv=new vector<Geometry*>();
	gv->push_back(cbr->removeCommonBits(geom0->clone()));
	gv->push_back(cbr->removeCommonBits(geom1->clone()));
	delete cbr;
	return gv;
}
}
