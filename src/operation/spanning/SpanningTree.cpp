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
#include <geos/geom/LineString.h>
#include <geos/planargraph/Node.h>
#include <geos/planargraph/DirectedEdge.h>

#include <vector>
#include <algorithm>
#include <map>

using namespace geos::planargraph;
using namespace geos::geom;

namespace geos {
namespace operation { // geos::operation
namespace spanning { // geos::operation::spanning

namespace {

    // Union-Find data structure
    struct UnionFind {
        std::map<Node*, Node*> parent;
        
        Node* find(Node* n) {
            if (parent.find(n) == parent.end()) {
                parent[n] = n;
                return n;
            }
            if (parent[n] == n) return n;
            parent[n] = find(parent[n]); // path compression
            return parent[n];
        }
        
        void unite(Node* a, Node* b) {
            Node* rootA = find(a);
            Node* rootB = find(b);
            if (rootA != rootB) {
                parent[rootA] = rootB;
            }
        }
    };

} // anonymous namespace

void
SpanningTree::mst(const std::vector<const geom::LineString*>& lines, std::vector<int>& result)
{
    SpanningTreeGraph graph;
    
    // Resize result to match input and initialize with 0
    result.assign(lines.size(), 0);
    
    for (std::size_t i = 0; i < lines.size(); ++i) {
        if (lines[i]) {
            graph.addEdge(lines[i], i);
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
    int componentId = 0;
    std::map<Node*, int> rootToComponentId;
    
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
