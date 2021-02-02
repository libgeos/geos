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

template<typename ItemType>
class TemplateSTRNode {
private:
    union Body {
        ItemType item;
        const TemplateSTRNode* childrenEnd;

        explicit Body (ItemType&& p_item) : item(std::forward<ItemType>(p_item)) {}

        explicit Body (const ItemType& p_item) : item(p_item) {}

        explicit Body (const TemplateSTRNode<ItemType>* p_childrenEnd) : childrenEnd(p_childrenEnd) {}

        ~Body() {
            // destruction handled by Node
            // FIXME todo
        }
    } data;
    const TemplateSTRNode* children;

    geom::Envelope bounds;

    double sortVal = std::numeric_limits<double>::quiet_NaN();

    bool deleted;

public:
    //TemplateSTRNode() = delete;

    TemplateSTRNode(ItemType&& p_item, const geom::Envelope& env) :
        data(std::forward<ItemType>(p_item)),
        children(nullptr),
        bounds(env),
        deleted(false) {}

    TemplateSTRNode(const ItemType& p_item, const geom::Envelope& env) :
            data(p_item),
            children(nullptr),
            bounds(env),
            deleted(false) {}

    TemplateSTRNode(const TemplateSTRNode* begin, const TemplateSTRNode* end) :
        data(end),
        children(begin),
        deleted(false)
    {
        computeEnvelopeFromChildren();
    }

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

    bool envelopeIntersects(const geom::Envelope &queryEnv) const {
        return getEnvelope().intersects(queryEnv);
    }

    void computeEnvelopeFromChildren() {
        bounds.setToNull();
        for (auto *child = children; child < data.childrenEnd; ++child) {
            bounds.expandToInclude(child->getEnvelope());
        }
    }

    double getArea() const {
        return getEnvelope().getArea();
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

    const geom::Envelope& getEnvelope() const {
        return bounds;
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

