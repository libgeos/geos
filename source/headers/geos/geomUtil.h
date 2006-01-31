/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#ifndef GEOS_GEOMUTIL_H
#define GEOS_GEOMUTIL_H

#include <geos/geom.h>
#include <geos/platform.h>
#include <vector>

using namespace std;

namespace geos {
namespace geom { // geos.geom
namespace util { // geos.geom.util

/**
 * Extracts all the 2-dimensional (Polygon) components from a Geometry.
 */
class PolygonExtracter: public GeometryFilter {

private:

	/// Reference to provided vector
	Polygon::ConstVect& comps;

public:

	/**
	 * Pushes the Polygon components from a single geometry into
	 * the provided vector.
	 * If more than one geometry is to be processed, it is more
	 * efficient to create a single PolygonExtracterFilter instance
	 * and pass it to multiple geometries.
	 */
	static void getPolygons(const Geometry &geom, Polygon::ConstVect& ret)
	{
		PolygonExtracter pe(ret);
		geom.apply_ro(&pe);
	}

	/**
	 * Constructs a PolygonExtracterFilter with a list in which
	 * to store Polygons found.
	 */
	PolygonExtracter(Polygon::ConstVect& newComps)
		:
		comps(newComps)
		{}

	void filter_rw(Geometry *geom) {
		if ( const Polygon *p=dynamic_cast<const Polygon *>(geom) )
		{
			comps.push_back(p);
		}
	}

	void filter_ro(const Geometry *geom)
	{
		if ( const Polygon *p=dynamic_cast<const Polygon *>(geom) )
		{
			comps.push_back(p);
		}
	}

};

/**
 * Extracts all the 0-dimensional (Point) components from a Geometry.
 */
class PointExtracter: public GeometryFilter {

private:
	Point::ConstVect& comps;

public:
	/**
	 * Returns the Point components from a single geometry.
	 * If more than one geometry is to be processed, it is more
	 * efficient to create a single {@link PointExtracterFilter} instance
	 * and pass it to multiple geometries.
	 */
	static void getPoints(const Geometry &geom, Point::ConstVect &ret)
	{
		PointExtracter pe(ret);
		geom.apply_ro(&pe);
	}

	/**
	 * Constructs a PointExtracterFilter with a list in which
	 * to store Points found.
	 */
	PointExtracter(Point::ConstVect& newComps)
		:
		comps(newComps)
		{}

	void filter_rw(Geometry *geom)
	{
if ( const Point *p=dynamic_cast<const Point *>(geom) )
		comps.push_back(p);
	}

	void filter_ro(const Geometry *geom)
	{
if ( const Point *p=dynamic_cast<const Point *>(geom) )
		comps.push_back(p);
	}

};

/**
 * Extracts all the 1-dimensional (LineString) components from a Geometry.
 */
class LinearComponentExtracter: public GeometryComponentFilter {

private:

	LineString::ConstVect &comps;

public:
	/**
	 * Push the linear components from a single geometry into
	 * the provided vector.
	 * If more than one geometry is to be processed, it is more
	 * efficient to create a single LinearComponentExtracterFilter instance
	 * and pass it to multiple geometries.
	 */
	static void getLines(const Geometry &geom, LineString::ConstVect &ret)
	{
		LinearComponentExtracter lce(ret);
		geom.apply_ro(&lce);
	}

	/**
	 * Constructs a LinearComponentExtracterFilter with a list in which
	 * to store LineStrings found.
	 */
	LinearComponentExtracter(LineString::ConstVect &newComps)
		:
		comps(newComps)
		{}

	void filter_rw(Geometry *geom)
	{
if ( const LineString *ls=dynamic_cast<const LineString *>(geom) )
		comps.push_back(ls);
	}

	void filter_ro(const Geometry *geom)
	{
if ( const LineString *ls=dynamic_cast<const LineString *>(geom) )
		comps.push_back(ls);
	}

};

/**
 * A interface which specifies an edit operation for Geometries.
 */
class GeometryEditorOperation {

public:
	/**
	 * Edits a Geometry by returning a new Geometry with a modification.
	 * The returned Geometry might be the same as the Geometry passed in.
	 *
	 * @param geometry the Geometry to modify
	 *
	 * @param factory the factory with which to construct the modified
	 *                Geometry (may be different to the factory of the
	 *                input geometry)
	 *
	 * @return a new Geometry which is a modification of the input Geometry
	 */
	virtual Geometry* edit(const Geometry *geometry,
			const GeometryFactory *factory)=0;
};

/**
 * A GeometryEditorOperation which modifies the coordinate list of a
 * Geometry.
 * Operates on Geometry subclasses which contains a single coordinate list.
 */
class CoordinateOperation: public GeometryEditorOperation {

public:

