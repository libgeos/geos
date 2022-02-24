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
#include <mutex>

namespace geos {
namespace index {
namespace strtree {

/**
 * \brief
 * A query-only R-tree created using the Sort-Tile-Recursive (STR) algorithm.
 * For one- or two-dimensional spatial data.
 *
 * The STR packed R-tree is simple to implement and maximizes space
 * utilization; that is, as many leaves as possible are filled to capacity.
 * Overlap between nodes is far less than in a basic R-tree. However, once the
 * tree has been built (explicitly or on the first call to `query`), items may
 * not be added or removed.
 *
 * A user will instantiate `TemplateSTRtree` instead of `TemplateSTRtreeImpl`;
 * this structure is used so that `TemplateSTRtree` can implement the
 * requirements of the `SpatialIndex` interface, which is only possible when
 * `ItemType` is a pointer.
 *
 * Described in: P. Rigaux, Michel Scholl and Agnes Voisard. Spatial
 * Databases With Application To GIS. Morgan Kaufmann, San Francisco, 2002.
 *
 */
template<typename ItemType, typename BoundsTraits>
class TemplateSTRtreeImpl {
public:
    using Node = TemplateSTRNode<ItemType, BoundsTraits>;
    using NodeList = std::vector<Node>;
    using NodeListIterator = typename NodeList::iterator;
    using BoundsType = typename BoundsTraits::BoundsType;

    class Iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = ItemType;
        using difference_type = typename NodeList::const_iterator::difference_type;
        using pointer = ItemType*;
        using reference = ItemType&;

        Iterator(typename NodeList::const_iterator&& iter,
                 typename NodeList::const_iterator&& end) : m_iter(iter), m_end(end) {
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
            while(m_iter != m_end && m_iter->isDeleted()) {
                m_iter++;
            }
        }

        typename NodeList::const_iterator m_iter;
        typename NodeList::const_iterator m_end;
    };

    class Items {
    public:
        explicit Items(TemplateSTRtreeImpl& tree) : m_tree(tree) {}

        Iterator begin() {
            return Iterator(m_tree.nodes.cbegin(),
                            std::next(m_tree.nodes.cbegin(), static_cast<long>(m_tree.numItems)));
        }

        Iterator end() {
            return Iterator(std::next(m_tree.nodes.cbegin(), static_cast<long>(m_tree.numItems)),
                            std::next(m_tree.nodes.cbegin(), static_cast<long>(m_tree.numItems)));
        }
    private:
        TemplateSTRtreeImpl& m_tree;
    };

    /// \defgroup construct Constructors
    /// @{

    /**
     * Constructs a tree with the given maximum number of child nodes that
     * a node may have.
     */
    explicit TemplateSTRtreeImpl(size_t p_nodeCapacity = 10) :
        root(nullptr),
        nodeCapacity(p_nodeCapacity),
        numItems(0)
        {}

    /**
     * Constructs a tree with the given maximum number of child nodes that
     * a node may have, with the expected total number of items in the tree used
     * to pre-allocate storage.
     */
    TemplateSTRtreeImpl(size_t p_nodeCapacity, size_t itemCapacity) :
        root(nullptr),
        nodeCapacity(p_nodeCapacity),
        numItems(0) {
        auto finalSize = treeSize(itemCapacity);
        nodes.reserve(finalSize);
    }

    /**
     * Copy constructor, needed because mutex is not copyable
     */
    TemplateSTRtreeImpl(const TemplateSTRtreeImpl& other) :
        root(other.root),
        nodeCapacity(other.nodeCapacity),
        numItems(other.numItems) {
        nodes = other.nodes;
    }

    TemplateSTRtreeImpl& operator=(TemplateSTRtreeImpl other)
    {
        root = other.root;
        nodeCapacity = other.nodeCapacity;
        numItems = other.numItems;
        nodes = other.nodes;
        return *this;
    }

    /// @}
    /// \defgroup insert Insertion
    /// @{

    /** Move the given item into the tree */
    void insert(ItemType&& item) {
        insert(BoundsTraits::fromItem(item), std::forward<ItemType>(item));
    }

