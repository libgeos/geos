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
 * Revision 1.64  2004/07/17 10:48:04  strk
 * fixed typo in documentation
 *
 * Revision 1.63  2004/07/17 09:18:54  strk
 * Added geos::version()
 *
 * Revision 1.62  2004/07/14 21:20:58  strk
 * Added GeometricShapeFactory note on doxygen mainpage
 *
 * Revision 1.61  2004/07/08 19:34:49  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added DefaultCoordinateSequenceFactory::instance() function.
 *
 * Revision 1.60  2004/07/07 09:38:12  strk
 * Dropped WKTWriter::stringOfChars (implemented by std::string).
 * Dropped WKTWriter default constructor (internally created GeometryFactory).
 * Updated XMLTester to respect the changes.
 * Main documentation page made nicer.
 *
 * Revision 1.59  2004/07/06 17:58:21  strk
 * Removed deprecated Geometry constructors based on PrecisionModel and
 * SRID specification. Removed SimpleGeometryPrecisionReducer capability
 * of changing Geometry's factory. Reverted Geometry::factory member
 * to be a reference to external factory.
 *
 * Revision 1.58  2004/07/05 19:40:48  strk
 * Added GeometryFactory::destroyGeometry(Geometry *)
 *
 * Revision 1.57  2004/07/05 15:20:18  strk
 * Documentation again.
 *
 * Revision 1.56  2004/07/05 10:50:20  strk
 * deep-dopy construction taken out of Geometry and implemented only
 * in GeometryFactory.
 * Deep-copy geometry construction takes care of cleaning up copies
 * on exception.
 * Implemented clone() method for CoordinateSequence
 * Changed createMultiPoint(CoordinateSequence) signature to reflect
 * copy semantic (by-ref instead of by-pointer).
 * Cleaned up documentation.
 *
 * Revision 1.55  2004/07/03 12:51:37  strk
 * Documentation cleanups for DoxyGen.
 *
 * Revision 1.54  2004/07/02 13:28:26  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.53  2004/07/01 17:34:07  strk
 * GeometryFactory argument in Geometry constructor reverted
 * to its copy-and-destroy semantic.
 *
 * Revision 1.52  2004/07/01 14:12:44  strk
 *
 * Geometry constructors come now in two flavors:
 * 	- deep-copy args (pass-by-reference)
 * 	- take-ownership of args (pass-by-pointer)
 * Same functionality is available through GeometryFactory,
 * including buildGeometry().
 *
 * Revision 1.51  2004/06/30 20:59:12  strk
 * Removed GeoemtryFactory copy from geometry constructors.
 * Enforced const-correctness on GeometryFactory arguments.
 *
 * Revision 1.50  2004/05/21 13:58:47  strk
 * ::intersection missed to invalidate geometryCollection inputs
 *
 * Revision 1.49  2004/05/17 07:23:05  strk
 * ::getCeontroid(): reduced dynamic allocations, added missing check for isEmpty
 *
 * Revision 1.48  2004/05/14 12:14:08  strk
 * const correctness
 *
 * Revision 1.47  2004/05/07 09:05:13  strk
 * Some const correctness added. Fixed bug in GeometryFactory::createMultiPoint
 * to handle NULL CoordinateSequence.
 *
 * Revision 1.46  2004/05/05 16:51:29  strk
 * avoided copy constructor in Geometry::geometryChangedFilter initializzazion
 *
 * Revision 1.45  2004/05/05 10:54:48  strk
 * Removed some private static heap explicit allocation, less cleanup done by
 * the unloader.
 *
 * Revision 1.44  2004/04/30 09:15:28  strk
 * Enlarged exception specifications to allow for AssertionFailedException.
 * Added missing initializers.
 *
 * Revision 1.43  2004/04/20 10:14:20  strk
 * Memory leaks removed.
 *
 * Revision 1.42  2004/04/14 13:56:26  strk
 * All geometries returned by {from,to}InternalGeometry calls are
 * now deleted after use (unless NOT new).
 * Some 'commented' throw specifications in geom.h
 *
 * Revision 1.41  2004/04/14 07:29:43  strk
 * Fixed GeometryFactory constructors to copy given PrecisionModel. Added GeometryFactory copy constructor. Fixed Geometry constructors to copy GeometryFactory.
 *
 * Revision 1.40  2004/04/01 10:44:33  ybychkov
 * All "geom" classes from JTS 1.3 upgraded to JTS 1.4
 *
 * Revision 1.39  2004/03/17 02:00:33  ybychkov
 * "Algorithm" upgraded to JTS 1.4
 *
 * Revision 1.38  2004/03/01 22:04:59  strk
 * applied const correctness changes by Manuel Prieto Villegas <ManuelPrietoVillegas@telefonica.net>
 *
 * Revision 1.37  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 * Revision 1.36  2003/10/20 15:41:34  strk
 * Geometry::checkNotGeometryCollection made static and non-distructive.
 *
 * Revision 1.35  2003/10/13 12:51:28  strk
 * removed sortedClasses strings array from all geometries.
 *
 **********************************************************************/


