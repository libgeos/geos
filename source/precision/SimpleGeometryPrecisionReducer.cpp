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
 * Revision 1.1  2004/04/10 22:41:25  ybychkov
 * "precision" upgraded to JTS 1.4
 *
 *
 **********************************************************************/


#include "../headers/precision.h"
#include <typeinfo>

namespace geos {

SimpleGeometryPrecisionReducer::SimpleGeometryPrecisionReducer(PrecisionModel *pm){
	removeCollapsed = true;
	changePrecisionModel = false;
	newPrecisionModel = pm;
}

/**
* Sets whether the reduction will result in collapsed components
* being removed completely, or simply being collapsed to an (invalid)
* Geometry of the same type.
*
* @param removeCollapsed if <code>true</code> collapsed components will be removed
*/
void SimpleGeometryPrecisionReducer::setRemoveCollapsedComponents(bool nRemoveCollapsed){
	removeCollapsed=nRemoveCollapsed;
}

/**
* Sets whether the {@link PrecisionModel} of the new reduced Geometry
* will be changed to be the {@link PrecisionModel} supplied to
* specify the reduction.  The default is to not change the precision model
*
* @param changePrecisionModel if <code>true</code> the precision model of the created Geometry will be the
* the precisionModel supplied in the constructor.
*/
void SimpleGeometryPrecisionReducer::setChangePrecisionModel(bool nChangePrecisionModel){
	changePrecisionModel=nChangePrecisionModel;
}

PrecisionModel* SimpleGeometryPrecisionReducer::getPrecisionModel() {
	return newPrecisionModel;
}

bool SimpleGeometryPrecisionReducer::getRemoveCollapsed() {
	return removeCollapsed;
}

Geometry* SimpleGeometryPrecisionReducer::reduce(Geometry *geom){
	GeometryEditor *geomEdit;
	if (changePrecisionModel) {
		GeometryFactory *newFactory = new GeometryFactory(newPrecisionModel, geom->getSRID());
		geomEdit=new GeometryEditor(newFactory);
	} else {
		// don't change geometry factory
		geomEdit = new GeometryEditor();
	}
	Geometry *g=geomEdit->edit(geom, new PrecisionReducerCoordinateOperation(this));
	delete geomEdit;
	return g;
}

PrecisionReducerCoordinateOperation::PrecisionReducerCoordinateOperation(SimpleGeometryPrecisionReducer *newSgpr) {
	sgpr=newSgpr;
}
CoordinateList* PrecisionReducerCoordinateOperation::edit(CoordinateList *coordinates, Geometry *geom) {
	if (coordinates->getSize()==0) return NULL;
	CoordinateList *reducedCoords =CoordinateListFactory::internalFactory->createCoordinateList(coordinates->getSize());
	// copy coordinates and reduce
	for (int i=0;i<coordinates->getSize(); i++) {
		Coordinate *coord=new Coordinate(coordinates->getAt(i));
		sgpr->getPrecisionModel()->makePrecise(coord);
		reducedCoords->setAt(*coord,i);
	}
	// remove repeated points, to simplify returned geometry as much as possible
	CoordinateList *noRepeatedCoords=CoordinateList::removeRepeatedPoints(reducedCoords);
	/**
	* Check to see if the removal of repeated points
	* collapsed the coordinate List to an invalid length
	* for the type of the parent geometry.
	* It is not necessary to check for Point collapses, since the coordinate list can
	* never collapse to less than one point.
	* If the length is invalid, return the full-length coordinate array
	* first computed, or null if collapses are being removed.
	* (This may create an invalid geometry - the client must handle this.)
	*/
	int minLength = 0;
	if (typeid(*geom)==typeid(LineString)) minLength = 2;
	if (typeid(*geom)==typeid(LinearRing)) minLength = 4;
	CoordinateList *collapsedCoords = reducedCoords;
	if (sgpr->getRemoveCollapsed()) collapsedCoords=NULL;
	// return null or orginal length coordinate array
	if (noRepeatedCoords->getSize()<minLength) {
		return collapsedCoords;
	}
	// ok to return shorter coordinate array
	return noRepeatedCoords;
}
}
