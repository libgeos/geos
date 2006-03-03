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

#include <geos/geom.h>
#include <string>
#include <vector>

using namespace std;

namespace geos {

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
MultiPolygon::MultiPolygon(vector<Geometry *> *newPolys, const GeometryFactory *factory)
	: GeometryCollection(newPolys,factory)
{}

MultiPolygon::~MultiPolygon(){}

int MultiPolygon::getDimension() const {
	return 2;
}

int MultiPolygon::getBoundaryDimension() const {
	return 1;
}

string MultiPolygon::getGeometryType() const {
	return "MultiPolygon";
}

bool MultiPolygon::isSimple() const {
	return true;
}

Geometry* MultiPolygon::getBoundary() const {
	if (isEmpty()) {
		return getFactory()->createGeometryCollection(NULL);
	}
	vector<Geometry *>* allRings=new vector<Geometry *>();
	for (unsigned int i = 0; i < geometries->size(); i++) {
		Polygon *pg=(Polygon *) (*geometries)[i];
		Geometry *g=pg->getBoundary();
		LineString *ls=dynamic_cast<LineString *>(g);
		if ( ls )
		{
			allRings->push_back(ls);
		}
		else
		{
			GeometryCollection* rings=(GeometryCollection*)g;
			for (int j = 0; j < rings->getNumGeometries(); j++)
			{
				allRings->push_back(new LineString(*(LineString*)rings->getGeometryN(j)));
			}
			delete g;
		}
	}

	Geometry *ret=getFactory()->createMultiLineString(allRings);
	//for (int i=0; i<allRings->size(); i++) delete (*allRings)[i];
	//delete allRings;
	return ret;
}

bool
MultiPolygon::equalsExact(const Geometry *other, double tolerance) const
{
    if (!isEquivalentClass(other)) {
      return false;
    }
	return GeometryCollection::equalsExact(other, tolerance);
}
GeometryTypeId
MultiPolygon::getGeometryTypeId() const {
	return GEOS_MULTIPOLYGON;
}

} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.25  2006/03/03 10:46:21  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.24  2006/01/31 19:07:33  strk
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
 * Revision 1.23  2004/12/08 14:33:20  strk
 * Checked inner polys getBoundary return for the single LineString case.
 *
 * Revision 1.22  2004/07/08 19:34:49  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added CoordinateArraySequenceFactory::instance() function.
 *
 * Revision 1.21  2004/07/06 17:58:22  strk
 * Removed deprecated Geometry constructors based on PrecisionModel and
 * SRID specification. Removed SimpleGeometryPrecisionReducer capability
 * of changing Geometry's factory. Reverted Geometry::factory member
 * to be a reference to external factory.
 *
 * Revision 1.20  2004/07/05 10:50:20  strk
 * deep-dopy construction taken out of Geometry and implemented only
 * in GeometryFactory.
 * Deep-copy geometry construction takes care of cleaning up copies
 * on exception.
 * Implemented clone() method for CoordinateSequence
 * Changed createMultiPoint(CoordinateSequence) signature to reflect
 * copy semantic (by-ref instead of by-pointer).
 * Cleaned up documentation.
 *
 * Revision 1.19  2004/07/02 13:28:26  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 **********************************************************************/

