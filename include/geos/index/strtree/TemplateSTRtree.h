/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2020 Daniel Baston
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/geom/Envelope.h>
#include <geos/geom/Geometry.h>
#include <geos/index/SpatialIndex.h> // for inheritance
#include <geos/index/chain/MonotoneChain.h>
#include <geos/index/ItemVisitor.h>
#include <geos/util.h>

#include <geos/index/strtree/EnvelopeUtil.h>
#include <geos/index/strtree/TemplateSTRNode.h>
#include <geos/index/strtree/TemplateSTRNodePair.h>
#include <geos/index/strtree/TemplateSTRtreeDistance.h>

#include <vector>
#include <queue>

namespace geos {
namespace index {
namespace strtree {

template<typename ItemType>
class TemplateSTRtree : public SpatialIndex {
public:
    using Node = TemplateSTRNode<ItemType>;
    using NodeList = std::vector<Node>;
    using NodeListIterator = typename NodeList::iterator;

    explicit TemplateSTRtree(size_t p_nodeCapacity = 10) : root(nullptr), nodeCapacity(p_nodeCapacity) {}

    TemplateSTRtree(size_t p_nodeCapacity, size_t itemCapacity) : root(nullptr), nodeCapacity(p_nodeCapacity) {
        auto finalSize = treeSize(itemCapacity);
        nodes.reserve(finalSize);
    }

    bool built() const {
        return root != nullptr;
    }

    const Node* getRoot() {
        build();
        return root;
    }

    void insert(const ItemType* item) {
        insert(*item->getEnvelopeInternal(), item);
    }

    void insert(const geom::Envelope &itemEnv, const ItemType *item) {
        createLeafNode(item, itemEnv);
    }

    void insert(const geom::Envelope *itemEnv, void *item) override {
        insert(*itemEnv, static_cast<ItemType *>(item));
    }

    template<typename ItemDistance>
    std::pair<const ItemType*, const ItemType*> nearestNeighbour(TemplateSTRtree<ItemType> & other) {
        if (!getRoot() || !other.getRoot()) {
            return { nullptr, nullptr };
        }

        TemplateSTRtreeDistance<ItemType, ItemDistance> td;
        return td.nearestNeighbour(*root, *other.root);
    }

    template<typename Visitor>
    void query(const geom::Envelope &queryEnv, Visitor &&visitor) {
        if (!built()) {
            build();
        }

        if (root->envelopeIntersects(queryEnv)) {
            if (root->isLeaf()) {
                visitor(root->getItem());
            } else {
                query(queryEnv, *root, visitor);
            }
        }
    }

    void query(const geom::Envelope * queryEnv, std::vector<const ItemType*> & results) {
        query(*queryEnv, [&results](const ItemType* x) {
            results.push_back(x);
        });
    }

    void query(const geom::Envelope *queryEnv, std::vector<void *> &results) override {
        query(*queryEnv, [&results](const ItemType *x) {
            results.push_back(const_cast<void*>(static_cast<const void*>(x)));
        });
    }

    void query(const geom::Envelope *queryEnv, ItemVisitor &visitor) override {
        query(*queryEnv, [&visitor](const ItemType *x) {
            visitor.visitItem(const_cast<void*>(static_cast<const void*>(x)));
        });
    }

    bool remove(const geom::Envelope *itemEnv, void *item) override {
        if (root == nullptr) {
            return false;
        }

        if (root->isLeaf()) {
            if (root->getItem() == item) {
                root->removeItem();
                return true;
            }
            return false;
        }

        return remove(*itemEnv, *root, static_cast<ItemType *>(item));
    }

private:

    NodeList nodes;
    Node* root;
    size_t nodeCapacity;

    void createLeafNode(const ItemType *item, const geom::Envelope &env) {
        nodes.emplace_back(item, env);
    }

    void createBranchNode(const Node *begin, const Node *end) {
        assert(nodes.size() < nodes.capacity());
        nodes.emplace_back(begin, end);
    }

    void build() {
        if (built()) {
            return;
        }

        // compute final size of tree and set it aside in a single
        // block of memory
        auto finalSize = treeSize(nodes.size());
        nodes.reserve(finalSize);

        // begin and end define a range of nodes needing parents
        auto begin = nodes.begin();
        auto end = nodes.end();

        while (std::distance(begin, end) > 1) {
            createParentNodes(begin, end);
            begin = end; // parents just added become children in the next round
            end = nodes.end();
        }

        assert(finalSize == nodes.size());

        root = &nodes.back();
    }

    // calculate what the tree size will be when it is build. This is simply
    // a version of createParentNodes that doesn't actually create anything.
    size_t treeSize(size_t numLeafNodes) {
        size_t nodesInTree = numLeafNodes;

        size_t nodesWithoutParents = numLeafNodes;
        while (nodesWithoutParents > 1) {
            auto numSlices = sliceCount(nodesWithoutParents);
            auto nodesPerSlice = sliceCapacity(nodesWithoutParents, numSlices);

            size_t parentNodesAdded = 0;
            for (size_t j = 0; j < numSlices; j++) {
                auto nodesInSlice = std::min(nodesWithoutParents, nodesPerSlice);
                nodesWithoutParents -= nodesInSlice;

                parentNodesAdded += static_cast<size_t>(std::ceil(
                        static_cast<double>(nodesInSlice) / static_cast<double>(nodeCapacity)));
            }

            nodesInTree += parentNodesAdded;
            nodesWithoutParents = parentNodesAdded;
        }

        return nodesInTree;
    }

