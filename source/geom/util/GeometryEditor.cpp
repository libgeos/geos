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
 * Revision 1.8  2004/07/01 14:12:44  strk
 * Geometry constructors come now in two flavors:
 * 	- deep-copy args (pass-by-reference)
 * 	- take-ownership of args (pass-by-pointer)
 * Same functionality is available through GeometryFactory,
 * including buildGeometry().
 *
 * Revision 1.7  2004/06/15 20:20:45  strk
 * updated to respect deep-copy GeometryCollection interface
 *
 * Revision 1.6  2004/05/05 12:20:22  strk
 * Memory leak plugged in editGeometryCollection
 *
 * Revision 1.5  2004/05/03 17:15:38  strk
 * leaks on exception fixed.
 *
 * Revision 1.4  2004/04/20 10:14:20  strk
 * Memory leaks removed.
 *
 * Revision 1.3  2004/04/20 08:52:01  strk
 * GeometryFactory and Geometry const correctness.
 * Memory leaks removed from SimpleGeometryPrecisionReducer
 * and GeometryFactory.
 *
 * Revision 1.2  2004/04/14 11:05:07  strk
 * Added support for LinearRing in GeometryEditor
 *
 * Revision 1.1  2004/04/04 06:29:11  ybychkov
 * "planargraph" and "geom/utill" upgraded to JTS 1.4
 *
 *
 **********************************************************************/


#include "../../headers/geomUtil.h"
#include "../../headers/util.h"
#include <typeinfo>

namespace geos {
/**
* Creates a new GeometryEditor object which will create
* an edited {@link Geometry} with the same {@link GeometryFactory} as the input Geometry.
*/
GeometryEditor::GeometryEditor(){
	factory=NULL;
}

/**
* Creates a new GeometryEditor object which will create
* the edited Geometry with the given {@link GeometryFactory}
*
* @param factory the GeometryFactory to create the edited Geometry with
*/
GeometryEditor::GeometryEditor(const GeometryFactory *newFactory){
	factory=newFactory;
}

/**
* Edit the input {@link Geometry} with the given edit operation.
* Clients will create subclasses of {@link GeometryEditorOperation} or
* {@link CoordinateOperation} to perform required modifications.
*
* @param geometry the Geometry to edit
* @param operation the edit operation to carry out
* @return a new {@link Geometry} which is the result of the editing
*/
Geometry* GeometryEditor::edit(const Geometry *geometry, GeometryEditorOperation *operation){
	// if client did not supply a GeometryFactory, use the one from the input Geometry
	if (factory == NULL)
		factory=geometry->getFactory();
	if ((typeid(*geometry)==typeid(GeometryCollection)) ||
				(typeid(*geometry)==typeid(MultiPoint)) ||
				(typeid(*geometry)==typeid(MultiPolygon)) ||
				(typeid(*geometry)==typeid(MultiLineString))) {
		return editGeometryCollection((const GeometryCollection*) geometry, operation);
	}

	if (typeid(*geometry)==typeid(Polygon)) {
		return editPolygon((Polygon*) geometry, operation);
	}

	if (typeid(*geometry)==typeid(Point)) {
		return operation->edit(geometry, factory);
	}

	if (typeid(*geometry)==typeid(LineString) || typeid(*geometry)==typeid(LinearRing)) {
		return operation->edit(geometry, factory);
	}

	Assert::shouldNeverReachHere("Unsupported Geometry classes should be caught in the GeometryEditorOperation.");
	return NULL;
}

Polygon* GeometryEditor::editPolygon(const Polygon *polygon,GeometryEditorOperation *operation) {
	Polygon* newPolygon=(Polygon*) operation->edit(polygon, factory);
	if (newPolygon->isEmpty()) {
		//RemoveSelectedPlugIn relies on this behaviour. [Jon Aquino]
		return newPolygon;
	}
	LinearRing* shell = (LinearRing*) edit(newPolygon->getExteriorRing(),operation);
	if (shell->isEmpty()) {
		//RemoveSelectedPlugIn relies on this behaviour. [Jon Aquino]
		delete shell;
		delete newPolygon;
		return factory->createPolygon(NULL,NULL);
	}

	vector<Geometry*> *holes=new vector<Geometry*>;
	for (int i=0;i<newPolygon->getNumInteriorRing(); i++) {
		LinearRing *hole =(LinearRing*) edit(newPolygon->getInteriorRingN(i),operation);
		if (hole->isEmpty()) {
			continue;
		}
		holes->push_back(hole);
	}
	delete newPolygon;
	return factory->createPolygon(shell,holes);
}

GeometryCollection*
GeometryEditor::editGeometryCollection(const GeometryCollection *collection, GeometryEditorOperation *operation)
{
	GeometryCollection *ret;
	GeometryCollection *newCollection = (GeometryCollection*) operation->edit(collection,factory);
	vector<Geometry*> *geometries = new vector<Geometry*>();
	for (int i = 0; i < newCollection->getNumGeometries(); i++) {
		Geometry *geometry = edit(newCollection->getGeometryN(i),
			operation);
		if (geometry->isEmpty()) {
			delete geometry;
			continue;
		}
		geometries->push_back(geometry);
	}
	delete newCollection;

	if (typeid(*newCollection)==typeid(MultiPoint)) {
		return factory->createMultiPoint(geometries);
	}
	else if (typeid(*newCollection)==typeid(MultiLineString)) {
		return factory->createMultiLineString(geometries);
	}
	else if (typeid(*newCollection)==typeid(MultiPolygon)) {
		return factory->createMultiPolygon(geometries);
	}
	else {
		return factory->createGeometryCollection(geometries);
	}
}

/**
 * Return a newly created geometry 
 */
Geometry*
CoordinateOperation::edit(const Geometry *geometry, const GeometryFactory *factory)
{

	if (typeid(*geometry)==typeid(LinearRing)) {
		CoordinateList *coords = geometry->getCoordinates();
		CoordinateList *newCoords = edit(coords,geometry);
		delete coords;
		return factory->createLinearRing(newCoords);
	}
	else if (typeid(*geometry)==typeid(LineString)) {
		CoordinateList *coords = geometry->getCoordinates();
		CoordinateList *newCoords = edit(coords,geometry);
		delete coords;
		return factory->createLineString(newCoords);
	}
	else if (typeid(*geometry)==typeid(Point)) {
		CoordinateList *coords = geometry->getCoordinates();
		CoordinateList *newCoords = edit(coords,geometry);
		delete coords;
		return factory->createPoint(newCoords);
	}

	return geometry->clone();
}

}
