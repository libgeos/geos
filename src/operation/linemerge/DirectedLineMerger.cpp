#include <geos/operation/linemerge/DirectedLineMerger.h>
#include <geos/operation/linemerge/DirectedEdgeString.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryComponentFilter.h>
#include <geos/geom/LineString.h>
#include <geos/planargraph/DirectedEdge.h>
#include <geos/planargraph/Node.h>

#include <cassert>
#include <set>
#include <vector>

#include <iostream>

#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif

#if GEOS_DEBUG
#include <iostream>
#endif

using namespace geos::geom;
using namespace geos::planargraph;

namespace geos {
namespace operation { // geos.operation
namespace linemerge { // geos.operation.linemerge

DirectedLineMerger::DirectedLineMerger():
    factory(nullptr)
{
}

DirectedLineMerger::~DirectedLineMerger()
{
    for(std::size_t i = 0, n = edgeStrings.size(); i < n; ++i) {
        delete edgeStrings[i];
    }
}

void
DirectedLineMerger::add(std::vector<const Geometry*>* geometries)
{
    for(const auto g : *geometries) {
        add(g);
    }
}

struct DLMGeometryComponentFilter: public GeometryComponentFilter {
    DirectedLineMerger* dlm;

    DLMGeometryComponentFilter(DirectedLineMerger* newDlm): dlm(newDlm) {}

