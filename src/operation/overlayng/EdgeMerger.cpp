/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2020 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/operation/overlayng/EdgeMerger.h>
#include <geos/geom/Dimension.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/util/Assert.h>


namespace geos {      // geos
namespace operation { // geos.operation
namespace overlayng { // geos.operation.overlayng

EdgeMerger::EdgeMerger(std::vector<Edge*>& p_edges)
    : edges(p_edges) {}

/*public static */
std::vector<Edge*>
EdgeMerger::merge(std::vector<Edge*>& edges)
{
    EdgeMerger merger(edges);
    return merger.merge();
}


/*public static */
std::vector<Edge*>
EdgeMerger::merge()
{
    std::vector<Edge*> mergedEdges;

    for (Edge* edge : edges) {
        EdgeKey edgeKey(edge);
        auto it = edgeMap.find(edgeKey);
        if (it == edgeMap.end()) {
            // this is the first (and maybe only) edge for this line
            edgeMap[edgeKey] = edge;
            //Debug.println("edge added: " + edge);
            //Debug.println(edge.toLineString());
        }
        else {
            // found an existing edge
            Edge *baseEdge = it->second;
            // Assert: edges are identical (up to direction)
            // this is a fast (but incomplete) sanity check
            //
            // NOTE: we throw an exception to avoid crashing processes
            // See https://trac.osgeo.org/geos/ticket/1051#comment:29
            //
            util::Assert::isTrue(
                baseEdge->size() == edge->size(),
                "Merge of edges of different sizes - probable noding error."
            );

            baseEdge->merge(edge);
            //Debug.println("edge merged: " + existing);
            //Debug.println(edge.toLineString());
        }
    }

    // copy map values into return vector
    for (auto it: edgeMap) {
        mergedEdges.push_back(it.second);
    }
    return mergedEdges;
}




} // namespace geos.operation.overlayng
} // namespace geos.operation
} // namespace geos
