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

#include <cassert>
#include <vector>
#include <typeinfo>

#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateArraySequenceFactory.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Point.h>
#include <geos/geom/LineString.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/MultiPoint.h>
#include <geos/geom/MultiLineString.h>
#include <geos/geom/MultiPolygon.h>
#include <geos/geom/GeometryCollection.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/geom/Envelope.h>
#include <geos/util/IllegalArgumentException.h>

#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif

#ifdef GEOS_DEBUG
#include <iostream>
#endif

#ifndef USE_INLINE
# include <geos/geom/GeometryFactory.inl>
#endif

using namespace std;

namespace geos {
namespace geom { // geos::geom

//namespace { 
//	class gfCoordinateOperation: public CoordinateOperation {
//	using CoordinateOperation::edit;
//	public:
//		virtual CoordinateSequence* edit(const CoordinateSequence *coordinates,
//				const Geometry *geometry);
//	};
//}



/*public*/
GeometryFactory::GeometryFactory()
{
#if GEOS_DEBUG
	std::cerr << "GEOS_DEBUG: GeometryFactory["<<this<<"]::GeometryFactory()" << std::endl;
#endif
	precisionModel=new PrecisionModel();
	SRID=0;
	coordinateListFactory=CoordinateArraySequenceFactory::instance();
}

/*public*/
GeometryFactory::GeometryFactory(const PrecisionModel *pm, int newSRID,CoordinateSequenceFactory *nCoordinateSequenceFactory)
{
	precisionModel=new PrecisionModel(*pm);
	coordinateListFactory=nCoordinateSequenceFactory;
	SRID=newSRID;
}

/*public*/
GeometryFactory::GeometryFactory(CoordinateSequenceFactory *nCoordinateSequenceFactory)
{
	precisionModel=new PrecisionModel();
	SRID=0;
	coordinateListFactory=nCoordinateSequenceFactory;
}

/*public*/
GeometryFactory::GeometryFactory(const PrecisionModel *pm) {
	precisionModel=new PrecisionModel(*pm);
	SRID=0;
	coordinateListFactory=CoordinateArraySequenceFactory::instance();
}

/*public*/
GeometryFactory::GeometryFactory(const PrecisionModel* pm, int newSRID){
    precisionModel=new PrecisionModel(*pm);
    SRID=newSRID;
	coordinateListFactory=CoordinateArraySequenceFactory::instance();
}

/*public*/
GeometryFactory::GeometryFactory(const GeometryFactory &gf){
    precisionModel=new PrecisionModel(*(gf.precisionModel));
    SRID=gf.SRID;
    coordinateListFactory=gf.coordinateListFactory;
}

GeometryFactory::~GeometryFactory(){
#if GEOS_DEBUG
	std::cerr << "GEOS_DEBUG: GeometryFactory["<<this<<"]::~GeometryFactory()" << std::endl;
#endif
	delete precisionModel;
}
  
Point*
GeometryFactory::createPointFromInternalCoord(const Coordinate* coord,
		const Geometry *exemplar) const
{
	assert(coord);
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
	CoordinateSequence *cl=CoordinateArraySequenceFactory::instance()->create(NULL);
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

const PrecisionModel* GeometryFactory::getPrecisionModel() const {
	return precisionModel;
}

Point*
GeometryFactory::createPoint() const {
	return new Point(NULL, this);
}

Point*
GeometryFactory::createPoint(const Coordinate& coordinate) const {
	if (coordinate==Coordinate::getNull()) {
		return createPoint();
	} else {
		CoordinateSequence *cl=coordinateListFactory->create(new vector<Coordinate>(1, coordinate));
		//cl->setAt(coordinate, 0);
		Point *ret = createPoint(cl);
		return ret;
	}
}

Point*
GeometryFactory::createPoint(CoordinateSequence *newCoords) const
{
	return new Point(newCoords,this);
}

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

MultiLineString*
GeometryFactory::createMultiLineString() const
{
	return new MultiLineString(NULL,this);
}

MultiLineString*
GeometryFactory::createMultiLineString(vector<Geometry *> *newLines)
	const
{
	return new MultiLineString(newLines,this);
}

MultiLineString*
GeometryFactory::createMultiLineString(const vector<Geometry *> &fromLines)
	const
{
	vector<Geometry *>*newGeoms = new vector<Geometry *>(fromLines.size());
	for (unsigned int i=0; i<fromLines.size(); i++)
	{
		const LineString *line = dynamic_cast<const LineString *>(fromLines[i]);
		if ( ! line ) throw geos::util::IllegalArgumentException("createMultiLineString called with a vector containing non-LineStrings");
		(*newGeoms)[i] = new LineString(*line);
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

GeometryCollection*
GeometryFactory::createGeometryCollection() const
{
	return new GeometryCollection(NULL,this);
}

Geometry*
GeometryFactory::createEmptyGeometry() const
{
	return new GeometryCollection(NULL,this);
}

GeometryCollection*
GeometryFactory::createGeometryCollection(vector<Geometry *> *newGeoms) const
{
	return new GeometryCollection(newGeoms,this);
}

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

MultiPolygon*
GeometryFactory::createMultiPolygon() const
{
	return new MultiPolygon(NULL,this);
}

MultiPolygon*
GeometryFactory::createMultiPolygon(vector<Geometry *> *newPolys) const
{
	return new MultiPolygon(newPolys,this);
}

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


LinearRing*
GeometryFactory::createLinearRing() const
{
	return new LinearRing(NULL,this);
}

LinearRing*
GeometryFactory::createLinearRing(CoordinateSequence* newCoords) const
{
	return new LinearRing(newCoords,this);
}

LinearRing*
GeometryFactory::createLinearRing(const CoordinateSequence& fromCoords) const
{
	CoordinateSequence *newCoords = fromCoords.clone();
	LinearRing *g = NULL;
	// construction failure will delete newCoords
	g = new LinearRing(newCoords, this);
	return g;
}

MultiPoint*
GeometryFactory::createMultiPoint(vector<Geometry *> *newPoints) const
{
	return new MultiPoint(newPoints,this);
}

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

MultiPoint*
GeometryFactory::createMultiPoint() const
{
	return new MultiPoint(NULL, this);
}

MultiPoint*
GeometryFactory::createMultiPoint(const CoordinateSequence &fromCoords) const
{
	unsigned int npts=fromCoords.getSize();
	vector<Geometry *> *pts=new vector<Geometry *>;
	pts->reserve(npts);
	for (unsigned int i=0; i<npts; ++i) {
		Point *pt=createPoint(fromCoords.getAt(i));
		pts->push_back(pt);
	}
	MultiPoint *mp = NULL;
	try {
		mp = createMultiPoint(pts);
	} catch (...) {
		for (unsigned int i=0; i<npts; ++i) delete (*pts)[i];
		delete pts;
		throw;
	}
	return mp;
}

Polygon*
GeometryFactory::createPolygon() const
{
	return new Polygon(NULL, NULL, this);
}

Polygon*
GeometryFactory::createPolygon(LinearRing *shell, vector<Geometry *> *holes)
	const
{
	return new Polygon(shell, holes, this);
}

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

LineString *
GeometryFactory::createLineString() const
{
	return new LineString(NULL, this);
}

LineString*
GeometryFactory::createLineString(CoordinateSequence *newCoords)
	const
{
	return new LineString(newCoords, this);
}

LineString*
GeometryFactory::createLineString(const CoordinateSequence &fromCoords)
	const
{
	CoordinateSequence *newCoords = fromCoords.clone();
	LineString *g = NULL;
	// construction failure will delete newCoords
	g = new LineString(newCoords, this);
	return g;
}

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
		} else {
			return createGeometryCollection(newGeoms);
		}
	}

	// since this is not a collection we can delete vector
	delete newGeoms;
	return geom0;
}

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
		assert(0); // buildGeomtry encountered an unkwnon geometry type
	}

