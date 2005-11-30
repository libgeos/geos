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

#include <geos/geomUtil.h>
#include <geos/util.h>
#include <typeinfo>

namespace geos {

/**
 * Creates a new GeometryEditor object which will create
 * an edited Geometry with the same GeometryFactory as the input Geometry.
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
Geometry*
GeometryEditor::edit(const Geometry *geometry, GeometryEditorOperation *operation)
{
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

Polygon*
GeometryEditor::editPolygon(const Polygon *polygon,GeometryEditorOperation *operation)
{
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

	if (typeid(*newCollection)==typeid(MultiPoint)) {
		delete newCollection;
		return factory->createMultiPoint(geometries);
	}
	else if (typeid(*newCollection)==typeid(MultiLineString)) {
		delete newCollection;
		return factory->createMultiLineString(geometries);
	}
	else if (typeid(*newCollection)==typeid(MultiPolygon)) {
		delete newCollection;
		return factory->createMultiPolygon(geometries);
	}
	else {
		delete newCollection;
		return factory->createGeometryCollection(geometries);
	}
}

/**
 * Return a newly created geometry 
 */
Geometry*
CoordinateOperation::edit(const Geometry *geometry, const GeometryFactory *factory)
{

	const LinearRing *ring = dynamic_cast<const LinearRing *>(geometry);
	if (ring) {
		const CoordinateSequence *coords = ring->getCoordinatesRO();
		CoordinateSequence *newCoords = edit(coords,geometry);
		return factory->createLinearRing(newCoords);
	}
	const LineString *line = dynamic_cast<const LineString *>(geometry);
	if (line) {
		const CoordinateSequence *coords = line->getCoordinatesRO();
		CoordinateSequence *newCoords = edit(coords,geometry);
		return factory->createLineString(newCoords);
	}
	if (typeid(*geometry)==typeid(Point)) {
		CoordinateSequence *coords = geometry->getCoordinates();
		CoordinateSequence *newCoords = edit(coords,geometry);
		delete coords;
		return factory->createPoint(newCoords);
	}

	return geometry->clone();
}

} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.11.2.1  2005/05/24 07:39:26  strk
 * Segfault fix in GeometryEditor::editPolygon(), CoordinateSequence copy
 * reduction
 *
 * Revision 1.11  2004/12/08 13:54:43  strk
 * gcc warnings checked and fixed, general cleanups.
 *
 * Revision 1.10  2004/07/08 19:34:49  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added DefaultCoordinateSequenceFactory::instance() function.
 *
 * Revision 1.9  2004/07/02 13:28:26  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 **********************************************************************/

