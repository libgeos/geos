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
 * Revision 1.32  2004/04/14 07:29:43  strk
 * Fixed GeometryFactory constructors to copy given PrecisionModel. Added GeometryFactory copy constructor. Fixed Geometry constructors to copy GeometryFactory.
 *
 * Revision 1.31  2004/04/04 06:29:11  ybychkov
 * "planargraph" and "geom/utill" upgraded to JTS 1.4
 *
 * Revision 1.30  2004/04/01 10:44:33  ybychkov
 * All "geom" classes from JTS 1.3 upgraded to JTS 1.4
 *
 * Revision 1.29  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 * Revision 1.28  2003/10/15 16:39:03  strk
 * Made Edge::getCoordinates() return a 'const' value. Adapted code set.
 *
 * Revision 1.27  2003/10/14 15:58:51  strk
 * Useless vector<Geometry *> leaking allocations removed
 *
 * Revision 1.26  2003/10/11 01:56:08  strk
 *
 * Code base padded with 'const' keywords ;)
 *
 * Revision 1.25  2003/10/09 15:35:13  strk
 * added 'const' keyword to GeometryFactory constructor, Log on top of geom.h
 *
 * Revision 1.24  2003/10/09 10:14:06  strk
 * just a style change in top Log comment.
 *
 **********************************************************************/


#include "../headers/geom.h"
#include "../headers/geomUtil.h"
#include "../headers/util.h"
#include <typeinfo>

