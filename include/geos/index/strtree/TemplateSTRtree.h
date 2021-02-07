/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2020-2021 Daniel Baston
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/geom/Geometry.h>
#include <geos/index/SpatialIndex.h> // for inheritance
#include <geos/index/chain/MonotoneChain.h>
#include <geos/index/ItemVisitor.h>
#include <geos/util.h>

#include <geos/index/strtree/TemplateSTRNode.h>
#include <geos/index/strtree/TemplateSTRNodePair.h>
#include <geos/index/strtree/TemplateSTRtreeDistance.h>
#include <geos/index/strtree/Interval.h>

#include <vector>
#include <queue>

namespace geos {
namespace index {
namespace strtree {

template<typename ItemType, typename BoundsTraits>
class TemplateSTRtreeImpl {
public:
    using Node = TemplateSTRNode<ItemType, BoundsTraits>;
    using NodeList = std::vector<Node>;
    using NodeListIterator = typename NodeList::iterator;
    using BoundsType = typename BoundsTraits::BoundsType;

    explicit TemplateSTRtreeImpl(size_t p_nodeCapacity = 10) : root(nullptr), nodeCapacity(p_nodeCapacity) {}

    TemplateSTRtreeImpl(size_t p_nodeCapacity, size_t itemCapacity) : root(nullptr), nodeCapacity(p_nodeCapacity) {
        auto finalSize = treeSize(itemCapacity);
        nodes.reserve(finalSize);
    }

protected:
    // Prevent instantiation of base class.
    ~TemplateSTRtreeImpl() {}
public:

    bool built() const {
        return root != nullptr;
    }

    const Node* getRoot() {
        build();
        return root;
    }

    void insert(ItemType&& item) {
        insert(*item->getEnvelopeInternal(), std::forward<ItemType>(item));
    }

    void insert(const ItemType& item) {
        insert(*item->getEnvelopeInternal(), item);
    }

    void insert(const BoundsType& itemEnv, ItemType&& item) {
        if (!BoundsTraits::isNull(itemEnv)) {
            createLeafNode(std::forward<ItemType>(item), itemEnv);
        }
    }

    void insert(const BoundsType& itemEnv, const ItemType& item) {
        if (!BoundsTraits::isNull(itemEnv)) {
            createLeafNode(item, itemEnv);
        }
    }

    template<typename ItemDistance>
    std::pair<ItemType, ItemType> nearestNeighbour(ItemDistance & distance) {
        return nearestNeighbour(*this, distance);
    }

    template<typename ItemDistance>
    std::pair<ItemType, ItemType> nearestNeighbour() {
        ItemDistance id;
        return nearestNeighbour(*this);
    }

    template<typename ItemDistance>
    std::pair<ItemType, ItemType> nearestNeighbour(TemplateSTRtreeImpl<ItemType, BoundsTraits> & other,
                                                   ItemDistance & distance) {
        if (!getRoot() || !other.getRoot()) {
            return { nullptr, nullptr };
        }

        TemplateSTRtreeDistance<ItemType, BoundsTraits, ItemDistance> td(distance);
        return td.nearestNeighbour(*root, *other.root);
    }

    template<typename ItemDistance>
    std::pair<ItemType, ItemType> nearestNeighbour(TemplateSTRtreeImpl<ItemType, BoundsTraits> & other) {
        ItemDistance id;
        return nearestNeighbour(other, id);
    }

    template<typename ItemDistance>
    ItemType nearestNeighbour(const BoundsType& env, const ItemType& item, ItemDistance& itemDist) {
        build();

        if (getRoot() == nullptr) {
            return nullptr;
        }

        TemplateSTRNode<ItemType, BoundsTraits> bnd(item, env);
        TemplateSTRNodePair<ItemType, BoundsTraits, ItemDistance> pair(*getRoot(), bnd, itemDist);

        TemplateSTRtreeDistance<ItemType, BoundsTraits, ItemDistance> td(itemDist);
        return td.nearestNeighbour(pair).first;
    }

    template<typename ItemDistance>
    ItemType nearestNeighbour(const BoundsType& env, const ItemType& item) {
        ItemDistance id;
        return nearestNeighbour(env, item, id);
    }

