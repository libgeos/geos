/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: operation/overlay/MinimalEdgeRing.java rev. 1.13 (JTS-1.10)
 *
 **********************************************************************/

#pragma once


#include <geos/export.h>

#include <geos/geomgraph/EdgeRing.h> // for inheritance
#include <geos/geomgraph/DirectedEdge.h> // for inlines

#include <vector>

// Forward declarations
namespace geos {
namespace geom {
class GeometryFactory;
}
namespace geomgraph {
class DirectedEdge;
class EdgeRing;
}
}

namespace geos {
namespace operation { // geos::operation
namespace buffer { // geos::operation::buffer

/** \brief
 * A ring of [Edges](@ref geomgraph::Edge) with the property that no node
 * has degree greater than 2.
 *
 * These are the form of rings required to represent polygons
 * under the OGC SFS spatial data model.
 *
 * @see operation::buffer::MaximalEdgeRing
 *
 */
class GEOS_DLL MinimalEdgeRing: public geomgraph::EdgeRing {

public:

    MinimalEdgeRing(geomgraph::DirectedEdge* start,
                    const geom::GeometryFactory* geometryFactory);

    ~MinimalEdgeRing() override {};

    geomgraph::DirectedEdge* getNext(geomgraph::DirectedEdge* de) override
    {
        return de->getNextMin();
    };

    void setEdgeRing(geomgraph::DirectedEdge* de,
                     geomgraph::EdgeRing* er) override
    {
        de->setMinEdgeRing(er);
    };

};


} // namespace geos::operation::buffer
} // namespace geos::operation
} // namespace geos

