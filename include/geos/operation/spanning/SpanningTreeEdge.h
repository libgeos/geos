/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2026 Paul Ramsey
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/export.h>
#include <geos/planargraph/Edge.h> // for inheritance
#include <cstddef> // for std::size_t

// Forward declarations
namespace geos {
namespace geom {
class LineString;
}
namespace planargraph {
class DirectedEdge;
}
}

namespace geos {
namespace operation { // geos::operation
namespace spanning { // geos::operation::spanning

/** \brief
 * An edge of a SpanningTreeGraph.
 */
class GEOS_DLL SpanningTreeEdge: public planargraph::Edge {

private:
    const geom::LineString* line;
    std::size_t index;

public:

    /** \brief
     * Constructs a SpanningTreeEdge initialized with the given
     * LineString and its index.
     */
    SpanningTreeEdge(const geom::LineString* newLine, std::size_t newIndex);

    const geom::LineString* getLineString() const;
    
    double getLength() const;
    
    std::size_t getIndex() const;
};

} // namespace geos::operation::spanning
} // namespace geos::operation
} // namespace geos