    template<typename Visitor>
    void query(const BoundsType& queryEnv, Visitor &&visitor) {
        if (!built()) {
            build();
        }

        if (root && root->boundsIntersect(queryEnv)) {
            if (root->isLeaf()) {
                visitor(root->getItem());
            } else {
                query(queryEnv, *root, visitor);
            }
        }
    }

    void query(const BoundsType& queryEnv, std::vector<ItemType>& results) {
        query(queryEnv, [&results](const ItemType& x) {
            results.push_back(x);
        });
    }

    bool remove(const BoundsType& itemEnv, const ItemType& item) {
        if (root == nullptr) {
            return false;
        }

        if (root->isLeaf()) {
            if (!root->isDeleted() && root->getItem() == item) {
                root->removeItem();
                return true;
            }
            return false;
        }

        return remove(itemEnv, *root, item);
    }

    template<typename F>
    void iterate(F&& func) {
        auto n = built() ? numItems : nodes.size();
        for (size_t i = 0; i < n; i++) {
            func(nodes[i].getItem());
        }
    }

    void build() {
        if (built()) {
            return;
        }

        if (nodes.empty()) {
            return;
        }

        numItems = nodes.size();

        // compute final size of tree and set it aside in a single
        // block of memory
        auto finalSize = treeSize(numItems);
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

protected:

    NodeList nodes;
    Node* root;
    size_t nodeCapacity;
    size_t numItems;

    void createLeafNode(ItemType&& item, const BoundsType& env) {
        nodes.emplace_back(std::forward<ItemType>(item), env);
    }

    void createLeafNode(const ItemType& item, const BoundsType& env) {
        nodes.emplace_back(item, env);
    }

    void createBranchNode(const Node *begin, const Node *end) {
        assert(nodes.size() < nodes.capacity());
        nodes.emplace_back(begin, end);
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
            //partialSortNodes(startOfSlice, endOfSlice, end);

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
            n.setSortVal(BoundsTraits::getX(n.getBounds()));
        });
    }

    void setSortValuesY(const NodeListIterator &begin, const NodeListIterator &end) {
        std::for_each(begin, end, [](Node &n) {
            n.setSortVal(BoundsTraits::getY(n.getBounds()));
        });
    }

    void sortNodes(NodeListIterator &begin, NodeListIterator &end) {
        std::sort(begin, end, [](const Node &a, const Node &b) {
            return a.getSortVal() < b.getSortVal();
        });
    }

    // Partially sort nodes between `begin` and `end` such that all nodes less than `mid` are placed before `mid`.
    void partialSortNodes(const NodeListIterator &begin, const NodeListIterator &mid, const NodeListIterator &end) {
        std::nth_element(begin, mid, end, [](const Node &a, const Node &b) {
            return a.getSortVal() < b.getSortVal();
        });
    }

    template<typename Visitor>
    void query(const BoundsType& queryEnv,
               const Node& node,
               Visitor&& visitor) {

        assert(!node.isLeaf());

        for (auto *child = node.beginChildren(); child < node.endChildren(); ++child) {
            if (child->boundsIntersect(queryEnv)) {
                if (child->isLeaf() && !child->isDeleted()) {
                    visitor(child->getItem());
                } else {
                    query(queryEnv, *child, visitor);
                }
            }
        }
    }