#include <geos/geom.h>
#include <geos/util.h>
#include <typeinfo>
#include <algorithm>
#include <geos/geosAlgorithm.h>
#include <geos/operation.h>
#include <geos/opRelate.h>
#include <geos/opValid.h>
#include <geos/opDistance.h>
#include <geos/opOverlay.h>
#include <geos/opBuffer.h>
#include <geos/io.h>
#include <geos/version.h>

namespace geos {

/** \mainpage 
 *
 * \section intro_sec Introduction
 *
 * Geometry Engine Open Source is a C++ port of the Java Topology Suite.
 *
 * \section getstart_sec Getting Started
 *
 * Main class is geos::Geometry, from which all geometry types
 * derive.
 *
 * Construction and destruction of Geometries is done
 * using geos::GeometryFactory.
 *
 * You'll feed it geos::CoordinateSequence
 * for base geometries or vectors of geometries for collections.
 *
 * If you need to construct geometric shaped geometries, you
 * can use geos::GeometricShapeFactory.
 *
 * \section io_sect Input / Output
 *
 * For WKT input/output you can use geos::WKTReader and geos::WKTWriter
 *
 * \section exc_sect Exceptions
 *
 * Internal exceptions are thrown as pointers to geos::GEOSException.
 *
 * Other standard exceptions are not mapped to this handler, nor
 * does GEOSException inerit from standard exception, so you'll need
 * to catch both if you care (this might change in the future)
 *
 */ 

string
version()
{
	return "GEOS "GEOS_VERSION" ported from JTS "GEOS_JTS_PORT;
}

GeometryComponentFilter Geometry::geometryChangedFilter;

const GeometryFactory* Geometry::INTERNAL_GEOMETRY_FACTORY=new GeometryFactory();

Geometry::Geometry(const GeometryFactory *newFactory) {
	factory=newFactory; //new GeometryFactory(*fromFactory);
	SRID=factory->getSRID();
	envelope=new Envelope();
	userData=NULL;
}

Geometry::Geometry(const Geometry &geom) {
	factory=geom.factory; //new GeometryFactory(*(geom.factory));
	envelope=new Envelope(*(geom.envelope));
	SRID=factory->getSRID();
	userData=NULL;
}

bool
Geometry::hasNonEmptyElements(const vector<Geometry *>* geometries) 
{
	for (unsigned int i=0; i<geometries->size(); i++) {
		if (!(*geometries)[i]->isEmpty()) {
			return true;
		}
	}
	return false;
}

bool
Geometry::hasNullElements(const CoordinateSequence* list) 
{
	for (int i = 0; i<list->getSize(); i++) {
		if (list->getAt(i)==Coordinate::getNull()) {
			return true;
		}
	}
	return false;
}

bool
Geometry::hasNullElements(const vector<Geometry *>* lrs) 
{
	for (unsigned int i = 0; i<lrs->size(); i++) {
		if ((*lrs)[i]==NULL) {
			return true;
		}
	}
	return false;
}
	
//void Geometry::reversePointOrder(CoordinateSequence* coordinates) {
//	int length=coordinates->getSize();
//	vector<Coordinate> v(length);
//	for (int i=0; i<length; i++) {
//		v[i]=coordinates->getAt(length - 1 - i);
//	}
//	coordinates->setPoints(v);
//}
	
//Coordinate& Geometry::minCoordinate(CoordinateSequence* coordinates){
//	vector<Coordinate> v(*(coordinates->toVector()));
//	sort(v.begin(),v.end(),lessThen);
//	return v.front();
//}

//void Geometry::scroll(CoordinateSequence* coordinates,Coordinate* firstCoordinate) {
//	int ind=indexOf(firstCoordinate,coordinates);
//	Assert::isTrue(ind > -1);
//	int length=coordinates->getSize();
//	vector<Coordinate> v(length);
//	for (int i=ind; i<length; i++) {
//		v[i-ind]=coordinates->getAt(i);
//	}
//	for (int j=0; j<ind; j++) {
//		v[length-ind+j]=coordinates->getAt(j);
//	}
//	coordinates->setPoints(v);
//}
//
//int Geometry::indexOf(Coordinate* coordinate,CoordinateSequence* coordinates) {
//	for (int i=0; i<coordinates->getSize(); i++) {
//		if ((*coordinate)==coordinates->getAt(i)) {
//			return i;
//		}
//	}
//	return -1;
//}

/**
* Tests whether the distance from this <code>Geometry</code>
* to another is less than or equal to a specified value.
*
* @param geom the Geometry to check the distance to
* @param distance the distance value to compare
* @return <code>true</code> if the geometries are less than <code>distance</code> apart.
*/
bool Geometry::isWithinDistance(const Geometry *geom,double cDistance) {
	Envelope *env0=getEnvelopeInternal();
	Envelope *env1=geom->getEnvelopeInternal();
	double envDist=env0->distance(env1);
	delete env0;
	delete env1;
	if (envDist>cDistance)
	{
		return false;
	}
	// NOTE: this could be implemented more efficiently
	double geomDist=distance(geom);
	if (geomDist>cDistance)
	{
		return false;
	}
	return true;
}

/**
* Computes the centroid of this <code>Geometry</code>.
* The centroid
* is equal to the centroid of the set of component Geometrys of highest
* dimension (since the lower-dimension geometries contribute zero
* "weight" to the centroid)
*
* @return a {@link Point} which is the centroid of this Geometry
*/
Point* Geometry::getCentroid() const {
	if ( isEmpty() ) { return NULL; }
	Coordinate* centPt;
	int dim=getDimension();
	Geometry *in = toInternalGeometry(this);
	if(dim==0) {
		CentroidPoint cent; 
		cent.add(in);
		centPt=cent.getCentroid();
	} else if (dim==1) {
		CentroidLine cent;
		cent.add(in);
		centPt=cent.getCentroid();
	} else {
		CentroidArea cent;
		cent.add(in);
		centPt=cent.getCentroid();
	}
	Point *pt=createPointFromInternalCoord(centPt,this);
	delete centPt;
	if ( in != this ) delete(in);
	return pt;
}

/**
* Computes an interior point of this <code>Geometry</code>.
* An interior point is guaranteed to lie in the interior of the Geometry,
* if it possible to calculate such a point exactly. Otherwise,
* the point may lie on the boundary of the geometry.
*
* @return a {@link Point} which is in the interior of this Geometry
*/
Point* Geometry::getInteriorPoint() {
	const Coordinate* interiorPt;
	int dim=getDimension();
	Geometry *in = toInternalGeometry(this);
	if (dim==0) {
		InteriorPointPoint* intPt=new InteriorPointPoint(in);
		interiorPt=intPt->getInteriorPoint();
		delete intPt;
	} else if (dim==1) {
		InteriorPointLine* intPt=new InteriorPointLine(in);
		interiorPt=intPt->getInteriorPoint();
		delete intPt;
	} else {
		InteriorPointArea* intPt=new InteriorPointArea(in);
		interiorPt=intPt->getInteriorPoint();
		delete intPt;
	}
	Point *p=createPointFromInternalCoord(interiorPt,this);
	delete interiorPt;
	if ( in != this ) delete (in);
	return p;
}

/**
* Notifies this Geometry that its Coordinates have been changed by an external
* party (using a CoordinateFilter, for example). The Geometry will flush
* and/or update any information it has cached (such as its {@link Envelope} ).
*/
void Geometry::geometryChanged() {
	apply_rw(&geometryChangedFilter);
}

/**
* Notifies this Geometry that its Coordinates have been changed by an external
* party. When #geometryChanged is called, this method will be called for
* this Geometry and its component Geometries.
* @see #apply(GeometryComponentFilter)
*/
void Geometry::geometryChangedAction() {
	envelope=NULL;
}

int Geometry::getSRID() const {return SRID;}
void Geometry::setSRID(int newSRID) {SRID=newSRID;}

/**
* Gets the factory which contains the context in which this geometry was created.
*
* @return the factory for this geometry
*/
const GeometryFactory*
Geometry::getFactory() const{
	return factory;
}

/**
* Gets the user data object for this geometry, if any.
*
* @return the user data object, or <code>null</code> if none set
*/
void* Geometry::getUserData() {
	return userData;
}

/**
* A simple scheme for applications to add their own custom data to a Geometry.
* An example use might be to add an object representing a Coordinate Reference System.
* <p>
* Note that user data objects are not present in geometries created by
* construction methods.
*
* @param userData an object, the semantics for which are defined by the
* application using this Geometry
*/
void Geometry::setUserData(void* newUserData) {
	userData=newUserData;
}

const PrecisionModel* Geometry::getPrecisionModel() const {
	return factory->getPrecisionModel();
}

bool Geometry::isValid() const {
	Geometry *in = toInternalGeometry(this);
	IsValidOp isValidOp(in);
	bool ret = isValidOp.isValid();
	if (in != this) delete (in);
	return ret;
}

Geometry* Geometry::getEnvelope() const {
	return getFactory()->toGeometry(getEnvelopeInternal());
}

Envelope* Geometry::getEnvelopeInternal() const {
	if (envelope->isNull()) {
		return computeEnvelopeInternal();
	} else 
		return new Envelope(*envelope);
}

bool Geometry::disjoint(const Geometry *g) const{
	IntersectionMatrix *im=relate(g);
	bool res=im->isDisjoint();
	delete im;
	return res;
}

bool Geometry::touches(const Geometry *g) const{
	IntersectionMatrix *im=relate(g);
	bool res=im->isTouches(getDimension(), g->getDimension());
	delete im;
	return res;
}

bool Geometry::intersects(const Geometry *g) const{
	IntersectionMatrix *im=relate(g);
	bool res=im->isIntersects();
	delete im;
	return res;
}

bool Geometry::crosses(const Geometry *g) const{
	IntersectionMatrix *im=relate(g);
	bool res=im->isCrosses(getDimension(), g->getDimension());
	delete im;
	return res;
}

bool Geometry::within(const Geometry *g) const{
	IntersectionMatrix *im=relate(g);
	bool res=im->isWithin();
	delete im;
	return res;
}

bool Geometry::contains(const Geometry *g) const{
	IntersectionMatrix *im=relate(g);
	bool res=im->isContains();
	delete im;
	return res;
}

bool Geometry::overlaps(const Geometry *g) const{
	IntersectionMatrix *im=relate(g);
	bool res=im->isOverlaps(getDimension(), g->getDimension());
	delete im;
	return res;
}

bool Geometry::relate(const Geometry *g, string intersectionPattern) const {
	IntersectionMatrix *im=relate(g);
	bool res=im->matches(intersectionPattern);
	delete im;
	return res;
}

bool Geometry::equals(const Geometry *g) const {
	IntersectionMatrix *im=relate(g);
	bool res=im->isEquals(getDimension(), g->getDimension());
	delete im;
	return res;
}

IntersectionMatrix*
Geometry::relate(const Geometry *other) const
	//throw(IllegalArgumentException *)
{
	checkNotGeometryCollection(this);
	checkNotGeometryCollection(other);
	Geometry *in1 = toInternalGeometry(this);
	Geometry *in2 = toInternalGeometry(other);
	IntersectionMatrix *im = NULL;
	try {
		im = RelateOp::relate(in1, in2);
	}
	catch (...) {
		if ( in1 != this ) delete (in1);
		if ( in2 != other ) delete (in2);
		throw;
	}
	if ( in1 != this ) delete (in1);
	if ( in2 != other ) delete (in2);
	return im;
}

string Geometry::toString() const {
	return toText();
}

string Geometry::toText() const {
	WKTWriter writer;
	return writer.write(this);
}

Geometry* Geometry::buffer(double distance) const {
	Geometry *in1 = toInternalGeometry(this);
	Geometry *out = NULL;
	try {
		out = BufferOp::bufferOp(in1, distance);
	}
	catch(...) {
		if ( in1 != this ) delete (in1);
		throw;
	}
	if ( in1 != this ) delete (in1);
	Geometry *ret = fromInternalGeometry(out);
	if ( ret != out ) delete (out);
	return ret;
}


/**
* The JTS algorithms assume that Geometry#getCoordinate and #getCoordinates
* are fast, which may not be the case if the CoordinateSequence is not a
* DefaultCoordinateSequence (e.g. if it were implemented using separate arrays
* for the x- and y-values), in which case frequent construction of Coordinates
* takes up much space and time. To solve this performance problem,
* #toInternalGeometry converts the Geometry to a DefaultCoordinateSequence
* implementation before sending it to the JTS algorithms.
*
* Note: if the Geometry is already implemented with DefaultCoordinateSequence
* it is returned untouched, so you should check returned value before
* releasing memory associated with the one used as argument.
*/
Geometry* Geometry::toInternalGeometry(const Geometry *g) const {
	if (DefaultCoordinateSequenceFactory::instance()==factory->getCoordinateSequenceFactory()) {
		return (Geometry*)g;
	}
	return INTERNAL_GEOMETRY_FACTORY->createGeometry(g);
}

Geometry* Geometry::fromInternalGeometry(const Geometry* g) const {
	if (DefaultCoordinateSequenceFactory::instance()==factory->getCoordinateSequenceFactory()) {
		return (Geometry*)g;
	}
	return getFactory()->createGeometry(g);
}

/**
*  Returns a buffer region around this <code>Geometry</code> having the given
*  width and with a specified number of segments used to approximate curves.
* The buffer of a Geometry is the Minkowski sum of the Geometry with
* a disc of radius <code>distance</code>.  Curves in the buffer polygon are
* approximated with line segments.  This method allows specifying the
* accuracy of that approximation.
*
*@param  distance  the width of the buffer, interpreted according to the
*      <code>PrecisionModel</code> of the <code>Geometry</code>
*@param quadrantSegments the number of segments to use to approximate a quadrant of a circle
*@return           all points whose distance from this <code>Geometry</code>
*      are less than or equal to <code>distance</code>
*/
Geometry* Geometry::buffer(double distance,int quadrantSegments) const {
	Geometry *in = toInternalGeometry(this);
	Geometry *out = BufferOp::bufferOp(in, distance, quadrantSegments);
	if ( in != this ) delete(in);
	Geometry *ret = fromInternalGeometry(out);
	if ( out != ret ) delete(out);
	return ret;
}

Geometry*
Geometry::convexHull() const
{
	Geometry *in = toInternalGeometry(this);
	ConvexHull *ch = new ConvexHull(in);
	Geometry *out=ch->getConvexHull();
	delete ch;
	if ( in != this ) delete(in);
	Geometry *ret = fromInternalGeometry(out);
	if ( out != ret ) delete(out);
	return ret;
}

Geometry* Geometry::intersection(const Geometry *other) const {
	checkNotGeometryCollection(this);
	checkNotGeometryCollection(other);
	Geometry *in1 = toInternalGeometry(this);
	Geometry *in2 = toInternalGeometry(other);
	Geometry *out = NULL;
	try {
		out = OverlayOp::overlayOp(in1,in2,OverlayOp::INTERSECTION);
	}
	catch (...) {
		if ( in1 != this ) delete (in1);
		if ( in2 != other ) delete (in2);
		throw;
	}
	if ( in1 != this ) delete (in1);
	if ( in2 != other ) delete (in2);
	Geometry *ret = fromInternalGeometry(out);
	if ( ret != out ) delete (out);
	return ret;
}

Geometry*
Geometry::Union(const Geometry *other) const
	//throw(TopologyException *, IllegalArgumentException *)
{
	checkNotGeometryCollection(this);
	checkNotGeometryCollection(other);
	Geometry *in1 = toInternalGeometry(this);
	Geometry *in2 = toInternalGeometry(other);
	Geometry *out = NULL;
	try {
		out = OverlayOp::overlayOp(in1,in2,OverlayOp::UNION);
	}
	catch (...) {
		if ( in1 != this ) delete (in1);
		if ( in2 != other ) delete (in2);
		throw;
	}
	if ( in1 != this ) delete (in1);
	if ( in2 != other ) delete (in2);
	Geometry *ret = fromInternalGeometry(out);
	if ( ret != out ) delete (out);
	return ret;
}

Geometry*
Geometry::difference(const Geometry *other) const
	//throw(IllegalArgumentException *)
{
	checkNotGeometryCollection(this);
	checkNotGeometryCollection(other);
	Geometry *in1 = toInternalGeometry(this);
	Geometry *in2 = toInternalGeometry(other);
	Geometry *out = NULL;
	try {
		out = OverlayOp::overlayOp(in1,in2,OverlayOp::DIFFERENCE);
	}
	catch (...) {
		if ( in1 != this ) delete (in1);
		if ( in2 != other ) delete (in2);
		throw;
	}
	if ( in1 != this ) delete (in1);
	if ( in2 != other ) delete (in2);
	Geometry *ret = fromInternalGeometry(out);
	if ( ret != out ) delete (out);
	return ret;
}

Geometry* Geometry::symDifference(const Geometry *other) const {
	checkNotGeometryCollection(this);
	checkNotGeometryCollection(other);
	Geometry *in1 = toInternalGeometry(this);
	Geometry *in2 = toInternalGeometry(other);
	Geometry *out = NULL;
	try {
		out = OverlayOp::overlayOp(in1,in2,OverlayOp::SYMDIFFERENCE);
	}
	catch (...) {
		if ( in1 != this ) delete (in1);
		if ( in2 != other ) delete (in2);
		throw;
	}
	if ( in1 != this ) delete (in1);
	if ( in2 != other ) delete (in2);
	Geometry *ret = fromInternalGeometry(out);
	if ( ret != out ) delete (out);
	return ret;
}

int Geometry::compareTo(const Geometry *geom) const {
	if (getClassSortIndex()!=geom->getClassSortIndex()) {
		return getClassSortIndex()-geom->getClassSortIndex();
	}
	if (isEmpty() && geom->isEmpty()) {
		return 0;
	}
	if (isEmpty()) {
		return -1;
	}
	if (geom->isEmpty()) {
		return 1;
	}
	return compareToSameClass(geom);
}

bool Geometry::isEquivalentClass(const Geometry *other) const {
	if (typeid(*this)==typeid(*other))
		return true;
	else
		return false;
}

void
Geometry::checkNotGeometryCollection(const Geometry *g)
	//throw(IllegalArgumentException *)
{
	if ((typeid(*g)==typeid(GeometryCollection))) {
		throw new IllegalArgumentException("This method does not support GeometryCollection arguments\n");
	}
}

int Geometry::getClassSortIndex() const {
	//const type_info &t=typeid(*this);

	     if ( typeid(*this) == typeid(Point)              ) return 0;
	else if ( typeid(*this) == typeid(MultiPoint)         ) return 1;
	else if ( typeid(*this) == typeid(LineString)         ) return 2;
	else if ( typeid(*this) == typeid(LinearRing)         ) return 3;
	else if ( typeid(*this) == typeid(MultiLineString)    ) return 4;
	else if ( typeid(*this) == typeid(Polygon)            ) return 5;
	else if ( typeid(*this) == typeid(MultiPolygon)       ) return 6;
	else if ( typeid(*this) == typeid(GeometryCollection) ) return 7;

	string str="Class not supported: ";
	str.append(typeid(*this).name());
	str.append("");
	Assert::shouldNeverReachHere(str);
	return -1;
}

int Geometry::compare(vector<Coordinate> a, vector<Coordinate> b) const {
	unsigned int i=0;
	unsigned int j=0;
	while (i<a.size() && j<b.size()) {
		Coordinate& aCoord=a[i];
		Coordinate& bCoord=b[j];
		int comparison=aCoord.compareTo(bCoord);
		if (comparison!=0) {
			return comparison;
		}
		i++;
		j++;
	}
	if (i<a.size()) {
		return 1;
	}
	if (j<b.size()) {
		return -1;
	}
	return 0;
}

int Geometry::compare(vector<Geometry *> a, vector<Geometry *> b) const {
	unsigned int i=0;
	unsigned int j=0;
	while (i<a.size() && j<b.size()) {
		Geometry *aGeom=a[i];
		Geometry *bGeom=b[j];
		int comparison=aGeom->compareTo(bGeom);
		if (comparison!=0) {
			return comparison;
		}
		i++;
		j++;
	}
	if (i<a.size()) {
		return 1;
	}
	if (j<b.size()) {
		return -1;
	}
	return 0;
}

/**
*  Returns the minimum distance between this <code>Geometry</code>
*  and the <code>Geometry</code> g
*
*@param  g  the <code>Geometry</code> from which to compute the distance
*/
double Geometry::distance(const Geometry *other) const {
	Geometry *in1 = toInternalGeometry(this);
	Geometry *in2 = toInternalGeometry(other);
	double ret;
	try {
		ret = DistanceOp::distance(in1,in2);
	}
	catch (...) {
		if ( in1 != this ) delete (in1);
		if ( in2 != other ) delete (in2);
		throw;
	}
	if ( in1 != this ) delete (in1);
	if ( in2 != other ) delete (in2);
	return ret;
}

/**
*  Returns the area of this <code>Geometry</code>.
*  Areal Geometries have a non-zero area.
*  They override this function to compute the area.
*  Others return 0.0
*
* @return the area of the Geometry
*/
double Geometry::getArea() const {
	return 0.0;
}

/**
*  Returns the length of this <code>Geometry</code>.
*  Linear geometries return their length.
*  Areal geometries return their perimeter.
*  They override this function to compute the area.
*  Others return 0.0
*
*@return the length of the Geometry
*/
double Geometry::getLength() const {
	return 0.0;
}



Geometry::~Geometry(){
	//delete factory;
	delete envelope;
	//delete userData; /* TODO: make this a Template type (not void*) */
}

bool lessThen(Coordinate& a, Coordinate& b) {
	if (a.compareTo(b)<=0)
		return true;
	else
		return false;
}

bool greaterThen(Geometry *first, Geometry *second) {
	if (first->compareTo(second)>=0)
		return true;
	else
		return false;
}

bool
Geometry::equal(const Coordinate& a, const Coordinate& b,double tolerance) const
{
	if (tolerance==0) {return a==b;}
	return a.distance(b)<=tolerance;
}

void Geometry::apply_ro(GeometryFilter *filter) const {
	filter->filter_ro(this);
}

void Geometry::apply_rw(GeometryFilter *filter) {
	filter->filter_rw(this);
}

void Geometry::apply_ro(GeometryComponentFilter *filter) const {
	filter->filter_ro(this);
}

void Geometry::apply_rw(GeometryComponentFilter *filter) {
	filter->filter_rw(this);
}
Point* Geometry::createPointFromInternalCoord(const Coordinate* coord,const Geometry *exemplar) const{
	Coordinate newcoord = *coord;
	exemplar->getPrecisionModel()->makePrecise(&newcoord);
	return exemplar->getFactory()->createPoint(newcoord);
}

}

