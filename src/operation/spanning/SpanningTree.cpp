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

#include <geos/operation/spanning/SpanningTree.h>
#include <geos/geom/Curve.h>
#include <geos/geom/Point.h>
#include <geos/geom/Coordinate.h>

#include <vector>
#include <algorithm>
#include <unordered_map>
#include <numeric>

using namespace geos::geom;

namespace geos {
namespace operation { // geos::operation
namespace spanning { // geos::operation::spanning

namespace {

    // Internal edge representation for Kruskal's
    struct MSTEdge {
        std::size_t u;
        std::size_t v;
        double weight;
        std::size_t originalIndex;
    };

    // Union-Find data structure using vector for efficiency
    struct UnionFind {
        std::vector<std::size_t> parent;
        std::vector<int> rank;

        UnionFind(std::size_t n) : parent(n), rank(n, 0) {
            std::iota(parent.begin(), parent.end(), 0);
        }

        std::size_t find(std::size_t i) {
            if (parent[i] == i)
                return i;
            else
                parent[i] = find(parent[i]);
            return parent[i]; // path compression
        }

        bool unite(std::size_t i, std::size_t j) {
            std::size_t root_i = find(i);
            std::size_t root_j = find(j);
            if (root_i != root_j) {
                // Union by rank
                if (rank[root_i] < rank[root_j])
                    parent[root_i] = root_j;
                else if (rank[root_i] > rank[root_j])
                    parent[root_j] = root_i;
                else {
                    parent[root_i] = root_j;
                    rank[root_j]++;
                }
                return true;
            }
            return false;
        }
    };

    // Helper for coordinate to node ID mapping
    struct NodeMapping {
        std::unordered_map<CoordinateXY, std::size_t, CoordinateXY::HashCode> coordToId;

        std::size_t getId(const CoordinateXY& c) {
            auto it = coordToId.find(c);
            if (it != coordToId.end()) return it->second;
            std::size_t id = coordToId.size();
            coordToId[c] = id;
            return id;
        }

        std::size_t size() const {
            return coordToId.size();
        }
    };

} // anonymous namespace

void
SpanningTree::mst(const std::vector<const geom::Curve*>& curves, std::vector<std::size_t>& result)
{
    // Resize result to match input and initialize with 0
    result.assign(curves.size(), 0);
    if (curves.empty()) return;

    NodeMapping mapping;
    std::vector<MSTEdge> edges;

    for (std::size_t i = 0; i < curves.size(); ++i) {
        const Curve* curve = curves[i];
        if (!curve || curve->isEmpty()) continue;

        auto startPoint = curve->getStartPoint();
        auto endPoint = curve->getEndPoint();
        if (!startPoint || !endPoint) continue;

        CoordinateXY startCoord(*(startPoint->getCoordinate()));
        CoordinateXY endCoord(*(endPoint->getCoordinate()));

        // Ignore zero-length edges (where start == end in 2D)
        if (startCoord.equals2D(endCoord)) continue;

        std::size_t u = mapping.getId(startCoord);
        std::size_t v = mapping.getId(endCoord);
        double length = curve->getLength();

        edges.push_back({u, v, length, i});
    }

    if (edges.empty()) return;

    // Sort edges by length
    std::sort(edges.begin(), edges.end(), [](const MSTEdge& a, const MSTEdge& b) {
        return a.weight < b.weight;
    });

    // Kruskal's algorithm
    UnionFind uf(mapping.size());
    std::vector<const MSTEdge*> treeEdges;

    for (const auto& edge : edges) {
        if (uf.unite(edge.u, edge.v)) {
            treeEdges.push_back(&edge);
        }
    }

    // Assign component IDs
    std::size_t componentId = 0;
    std::unordered_map<std::size_t, std::size_t> rootToComponentId;

    for (const auto* edge : treeEdges) {
        std::size_t root = uf.find(edge->u);
        if (rootToComponentId.find(root) == rootToComponentId.end()) {
            rootToComponentId[root] = ++componentId;
        }
        result[edge->originalIndex] = rootToComponentId[root];
    }
}

} // namespace geos::operation::spanning
} // namespace geos::operation
} // namespace geos