    /** Insert a copy of the given item into the tree */
    void insert(const ItemType& item) {
        insert(BoundsTraits::fromItem(item), item);
    }

    /** Move the given item into the tree */
    void insert(const BoundsType& itemEnv, ItemType&& item) {
        if (!BoundsTraits::isNull(itemEnv)) {
            createLeafNode(std::forward<ItemType>(item), itemEnv);
        }
    }

    /** Insert a copy of the given item into the tree */
    void insert(const BoundsType& itemEnv, const ItemType& item) {
        if (!BoundsTraits::isNull(itemEnv)) {
            createLeafNode(item, itemEnv);
        }
    }

    /// @}
    /// \defgroup NN Nearest-neighbor
    /// @{

    /** Determine the two closest items in the tree using distance metric `distance`. */
    template<typename ItemDistance>
    std::pair<ItemType, ItemType> nearestNeighbour(ItemDistance& distance) {
        return nearestNeighbour(*this, distance);
    }

    /** Determine the two closest items in the tree using distance metric `distance`. */
    template<typename ItemDistance>
    std::pair<ItemType, ItemType> nearestNeighbour() {
        ItemDistance id;
        return nearestNeighbour(*this);
    }

    /** Determine the two closest items this tree and `other` tree using distance metric `distance`. */
    template<typename ItemDistance>
    std::pair<ItemType, ItemType> nearestNeighbour(TemplateSTRtreeImpl<ItemType, BoundsTraits> & other,
                                                   ItemDistance & distance) {
        if (!getRoot() || !other.getRoot()) {
            return { nullptr, nullptr };
        }

        TemplateSTRtreeDistance<ItemType, BoundsTraits, ItemDistance> td(distance);
        return td.nearestNeighbour(*root, *other.root);
    }