    void
    filter(const Geometry* geom)
    {
        const auto ls = dynamic_cast<const LineString*>(geom);
        if(ls) {
            dlm->add(ls);
        }
    }
};

void
DirectedLineMerger::add(const Geometry* geometry)
{
    DLMGeometryComponentFilter dlmgcf(this);
    geometry->applyComponentFilter(dlmgcf);
}

void
DirectedLineMerger::add(const LineString* lineString)
{
    if(factory == nullptr) {
        factory = lineString->getFactory();
    }
    graph.addEdge(lineString);
}

void
DirectedLineMerger::merge()
{
    if(!mergedLineStrings.empty()) {
        return;
    }

    // reset marks
    GraphComponent::setMarkedMap(
        graph.nodeIterator(), graph.nodeEnd(),
        false);
    GraphComponent::setMarked(
        graph.dirEdgeIterator(), graph.dirEdgeEnd(),
        false);

    for(std::size_t i = 0, n = edgeStrings.size(); i < n; ++i) {
        delete edgeStrings[i];
    }
    edgeStrings.clear();

    buildEdgeStringsForObviousStartNodes();
    buildEdgeStringsForUnprocessedNodes();
    buildEdgeStringsForUnprocessedEdges();

    auto numEdgeStrings = edgeStrings.size();
    mergedLineStrings.reserve(numEdgeStrings);
    for(std::size_t i = 0; i < numEdgeStrings; ++i) {
        DirectedEdgeString* edgeString = edgeStrings[i];
        mergedLineStrings.emplace_back(edgeString->toLineString());
    }
}

void
DirectedLineMerger::buildEdgeStringsForObviousStartNodes()
{
#if GEOS_DEBUG
    std::cerr << __FUNCTION__ << std::endl;
#endif
    std::vector<Node*> nodesToProcess;
    std::set<Node*> nodesWithIncomingEdges;

    // Mark nodes with multiple outgoing edges
#if GEOS_DEBUG
    std::cerr << "Nodes with multiple outgoing edges:" << std::endl;
#endif
    for(auto it = graph.nodeIterator(); it != graph.nodeEnd(); ++it) {
        Node* node = it->second;
        if(node->getDegree() > 1) {
#if GEOS_DEBUG
            std::cerr << "  " << *node << std::endl;
#endif
            node->setMarked(true);
            nodesToProcess.push_back(node);
        }
    }

    // Mark nodes with multiple incoming edges
#if GEOS_DEBUG
    std::cerr << "Nodes with multiple incoming edges:" << std::endl;
#endif
    for(auto it = graph.dirEdgeIterator(); it != graph.dirEdgeEnd(); ++it) {
        Node* node = (*it)->getToNode();
        if(!node->isMarked()) {
            if(!nodesWithIncomingEdges.insert(node).second) {
#if GEOS_DEBUG
                std::cerr << "  " << *node << std::endl;
#endif
                node->setMarked(true);
                nodesToProcess.push_back(node);
            }
        }
    }

    // Process collected nodes
    for(auto it = nodesToProcess.begin(); it != nodesToProcess.end(); ++it) {
        Node* node = *it;
        buildEdgeStringsStartingAt(node, true); // stop at marked nodes
    }

    // Mark nodes with single incoming edge, not marked yet
#if GEOS_DEBUG
    std::cerr << "Nodes with single incoming edge and not marked yet:" << std::endl;
#endif
    for(auto it = nodesWithIncomingEdges.begin();
         it != nodesWithIncomingEdges.end();
         ++it)
    {
        Node* node = *it;
        if(!node->isMarked()) {
            node->setMarked(true);
#if GEOS_DEBUG
            std::cerr << "  " << *node << std::endl;
#endif
        }
    }
}

void
DirectedLineMerger::buildEdgeStringsForUnprocessedNodes()
{
#if GEOS_DEBUG
    std::cerr << __FUNCTION__ << std::endl;
#endif
    for(auto it = graph.nodeIterator(); it != graph.nodeEnd(); ++it) {
        Node* node = it->second;
        if(!node->isMarked()) {
            assert(node->getDegree() == 1);
#if GEOS_DEBUG
            std::cerr << "  " << *node << std::endl;
#endif
            buildEdgeStringsStartingAt(node, false);
            node->setMarked(true);
        }
    }
}

// Edges can remain unprocessed at this stage if they are a part of a loop
void
DirectedLineMerger::buildEdgeStringsForUnprocessedEdges()
{
#if GEOS_DEBUG
    std::cerr << __FUNCTION__ << std::endl;
#endif
    for(auto it = graph.dirEdgeIterator(); it != graph.dirEdgeEnd(); ++it) {
        DirectedEdge* directedEdge = *it;

        if(!directedEdge->isMarked()) {
#if GEOS_DEBUG
            std::cerr << "  " << *directedEdge << std::endl;
#endif
            edgeStrings.push_back(
                buildEdgeStringStartingWith(directedEdge, false));
        }
    }
}

void
DirectedLineMerger::buildEdgeStringsStartingAt(
    Node* node,
    bool stopAtMarkedNode)
{
    std::vector<DirectedEdge*>& edges = node->getOutEdges()->getEdges();
    std::size_t size = edges.size();
    for(std::size_t i = 0; i < size; i++) {
        DirectedEdge* directedEdge = edges[i];
        if(directedEdge->isMarked()) {
            continue;
        }
        edgeStrings.push_back(
            buildEdgeStringStartingWith(directedEdge, stopAtMarkedNode));
    }
}

DirectedEdgeString*
DirectedLineMerger::buildEdgeStringStartingWith(
    DirectedEdge* start,
    bool stopAtMarkedNode)
{
    auto edgeString = new DirectedEdgeString(factory);
    DirectedEdge* current = start;
    do {
        edgeString->add(current);
        current->setMarked(true);

        Node* toNode = current->getToNode();
        if(toNode->getDegree() == 0 || (stopAtMarkedNode && toNode->isMarked())) {
            current = nullptr;
        } else {
            current = toNode->getOutEdges()->getEdges()[0];
            assert(toNode->getDegree() == 1 || current->isMarked());
            if (current->isMarked()) {
                current = nullptr;
            }
        }
    }
    while(current != nullptr && current != start);
    return edgeString;
}

std::vector<std::unique_ptr<LineString>>
DirectedLineMerger::getMergedLineStrings()
{
    merge();

    // Explicitly give ownership to the caller.
    auto ret = std::move(mergedLineStrings);
    mergedLineStrings.clear();
    return ret;
}

} // namespace geos.operation.linemerge
} // namespace geos.operation
} // namespace geos