    void createParentNodes(const NodeListIterator &begin, const NodeListIterator &end) {
        // Arrange child nodes in two dimensions.
        // First, divide them into vertical slices of a given size (left-to-right)
        // Then create nodes within those slices (bottom-to-top)
        auto numChildren = std::distance(begin, end);
        auto numSlices = sliceCount(numChildren);
        auto nodesPerSlice = sliceCapacity(numChildren, numSlices);

        // Cache a sort value to avoid computing it repeatedly. Not a huge win, but
        // we aren't doing anything else with the last 8 bytes in our Node struct.
        setSortValuesX(begin, end);

        // We could sort all of the nodes here, but we don't actually need them to be
        // completely sorted. They need to be sorted enough for each node to end up
        // in the right vertical slice, but their relative position within the slice
        // doesn't matter. So we do a partial sort for each slice below instead.
        //sortNodesX(begin, end);

        auto startOfSlice = begin;
        for (size_t j = 0; j < numSlices; j++) {
            auto nodesRemaining = static_cast<size_t>(std::distance(startOfSlice, end));
            auto nodesInSlice = std::min(nodesRemaining, nodesPerSlice);
            auto endOfSlice = std::next(startOfSlice, nodesInSlice);

            // Make sure that every node that should be in this slice ends up somewhere
            // between startOfSlice and endOfSlice. We don't require any ordering among
            // nodes between startOfSlice and endOfSlice.
            partialSortNodes(startOfSlice, endOfSlice, end);

            addParentNodesFromVerticalSlice(startOfSlice, endOfSlice);

            startOfSlice = endOfSlice;
        }
    }

    void addParentNodesFromVerticalSlice(const NodeListIterator &begin, const NodeListIterator &end) {
        setSortValuesY(begin, end);
        //sortNodesY(begin, end);

        // Arrange the nodes vertically and full up parent nodes sequentially until they're full.
        // A possible improvement would be to rewowk this such so that if we have 81 nodes we
        // put 9 into each parent instead of 10 or 1.
        auto firstChild = begin;
        while (firstChild != end) {
            auto childrenRemaining = static_cast<size_t>(std::distance(firstChild, end));
            auto childrenForNode = std::min(nodeCapacity, childrenRemaining);
            auto lastChild = std::next(firstChild, childrenForNode);

            partialSortNodes(firstChild, lastChild, end);

            // Ideally we would be able to store firstChild and lastChild instead of
            // having to convert them to pointers, but I wasn't sure how to access
            // the NodeListIterator type from within Node without creating some weird
            // circular dependency.
            const Node *ptr_first = &*firstChild;
            const Node *ptr_end = ptr_first + childrenForNode;

            createBranchNode(ptr_first, ptr_end);
            firstChild = lastChild;
        }
    }

    void setSortValuesX(const NodeListIterator &begin, const NodeListIterator &end) {
        std::for_each(begin, end, [](Node &n) {
            const geom::Envelope &e = n.getEnvelope();
            n.setSortVal(e.getMinX() + e.getMaxX());
        });
    }

    void setSortValuesY(const NodeListIterator &begin, const NodeListIterator &end) {
        std::for_each(begin, end, [](Node &n) {
            const geom::Envelope &e = n.getEnvelope();
            n.setSortVal(e.getMinY() + e.getMaxY());
        });
    }

#if 0
    void sortNodes(NodeListIterator &begin, NodeListIterator &end) {
        std::sort(begin, end, [](const Node &a, const Node &b) {
            return a.getSortVal() < b.getSortVal();
        });
    }
#endif

    // Partially sort nodes between `begin` and `end` such that all nodes less than `mid` are placed before `mid`.
    void partialSortNodes(const NodeListIterator &begin, const NodeListIterator &mid, const NodeListIterator &end) {
        std::nth_element(begin, mid, end, [](const Node &a, const Node &b) {
            return a.getSortVal() < b.getSortVal();
        });
    }

    template<typename Visitor>
    void query(const geom::Envelope &queryEnv,
               const Node &node,
               Visitor &&visitor) {

        assert(!node.isLeaf());

        for (auto *child = node.beginChildren(); child < node.endChildren(); ++child) {
            if (child->envelopeIntersects(queryEnv)) {
                if (child->isLeaf() && !child->isDeleted()) {
                    visitor(child->getItem());
                } else {
                    query(queryEnv, *child, visitor);
                }
            }
        }
    }

    bool remove(const geom::Envelope &queryEnv,
                const Node &node,
                const ItemType *item) {

        assert(!node.isLeaf());

        for (auto *child = node.beginChildren(); child < node.endChildren(); ++child) {
            if (child->envelopeIntersects(queryEnv)) {
                if (child->isLeaf()) {
                    if (!child->isDeleted() && child->getItem() == item) {
                        // const cast is ugly, but alternative seems to be to remove all
                        // const qualifiers in Node and open up mutability everywhere?
                        auto mutableChild = const_cast<Node *>(child);
                        mutableChild->removeItem();
                        return true;
                    }
                } else {
                    bool removed = remove(queryEnv, *child, item);
                    if (removed) {
                        return true;
                    }
                }
            }
        }

        return false;
    }

    size_t sliceCount(size_t numNodes) const {
        double minLeafCount = std::ceil(static_cast<double>(numNodes) / static_cast<double>(nodeCapacity));

        return static_cast<size_t>(std::ceil(std::sqrt(minLeafCount)));
    }

    static size_t sliceCapacity(size_t numNodes, size_t numSlices) {
        return static_cast<size_t>(std::ceil(static_cast<double>(numNodes) / static_cast<double>(numSlices)));
    }


};




}
}
}
