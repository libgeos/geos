/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2010      Sandro Santilli <strk@keybit.net>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: original work
 *
 **********************************************************************/

#ifndef GEOS_OPERATION_SHAREDPATHSOP_H
#define GEOS_OPERATION_SHAREDPATHSOP_H

#include <geos/export.h> // for GEOS_DLL

#include <vector> 

// Forward declarations
namespace geos {
  namespace geom {
    class LineString;
    class Geometry;
    class GeometryFactory;
  }
}


namespace geos {
namespace operation { // geos.operation
namespace sharedpaths { // geos.operation.sharedpaths

/** \brief
 * Find shared paths among two linear Geometry objects
 *
 * For each shared path report if it direction is the same
 * or opposite.
 *
 */
class GEOS_DLL SharedPathsOp
{
public:

  /// Find paths shared between two linear geometries
  //
  /// @param g1
  ///   First geometry. Must be linear.
  ///
  /// @param g2
  ///   Second geometry. Must be linear.
  ///
  /// @param tol
  ///   Tolerance by which very close paths are considered shared.
  ///   TODO: specify more about the semantic, check SnapOp 
  ///
  /// @param sameDir
  ///   Shared edges having the same direction are pushed
  ///   onto this vector. They'll be of type LineString.
  ///   Ownership of the edges is tranferred.
  ///
  /// @param oppositeDir
  ///   Shared edges having the opposite direction are pushed
  ///   onto this vector. They'll be of type geom::LineString.
  ///   Ownership of the edges is tranferred.
  ///
  static void sharedPathsOp(const geom::Geometry& g1,
                            const geom::Geometry& g2,
                            double tol,
                            std::vector<geom::Geometry*>& sameDirection,
                            std::vector<geom::Geometry*>& oppositeDirection);

  /// Constructor
  //
  /// @param g1
  ///   First geometry. Must be linear.
  ///
  /// @param g2
  ///   Second geometry. Must be linear.
  ///
  SharedPathsOp(const geom::Geometry& g1, const geom::Geometry& g2);

  /// Get shared paths gith a given tolerance 
  //
  /// @param tol
  ///   Tolerance by which very close paths are considered shared.
  ///   TODO: specify more about the semantic, check SnapOp 
  ///
  /// @param sameDir
  ///   Shared edges having the same direction are pushed
  ///   onto this vector. They'll be of type geom::LineString.
  ///   Ownership of the edges is tranferred.
  ///
  /// @param oppositeDir
  ///   Shared edges having the opposite direction are pushed
  ///   onto this vector. They'll be of type geom::LineString.
  ///   Ownership of the edges is tranferred.
  ///
  void getSharedPaths(double tolerance,
                      std::vector<geom::Geometry*>& sameDirection,
                      std::vector<geom::Geometry*>& oppositeDirection);

private:

  /// LineString vector (list of edges)
  typedef std::vector<geom::LineString*> EdgeList;

  /// Delete all edges in the list
  static void clearEdges(EdgeList& from);

  /// Get all the linear intersections
  //
  /// Ownership of linestring pushed to the given container
  /// is transferred to caller. See clearEdges for a deep
  /// release if you need one.
  ///
  void findLinearIntersections(EdgeList& to);

  /// Check if the given edge goes forward or backward on the given line.
  //
  /// PRECONDITION: It is assumed the edge fully lays on the geometry
  ///
  bool isForward(const geom::LineString& edge,
                 const geom::Geometry& geom);

  // Check if the given edge goes in the same direction over
  // the two geometries.
  bool isSameDirection(const geom::LineString& edge) {
    return (isForward(edge, _g1) == isForward(edge, _g2));
  }

  const geom::Geometry& _g1;
  const geom::Geometry& _g2;
  const geom::GeometryFactory& _gf;

};

} // namespace geos.operation.sharedpaths
} // namespace geos.operation
} // namespace geos

#endif