    bool remove(const BoundsType& queryEnv,
                const Node& node,
                const ItemType& item) {

        assert(!node.isLeaf());

        for (auto *child = node.beginChildren(); child < node.endChildren(); ++child) {
            if (child->boundsIntersect(queryEnv)) {
                if (child->isLeaf()) {
                    if (!child->isDeleted() && child->getItem() == item) {
                        // const cast is ugly, but alternative seems to be to remove all
                        // const qualifiers in Node and open up mutability everywhere?
                        auto mutableChild = const_cast<Node*>(child);
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

    class Iterator : public std::iterator<std::forward_iterator_tag, ItemType> {
    public:
        Iterator(typename NodeList::const_iterator && iter,
                 typename NodeList::const_iterator && end) : m_iter(iter), m_end(end) {
            skipDeleted();
        }

        const ItemType& operator*() const {
            return m_iter->getItem();
        }

        Iterator& operator++() {
            m_iter++;
            skipDeleted();
            return *this;
        }

        friend bool operator==(const Iterator& a, const Iterator& b) {
            return a.m_iter == b.m_iter;
        }

        friend bool operator!=(const Iterator& a, const Iterator& b) {
            return a.m_iter != b.m_iter;
        }

    private:
        void skipDeleted() {
            while(m_iter->isDeleted() && m_iter != m_end) {
                m_iter++;
            }
        }

        typename NodeList::const_iterator m_iter;
        typename NodeList::const_iterator m_end;
    };

    class Items {
    public:
        explicit Items(TemplateSTRtreeImpl<ItemType, BoundsTraits>& tree) : m_tree(tree) {}

        Iterator begin() {
            return Iterator(m_tree.nodes.cbegin(),
                            std::next(m_tree.nodes.cbegin(), m_tree.numItems));
        }

        Iterator end() {
            return Iterator(std::next(m_tree.nodes.cbegin(), m_tree.numItems),
                            std::next(m_tree.nodes.cbegin(), m_tree.numItems));
        }
    private:

        TemplateSTRtreeImpl<ItemType, BoundsTraits>& m_tree;
    };

public:
    Items items() {
        build();
        return Items(*this);
    }


};

struct EnvelopeTraits {
    using BoundsType = geom::Envelope;

    static bool intersects(const BoundsType& a, const BoundsType& b) {
        return a.intersects(b);
    }

    static double size(const BoundsType & a) {
        return a.getArea();
    }

    static double distance(const BoundsType& a, const BoundsType& b) {
        return a.distance(b);
    }

    static BoundsType empty() {
        return {};
    }

    static double getX(const BoundsType& a) {
        return 0.5*(a.getMinX() + a.getMaxX());
    }

    static double getY(const BoundsType& a) {
        return 0.5*(a.getMinY() + a.getMaxY());
    }

    static void expandToInclude(BoundsType& a, const BoundsType& b) {
        a.expandToInclude(b);
    }

    static bool isNull(const BoundsType& a) {
        return a.isNull();
    }
};

struct IntervalTraits {
    using BoundsType = Interval;

    static bool intersects(const BoundsType& a, const BoundsType& b) {
        return a.intersects(&b);
    }

    static double size(const BoundsType & a) {
        return a.getWidth();
    }

    static double getX(const BoundsType& a) {
        return a.getCentre();
    }

    static double getY(const BoundsType& a) {
        return a.getCentre();
    }

    static void expandToInclude(BoundsType& a, const BoundsType& b) {
        a.expandToInclude(&b);
    }

    static bool isNull(const BoundsType& a) {
        (void) a;
        return false;
    }
};


template<typename ItemType, typename BoundsTraits = EnvelopeTraits>
class TemplateSTRtree : public TemplateSTRtreeImpl<ItemType, BoundsTraits> {
public:
    using TemplateSTRtreeImpl<ItemType, BoundsTraits>::TemplateSTRtreeImpl;
};

// When ItemType is a pointer and our bounds are geom::Envelope, adopt
// the SpatialIndex interface which requires queries via an envelope
// and items to be representable as void*.
template<typename ItemType>
class TemplateSTRtree<ItemType*, EnvelopeTraits> : public TemplateSTRtreeImpl<ItemType*, EnvelopeTraits>, public SpatialIndex {
public:
    using TemplateSTRtreeImpl<ItemType*, EnvelopeTraits>::TemplateSTRtreeImpl;
    using TemplateSTRtreeImpl<ItemType*, EnvelopeTraits>::insert;
    using TemplateSTRtreeImpl<ItemType*, EnvelopeTraits>::query;
    using TemplateSTRtreeImpl<ItemType*, EnvelopeTraits>::remove;

    // The SpatialIndex methods only work when we are storing a pointer type.
    void query(const geom::Envelope *queryEnv, std::vector<void*> &results) override {
        query(*queryEnv, [&results](const ItemType* x) {
            results.push_back(const_cast<void*>(static_cast<const void*>(x)));
        });
    }

    void query(const geom::Envelope *queryEnv, ItemVisitor &visitor) override {
        query(*queryEnv, [&visitor](const ItemType* x) {
            visitor.visitItem(const_cast<void*>(static_cast<const void*>(x)));
        });
    }

    bool remove(const geom::Envelope *itemEnv, void* item) override {
        return remove(*itemEnv, static_cast<ItemType*>(item));
    }

    void insert(const geom::Envelope *itemEnv, void *item) override {
        insert(*itemEnv, std::move(static_cast<ItemType*>(item)));
    }
};


}
}
}