    /** Determine the two closest items this tree and `other` tree using distance metric `distance`. */
    template<typename ItemDistance>
    std::pair<ItemType, ItemType> nearestNeighbour(TemplateSTRtreeImpl<ItemType, BoundsTraits>& other) {
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

    /// @}
    /// \defgroup query Query
    /// @{

    // Query the tree using the specified visitor. The visitor must be callable
    // either with a single argument of `const ItemType&` or with the
    // arguments `(const BoundsType&, const ItemType&).
    // The visitor need not return a value, but if it does return a value,
    // false values will be taken as a signal to stop the query.
    template<typename Visitor>
    void query(const BoundsType& queryEnv, Visitor &&visitor) {
        if (!built()) {
            build();
        }

        if (root && root->boundsIntersect(queryEnv)) {
            if (root->isLeaf()) {
                visitLeaf(visitor, *root);
            } else {
                query(queryEnv, *root, visitor);
            }
        }
    }

    // Query the tree and collect items in the provided vector.
    void query(const BoundsType& queryEnv, std::vector<ItemType>& results) {
        query(queryEnv, [&results](const ItemType& x) {
            results.push_back(x);
        });
    }

    /**
     * Returns a depth-first iterator over all items in the tree.
     */
    Items items() {
        build();
        return Items(*this);
    }

    /**
     * Iterate over all items added thus far.  Explicitly does not build
     * the tree.
     */
    template<typename F>
    void iterate(F&& func) {
        auto n = built() ? numItems : nodes.size();
        for (size_t i = 0; i < n; i++) {
            func(nodes[i].getItem());
        }
    }

    /// @}
    /// \defgroup remove Item removal
    /// @{

    bool remove(const BoundsType& itemEnv, const ItemType& item) {
        build();

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

    /// @}
    /// \defgroup introspect Introspection
    /// @{

    /** Determine whether the tree has been built, and no more items may be added. */
    bool built() const {
        return root != nullptr;
    }

    /** Determine whether the tree has been built, and no more items may be added. */
    const Node* getRoot() {
        build();
        return root;
    }

    /// @}

    /** Build the tree if it has not already been built. */
    void build() {
        std::lock_guard<std::mutex> lock(lock_);

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
        auto number = static_cast<size_t>(std::distance(begin, nodes.end()));

        while (number > 1) {
            createParentNodes(begin, number);
            std::advance(begin, static_cast<long>(number)); // parents just added become children in the next round
            number = static_cast<size_t>(std::distance(begin, nodes.end()));
        }

        assert(finalSize == nodes.size());

        root = &nodes.back();
    }

protected:
    std::mutex lock_;
    NodeList nodes;      //**< a list of all leaf and branch nodes in the tree. */
    Node* root;          //**< a pointer to the root node, if the tree has been built. */
    size_t nodeCapacity; //*< maximum number of children of each node */
    size_t numItems;     //*< total number of items in the tree, if it has been built. */

    // Prevent instantiation of base class.
    // ~TemplateSTRtreeImpl() = default;

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

    void createParentNodes(const NodeListIterator& begin, size_t number) {
        // Arrange child nodes in two dimensions.
        // First, divide them into vertical slices of a given size (left-to-right)
        // Then create nodes within those slices (bottom-to-top)
        auto numSlices = sliceCount(number);
        std::size_t nodesPerSlice = sliceCapacity(number, numSlices);

        // We could sort all of the nodes here, but we don't actually need them to be
        // completely sorted. They need to be sorted enough for each node to end up
        // in the right vertical slice, but their relative position within the slice
        // doesn't matter. So we do a partial sort for each slice below instead.
        auto end = begin + static_cast<long>(number);
        sortNodesX(begin, end);

        auto startOfSlice = begin;
        for (decltype(numSlices) j = 0; j < numSlices; j++) {
            // end iterator is being invalidated at each iteration
            end = begin + static_cast<long>(number);
            auto nodesRemaining = static_cast<size_t>(std::distance(startOfSlice, end));
            auto nodesInSlice = std::min(nodesRemaining, nodesPerSlice);
            auto endOfSlice = std::next(startOfSlice, static_cast<long>(nodesInSlice));

            // Make sure that every node that should be in this slice ends up somewhere
            // between startOfSlice and endOfSlice. We don't require any ordering among
            // nodes between startOfSlice and endOfSlice.
            //partialSortNodes(startOfSlice, endOfSlice, end);

            addParentNodesFromVerticalSlice(startOfSlice, endOfSlice);

            startOfSlice = endOfSlice;
        }
    }

    void addParentNodesFromVerticalSlice(const NodeListIterator& begin, const NodeListIterator& end) {
        if (BoundsTraits::TwoDimensional::value) {
            sortNodesY(begin, end);
        }

        // Arrange the nodes vertically and full up parent nodes sequentially until they're full.
        // A possible improvement would be to rework this such so that if we have 81 nodes we
        // put 9 into each parent instead of 10 or 1.
        auto firstChild = begin;
        while (firstChild != end) {
            auto childrenRemaining = static_cast<size_t>(std::distance(firstChild, end));
            auto childrenForNode = std::min(nodeCapacity, childrenRemaining);
            auto lastChild = std::next(firstChild, static_cast<long>(childrenForNode));

            //partialSortNodes(firstChild, lastChild, end);

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

    void sortNodesX(const NodeListIterator& begin, const NodeListIterator& end) {
        std::sort(begin, end, [](const Node &a, const Node &b) {
            return BoundsTraits::getX(a.getBounds()) < BoundsTraits::getX(b.getBounds());
        });
    }

    void sortNodesY(const NodeListIterator& begin, const NodeListIterator& end) {
        std::sort(begin, end, [](const Node &a, const Node &b) {
            return BoundsTraits::getY(a.getBounds()) < BoundsTraits::getY(b.getBounds());
        });
    }

    // Helper function to visit an item using a visitor that has no return value.
    // In this case, we will always return true, indicating that querying should
    // continue.
    template<typename Visitor,
            typename std::enable_if<std::is_void<decltype(std::declval<Visitor>()(std::declval<ItemType>()))>::value, std::nullptr_t>::type = nullptr >
    bool visitLeaf(Visitor&& visitor, const Node& node)
    {
        visitor(node.getItem());
        return true;
    }

    // MSVC 2015 does not implement C++11 expression SFINAE and considers this a
    // redefinition of a previous method
#if !defined(_MSC_VER) || _MSC_VER >= 1910
    template<typename Visitor,
             typename std::enable_if<std::is_void<decltype(std::declval<Visitor>()(std::declval<BoundsType>(), std::declval<ItemType>()))>::value, std::nullptr_t>::type = nullptr >
    bool visitLeaf(Visitor&& visitor, const Node& node)
    {
        visitor(node.getBounds(), node.getItem());
        return true;
    }
#endif

    // If the visitor function does return a value, we will use this to indicate
    // that querying should continue.
    template<typename Visitor,
             typename std::enable_if<!std::is_void<decltype(std::declval<Visitor>()(std::declval<ItemType>()))>::value, std::nullptr_t>::type = nullptr>
    bool visitLeaf(Visitor&& visitor, const Node& node)
    {
        return visitor(node.getItem());
    }

    // MSVC 2015 does not implement C++11 expression SFINAE and considers this a
    // redefinition of a previous method
#if !defined(_MSC_VER) || _MSC_VER >= 1910
    template<typename Visitor,
             typename std::enable_if<!std::is_void<decltype(std::declval<Visitor>()(std::declval<BoundsType>(), std::declval<ItemType>()))>::value, std::nullptr_t>::type = nullptr>
    bool visitLeaf(Visitor&& visitor, const Node& node)
    {
        return visitor(node.getBounds(), node.getItem());
    }
#endif

    template<typename Visitor>
    bool query(const BoundsType& queryEnv,
               const Node& node,
               Visitor&& visitor) {

        assert(!node.isLeaf());

        for (auto *child = node.beginChildren(); child < node.endChildren(); ++child) {
            if (child->boundsIntersect(queryEnv)) {
                if (child->isLeaf()) {
                    if (!child->isDeleted()) {
                        if (!visitLeaf(visitor, *child)) {
                            return false; // abort query
                        }
                    }
                } else {
                    if (!query(queryEnv, *child, visitor)) {
                        return false; // abort query
                    }
                }
            }
        }
        return true; // continue searching
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
};

struct EnvelopeTraits {
    using BoundsType = geom::Envelope;
    using TwoDimensional = std::true_type;

    static bool intersects(const BoundsType& a, const BoundsType& b) {
        return a.intersects(b);
    }

    static double size(const BoundsType& a) {
        return a.getArea();
    }

    static double distance(const BoundsType& a, const BoundsType& b) {
        return a.distance(b);
    }

    static BoundsType empty() {
        return {};
    }

    template<typename ItemType>
    static const BoundsType& fromItem(const ItemType& i) {
        return *(i->getEnvelopeInternal());
    }

    template<typename ItemType>
    static const BoundsType& fromItem(ItemType&& i) {
        return *(i->getEnvelopeInternal());
    }

    static double getX(const BoundsType& a) {
        return a.getMinX() + a.getMaxX();
    }

    static double getY(const BoundsType& a) {
        return a.getMinY() + a.getMaxY();
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
    using TwoDimensional = std::false_type;

    static bool intersects(const BoundsType& a, const BoundsType& b) {
        return a.intersects(&b);
    }

    static double size(const BoundsType& a) {
        return a.getWidth();
    }

    static double getX(const BoundsType& a) {
        return a.getMin() + a.getMax();
    }

    static double getY(const BoundsType& a) {
        return a.getMin() + a.getMax();
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
    void query(const geom::Envelope* queryEnv, std::vector<void*>& results) override {
        query(*queryEnv, [&results](const ItemType* x) {
            results.push_back(const_cast<void*>(static_cast<const void*>(x)));
        });
    }

    void query(const geom::Envelope* queryEnv, ItemVisitor& visitor) override {
        query(*queryEnv, [&visitor](const ItemType* x) {
            visitor.visitItem(const_cast<void*>(static_cast<const void*>(x)));
        });
    }

    bool remove(const geom::Envelope* itemEnv, void* item) override {
        return remove(*itemEnv, static_cast<ItemType*>(item));
    }

    void insert(const geom::Envelope* itemEnv, void* item) override {
        insert(*itemEnv, std::move(static_cast<ItemType*>(item)));
    }
};


}
}
}