	return geom0->clone();
}


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

void
GeometryFactory::destroyGeometry(Geometry *g) const
{
	delete g;
}

} // namespace geos::geom
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.62  2006/03/20 10:11:50  strk
 * Bug #67 - Debugging helpers in GeometryFactory class
 *
 * Revision 1.61  2006/03/09 16:46:47  strk
 * geos::geom namespace definition, first pass at headers split
 *
 * Revision 1.60  2006/03/06 19:40:46  strk
 * geos::util namespace. New GeometryCollection::iterator interface, many cleanups.
 *
 * Revision 1.59  2006/03/03 10:46:21  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.58  2006/03/01 18:37:08  strk
 * Geometry::createPointFromInternalCoord dropped (it's a duplication of GeometryFactory::createPointFromInternalCoord).
 * Fixed bugs in InteriorPoint* and getCentroid() inserted by previous commits.
 *
 * Revision 1.57  2006/02/23 23:17:52  strk
 * - Coordinate::nullCoordinate made private
 * - Simplified Coordinate inline definitions
 * - LMGeometryComponentFilter definition moved to LineMerger.cpp file
 * - Misc cleanups
 *
 * Revision 1.56  2006/02/09 15:52:47  strk
 * GEOSException derived from std::exception; always thrown and cought by const ref.
 *
 * Revision 1.55  2006/01/31 19:07:33  strk
 * - Renamed DefaultCoordinateSequence to CoordinateArraySequence.
 * - Moved GetNumGeometries() and GetGeometryN() interfaces
 *   from GeometryCollection to Geometry class.
 * - Added getAt(int pos, Coordinate &to) funtion to CoordinateSequence class.
 * - Reworked automake scripts to produce a static lib for each subdir and
 *   then link all subsystem's libs togheter
 * - Moved C-API in it's own top-level dir capi/
 * - Moved source/bigtest and source/test to tests/bigtest and test/xmltester
 * - Fixed PointLocator handling of LinearRings
 * - Changed CoordinateArrayFilter to reduce memory copies
 * - Changed UniqueCoordinateArrayFilter to reduce memory copies
 * - Added CGAlgorithms::isPointInRing() version working with
 *   Coordinate::ConstVect type (faster!)
 * - Ported JTS-1.7 version of ConvexHull with big attention to
 *   memory usage optimizations.
 * - Improved XMLTester output and user interface
 * - geos::geom::util namespace used for geom/util stuff
 * - Improved memory use in geos::geom::util::PolygonExtractor
 * - New ShortCircuitedGeometryVisitor class
 * - New operation/predicate package
 *
 * Revision 1.54  2005/11/24 23:09:15  strk
 * CoordinateSequence indexes switched from int to the more
 * the correct unsigned int. Optimizations here and there
 * to avoid calling getSize() in loops.
 * Update of all callers is not complete yet.
 *
 * Revision 1.53  2005/06/22 00:46:38  strk
 * Fixed bugus handling of collections in ::buildGeometry
 *
 * Revision 1.52  2005/06/17 14:58:51  strk
 * Fixed segfault in LinearRing and LineString constructors
 *
 * Revision 1.51  2004/12/30 10:14:09  strk
 * input checking and class promoting in createMultiLineString()
 *
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
 * Added CoordinateArraySequenceFactory::instance() function.
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

