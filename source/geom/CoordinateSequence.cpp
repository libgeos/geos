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
 **********************************************************************/

#include <cstdio>
#include <algorithm>
#include <vector>

//#include <geos/geom.h>
#include <geos/profiler.h>
#include <geos/geom/CoordinateSequence.h>
// FIXME: we should probably not be using CoordinateArraySequenceFactory
#include <geos/geom/CoordinateArraySequenceFactory.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/Envelope.h>

using namespace std;

namespace geos {
namespace geom { // geos::geom

#if PROFILE
static Profiler *profiler = Profiler::instance();
#endif

bool
CoordinateSequence::hasRepeatedPoints() const
{
	int size=getSize();
	for(int i=1; i<size; i++) {
		if (getAt(i-1)==getAt(i)) {
			return true;
		}
	}
	return false;
}

/*
 * Returns either the given coordinate array if its length is greater than the
 * given amount, or an empty coordinate array.
 */
CoordinateSequence *
CoordinateSequence::atLeastNCoordinatesOrNothing(unsigned int n, CoordinateSequence *c)
{
	// FIXME: return NULL rather then empty coordinate array
	return c->getSize()>=n?c:CoordinateArraySequenceFactory::instance()->create(NULL);
}      


bool
CoordinateSequence::hasRepeatedPoints(const CoordinateSequence *cl) {
	int size=cl->getSize();
	for(int i=1;i<size; i++) {
		if (cl->getAt(i-1)==cl->getAt(i)) {
			return true;
		}
	}
	return false;
}

const Coordinate* CoordinateSequence::minCoordinate() const {
	const Coordinate* minCoord=NULL;
	int size=getSize();
	for(int i=0; i<size; i++) {
		if(minCoord==NULL || minCoord->compareTo(getAt(i))>0) {
			minCoord=&getAt(i);
		}
	}
	return minCoord;
}

const Coordinate*
CoordinateSequence::minCoordinate(CoordinateSequence *cl)
{
	const Coordinate* minCoord=NULL;
	int size=cl->getSize();
	for(int i=0;i<size; i++) {
		if(minCoord==NULL || minCoord->compareTo(cl->getAt(i))>0) {
			minCoord=&(cl->getAt(i));
		}
	}
	return minCoord;
}

int
CoordinateSequence::indexOf(const Coordinate *coordinate, const CoordinateSequence *cl)
{
	int size=cl->getSize();
	for (int i=0; i<size; i++) {
		if ((*coordinate)==cl->getAt(i)) {
			return i;
		}
	}
	return -1;
}

void
CoordinateSequence::scroll(CoordinateSequence* cl,const Coordinate* firstCoordinate)
{
	int i, j=0;
	int ind=indexOf(firstCoordinate,cl);
	if (ind<1) return; // not found or already first
	int length=cl->getSize();
	vector<Coordinate> v(length);
	for (i=ind; i<length; i++) {
		v[j++]=cl->getAt(i);
	}
	for (i=0; i<ind; i++) {
		v[j++]=cl->getAt(i);
	}
	cl->setPoints(v);
}

void CoordinateSequence::reverse(CoordinateSequence *cl){
	int last=cl->getSize()-1;
	int mid=last/2;
	for(int i=0;i<=mid;i++) {
		const Coordinate tmp=cl->getAt(i);
		cl->setAt(cl->getAt(last-i),i);
		cl->setAt(tmp,last-i);
	}
}

bool CoordinateSequence::equals(CoordinateSequence *cl1, CoordinateSequence *cl2){
	if (cl1==cl2) return true;
	if (cl1==NULL||cl2==NULL) return false;
	unsigned int npts1=cl1->getSize();
	if (npts1!=cl2->getSize()) return false;
	for (unsigned int i=0; i<npts1; i++) {
		if (!(cl1->getAt(i)==cl2->getAt(i))) return false;
	}
	return true;
}

/** Add an array of coordinates
* @param vc The coordinates
* @param allowRepeated if set to false, repeated coordinates are collapsed
* @return true (as by general collection contract)
*/
void CoordinateSequence::add(const vector<Coordinate>* vc,bool allowRepeated) {
	for(int i=0;i<(int)vc->size();i++) {
		add((*vc)[i],allowRepeated);
	}
}

/** Add a coordinate
* @param c The coordinate to add
* @param allowRepeated if set to false, repeated coordinates are collapsed
* @return true (as by general collection contract)
*/
void CoordinateSequence::add(const Coordinate& c,bool allowRepeated) {
	if (!allowRepeated) {
		int npts=getSize();
		if (npts>=1) {
			const Coordinate& last=getAt(npts-1);
			if (last.equals2D(c)) return;
		}
	}
	add(c);
}

/* Here for backward compatibility */
void
CoordinateSequence::add(CoordinateSequence *cl,bool allowRepeated,bool direction)
{
	add((const CoordinateSequence *)cl,allowRepeated,direction);
}

/**
 * Add an array of coordinates
 * @param cl The coordinates
 * @param allowRepeated if set to false, repeated coordinates are collapsed
 * @param direction if false, the array is added in reverse order
 * @return true (as by general collection contract)
 */
void
CoordinateSequence::add(const CoordinateSequence *cl,bool allowRepeated,bool direction)
{
	int npts=cl->getSize();
	if (direction) {
		for (int i=0; i<npts; i++) {
			add(cl->getAt(i), allowRepeated);
		}
	} else {
		for (int j=npts-1; j>=0; j--) {
			add(cl->getAt(j), allowRepeated);
		}
	}
}


/**
 * This function allocates space for a CoordinateSequence object
 * being a copy of the input one with consecutive equal points
 * removed.
 */
CoordinateSequence*
CoordinateSequence::removeRepeatedPoints(const CoordinateSequence *cl)
{
#if PROFILE
	static Profile *prof= profiler->get("CoordinateSequence::removeRepeatedPoints()");
	prof->start();
#endif
	const vector<Coordinate> *v=cl->toVector();

	vector<Coordinate> *nv=new vector<Coordinate>;
	nv->reserve(v->size());
	unique_copy(v->begin(), v->end(), back_inserter(*nv));
	CoordinateSequence* ret=CoordinateArraySequenceFactory::instance()->create(nv);

#if PROFILE
	prof->stop();
#endif
	return ret;
}

void
CoordinateSequence::expandEnvelope(Envelope &env) const
{
	int size = getSize();
	for (int i=0; i<size; i++) env.expandToInclude(getAt(i));
}

} // namespace geos::geom
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.16  2006/03/13 21:54:56  strk
 * Streamlined headers inclusion.
 *
 * Revision 1.15  2006/03/09 16:46:47  strk
 * geos::geom namespace definition, first pass at headers split
 *
 * Revision 1.14  2006/03/03 10:46:21  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.13  2006/01/31 19:07:33  strk
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
 * Revision 1.12  2005/12/07 21:55:44  strk
 * Optimized memory allocations in ::removeRepeatedPoints
 *
 * Revision 1.11  2005/11/29 14:39:08  strk
 * More signed/unsigned fixes
 *
 * Revision 1.10  2005/11/24 23:09:15  strk
 * CoordinateSequence indexes switched from int to the more
 * the correct unsigned int. Optimizations here and there
 * to avoid calling getSize() in loops.
 * Update of all callers is not complete yet.
 *
 * Revision 1.9  2005/04/29 11:52:40  strk
 * Added new JTS interfaces for CoordinateSequence and factories,
 * removed example implementations to reduce maintainance costs.
 * Added first implementation of WKBWriter, made ByteOrderDataInStream
 * a template class.
 *
 * Revision 1.8  2005/02/22 17:10:47  strk
 * Reduced CoordinateSequence::getSize() calls.
 *
 * Revision 1.7  2005/02/03 09:17:07  strk
 * more profiling label
 *
 * Revision 1.6  2004/12/03 22:52:56  strk
 * enforced const return of CoordinateSequence::toVector() method to derivate classes.
 *
 * Revision 1.5  2004/11/04 19:08:06  strk
 * Cleanups, initializers list, profiling.
 *
 * Revision 1.4  2004/10/13 10:03:02  strk
 * Added missing linemerge and polygonize operation.
 * Bug fixes and leaks removal from the newly added modules and
 * planargraph (used by them).
 * Some comments and indentation changes.
 *
 * Revision 1.3  2004/09/23 21:36:22  strk
 * Fixed a bug in ::reverse (thanks to Elliott Edwards)
 *
 * Revision 1.2  2004/07/21 09:55:24  strk
 * CoordinateSequence::atLeastNCoordinatesOrNothing definition fix.
 * Documentation fixes.
 *
 * Revision 1.1  2004/07/08 19:38:56  strk
 * renamed from *List* equivalents
 *
 **********************************************************************/

