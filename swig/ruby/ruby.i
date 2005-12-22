/* =========================================================================
 * $Id$
 *
 * ruby.i
 * 
 * Copyright 2005 Charlie Savage, cfis@interserv.com
 *
 * Interface for a SWIG generated geos module.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 * ========================================================================= */

/* Include renames for methods */
%include renames.i

/* Next conflicts with a Ruby keyword */
%rename(next_) geos::GeometryCollectionIterator::next();

/* Need to deal with these ignores by renaming them */
%rename("build_geometry!") geos::GeometryFactory::buildGeometry(vector<Geometry * > *) const;
%rename("create_geometry_collection!") geos::GeometryFactory::createGeometryCollection(vector<Geometry * > *) const;
%rename("create_linear_ring!") geos::GeometryFactory::createLinearRing(CoordinateSequence *) const;
%rename("create_line_string!") geos::GeometryFactory::createLineString(CoordinateSequence *) const;
%rename("create_multi_line_string!") geos::GeometryFactory::createMultiLineString(vector<Geometry * > *) const;
%rename("create_multi_point!") geos::GeometryFactory::createMultiPoint(vector<Geometry * > *) const;
%rename("create_multi_polygon!") geos::GeometryFactory::createMultiPolygon(vector<Geometry * > *) const;
%rename("create_point!") geos::GeometryFactory::createPoint(CoordinateSequence *) const;
%rename("create_polygon!") geos::GeometryFactory::createPolygon(LinearRing *,vector<Geometry * > *) const;

/* All these classes do not follow the Ruby naming convention */
%rename("PlanarGraphComponent") geos::planarGraphComponent;
%rename("PlanarDirectedEdge") geos::planarDirectedEdge;
%rename("PlanarDirectedEdgeStar") geos::planarDirectedEdgeStar;
%rename("PlanarNode") geos::planarNode;
%rename("PlanarEdge") geos::planarEdge;
%rename("PlanarCoordLT") geos::planarCoordLT;
%rename("PlanarNodeMap") geos::planarNodeMap;
%rename("PlanarPlanarGraph") geos::planarPlanarGraph;

/* Ruby defines a macro called select in its win32.h header file.  However,
   the object geos::MonotoneChainSelectAction also defines a select method,
   thus causing compilation problems.  So #undef the macro. */
%runtime %{
#undef select
%}

/* Define various template classes */
%template("GeometryVector") std::vector<geos::Geometry *>;
%template("LineStringVector") std::vector<geos::LineString *>;
%template("PolygonVector") std::vector<geos::Polygon *>;