namespace geos {

/**
* Constructs a GeometryFactory that generates Geometries having a floating
* PrecisionModel and a spatial-reference ID of 0.
*/
GeometryFactory::GeometryFactory() {
	precisionModel=new PrecisionModel();
	SRID=0;
	coordinateListFactory=CoordinateListFactory::internalFactory;
}

/**
* Constructs a GeometryFactory that generates Geometries having the given
* PrecisionModel, spatial-reference ID, and CoordinateSequence implementation.
*/
GeometryFactory::GeometryFactory(const PrecisionModel *pm, int newSRID,CoordinateListFactory *nCoordinateListFactory) {
	precisionModel=new PrecisionModel(*pm);
	coordinateListFactory=nCoordinateListFactory;
	SRID=newSRID;
}

/**
* Constructs a GeometryFactory that generates Geometries having the given
* CoordinateList implementation, a double-precision floating PrecisionModel and a
* spatial-reference ID of 0.
*/
GeometryFactory::GeometryFactory(CoordinateListFactory *nCoordinateListFactory) {
	precisionModel=new PrecisionModel();
	SRID=0;
	coordinateListFactory=nCoordinateListFactory;
}

/**
* Constructs a GeometryFactory that generates Geometries having the given
* {@link PrecisionModel} and the default CoordinateSequence
* implementation.
*
* @param precisionModel the PrecisionModel to use
*/
GeometryFactory::GeometryFactory(const PrecisionModel *pm) {
	precisionModel=new PrecisionModel(*pm);
	SRID=0;
	coordinateListFactory=CoordinateListFactory::internalFactory;
}

/**
* Constructs a GeometryFactory that generates Geometries having the given
* {@link PrecisionModel} and spatial-reference ID, and the default CoordinateSequence
* implementation.
*
* @param precisionModel the PrecisionModel to use
* @param SRID the SRID to use
*/
GeometryFactory::GeometryFactory(const PrecisionModel* pm, int newSRID){
    precisionModel=new PrecisionModel(*pm);
    SRID=newSRID;
	coordinateListFactory=CoordinateListFactory::internalFactory;
}

GeometryFactory::GeometryFactory(const GeometryFactory &gf){
    precisionModel=new PrecisionModel(*(gf.precisionModel));
    SRID=gf.SRID;
    coordinateListFactory=gf.coordinateListFactory;
}
GeometryFactory::~GeometryFactory(){
	delete precisionModel;
}
  
Point* GeometryFactory::createPointFromInternalCoord(const Coordinate* coord,
		const Geometry *exemplar) {
	Coordinate newcoord = *coord;
	exemplar->getPrecisionModel()->makePrecise(&newcoord);
	return exemplar->getFactory()->createPoint(newcoord);
}


Geometry* GeometryFactory::toGeometry(Envelope* envelope) {
	if (envelope->isNull()) {
		return createPoint(*(new Coordinate()));
	}
	if (envelope->getMinX()==envelope->getMaxX() && envelope->getMinY()==envelope->getMaxY()) {
		return createPoint(*(new Coordinate(envelope->getMinX(),envelope->getMinY())));
	}
	CoordinateList *cl=CoordinateListFactory::internalFactory->createCoordinateList();
	cl->add(*(new Coordinate(envelope->getMinX(), envelope->getMinY())));
	cl->add(*(new Coordinate(envelope->getMaxX(), envelope->getMinY())));
	cl->add(*(new Coordinate(envelope->getMaxX(), envelope->getMaxY())));
	cl->add(*(new Coordinate(envelope->getMinX(), envelope->getMaxY())));
	cl->add(*(new Coordinate(envelope->getMinX(), envelope->getMinY())));
	return createPolygon(createLinearRing(cl),NULL);
}

/**
* Returns the PrecisionModel that Geometries created by this factory
* will be associated with.
*/
const PrecisionModel* GeometryFactory::getPrecisionModel() const {
	return precisionModel;
}

/**
* Creates a Point using the given Coordinate; a null Coordinate will create
* an empty Geometry.
*/
Point* GeometryFactory::createPoint(const Coordinate& coordinate){
	if (coordinate==Coordinate::nullCoord) {
		return createPoint(NULL);
	} else {
		CoordinateList *cl=coordinateListFactory->createCoordinateList();
		cl->add(coordinate);
		return createPoint(cl);
	}
}

/**
* Creates a Point using the given CoordinateSequence; a null or empty
* CoordinateSequence will create an empty Point.
*/
Point* GeometryFactory::createPoint(CoordinateList *coordinates) {
	return new Point(coordinates,this);
}



/**
* Creates a MultiLineString using the given LineStrings; a null or empty
* array will create an empty MultiLineString.
* @param lineStrings LineStrings, each of which may be empty but not null
*/
MultiLineString* GeometryFactory::createMultiLineString(vector<Geometry *> *lineStrings){
	return new MultiLineString(lineStrings,this);
}

/**
* Creates a GeometryCollection using the given Geometries; a null or empty
* array will create an empty GeometryCollection.
* @param geometries Geometries, each of which may be empty but not null
*/
GeometryCollection* GeometryFactory::createGeometryCollection(vector<Geometry *> *geometries){
	return new GeometryCollection(geometries,this);
}

/**
* Creates a MultiPolygon using the given Polygons; a null or empty array
* will create an empty Polygon. The polygons must conform to the
* assertions specified in the <A
* HREF="http://www.opengis.org/techno/specs.htm">OpenGIS Simple Features
* Specification for SQL</A>.
*
* @param polygons
*            Polygons, each of which may be empty but not null
*/
MultiPolygon* GeometryFactory::createMultiPolygon(vector<Geometry *> *polygons){
	return new MultiPolygon(polygons,this);
}

/**
* Creates a LinearRing using the given CoordinateSequence; a null or empty CoordinateSequence will
* create an empty LinearRing. The points must form a closed and simple
* linestring. Consecutive points must not be equal.
* @param coordinates a CoordinateSequence possibly empty, or null
*/
LinearRing* GeometryFactory::createLinearRing(CoordinateList* coordinates) {
	//if (coordinates->getSize()>0 && 
	//	!coordinates->getAt(0).equals2D(coordinates->getAt(coordinates->getSize() - 1))) {
	//		delete precisionModel;
	//		throw new IllegalArgumentException("LinearRing not closed");
	//}
	return new LinearRing(coordinates,this);
}

/**
* Creates a MultiPoint using the given Points; a null or empty array will
* create an empty MultiPoint.
* @param coordinates an array without null elements, or an empty array, or null
*/
MultiPoint* GeometryFactory::createMultiPoint(vector<Geometry *> *point) {
	return new MultiPoint(point,this);
}

/**
* Creates a MultiPoint using the given CoordinateSequence; a null or empty CoordinateSequence will
* create an empty MultiPoint.
* @param coordinates a CoordinateSequence possibly empty, or null
*/
MultiPoint* GeometryFactory::createMultiPoint(CoordinateList* coordinates) {
	vector<Geometry *> *pts=new vector<Geometry *>();
	for (int i=0; i<coordinates->getSize(); i++) {
		Point *pt=createPoint(coordinates->getAt(i));
		pts->push_back(pt);
	}
	delete coordinates;
	MultiPoint *mp = createMultiPoint(pts);
	delete pts;
	return mp;
}

/**
* Constructs a <code>Polygon</code> with the given exterior boundary and
* interior boundaries.
*
* @param shell
*            the outer boundary of the new <code>Polygon</code>, or
*            <code>null</code> or an empty <code>LinearRing</code> if
*            the empty geometry is to be created.
* @param holes
*            the inner boundaries of the new <code>Polygon</code>, or
*            <code>null</code> or empty <code>LinearRing</code> s if
*            the empty geometry is to be created.
*/
Polygon* GeometryFactory::createPolygon(LinearRing *shell, vector<Geometry *> *holes) {
	return new Polygon(shell, holes, this);
}

/**
* Creates a LineString using the given Coordinates; a null or empty array will
* create an empty LineString. Consecutive points must not be equal.
* @param coordinates an array without null elements, or an empty array, or null
*/
LineString* GeometryFactory::createLineString(const CoordinateList* coordinates) {
	return new LineString(coordinates, this);
}

/**
*  Build an appropriate <code>Geometry</code>, <code>MultiGeometry</code>, or
*  <code>GeometryCollection</code> to contain the <code>Geometry</code>s in
*  it.
* For example:<br>
*
*  <ul>
*    <li> If <code>geomList</code> contains a single <code>Polygon</code>,
*    the <code>Polygon</code> is returned.
*    <li> If <code>geomList</code> contains several <code>Polygon</code>s, a
*    <code>MultiPolygon</code> is returned.
*    <li> If <code>geomList</code> contains some <code>Polygon</code>s and
*    some <code>LineString</code>s, a <code>GeometryCollection</code> is
*    returned.
*    <li> If <code>geomList</code> is empty, an empty <code>GeometryCollection</code>
*    is returned
*  </ul>
*
* Note that this method does not "flatten" Geometries in the input, and hence if
* any MultiGeometries are contained in the input a GeometryCollection containing
* them will be returned.
*
*@param  geomList  the <code>Geometry</code>s to combine
*@return           a <code>Geometry</code> of the "smallest", "most
*      type-specific" class that can contain the elements of <code>geomList</code>
*      .
*/
Geometry* GeometryFactory::buildGeometry(vector<Geometry *> *geoms) {
	string geomClass("NULL");
	bool isHeterogeneous=false;
	bool isCollection=geoms->size()>1;
	unsigned int i;
    
	for (i=0; i<geoms->size(); i++) {
		string partClass(typeid(*(*geoms)[i]).name());
		if (geomClass=="NULL") {
			geomClass=partClass;
		}
		if (partClass!=geomClass) {
			isHeterogeneous = true;
		}
	}
    // for the empty geometry, return an empty GeometryCollection
	if (geomClass=="NULL") {
		return createGeometryCollection(NULL);
	}
	if (isHeterogeneous) {
		return createGeometryCollection(geoms);
	}
    // at this point we know the collection is hetereogenous.
    // Determine the type of the result from the first Geometry in the list
    // this should always return a geometry, since otherwise an empty collection would have already been returned
	Geometry *geom0=(*geoms)[0];
	if (isCollection) {
		if (typeid(*geom0)==typeid(Polygon)) {
			return createMultiPolygon(geoms);
		} else if (typeid(*geom0)==typeid(LineString)) {
			return createMultiLineString(geoms);
		} else if (typeid(*geom0)==typeid(Point)) {
			return createMultiPoint(geoms);
		}
		Assert::shouldNeverReachHere();
	}
	if (typeid(*geom0)==typeid(Polygon))
		return (Polygon*) geom0;
	// LineString also handles LinearRings
	else if (typeid(*geom0)==typeid(LineString))
		return (LineString*) geom0;
	else if (typeid(*geom0)==typeid(Point))
		return (Point*) geom0;
	else if (typeid(*geom0)==typeid(MultiPoint))
		return (MultiPoint*) geom0;
	else if (typeid(*geom0)==typeid(MultiLineString))
		return (MultiLineString*) geom0;
	else if (typeid(*geom0)==typeid(MultiPolygon))
		return (MultiPolygon*) geom0;
	else if (typeid(*geom0)==typeid(GeometryCollection))
		return (GeometryCollection*) geom0;
	else 
		return geom0;
}


CoordinateList* gfCoordinateOperation::edit(CoordinateList *coordinates, Geometry *geometry) {
	return coordinates;
}
//Remember to add this.
  /**
   * @return a clone of g based on a CoordinateSequence created by this
   * GeometryFactory's CoordinateSequenceFactory
   */
Geometry* GeometryFactory::createGeometry(const Geometry *g) const {
	// could this be cached to make this more efficient? Or maybe it isn't enough overhead to bother
	GeometryEditor *editor=new GeometryEditor((GeometryFactory*)this);
	return editor->edit((Geometry*)g,new gfCoordinateOperation());
}

}

