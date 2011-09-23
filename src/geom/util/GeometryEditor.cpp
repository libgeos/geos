/**********************************************************************
 * $Id: GeometryEditor.cpp 3182 2011-02-04 10:12:57Z strk $
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2011 Sandro Santilli <strk@keybit.net>
 * Copyright (C) 2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: geom/util/GeometryEditor.java r320 (JTS-1.12)
 *
 **********************************************************************/

#include <geos/geom/util/GeometryEditor.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/MultiPoint.h>
#include <geos/geom/MultiPolygon.h>
#include <geos/geom/MultiLineString.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/Point.h>
#include <geos/geom/LineString.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/GeometryCollection.h>
#include <geos/geom/util/GeometryEditorOperation.h>
#include <geos/util/UnsupportedOperationException.h>

#include <vector>
#include <cassert>
#include <typeinfo>

using namespace std;

namespace geos {
namespace geom { // geos.geom
namespace util { // geos.geom.util

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
 * Clients will create subclasses of GeometryEditorOperation or
 * CoordinateOperation to perform required modifications.
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

  if ( const GeometryCollection *gc =
            dynamic_cast<const GeometryCollection*>(geometry) )
  {
		return editGeometryCollection(gc, operation);
  }

  if ( const Polygon *p = dynamic_cast<const Polygon*>(geometry) )
  {
		return editPolygon(p, operation);
  }

  if ( dynamic_cast<const Point*>(geometry) )
  {
		return operation->edit(geometry, factory);
  }

  if ( dynamic_cast<const LineString*>(geometry) )
  {
		return operation->edit(geometry, factory);
  }

    // Unsupported Geometry classes should be caught in the GeometryEditorOperation.
    assert(!"SHOULD NEVER GET HERE");
    return NULL;
}

Polygon*
GeometryEditor::editPolygon(const Polygon *polygon,GeometryEditorOperation *operation)
{
	Polygon* newPolygon= dynamic_cast<Polygon*>(
    operation->edit(polygon, factory)
  );
	if (newPolygon->isEmpty()) {
		//RemoveSelectedPlugIn relies on this behaviour. [Jon Aquino]
		return newPolygon;
	}

	Geometry* editResult = edit(newPolygon->getExteriorRing(),operation);

	LinearRing* shell = dynamic_cast<LinearRing*>(editResult);
	if (shell->isEmpty()) {
		//RemoveSelectedPlugIn relies on this behaviour. [Jon Aquino]
		delete shell;
		delete newPolygon;
		return factory->createPolygon(NULL,NULL);
	}

	vector<Geometry*> *holes=new vector<Geometry*>;
	for (size_t i=0, n=newPolygon->getNumInteriorRing(); i<n; ++i)
	{

		Geometry *hole_geom = edit(newPolygon->getInteriorRingN(i),
			operation);

		LinearRing *hole = dynamic_cast<LinearRing*>(hole_geom);
		assert(hole);

		if (hole->isEmpty())
		{
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
	GeometryCollection *newCollection = dynamic_cast<GeometryCollection*>( operation->edit(collection,factory) );
	vector<Geometry*> *geometries = new vector<Geometry*>();
	for (unsigned int i=0, n=newCollection->getNumGeometries(); i<n; i++)
	{
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

} // namespace geos.geom.util
} // namespace geos.geom
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.18  2006/06/08 17:58:57  strk
 * Polygon::getNumInteriorRing() return size_t, Polygon::interiorRingN() takes size_t.
 *
 * Revision 1.17  2006/04/07 09:54:30  strk
 * Geometry::getNumGeometries() changed to return 'unsigned int'
 * rather then 'int'
 *
 * Revision 1.16  2006/03/09 16:46:47  strk
 * geos::geom namespace definition, first pass at headers split
 *
 * Revision 1.15  2006/03/06 19:40:46  strk
 * geos::util namespace. New GeometryCollection::iterator interface, many cleanups.
 *
 * Revision 1.14  2006/03/03 10:46:21  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.13  2006/01/31 19:07:34  strk
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
 * Revision 1.12  2004/12/22 09:42:45  strk
 * Fixed a premature Geometry destruction, Avoided CoordinateSequence copies
 * when possible.
 *
 * Revision 1.11  2004/12/08 13:54:43  strk
 * gcc warnings checked and fixed, general cleanups.
 *
 * Revision 1.10  2004/07/08 19:34:49  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added CoordinateArraySequenceFactory::instance() function.
 *
 * Revision 1.9  2004/07/02 13:28:26  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.8  2004/07/01 14:12:44  strk
 *
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

