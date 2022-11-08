/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2005-2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: geomgraph/NodeMap.java rev. 1.3 (JTS-1.10)
 *
 **********************************************************************/

#include <geos/geomgraph/NodeMap.h>
#include <geos/geomgraph/Node.h>
#include <geos/geomgraph/NodeFactory.h>
#include <geos/geomgraph/EdgeEnd.h>
#include <geos/geomgraph/Label.h>
#include <geos/geom/Location.h>
#include <geos/geom/Coordinate.h>

#include <vector>
#include <cassert>

#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif

using namespace geos::geom;

namespace geos {
namespace geomgraph { // geos.geomgraph

NodeMap::NodeMap(const NodeFactory& newNodeFact)
    :
    nodeFact(newNodeFact)
{
#if GEOS_DEBUG
    std::cerr << "[" << this << "] NodeMap::NodeMap" << std::endl;
#endif
}

NodeMap::~NodeMap() = default;

Node*
NodeMap::addNode(const Coordinate& coord)
{
#if GEOS_DEBUG
    std::cerr << "[" << this << "] NodeMap::addNode(" << coord.toString() << ")";
#endif
    Node* node = find(coord);
    if(node == nullptr) {
#if GEOS_DEBUG
        std::cerr << " is new" << std::endl;
#endif
        node = nodeFact.createNode(coord);
        Coordinate* c = const_cast<Coordinate*>(
                            &(node->getCoordinate()));
        nodeMap[c] = std::unique_ptr<Node>(node);
        node = nodeMap[c].get();
    }
    else {
#if GEOS_DEBUG
        std::cerr << " already found (" << node->getCoordinate().toString() << ") - adding Z" << std::endl;
#endif
        node->addZ(coord.z);
    }
    return node;
}

// first arg cannot be const because
// it is liable to label-merging ... --strk;
Node*
NodeMap::addNode(Node* n)
{
    assert(n);

#if GEOS_DEBUG
    std::cerr << "[" << this << "] NodeMap::addNode(" << n->print() << ")";
#endif
    Coordinate* c = const_cast<Coordinate*>(&n->getCoordinate());
    Node* node = find(*c);
    if(node == nullptr) {
#if GEOS_DEBUG
        std::cerr << " is new" << std::endl;
#endif
        nodeMap[c] = std::unique_ptr<Node>(n);
        return nodeMap[c].get();
    }
#if GEOS_DEBUG
    else {
        std::cerr << " found already, merging label" << std::endl;
        const std::vector<double>& zvals = n->getZ();
        for(unsigned int i = 0; i < zvals.size(); i++) {
            node->addZ(zvals[i]);
        }
    }
#endif // GEOS_DEBUG

    node->mergeLabel(*n);
    return node;
}

void
NodeMap::add(EdgeEnd* e)
{
    Coordinate& p = e->getCoordinate();
    Node* n = addNode(p);
    n->add(e);
}

void
NodeMap::add(std::unique_ptr<EdgeEnd>&& e)
{
    add(e.get());
    e.release();
}

/*
 * @return the node if found; null otherwise
 */
Node*
NodeMap::find(const Coordinate& coord) const
{
    Coordinate* c = const_cast<Coordinate*>(&coord);

    const auto& found = nodeMap.find(c);

    if(found == nodeMap.end()) {
        return nullptr;
    }
    else {
        return found->second.get();
    }
}

void
NodeMap::getBoundaryNodes(uint8_t geomIndex, std::vector<Node*>& bdyNodes) const
{
    for(const auto& it: nodeMap) {
        Node* node = it.second.get();
        if(node->getLabel().getLocation(geomIndex) == Location::BOUNDARY) {
            bdyNodes.push_back(node);
        }
    }
}

std::string
NodeMap::print() const
{
    std::string out = "";
    for(const auto& it: nodeMap) {
        const Node* node = it.second.get();
        out += node->print();
    }
    return out;
}

} // namespace geos.geomgraph
} // namespace geos

