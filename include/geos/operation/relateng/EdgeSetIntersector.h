/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (c) 2024 Martin Davis
 * Copyright (C) 2024 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once


#include <geos/index/strtree/TemplateSTRtree.h>
#include <geos/index/chain/MonotoneChain.h>
#include <geos/export.h>


// Forward declarations
namespace geos {
namespace geom {
    class Geometry;
    class Envelope;
}
namespace noding {
    class SegmentString;
}
namespace operation {
namespace relateng {
    class RelateSegmentString;
    class EdgeSegmentIntersector;
}
}
}


using geos::geom::Envelope;
using geos::geom::Geometry;
using geos::index::strtree::TemplateSTRtree;
using geos::index::chain::MonotoneChain;
using geos::operation::relateng::EdgeSegmentIntersector;


namespace geos {      // geos.
namespace operation { // geos.operation
namespace relateng { // geos.operation.relateng

class GEOS_DLL EdgeSetIntersector {

private:

    // Members
    TemplateSTRtree<const MonotoneChain*> index;
    // HPRtree index = new HPRtree();
    const Envelope* envelope = nullptr;
    std::deque<MonotoneChain> monoChains;
    int idCounter = 0;


    // Methods

    void addToIndex(const SegmentString* segStr);

    void addEdges(std::vector<const SegmentString*>& segStrings);


public:

    EdgeSetIntersector(
        std::vector<const SegmentString*>& edgesA,
        std::vector<const SegmentString*>& edgesB,
        const Envelope* env)
        : envelope(env)
        , idCounter(0)
        {
            addEdges(edgesA);
            addEdges(edgesB);
            // build index to ensure thread-safety
            // index.build();
        };

    void process(EdgeSegmentIntersector& intersector);


};

} // namespace geos.operation.relateng
} // namespace geos.operation
} // namespace geos

