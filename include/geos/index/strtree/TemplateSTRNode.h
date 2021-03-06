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

template<typename ItemType, typename BoundsTraits>
class TemplateSTRNode {
private:
    using BoundsType = typename BoundsTraits::BoundsType;

    BoundsType bounds;

    union Body {
        ItemType item;
        const TemplateSTRNode* childrenEnd;

        explicit Body (ItemType&& p_item) : item(std::forward<ItemType>(p_item)) {}

        explicit Body (const ItemType& p_item) : item(p_item) {}

        explicit Body (const TemplateSTRNode<ItemType, BoundsTraits>* p_childrenEnd) : childrenEnd(p_childrenEnd) {}

        ~Body() = default;
    } data;
    const TemplateSTRNode* children;

public:
    ~TemplateSTRNode() {
        if (isLeaf()) {
            data.item.~ItemType();
        }
    }

    TemplateSTRNode(ItemType&& p_item, const BoundsType& env) :
        bounds(env),
        data(std::forward<ItemType>(p_item)),
        children(nullptr)
        {}

    TemplateSTRNode(const ItemType& p_item, const BoundsType& env) :
            bounds(env),
            data(p_item),
            children(nullptr)
            {}

    TemplateSTRNode(const TemplateSTRNode* begin, const TemplateSTRNode* end) :
        bounds(boundsFromChildren(begin, end)),
        data(end),
        children(begin)
    {}

    const TemplateSTRNode* beginChildren() const {
        return children;
    }

    const TemplateSTRNode* endChildren() const {
        return data.childrenEnd;
    }

    bool isDeleted() const {
        return children == this;
    }

    bool isLeaf() const {
        return children == nullptr || children == this;
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

    static BoundsType boundsFromChildren(const TemplateSTRNode* from, const TemplateSTRNode* to) {
        BoundsType bnds = from->getBounds();

        for (auto *child = from + 1; child < to; ++child) {
            BoundsTraits::expandToInclude(bnds, child->getBounds());
        }

        return bnds;
    }

    BoundsType boundsFromChildren() const {
        return boundsFromChildren(children, data.childrenEnd);
    }

    const BoundsType& getBounds() const {
        return bounds;
    }

    std::size_t getNumNodes() const
    {
        if (isLeaf()) {
            return isDeleted() ? 0 : 1;
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
            return isDeleted() ? 0 : 1;
        }

        std::size_t count = 0;
        for (const auto* child = beginChildren(); child != endChildren(); ++child) {
            count += child->getNumNodes();
        }
        return count;
    }

    const ItemType& getItem() const {
        assert(!isDeleted());
        return data.item;
    }

    void removeItem() {
        children = this;
    }

};

}
}
}

