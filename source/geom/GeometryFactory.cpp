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
 * Revision 1.50  2004/07/27 16:35:46  strk
 * Geometry::getEnvelopeInternal() changed to return a const Envelope *.
 * This should reduce object copies as once computed the envelope of a
 * geometry remains the same.
 *
 * Revision 1.49  2004/07/19 13:19:30  strk
 * Documentation fixes
 *
 * Revision 1.48  2004/07/13 08:33:52  strk
 * Added missing virtual destructor to virtual classes.
 * Fixed implicit unsigned int -> int casts
 *
 * Revision 1.47  2004/07/08 19:34:49  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added DefaultCoordinateSequenceFactory::instance() function.
 *
 * Revision 1.46  2004/07/05 19:40:48  strk
 * Added GeometryFactory::destroyGeometry(Geometry *)
 *
 * Revision 1.45  2004/07/05 14:23:03  strk
 * More documentation cleanups.
 *
 * Revision 1.44  2004/07/05 10:50:20  strk
 * deep-dopy construction taken out of Geometry and implemented only
 * in GeometryFactory.
 * Deep-copy geometry construction takes care of cleaning up copies
 * on exception.
 * Implemented clone() method for CoordinateSequence
 * Changed createMultiPoint(CoordinateSequence) signature to reflect
 * copy semantic (by-ref instead of by-pointer).
 * Cleaned up documentation.
 *
 * Revision 1.43  2004/07/03 12:51:37  strk
 * Documentation cleanups for DoxyGen.
 *
 * Revision 1.42  2004/07/02 14:27:32  strk
 * Added deep-copy / take-ownerhship for Point type.
 *
 * Revision 1.41  2004/07/02 13:28:26  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.40  2004/07/01 14:12:44  strk
 *
 * Geometry constructors come now in two flavors:
 * 	- deep-copy args (pass-by-reference)
 * 	- take-ownership of args (pass-by-pointer)
 * Same functionality is available through GeometryFactory,
 * including buildGeometry().
 *
 * Revision 1.39  2004/06/16 13:13:25  strk
 * Changed interface of SegmentString, now copying CoordinateSequence argument.
 * Fixed memory leaks associated with this and MultiGeometry constructors.
 * Other associated fixes.
 *
 * Revision 1.38  2004/06/15 21:35:32  strk
 * fixed buildGeometry to always return a newly allocated geometry
 *
 * Revision 1.37  2004/06/15 20:07:51  strk
 * GeometryCollections constructors make a deep copy of Geometry vector argument.
 *
 * Revision 1.36  2004/05/07 09:05:13  strk
 * Some const correctness added. Fixed bug in GeometryFactory::createMultiPoint
 * to handle NULL CoordinateSequence.
 *
 * Revision 1.35  2004/04/20 08:52:01  strk
 * GeometryFactory and Geometry const correctness.
 * Memory leaks removed from SimpleGeometryPrecisionReducer
 * and GeometryFactory.
 *
 * Revision 1.34  2004/04/16 08:35:52  strk
 * Memory leaks fixed and const correctness applied for Point class.
 *
 * Revision 1.33  2004/04/14 12:28:43  strk
 * shouldNeverReachHere exceptions made more verbose
 *
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


#include <geos/geom.h>
#include <geos/geomUtil.h>
#include <geos/util.h>
#include <typeinfo>

