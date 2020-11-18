/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: index/strtree/STRtree.java rev. 1.11
 *
 **********************************************************************/

#include <geos/index/strtree/SimpleSTRtree.h>
#include <geos/index/ItemVisitor.h>
#include <geos/geom/Envelope.h>
#include <geos/geom/Geometry.h>

#include <vector>
#include <cassert>
#include <cmath>
#include <algorithm> // std::sort
#include <iostream> // for debugging
#include <limits>
#include <geos/util/GEOSException.h>

using namespace geos::geom;

namespace geos {
namespace index { // geos.index
namespace strtree { // geos.index.strtree

/* private */
SimpleSTRnode*
SimpleSTRtree::createNode(int newLevel, const geom::Envelope* itemEnv, void* item)
{
    storedNodes.emplace_back(newLevel, itemEnv, item, nodeCapacity);
    SimpleSTRnode* node = &(storedNodes.back());
    return node;
}

/* private */
SimpleSTRnode*
SimpleSTRtree::createNode(int newLevel)
{
    return createNode(newLevel, nullptr, nullptr);
}

/* public */
void
SimpleSTRtree::insert(geom::Geometry* geom)
{
    insert(geom->getEnvelopeInternal(), static_cast<void*>(geom));
}

/* public */
void
SimpleSTRtree::insert(const geom::Envelope* itemEnv, void* item)
{
    SimpleSTRnode *node = createNode(0, itemEnv, item);
    nodes.push_back(node);
}

/* private static */
void
SimpleSTRtree::sortNodesY(std::vector<SimpleSTRnode*>& nodeList)
{
    struct {
        bool operator()(SimpleSTRnode* a, SimpleSTRnode* b) const
        {
            const geom::Envelope& ea = a->getBounds();
            const geom::Envelope& eb = b->getBounds();
            double ya = (ea.getMinY() + ea.getMaxY()) / 2.0;
            double yb = (eb.getMinY() + eb.getMaxY()) / 2.0;
            return ya < yb;
        }
    } nodeSortByY;

    std::sort(nodeList.begin(), nodeList.end(), nodeSortByY);
}

/* private static */
void
SimpleSTRtree::sortNodesX(std::vector<SimpleSTRnode*>& nodeList)
{
    struct {
        bool operator()(SimpleSTRnode* a, SimpleSTRnode* b) const
        {
            const geom::Envelope& ea = a->getBounds();
            const geom::Envelope& eb = b->getBounds();
            double xa = (ea.getMinX() + ea.getMaxX()) / 2.0;
            double xb = (eb.getMinX() + eb.getMaxX()) / 2.0;
            return xa < xb;
        }
    } nodeSortByX;

    std::sort(nodeList.begin(), nodeList.end(), nodeSortByX);
}

/* private */
std::vector<SimpleSTRnode*>
SimpleSTRtree::createParentNodes(
    std::vector<SimpleSTRnode*>& childNodes,
    int newLevel)
{
    assert(!childNodes.empty());

    std::size_t minLeafCount = (std::size_t)std::ceil((double)childNodes.size() / (double)nodeCapacity);
    std::size_t sliceCount = (std::size_t)std::ceil(std::sqrt((double)minLeafCount));
    std::size_t sliceCapacity = (std::size_t)std::ceil(childNodes.size() / (double)sliceCount);

    sortNodesX(childNodes);

    std::size_t i = 0;
    std::size_t nChildren = childNodes.size();
    std::vector<SimpleSTRnode*> parentNodes;
    std::vector<SimpleSTRnode*> verticalSlice(sliceCapacity);
    for (std::size_t j = 0; j < sliceCount; j++) {
        verticalSlice.clear();
        std::size_t nodesAddedToSlice = 0;
        while(i < nChildren && nodesAddedToSlice < sliceCapacity) {
            verticalSlice.push_back(childNodes[i++]);
            ++nodesAddedToSlice;
        }
        addParentNodesFromVerticalSlice(verticalSlice, newLevel, parentNodes);
    }
    return parentNodes;
}

/* private */
void
SimpleSTRtree::addParentNodesFromVerticalSlice(
    std::vector<SimpleSTRnode*>& verticalSlice,
    int newLevel,
    std::vector<SimpleSTRnode*>& parentNodes)
{
    sortNodesY(verticalSlice);

    SimpleSTRnode* parent = nullptr;
    for (auto* node: verticalSlice) {
        if (!parent) {
            parent = createNode(newLevel);
        }
        parent->addChildNode(node);
        if (parent->size() == nodeCapacity) {
            parentNodes.push_back(parent);
            parent = nullptr;
        }
    }
    if (parent)
        parentNodes.push_back(parent);

    return;
}

/* private */
std::vector<SimpleSTRnode*>
SimpleSTRtree::createHigherLevels(
    std::vector<SimpleSTRnode*>& nodesOfALevel, int level)
{
    int nextLevel = level+1;
    std::vector<SimpleSTRnode*> parentNodes = createParentNodes(nodesOfALevel, nextLevel);
    if (parentNodes.size() == 1) {
        return parentNodes;
    }
    return createHigherLevels(parentNodes, nextLevel);
}

/* private */
void
SimpleSTRtree::build()
{
    if (built) return;

    if (nodes.empty()) {
        root = createNode(0);
    }
    else {
        std::vector<SimpleSTRnode*> nodeTree = createHigherLevels(nodes, -1);
        assert(nodeTree.size()==1);
        root = nodeTree[0];
    }
    built = true;
}

/* public */
void
SimpleSTRtree::query(const geom::Envelope* searchEnv, ItemVisitor& visitor)
{
    if(!built) {
        build();
    }

    if(nodes.empty()) {
        assert(root == nullptr);
        return;
    }

    if(root->getBounds().intersects(searchEnv)) {
        query(searchEnv, root, visitor);
    }
}

/* private */
void
SimpleSTRtree::query(const geom::Envelope* searchEnv,
    const SimpleSTRnode* node, ItemVisitor& visitor)
{
    for(auto* childNode: node->getChildNodes()) {

        if(!childNode->getBounds().intersects(searchEnv)) {
            continue;
        }

        if (childNode->isLeaf()) {
            visitor.visitItem(childNode->getItem());
        } else {
            query(searchEnv, childNode, visitor);
        }
    }
}

/* public */
void
SimpleSTRtree::query(const geom::Envelope* searchEnv, std::vector<void*>& matches)
{
    if(!built) {
        build();
    }

    if(nodes.empty()) {
        assert(root == nullptr);
        return;
    }

    if(root->getBounds().intersects(searchEnv)) {
        query(searchEnv, root, matches);
    }
}

/* private */
void
SimpleSTRtree::query(const geom::Envelope* searchEnv,
    const SimpleSTRnode* node, std::vector<void*>& matches)
{
    assert(node);

    for(auto* childNode: node->getChildNodes()) {

        if(!childNode->getBounds().intersects(searchEnv)) {
            continue;
        }

        if (childNode->isLeaf()) {
            matches.push_back(childNode->getItem());
        } else {
            query(searchEnv, childNode, matches);
        }
    }
}

/* public */
bool
SimpleSTRtree::remove(const geom::Envelope* itemEnv, void* item)
{
    // no implementation of remove() yet!
    return false;
}


/*public static*/
std::ostream&
operator<<(std::ostream& os, const SimpleSTRtree& tree)
{

    os << "nodeCapacity: " << tree.getNodeCapacity() << std::endl;
    os << "nodes.size(): " << tree.getNumLeafNodes() << std::endl;
    os << "built: " << tree.getBuilt() << std::endl;
    os << "tree: " << std::endl;

    if (tree.getRoot())
        tree.getRoot()->toString(os, 1);
    return os;
}



} // namespace geos.index.strtree
} // namespace geos.index
} // namespace geos
