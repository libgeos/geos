/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2012 Excensus LLC.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: triangulate/DelaunayTriangulationBuilder.java r524
 *
 **********************************************************************/

#ifndef GEOS_TRIANGULATE_VORONOIDIAGRAMBUILDER_H
#define GEOS_TRIANGULATE_VORONOIDIAGRAMBUILDER_H

#include <geos/triangulate/VoronoiDiagramBuilder.h>

#include <memory>

namespace geos {
  namespace geom{
	  class CoordinateSequence;
	  class Geometry;
	  class MultiLineString;
	  class GeometryCollection;
	  class GeometryFactory;
	  class CoordinateSequence;
	  class Envelope;
  }
  namespace triangulate { 
    namespace quadedge { 
      class QuadEdgeSubdivision;
    }
  }
}

namespace geos {
namespace triangulate { //geos.triangulate


class VoronoiDiagramBuilder
{

public:
   VoronoiDiagramBuilder();

   ~VoronoiDiagramBuilder();

   void setSites(const geom::Geometry& geom);
   
   void setSites(const geom::CoordinateSequence& coords);

   void setClipEnvelope(const geom::Envelope& clpEnv);

   void setTolerance(const double toler);


private:
   geom::CoordinateSequence* siteCoords;
   double tolerance;
   quadedge::QuadEdgeSubdivision* subdiv;
   geom::Envelope* clipEnv;
   geom::Envelope* diagramEnv;

private:
   void create();

   static std::auto_ptr<geom::GeometryCollection> clipGeometryCollection(const geom::GeometryCollection& geom, const geom::Envelope& clipEnv);

public:
   quadedge::QuadEdgeSubdivision* getSubdivision();

   std::auto_ptr<geom::GeometryCollection> getDiagram(const geom::GeometryFactory& geomFact);
};

} //namespace geos.triangulate
} //namespace goes

#endif //GEOS_TRIANGULATE_VORONOIDIAGRAMBUILDER_H