namespace geos {

/**
 * Constructs a GeometryFactory that generates Geometries having a floating
 * PrecisionModel and a spatial-reference ID of 0.
 */
GeometryFactory::GeometryFactory() {
	precisionModel=new PrecisionModel();
	SRID=0;
	coordinateListFactory=DefaultCoordinateSequenceFactory::instance();
}

/**
 * Constructs a GeometryFactory that generates Geometries having the given
 * PrecisionModel, spatial-reference ID, and CoordinateSequence implementation.
 */
GeometryFactory::GeometryFactory(const PrecisionModel *pm, int newSRID,CoordinateSequenceFactory *nCoordinateSequenceFactory) {
	precisionModel=new PrecisionModel(*pm);
	coordinateListFactory=nCoordinateSequenceFactory;
	SRID=newSRID;
}

/**
 * Constructs a GeometryFactory that generates Geometries having the given
 * CoordinateSequence implementation, a double-precision floating
 * PrecisionModel and a spatial-reference ID of 0.
 */
GeometryFactory::GeometryFactory(CoordinateSequenceFactory *nCoordinateSequenceFactory) {
	precisionModel=new PrecisionModel();
	SRID=0;
	coordinateListFactory=nCoordinateSequenceFactory;
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
	coordinateListFactory=DefaultCoordinateSequenceFactory::instance();
}

/**
 * Constructs a GeometryFactory that generates Geometries having the given
 * PrecisionModel and spatial-reference ID, and the default
 * CoordinateSequence implementation.
 *
 * @param precisionModel the PrecisionModel to use
 * @param SRID the SRID to use
 */
GeometryFactory::GeometryFactory(const PrecisionModel* pm, int newSRID){
    precisionModel=new PrecisionModel(*pm);
    SRID=newSRID;
	coordinateListFactory=DefaultCoordinateSequenceFactory::instance();
}

GeometryFactory::GeometryFactory(const GeometryFactory &gf){
    precisionModel=new PrecisionModel(*(gf.precisionModel));
    SRID=gf.SRID;
    coordinateListFactory=gf.coordinateListFactory;
}
GeometryFactory::~GeometryFactory(){
	delete precisionModel;
}
  
Point*
GeometryFactory::createPointFromInternalCoord(const Coordinate* coord,
		const Geometry *exemplar) const {
	Coordinate newcoord = *coord;
	exemplar->getPrecisionModel()->makePrecise(&newcoord);
	return exemplar->getFactory()->createPoint(newcoord);
}


/**
 * Converts an Envelope to a Geometry.
 * Returned Geometry can be a Point, a Polygon or an EMPTY geom.
 */
Geometry*
GeometryFactory::toGeometry(const Envelope* envelope) const
{
	Coordinate coord;

	if (envelope->isNull()) {
		return createPoint();
	}
	if (envelope->getMinX()==envelope->getMaxX() && envelope->getMinY()==envelope->getMaxY()) {
		coord.x = envelope->getMinX();
		coord.y = envelope->getMinY();
		return createPoint(coord);
	}
	CoordinateSequence *cl=DefaultCoordinateSequenceFactory::instance()->create(NULL);
	coord.x = envelope->getMinX();
	coord.y = envelope->getMinY();
	cl->add(coord);
	coord.x = envelope->getMaxX();
	coord.y = envelope->getMinY();
	cl->add(coord);
	coord.x = envelope->getMaxX();
	coord.y = envelope->getMaxY();
	cl->add(coord);
	coord.x = envelope->getMinX();
	coord.y = envelope->getMaxY();
	cl->add(coord);
	coord.x = envelope->getMinX();
	coord.y = envelope->getMinY();
	cl->add(coord);

	Polygon *p = createPolygon(createLinearRing(cl), NULL);
	return p;
}

/**
 * Returns the PrecisionModel that Geometries created by this factory
 * will be associated with.
 */
const PrecisionModel* GeometryFactory::getPrecisionModel() const {
	return precisionModel;
}

/**
 * Creates the EMPTY Point
 */
Point*
GeometryFactory::createPoint() const {
	return new Point(NULL, this);
}

/**
* Creates a Point using the given Coordinate; a null Coordinate will create
* an empty Geometry.
*/
Point*
GeometryFactory::createPoint(const Coordinate& coordinate) const {
	if (coordinate==Coordinate::nullCoord) {
		return createPoint();
	} else {
		CoordinateSequence *cl=coordinateListFactory->create(new vector<Coordinate>(1, coordinate));
		//cl->setAt(coordinate, 0);
		Point *ret = createPoint(cl);
		return ret;
	}
}

/**
 * Creates a Point using the given CoordinateSequence (must have 1 element)
 *
 * @param  newCoords
 *	contains the single coordinate on which to base this
 *	<code>Point</code> or <code>null</code> to create
 *	the empty geometry.
 *
 *	If not null the created Point will take ownership of newCoords.
 */  
Point*
GeometryFactory::createPoint(CoordinateSequence *newCoords) const
{
	return new Point(newCoords,this);
}

/**
 * Creates a Point using the given CoordinateSequence (must have 1 element)
 *
 * @param  fromCoords
 *	contains the single coordinate on which to base this
 *	<code>Point</code>. 
 */
Point*
GeometryFactory::createPoint(const CoordinateSequence &fromCoords) const
{
	CoordinateSequence *newCoords = fromCoords.clone();
	Point *g = NULL;
	try {
		g = new Point(newCoords,this); 
	} catch (...) {
		delete newCoords;
		throw;
	}
	return g;

}

/**
 * Construct an EMPTY MultiLineString
 */
MultiLineString*
GeometryFactory::createMultiLineString() const
{
	return new MultiLineString(NULL,this);
}

/**
 * Constructs a <code>MultiLineString</code>.
 *
 * @param  newLines
 *	the <code>LineStrings</code>s for this
 *	<code>MultiLineString</code>, or <code>null</code>
 *	or an empty array to create the empty geometry.
 *	Elements may be empty <code>LineString</code>s,
 *	but not <code>null</code>s.
 *
 *	Constructed object will take ownership of
 *	the vector and its elements.
 */
MultiLineString*
GeometryFactory::createMultiLineString(vector<Geometry *> *newLines)
	const
{
	return new MultiLineString(newLines,this);
}

/**
 * Constructs a <code>MultiLineString</code>.
 *
 * @param  fromLines
 *	the <code>LineStrings</code>s for this
 *	<code>MultiLineString</code>, or an empty array
 *	to create the empty geometry.
 *	Elements may be empty <code>LineString</code>s,
 *	but not <code>null</code>s.
 *
 *	Constructed object will copy 
 *	the vector and its elements.
 */
MultiLineString*
GeometryFactory::createMultiLineString(const vector<Geometry *> &fromLines)
	const
{
	vector<Geometry *>*newGeoms = new vector<Geometry *>(fromLines.size());
	for (unsigned int i=0; i<fromLines.size(); i++)
	{
		(*newGeoms)[i] = fromLines[i]->clone();
	}
	MultiLineString *g = NULL;
	try {
		g = new MultiLineString(newGeoms,this);
	} catch (...) {
		for (unsigned int i=0; i<newGeoms->size(); i++) {
			delete (*newGeoms)[i];
		}
		delete newGeoms;
		throw;
	}
	return g;
}

/**
 * Constructs an EMPTY <code>GeometryCollection</code>.
 */
GeometryCollection*
GeometryFactory::createGeometryCollection() const
{
	return new GeometryCollection(NULL,this);
}

/**
 * Constructs a GeometryCollection.
 *
 * @param newGeoms
 *	The <code>Geometry</code>s for this
 *	<code>GeometryCollection</code>,
 *	or <code>null</code> or an empty array to
 *	create the empty geometry.
 *	Elements may be empty <code>Geometry</code>s,
 *	but not <code>null</code>s.
 *
 *	If construction succeed the created object will take
 *	ownership of newGeoms vector and elements.
 *
 *	If construction	fails "IllegalArgumentException *"
 *	is thrown and it is your responsibility to delete newGeoms
 *	vector and content.
 */
GeometryCollection*
GeometryFactory::createGeometryCollection(vector<Geometry *> *newGeoms) const
{
	return new GeometryCollection(newGeoms,this);
}

/**
 * @param fromGeoms
 *            the <code>Geometry</code>s for this
 *	     <code>GeometryCollection</code>,
 *	     Elements may be empty <code>Geometry</code>s,
 *            but not <code>null</code>s.
 *	     
 *            fromGeoms vector and elements will be copied. 
 */
GeometryCollection*
GeometryFactory::createGeometryCollection(const vector<Geometry *> &fromGeoms) const
{
	vector<Geometry *> *newGeoms = new vector<Geometry *>(fromGeoms.size());
	for (unsigned int i=0; i<fromGeoms.size(); i++) {
		(*newGeoms)[i] = fromGeoms[i]->clone();
	}
	GeometryCollection *g = NULL;
	try {
		g = new GeometryCollection(newGeoms,this);
	} catch (...) {
		for (unsigned int i=0; i<newGeoms->size(); i++) {
			delete (*newGeoms)[i];
		}
		delete newGeoms;
		throw;
	}
	return g;
}

/*
 * Create an EMPTY MultiPolygon
 */
MultiPolygon*
GeometryFactory::createMultiPolygon() const
{
	return new MultiPolygon(NULL,this);
}

/**
 * @param newPolys
 *	the <code>Polygon</code>s for this <code>MultiPolygon</code>,
 *	or <code>null</code> or an empty array to create the empty
 *	geometry. Elements may be empty <code>Polygon</code>s, but
 *	not <code>null</code>s.
 *	The polygons must conform to the assertions specified in the
 *	<A HREF="http://www.opengis.org/techno/specs.htm">
 *	OpenGIS Simple Features Specification for SQL
 *	</A>.
 *
 *	Constructed object will take ownership of
 *	the vector and its elements.
 */
MultiPolygon*
GeometryFactory::createMultiPolygon(vector<Geometry *> *newPolys) const
{
	return new MultiPolygon(newPolys,this);
}

/**
 * @param fromPolys
 *	the <code>Polygon</code>s for this <code>MultiPolygon</code>,
 *	or an empty array to create the empty geometry.
 *	Elements may be empty <code>Polygon</code>s, but
 *	not <code>null</code>s.
 *	The polygons must conform to the assertions specified in the
 *	<A HREF="http://www.opengis.org/techno/specs.htm">
 *	OpenGIS Simple Features Specification for SQL
 *	</A>.
 *
 *	Constructed object will copy 
 *	the vector and its elements.
 */
MultiPolygon*
GeometryFactory::createMultiPolygon(const vector<Geometry *> &fromPolys) const
{
	vector<Geometry *>*newGeoms = new vector<Geometry *>(fromPolys.size());
	for (unsigned int i=0; i<fromPolys.size(); i++)
	{
		(*newGeoms)[i] = fromPolys[i]->clone();
	}
	MultiPolygon *g = NULL;
	try {
		g = new MultiPolygon(newGeoms,this);
	} catch (...) {
		for (unsigned int i=0; i<newGeoms->size(); i++) {
			delete (*newGeoms)[i];
		}
		delete newGeoms;
		throw;
	}
	return g;
}
 
/**
 * Creates an EMPTY LinearRing 
 */
LinearRing*
GeometryFactory::createLinearRing() const
{
	return new LinearRing(NULL,this);
}

/**
 * Creates a LinearRing using the given CoordinateSequence;
 * a null or empty CoordinateSequence will
 * create an empty LinearRing. The points must form a closed and simple
 * linestring. Consecutive points must not be equal.
 *
 * @param newCoords a CoordinateSequence possibly empty, or null.
 *
 * LinearRing will take ownership of coordinates.
 * 
 */
LinearRing*
GeometryFactory::createLinearRing(CoordinateSequence* newCoords) const
{
	return new LinearRing(newCoords,this);
}

/*
 * Creates a LinearRing using a copy of the given CoordinateSequence.
 * An empty CoordinateSequence will create an empty LinearRing.
 * The points must form a closed and simple
 * linestring. Consecutive points must not be equal.
 * @param fromCoords a CoordinateSequence possibly empty.
 */
LinearRing*
GeometryFactory::createLinearRing(const CoordinateSequence& fromCoords) const
{
	CoordinateSequence *newCoords = fromCoords.clone();
	LinearRing *g = NULL;
	try {
		g = new LinearRing(newCoords, this);
	} catch (...) {
		delete newCoords;
		throw;
	}
	return g;
}

/**
 * Constructs a <code>MultiPoint</code>.
 *
 * @param  newPoints
 *	the <code>Point</code>s for this <code>MultiPoint</code>,
 *	or <code>null</code> or an empty array to create the empty
 * 	geometry.
 *	Elements may be empty <code>Point</code>s,
 *	but not <code>null</code>s.
 *
 *	Constructed object will take ownership of
 *	the vector and its elements.
 */
MultiPoint*
GeometryFactory::createMultiPoint(vector<Geometry *> *newPoints) const
{
	return new MultiPoint(newPoints,this);
}

/**
 * Constructs a <code>MultiPoint</code>.
 *
 * @param  fromPoints
 *	the <code>Point</code>s for this <code>MultiPoint</code>,
 *	or an empty array to create the empty geometry.
 *	Elements may be empty <code>Point</code>s,
 *	but not <code>null</code>s.
 *
 *	Constructed object will copy 
 *	the vector and its elements.
 */
MultiPoint*
GeometryFactory::createMultiPoint(const vector<Geometry *> &fromPoints) const
{
	vector<Geometry *>*newGeoms = new vector<Geometry *>(fromPoints.size());
	for (unsigned int i=0; i<fromPoints.size(); i++)
	{
		(*newGeoms)[i] = fromPoints[i]->clone();
	}

	MultiPoint *g = NULL;
	try {
		g = new MultiPoint(newGeoms,this);
	} catch (...) {
		for (unsigned int i=0; i<newGeoms->size(); i++) {
			delete (*newGeoms)[i];
		}
		delete newGeoms;
		throw;
	}
	return g;
}

/**
 * Creates an EMPTY MultiPoint 
 */
MultiPoint*
GeometryFactory::createMultiPoint() const
{
	return new MultiPoint(NULL, this);
}

/**
 * Creates a MultiPoint using the given CoordinateSequence.
 * @param fromCoords a CoordinateSequence used for Points construction.
 */
MultiPoint*
GeometryFactory::createMultiPoint(const CoordinateSequence &fromCoords) const
{
	vector<Geometry *> *pts=new vector<Geometry *>();
	for (int i=0; i<fromCoords.getSize(); i++) {
		Point *pt=createPoint(fromCoords.getAt(i));
		pts->push_back(pt);
	}
	MultiPoint *mp = NULL;
	try {
		mp = createMultiPoint(pts);
	} catch (...) {
		for (unsigned int i=0; i<pts->size(); i++) delete (*pts)[i];
		delete pts;
		throw;
	}
	return mp;
}

/**
 * Constructs an EMPTY Polygon
 */
Polygon*
GeometryFactory::createPolygon() const
{
	return new Polygon(NULL, NULL, this);
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
Polygon*
GeometryFactory::createPolygon(LinearRing *shell, vector<Geometry *> *holes)
	const
{
	return new Polygon(shell, holes, this);
}

/**
 * Constructs a <code>Polygon</code> with the given exterior boundary and
 * interior boundaries.
 *
 * @param shell
 *            the outer boundary of the new <code>Polygon</code>
 * @param holes
 *            the inner boundaries of the new <code>Polygon</code>
 *            Note that these must be LinearRings
 */
Polygon*
GeometryFactory::createPolygon(const LinearRing &shell, const vector<Geometry *> &holes)
	const
{
	LinearRing *newRing = (LinearRing *)shell.clone();
	vector<Geometry *>*newHoles = new vector<Geometry *>(holes.size());
	for (unsigned int i=0; i<holes.size(); i++)
	{
		(*newHoles)[i] = holes[i]->clone();
	}
	Polygon *g = NULL;
	try {
		g = new Polygon(newRing, newHoles, this);
	} catch (...) {
		delete newRing;
		for (unsigned int i=0; i<holes.size(); i++)
			delete (*newHoles)[i];
		delete newHoles;
		throw;
	}
	return g;
}

/**
 * Constructs an EMPTY LineString
 */
LineString *
GeometryFactory::createLineString() const
{
	return new LineString(NULL, this);
}

/**
 * Constructs a <code>LineString</code> taking ownership of the
 * given CoordinateSequence.
 *
 * @param newCoords the list of coordinates making up the linestring,
 *	or <code>null</code> to create the empty geometry.
 *	Consecutive points may not be equal. Created object will
 *	take ownership of CoordinateSequence.
 *
 */  
LineString*
GeometryFactory::createLineString(CoordinateSequence *newCoords)
	const
{
	return new LineString(newCoords, this);
}

/**
 * Constructs a <code>LineString</code> copying the
 * given CoordinateSequence.
 *
 * @param fromCoords the list of coordinates making up the linestring.
 *	Consecutive points may not be equal.
 */  
LineString*
GeometryFactory::createLineString(const CoordinateSequence &fromCoords)
	const
{
	CoordinateSequence *newCoords = fromCoords.clone();
	LineString *g = NULL;
	try {
		g = new LineString(newCoords, this);
	} catch (...) {
		delete newCoords;
		throw;
	}
	return g;
}

/**
*  Build an appropriate <code>Geometry</code>, <code>MultiGeometry</code>, or
*  <code>GeometryCollection</code> to contain the <code>Geometry</code>s in
*  it.
*
*  For example:
*
*    - If <code>geomList</code> contains a single <code>Polygon</code>,
*      the <code>Polygon</code> is returned.
*    - If <code>geomList</code> contains several <code>Polygon</code>s, a
*      <code>MultiPolygon</code> is returned.
*    - If <code>geomList</code> contains some <code>Polygon</code>s and
*      some <code>LineString</code>s, a <code>GeometryCollection</code> is
*      returned.
*    - If <code>geomList</code> is empty, an empty
*      <code>GeometryCollection</code> is returned
*    .
*
* Note that this method does not "flatten" Geometries in the input,
* and hence if any MultiGeometries are contained in the input a
* GeometryCollection containing them will be returned.
*
* @param  newGeoms  the <code>Geometry</code>s to combine
*
* @return
*	A <code>Geometry</code> of the "smallest", "most type-specific"
*	class that can contain the elements of <code>geomList</code>.
*
* NOTE: the returned Geometry will take ownership of the
* 	given vector AND its elements 
*/
Geometry*
GeometryFactory::buildGeometry(vector<Geometry *> *newGeoms) const
{
	string geomClass("NULL");
	bool isHeterogeneous=false;
	bool isCollection=newGeoms->size()>1;
	unsigned int i;
    
	for (i=0; i<newGeoms->size(); i++) {
		string partClass(typeid(*(*newGeoms)[i]).name());
		if (geomClass=="NULL") {
			geomClass=partClass;
		} else if (geomClass!=partClass) {
			isHeterogeneous = true;
		}
	}

	// for the empty geometry, return an empty GeometryCollection
	if (geomClass=="NULL") {
		// we do not need the vector anymore
		delete newGeoms;
		return createGeometryCollection();
	}
	if (isHeterogeneous) {
		return createGeometryCollection(newGeoms);
	}

	// At this point we know the collection is not hetereogenous.
	// Determine the type of the result from the first Geometry in the
	// list. This should always return a geometry, since otherwise
	// an empty collection would have already been returned
	Geometry *geom0=(*newGeoms)[0];
	if (isCollection) {
		if (typeid(*geom0)==typeid(Polygon)) {
			return createMultiPolygon(newGeoms);
		} else if (typeid(*geom0)==typeid(LineString)) {
			return createMultiLineString(newGeoms);
		} else if (typeid(*geom0)==typeid(LinearRing)) {
			return createMultiLineString(newGeoms);
		} else if (typeid(*geom0)==typeid(Point)) {
			return createMultiPoint(newGeoms);
		}
		Assert::shouldNeverReachHere("buildGeomtry encountered an unkwnon geometry type");
	}

	// since this is not a collection we can delete vector
	delete newGeoms;
	return geom0;
}

/**
 * This function does the same thing of the omonimouse function
 * taking vector pointer instead of reference. 
 * The difference is that this version will copy needed data
 * leaving ownership to the caller.
 */
Geometry*
GeometryFactory::buildGeometry(const vector<Geometry *> &fromGeoms) const
{
	string geomClass("NULL");
	bool isHeterogeneous=false;
	bool isCollection=fromGeoms.size()>1;
	unsigned int i;
    
	for (i=0; i<fromGeoms.size(); i++) {
		string partClass(typeid(*fromGeoms[i]).name());
		if (geomClass=="NULL") {
			geomClass=partClass;
		} else if (geomClass!=partClass) {
			isHeterogeneous = true;
		}
	}

	// for the empty geometry, return an empty GeometryCollection
	if (geomClass=="NULL") {
		return createGeometryCollection();
	}
	if (isHeterogeneous) {
		return createGeometryCollection(fromGeoms);
	}

	// At this point we know the collection is not hetereogenous.
	// Determine the type of the result from the first Geometry in the
	// list. This should always return a geometry, since otherwise
	// an empty collection would have already been returned
	Geometry *geom0=fromGeoms[0];
	if (isCollection) {
		if (typeid(*geom0)==typeid(Polygon)) {
			return createMultiPolygon(fromGeoms);
		} else if (typeid(*geom0)==typeid(LineString)) {
			return createMultiLineString(fromGeoms);
		} else if (typeid(*geom0)==typeid(LinearRing)) {
			return createMultiLineString(fromGeoms);
		} else if (typeid(*geom0)==typeid(Point)) {
			return createMultiPoint(fromGeoms);
		}
		Assert::shouldNeverReachHere("buildGeomtry encountered an unkwnon geometry type");
	}

	return geom0->clone();
}


/**
 * @return a clone of g based on a CoordinateSequence created by this
 * GeometryFactory's CoordinateSequenceFactory
 */
Geometry*
GeometryFactory::createGeometry(const Geometry *g) const
{
	// could this be cached to make this more efficient? Or maybe it isn't enough overhead to bother
	return g->clone();
	//GeometryEditor *editor=new GeometryEditor(this);
	//gfCoordinateOperation *coordOp = new gfCoordinateOperation();
	//Geometry *ret=editor->edit(g, coordOp);
	//delete coordOp;
	//delete editor;
	//return ret;
}

/**
 * Destroy a Geometry, or release it.
 */
void
GeometryFactory::destroyGeometry(Geometry *g) const
{
	delete g;
}

}