	virtual Geometry* edit(const Geometry *geometry,
			const GeometryFactory *factory);

	/**
	 * Edits the array of Coordinate from a Geometry.
	 *
	 * @param coordinates the coordinate array to operate on
	 * @param geometry the geometry containing the coordinate list
	 * @return an edited coordinate array (which may be the same as
	 *         the input)
	 */
	virtual CoordinateSequence* edit(const CoordinateSequence* coordinates,
			const Geometry *geometry)=0;
};

class gfCoordinateOperation: public CoordinateOperation {

using CoordinateOperation::edit;

public:

	virtual CoordinateSequence* edit(const CoordinateSequence *coordinates,
			const Geometry *geometry);
};


/**
 * Supports creating a new Geometry which is a modification of an existing one.
 * Geometry objects are intended to be treated as immutable.
 * This class allows you to "modify" a Geometry
 * by traversing it and creating a new Geometry with the same overall
 * structure but possibly modified components.
 *
 * The following kinds of modifications can be made:
 *
 * - the values of the coordinates may be changed.
 *   Changing coordinate values may make the result Geometry invalid;
 *   this is not checked by the GeometryEditor
 * - the coordinate lists may be changed
 *   (e.g. by adding or deleting coordinates).
 *   The modifed coordinate lists must be consistent with their original
 *   parent component
 *   (e.g. a LinearRing must always have at least 4 coordinates, and the
 *   first and last coordinate must be equal)
 * - components of the original geometry may be deleted
 *   (e.g. holes may be removed from a Polygon, or LineStrings removed
 *   from a MultiLineString). Deletions will be propagated up the component
 *   tree appropriately.
 * 
 * Note that all changes must be consistent with the original Geometry's
 * structure
 * (e.g. a Polygon cannot be collapsed into a LineString).
 *
 * The resulting Geometry is not checked for validity.
 * If validity needs to be enforced, the new Geometry's isValid should
 * be checked.
 *
 * @see Geometry::isValid
 *
 */
class GeometryEditor {
private:
	/**
	 * The factory used to create the modified Geometry
	 */
	const GeometryFactory* factory;

	Polygon* editPolygon(const Polygon *polygon,
			GeometryEditorOperation *operation);

	GeometryCollection* editGeometryCollection(
			const GeometryCollection *collection,
			GeometryEditorOperation *operation);

public:

	/**
	 * Creates a new GeometryEditor object which will create
	 * an edited Geometry with the same GeometryFactory as the
	 * input Geometry.
	 */
	GeometryEditor();

	/**
	 * Creates a new GeometryEditor object which will create
	 * the edited Geometry with the given GeometryFactory
	 *
	 * @param factory the GeometryFactory to create the edited
	 * Geometry with
	 */
	GeometryEditor(const GeometryFactory *newFactory);

	/**
	 * Edit the input Geometry with the given edit operation.
	 * Clients will create subclasses of GeometryEditorOperation or
	 * CoordinateOperation to perform required modifications.
	 *
	 * @param geometry the Geometry to edit
	 * @param operation the edit operation to carry out
	 * @return a new Geometry which is the result of the editing
	 */
	Geometry* edit(const Geometry *geometry,
			GeometryEditorOperation *operation);
};

/**
 * A visitor to Geometry elements which can
 * be short-circuited by a given condition
 *
 * Last port: geom/util/ShortCircuitedGeometryVisitor.java rev. 1.1 (JTS-1.7)
 */
class ShortCircuitedGeometryVisitor
{

private:

	bool done;

protected:

	virtual void visit(const Geometry &element)=0;
	virtual bool isDone()=0;

public:

	ShortCircuitedGeometryVisitor()
		:
		done(false)
		{}

	void applyTo(const Geometry &geom);

};

} // namespace geos.geom.util
} // namespace geos.geom
} // namespace geos

#endif

/**********************************************************************
 * $Log$
 * Revision 1.5  2006/01/31 19:07:34  strk
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
 * Revision 1.4  2005/11/10 15:20:32  strk
 * Made virtual overloads explicit.
 *
 * Revision 1.3  2004/07/19 13:19:31  strk
 * Documentation fixes
 *
 * Revision 1.2  2004/07/08 19:34:49  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added DefaultCoordinateSequenceFactory::instance() function.
 *
 * Revision 1.1  2004/07/02 13:20:42  strk
 * Header files moved under geos/ dir.
 *
 **********************************************************************/
