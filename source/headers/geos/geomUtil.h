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
 * Revision 1.4  2004/05/14 14:45:28  strk
 * Fixed bogus inheritance of LinearComponentExtracter
 *
 * Revision 1.3  2004/05/14 13:42:46  strk
 * DistanceOp bug removed, cascading errors fixed.
 *
 * Revision 1.2  2004/04/20 08:52:01  strk
 * GeometryFactory and Geometry const correctness.
 * Memory leaks removed from SimpleGeometryPrecisionReducer
 * and GeometryFactory.
 *
 * Revision 1.1  2004/04/04 06:29:11  ybychkov
 * "planargraph" and "geom/utill" upgraded to JTS 1.4
 *
 *
 */
#ifndef GEOS_GEOMUTIL_H
#define GEOS_GEOMUTIL_H

#include <geos/geom.h>
#include <geos/platform.h>
#include <vector>

using namespace std;

namespace geos {

/*
 * Extracts all the 2-dimensional (Polygon) components from a Geometry.
 */
class PolygonExtracter: public GeometryFilter {
public:
	/**
	* Returns the Polygon components from a single geometry.
	* If more than one geometry is to be processed, it is more
	* efficient to create a single {@link PolygonExtracterFilter} instance
	* and pass it to multiple geometries.
	*/
	static vector<Geometry*>* getPolygons(const Geometry *geom);
	/**
	* Constructs a PolygonExtracterFilter with a list in which to store Polygons found.
	*/
	PolygonExtracter(vector<Geometry*>* newComps);
	void filter_rw(Geometry *geom);
	void filter_ro(const Geometry *geom);
private:
	vector<Geometry*>* comps;
};

/*
 * Extracts all the 0-dimensional (Point) components from a Geometry.
 */
class PointExtracter: public GeometryFilter {
public:
	/**
	* Returns the Point components from a single geometry.
	* If more than one geometry is to be processed, it is more
	* efficient to create a single {@link PointExtracterFilter} instance
	* and pass it to multiple geometries.
	*/
	static vector<Geometry*>* getPoints(const Geometry *geom);
	/**
	* Constructs a PointExtracterFilter with a list in which to store Points found.
	*/
	PointExtracter(vector<Geometry*>* newComps);
	void filter_rw(Geometry *geom);
	void filter_ro(const Geometry *geom);
private:
	vector<Geometry*>* comps;
};

/*
 * Extracts all the 1-dimensional (LineString) components from a Geometry.
 */
class LinearComponentExtracter: public GeometryComponentFilter {
public:
	/**
	* Returns the linear components from a single geometry.
	* If more than one geometry is to be processed, it is more
	* efficient to create a single {@link LinearComponentExtracterFilter} instance
	* and pass it to multiple geometries.
	*/
	static vector<Geometry*>* getLines(const Geometry *geom);
	/**
	* Constructs a LinearComponentExtracterFilter with a list in which to store LineStrings found.
	*/
	LinearComponentExtracter(vector<Geometry*>* newComps);
	void filter_rw(Geometry *geom);
	void filter_ro(const Geometry *geom);
private:
	vector<Geometry*>* comps;
};

/*
 * A interface which specifies an edit operation for Geometries.
 */
class GeometryEditorOperation {
public:
	/**
	* Edits a Geometry by returning a new Geometry with a modification.
	* The returned Geometry might be the same as the Geometry passed in.
	*
	* @param geometry the Geometry to modify
	* @param factory the factory with which to construct the modified Geometry
	* (may be different to the factory of the input geometry)
	* @return a new Geometry which is a modification of the input Geometry
	*/
	virtual Geometry* edit(const Geometry *geometry, const GeometryFactory *factory)=0;
};

/*
 * A GeometryEditorOperation which modifies the coordinate list of a
 * Geometry.
 * Operates on Geometry subclasses which contains a single coordinate list.
 */
class CoordinateOperation: public GeometryEditorOperation {
public:
	virtual Geometry* edit(const Geometry *geometry, const GeometryFactory *factory);
	/**
	* Edits the array of {@link Coordinate}s from a {@link Geometry}.
	*
	* @param coordinates the coordinate array to operate on
	* @param geometry the geometry containing the coordinate list
	* @return an edited coordinate array (which may be the same as the input)
	*/
	virtual CoordinateSequence* edit(const CoordinateSequence* coordinates, const Geometry *geometry)=0;
};

class gfCoordinateOperation: public CoordinateOperation {
public:
	virtual CoordinateSequence* edit(const CoordinateSequence *coordinates, const Geometry *geometry);
};


/*
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
 * The modifed coordinate lists must be consistent with their original parent component
 * (e.g. a LinearRing must always have at least 4 coordinates, and the first and last
 * coordinate must be equal)
 * <li>components of the original geometry may be deleted
 * (e.g. holes may be removed from a Polygon, or LineStrings removed from a MultiLineString).
 * Deletions will be propagated up the component tree appropriately.
 * </ul>
 * Note that all changes must be consistent with the original Geometry's structure
 * (e.g. a Polygon cannot be collapsed into a LineString).
 * <p>
 * The resulting Geometry is not checked for validity.
 * If validity needs to be enforced, the new Geometry's #isValid should be checked.
 *
 * @see Geometry#isValid
 *
 */
class GeometryEditor {
private:
	/**
	* The factory used to create the modified Geometry
	*/
	const GeometryFactory* factory;
	Polygon* editPolygon(const Polygon *polygon, GeometryEditorOperation *operation);
	GeometryCollection* editGeometryCollection(const GeometryCollection *collection,GeometryEditorOperation *operation);
public:
	/**
	* Creates a new GeometryEditor object which will create
	* an edited {@link Geometry} with the same {@link GeometryFactory} as the input Geometry.
	*/
	GeometryEditor();
	/**
	* Creates a new GeometryEditor object which will create
	* the edited Geometry with the given {@link GeometryFactory}
	*
	* @param factory the GeometryFactory to create the edited Geometry with
	*/
	GeometryEditor(const GeometryFactory *newFactory);
	/**
	* Edit the input {@link Geometry} with the given edit operation.
	* Clients will create subclasses of {@link GeometryEditorOperation} or
	* {@link CoordinateOperation} to perform required modifications.
	*
	* @param geometry the Geometry to edit
	* @param operation the edit operation to carry out
	* @return a new {@link Geometry} which is the result of the editing
	*/
	Geometry* edit(const Geometry *geometry, GeometryEditorOperation *operation);
};


}
#endif
