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
 * Revision 1.9  2005/11/25 11:40:21  strk
 * Another getSize in for loop, another int-unsigned int warning
 *
 * Revision 1.8  2004/09/21 09:47:36  strk
 * fixed a mis-initialization bug in ::reduce
 *
 * Revision 1.7  2004/07/08 19:34:50  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added DefaultCoordinateSequenceFactory::instance() function.
 *
 * Revision 1.6  2004/07/06 17:58:22  strk
 * Removed deprecated Geometry constructors based on PrecisionModel and
 * SRID specification. Removed SimpleGeometryPrecisionReducer capability
 * of changing Geometry's factory. Reverted Geometry::factory member
 * to be a reference to external factory.
 *
 * Revision 1.5  2004/07/02 13:28:29  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.4  2004/05/03 17:15:38  strk
 * leaks on exception fixed.
 *
 * Revision 1.3  2004/04/20 10:14:20  strk
 * Memory leaks removed.
 *
 * Revision 1.2  2004/04/20 08:52:01  strk
 * GeometryFactory and Geometry const correctness.
 * Memory leaks removed from SimpleGeometryPrecisionReducer
 * and GeometryFactory.
 *
 * Revision 1.1  2004/04/10 22:41:25  ybychkov
 * "precision" upgraded to JTS 1.4
 *
 *
 **********************************************************************/


#include <geos/precision.h>
#include <typeinfo>

namespace geos {

SimpleGeometryPrecisionReducer::SimpleGeometryPrecisionReducer(PrecisionModel *pm){
	removeCollapsed = true;
	//changePrecisionModel = false;
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

/*
* Sets whether the PrecisionModel of the new reduced Geometry
* will be changed to be the PrecisionModel supplied to
* specify the reduction.  The default is to not change the precision model.
* Caller will need to take care of deletion of the newly created
* GeometryFactory if nChangePrecisionModel is set to true (see Geometry::getFactory())
*
* @param changePrecisionModel if <code>true</code> the precision model of the created Geometry will be the
* the precisionModel supplied in the constructor.
*/
//void SimpleGeometryPrecisionReducer::setChangePrecisionModel(bool nChangePrecisionModel){
//	changePrecisionModel=nChangePrecisionModel;
//}

PrecisionModel* SimpleGeometryPrecisionReducer::getPrecisionModel() {
	return newPrecisionModel;
}

bool SimpleGeometryPrecisionReducer::getRemoveCollapsed() {
	return removeCollapsed;
}

Geometry* SimpleGeometryPrecisionReducer::reduce(const Geometry *geom){
	GeometryEditor geomEdit;
	PrecisionReducerCoordinateOperation prco(this);
	Geometry *g=geomEdit.edit(geom, &prco);
	return g;
}

PrecisionReducerCoordinateOperation::PrecisionReducerCoordinateOperation(SimpleGeometryPrecisionReducer *newSgpr) {
	sgpr=newSgpr;
}

CoordinateSequence*
PrecisionReducerCoordinateOperation::edit(const CoordinateSequence *cs, const Geometry *geom)
{
	if (cs->getSize()==0) return NULL;

	unsigned int csSize=cs->getSize();

	vector<Coordinate> *vc = new vector<Coordinate>(csSize);

	// copy coordinates and reduce
	for (unsigned int i=0; i<csSize; ++i) {
		Coordinate coord=cs->getAt(i);
		sgpr->getPrecisionModel()->makePrecise(&coord);
		//reducedCoords->setAt(*coord,i);
		(*vc)[i] = coord;
	}

	CoordinateSequence *reducedCoords = geom->getFactory()->getCoordinateSequenceFactory()->create(vc);

	// remove repeated points, to simplify returned geometry as much as possible
	CoordinateSequence *noRepeatedCoords=CoordinateSequence::removeRepeatedPoints(reducedCoords);

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
	unsigned int minLength = 0;
	if (typeid(*geom)==typeid(LineString)) minLength = 2;
	if (typeid(*geom)==typeid(LinearRing)) minLength = 4;
	CoordinateSequence *collapsedCoords = reducedCoords;
	if (sgpr->getRemoveCollapsed()) collapsedCoords=NULL;
	// return null or orginal length coordinate array
	if (noRepeatedCoords->getSize()<minLength) {
		delete noRepeatedCoords;
		return collapsedCoords;
	}
	// ok to return shorter coordinate array
	delete reducedCoords;
	return noRepeatedCoords;
}

} // namespace geos
