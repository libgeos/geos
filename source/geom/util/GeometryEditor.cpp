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
GeometryEditor::GeometryEditor(GeometryFactory *newFactory){
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
Geometry* GeometryEditor::edit(Geometry *geometry, GeometryEditorOperation *operation){
	// if client did not supply a GeometryFactory, use the one from the input Geometry
	if (factory == NULL)
		factory=geometry->getFactory();
	if ((typeid(*geometry)==typeid(GeometryCollection)) ||
				(typeid(*geometry)==typeid(MultiPoint)) ||
				(typeid(*geometry)==typeid(MultiPolygon)) ||
				(typeid(*geometry)==typeid(MultiLineString))) {
		return editGeometryCollection((GeometryCollection*) geometry,operation);
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

Polygon* GeometryEditor::editPolygon(Polygon *polygon,GeometryEditorOperation *operation) {
	Polygon* newPolygon=(Polygon*) operation->edit(polygon, factory);
	if (newPolygon->isEmpty()) {
		//RemoveSelectedPlugIn relies on this behaviour. [Jon Aquino]
		return newPolygon;
	}
	LinearRing* shell = (LinearRing*) edit((Geometry*)newPolygon->getExteriorRing(),operation);
	if (shell->isEmpty()) {
		//RemoveSelectedPlugIn relies on this behaviour. [Jon Aquino]
		return factory->createPolygon(NULL,NULL);
	}

	vector<Geometry*> *holes=new vector<Geometry*>;
	for (int i=0;i<newPolygon->getNumInteriorRing(); i++) {
		LinearRing *hole =(LinearRing*) edit((Geometry*)newPolygon->getInteriorRingN(i),operation);
		if (hole->isEmpty()) {
			continue;
		}
		holes->push_back(hole);
	}
	return factory->createPolygon(shell,holes);
}

GeometryCollection* GeometryEditor::editGeometryCollection(GeometryCollection *collection, GeometryEditorOperation *operation) {
	GeometryCollection *newCollection = (GeometryCollection*) operation->edit(collection,factory);
	vector<Geometry*> *geometries = new vector<Geometry*>();
	for (int i = 0; i < newCollection->getNumGeometries(); i++) {
		Geometry *geometry = edit((Geometry*)newCollection->getGeometryN(i), operation);
		if (geometry->isEmpty()) {
			continue;
		}
		geometries->push_back(geometry);
	}
	if (typeid(*newCollection)==typeid(MultiPoint)) {
		return factory->createMultiPoint(geometries);
	}
	if (typeid(*newCollection)==typeid(MultiLineString)) {
		return factory->createMultiLineString(geometries);
	}
	if (typeid(*newCollection)==typeid(MultiPolygon)) {
		return factory->createMultiPolygon(geometries);
	}
	return factory->createGeometryCollection(geometries);
}

Geometry* CoordinateOperation::edit(Geometry *geometry, GeometryFactory *factory) {
	if (typeid(*geometry)==typeid(LinearRing)) {
		return factory->createLinearRing(edit(geometry->getCoordinates(),geometry));
	}
	if (typeid(*geometry)==typeid(LineString)) {
		return factory->createLineString(edit(geometry->getCoordinates(),geometry));
	}
	if (typeid(*geometry)==typeid(Point)) {
		CoordinateList *newCoordinates = edit(geometry->getCoordinates(),geometry);
		return factory->createPoint((newCoordinates->getSize()>0)? newCoordinates->getAt(0) : Coordinate::nullCoord);
	}
	return geometry;
}

}
