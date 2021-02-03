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

namespace geos {
namespace index {
namespace strtree {

template<typename X>
struct BoundsTraits {


    static bool intersects(const X& a, const X& b);

    static double size(const X& a);

    static double distance(const X& a, const X& b);

    static X empty();

    double getX(const X&);

    double getY(const X&);

    static void expandToInclude(X& a, const X& b);
};


template<typename ItemType, typename BoundsTraits>
class TemplateSTRNode {
private:
    using BoundsType = typename BoundsTraits::BoundsType;

    union Body {
        ItemType item;
        const TemplateSTRNode* childrenEnd;

        explicit Body (ItemType&& p_item) : item(std::forward<ItemType>(p_item)) {}

        explicit Body (const ItemType& p_item) : item(p_item) {}

        explicit Body (const TemplateSTRNode<ItemType, BoundsTraits>* p_childrenEnd) : childrenEnd(p_childrenEnd) {}

        ~Body() {
            // destruction handled by Node
            // FIXME todo
        }
    } data;
    const TemplateSTRNode* children;

    BoundsType bounds;

    double sortVal = std::numeric_limits<double>::quiet_NaN();

    bool deleted;

public:
    //TemplateSTRNode() = delete;

    TemplateSTRNode(ItemType&& p_item, const BoundsType& env) :
        data(std::forward<ItemType>(p_item)),
        children(nullptr),
        bounds(env),
        deleted(false) {}

    TemplateSTRNode(const ItemType& p_item, const BoundsType& env) :
            data(p_item),
            children(nullptr),
            bounds(env),
            deleted(false) {}

    TemplateSTRNode(const TemplateSTRNode* begin, const TemplateSTRNode* end) :
        data(end),
        children(begin),
        bounds(boundsFromChildren()),
        deleted(false)
    {}

    void setSortVal(double d) {
        sortVal = d;
    }

    double getSortVal() const {
        return sortVal;
    }

    const TemplateSTRNode* beginChildren() const {
        return children;
    }

    const TemplateSTRNode* endChildren() const {
        return data.childrenEnd;
    }

    bool isDeleted() const {
        return deleted;
    }

    bool isLeaf() const {
        return children == nullptr;
    }

    bool isComposite() const {
        return !isLeaf();
    }

    bool boundsIntersect(const BoundsType& queryBounds) const {
        return BoundsTraits::intersects(getBounds(), queryBounds);
    }

    double getSize() const {
        return BoundsTraits::size(getBounds());
    }

    BoundsType boundsFromChildren() {
        BoundsType bnds = children->getBounds();

        for (auto *child = children + 1; child < data.childrenEnd; ++child) {
            BoundsTraits::expandToInclude(bnds, child->getBounds());
        }

        return bnds;
    }

    const BoundsType& getBounds() const {
        return bounds;
    }

    std::size_t getNumNodes() const
    {
        if (isLeaf()) {
            return deleted ? 0 : 1;
        }

        std::size_t count = 1;
        for (const auto* child = beginChildren(); child != endChildren(); ++child) {
            count += child->getNumNodes();
        }

        return count;
    }

    std::size_t getNumLeafNodes() const
    {
        if (isLeaf()) {
            return deleted ? 0 : 1;
        }

        std::size_t count = 0;
        for (const auto* child = beginChildren(); child != endChildren(); ++child) {
            count += child->getNumNodes();
        }
        return count;
    }

    const ItemType& getItem() const {
        assert(!deleted);
        return data.item;
    }

    void removeItem() {
        deleted = true;
    }

};

}
}
}

