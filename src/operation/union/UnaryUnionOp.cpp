/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2011 Sandro Santilli <strk@keybit.net
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: operation/union/UnaryUnionOp.java r320 (JTS-1.12)
 *
 **********************************************************************/

#include <memory> // for auto_ptr
#include <cassert> // for assert
#include <algorithm> // for copy

#include <geos/operation/union/UnaryUnionOp.h> 
#include <geos/operation/union/CascadedUnion.h> 
#include <geos/operation/union/CascadedPolygonUnion.h> 
#include <geos/operation/union/PointGeometryUnion.h> 
#include <geos/geom/Coordinate.h> 
#include <geos/geom/Puntal.h> 
#include <geos/geom/Point.h>
#include <geos/geom/MultiPoint.h>
#include <geos/geom/MultiLineString.h>
#include <geos/geom/MultiPolygon.h>
#include <geos/geom/GeometryCollection.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/Location.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/util/GeometryCombiner.h>
#include <geos/algorithm/PointLocator.h>

namespace geos {
namespace operation { // geos::operation
namespace geounion {  // geos::operation::geounion

/*private*/
std::auto_ptr<geom::Geometry>
UnaryUnionOp::unionWithNull(std::auto_ptr<geom::Geometry> g0,
                            std::auto_ptr<geom::Geometry> g1)
{
  std::auto_ptr<geom::Geometry> ret;
  if ( ( ! g0.get() ) && ( ! g1.get() ) ) return ret;

  if ( ! g0.get() ) return g1;
  if ( ! g1.get() ) return g0;

  ret.reset( g0->Union(g1.get()) );
  return ret;
}

/*public*/
std::auto_ptr<geom::Geometry>
UnaryUnionOp::Union()
{
  using geom::Puntal;
  typedef std::auto_ptr<geom::Geometry> GeomAutoPtr;

  GeomAutoPtr ret;
  if ( ! geomFact ) return ret;

  /**
   * For points and lines, only a single union operation is
   * required, since the OGC model allowings self-intersecting
   * MultiPoint and MultiLineStrings.
   * This is not the case for polygons, so Cascaded Union is required.
   */

  GeomAutoPtr unionPoints;
  if (!points.empty()) {
      GeomAutoPtr ptGeom = geomFact->buildGeometry( points.begin(),
                                                    points.end()    );
      unionPoints = unionNoOpt(*ptGeom);
  }

  GeomAutoPtr unionLines;
  if (!lines.empty()) {
      /* JTS compatibility NOTE:
       * we use cascaded here for robustness [1]
       * but also add a final unionNoOpt step to deal with
       * self-intersecting lines [2]
       *
       * [1](http://trac.osgeo.org/geos/ticket/392
       * [2](http://trac.osgeo.org/geos/ticket/482
       *
       */
      unionLines.reset( CascadedUnion::Union( lines.begin(),
                                              lines.end()   ) );
      unionLines = unionNoOpt(*unionLines);
  }

  GeomAutoPtr unionPolygons;
  if (!polygons.empty()) {
      unionPolygons.reset( CascadedPolygonUnion::Union( polygons.begin(),
                                                        polygons.end()   ) );
  }

  /**
   * Performing two unions is somewhat inefficient,
   * but is mitigated by unioning lines and points first
   */

  GeomAutoPtr unionLA = unionWithNull(unionLines, unionPolygons);
  assert(!unionLines.get()); assert(!unionPolygons.get());

  if ( ! unionPoints.get() ) {
    ret = unionLA;
    assert(!unionLA.get()); 
  }
  else if ( ! unionLA.get() ) {
    ret = unionPoints;
    assert(!unionPoints.get()); 
  }
  else {
    Puntal& up = dynamic_cast<Puntal&>(*unionPoints);
    ret = PointGeometryUnion::Union(up, *unionLA);
  }

  if ( ! ret.get() ) {
          ret.reset( geomFact->createGeometryCollection() );
  }

  return ret;

}

} // namespace geos::operation::union
} // namespace geos::operation
} // namespace geos
