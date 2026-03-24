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
#include <geos/operation/spanning/SpanningTreeGraph.h>
#include <geos/operation/spanning/SpanningTreeEdge.h>
#include <geos/geom/Curve.h>
#include <geos/planargraph/Node.h>
#include <geos/planargraph/DirectedEdge.h>

#include <vector>
#include <algorithm>
#include <unordered_map>

using namespace geos::planargraph;
using namespace geos::geom;

namespace geos {
namespace operation { // geos::operation
namespace spanning { // geos::operation::spanning

namespace {

    // Union-Find data structure
    struct UnionFind {
        std::unordered_map<Node*, Node*> parent;
        std::unordered_map<Node*, int> rank;
        
        Node* find(Node* n) {
            auto it = parent.find(n);
            if (it == parent.end()) {
                parent[n] = n;
                rank[n] = 0;
                return n;
            }
            if (it->second == n) return n;
            it->second = find(it->second); // path compression
            return it->second;
        }
        
        void unite(Node* a, Node* b) {
            Node* rootA = find(a);
            Node* rootB = find(b);
            if (rootA != rootB) {
                // Union by rank
                if (rank[rootA] < rank[rootB]) {
                    parent[rootA] = rootB;
                }
                else if (rank[rootA] > rank[rootB]) {
                    parent[rootB] = rootA;
                }
                else {
                    parent[rootA] = rootB;
                    rank[rootB]++;
                }
            }
        }
    };

} // anonymous namespace

void
SpanningTree::mst(const std::vector<const geom::Curve*>& curves, std::vector<std::size_t>& result)
{
    SpanningTreeGraph graph;
    
    // Resize result to match input and initialize with 0
    result.assign(curves.size(), 0);
    
    for (std::size_t i = 0; i < curves.size(); ++i) {
        if (curves[i]) {
            graph.addEdge(curves[i], i);
        }
    }
    
    std::vector<SpanningTreeEdge*> edges;
    
    // Get edges from graph
    auto* graphEdges = graph.getEdges();
    for (auto* e : *graphEdges) {
        if (auto* ste = dynamic_cast<SpanningTreeEdge*>(e)) {
            edges.push_back(ste);
        }
    }
    
    // Sort edges by length
    std::sort(edges.begin(), edges.end(), [](SpanningTreeEdge* a, SpanningTreeEdge* b) {
        return a->getLength() < b->getLength();
    });
    
    // Kruskal's algorithm
    UnionFind uf;
    std::vector<SpanningTreeEdge*> treeEdges;
    
    for (auto* edge : edges) {
        Node* u = edge->getDirEdge(0)->getFromNode();
        Node* v = edge->getDirEdge(0)->getToNode();
        
        if (uf.find(u) != uf.find(v)) {
            uf.unite(u, v);
            treeEdges.push_back(edge);
        }
    }
    
    // Assign component IDs
    std::size_t componentId = 0;
    std::unordered_map<Node*, std::size_t> rootToComponentId;
    
    for (auto* edge : treeEdges) {
        Node* u = edge->getDirEdge(0)->getFromNode();
        Node* root = uf.find(u);
        
        if (rootToComponentId.find(root) == rootToComponentId.end()) {
            rootToComponentId[root] = ++componentId;
        }
        
        result[edge->getIndex()] = rootToComponentId[root];
    }
}

} // namespace geos::operation::spanning
} // namespace geos::operation
} // namespace geos
