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

#include <geos/index/strtree/TemplateSTRNode.h>

namespace geos {
namespace index {
namespace strtree {

template<typename ItemType, typename ItemDistance>
class TemplateSTRNodePair {
public:
    using Node = TemplateSTRNode<ItemType>;

    TemplateSTRNodePair(const Node &node1, const Node &node2)
            : m_node1(&node1), m_node2(&node2), m_distance(distance()) {}

    bool isLeaves() const {
        return getFirst().isLeaf() && getSecond().isLeaf();
    }

    double getDistance() const {
        return m_distance;
    }

    double maximumDistance() const {
        return EnvelopeUtil::maximumDistance(getFirst().getEnvelope(), getSecond().getEnvelope());
    }

    std::pair<const ItemType *, const ItemType *> getItems() const {
        assert(isLeaves());
        return std::make_pair(getFirst().getItem(), getSecond().getItem());
    }

    const Node &getFirst() const {
        return *m_node1;
    }

    const Node &getSecond() const {
        return *m_node2;
    }

    double distance() {
        if (isLeaves()) {
            // FIXME do we really need to instantiate ItemDistance?
            return ItemDistance()(getFirst().getItem(), getSecond().getItem());
        } else {
            return getFirst().getEnvelope().distance(getSecond().getEnvelope());
        }
    }

private:
    const Node* m_node1;
    const Node* m_node2;
    double m_distance;
};

}
}
}

