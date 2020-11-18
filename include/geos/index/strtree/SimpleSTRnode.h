/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2020 Paul Ramsey
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <cassert>
#include <cstddef>
#include <geos/export.h>
#include <geos/geom/Envelope.h>

#include <vector>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251) // warning C4251: needs to have dll-interface to be used by clients of class
#endif

namespace geos {
namespace index { // geos::index
namespace strtree { // geos::index::strtree

/** \brief
 * A node of the STR tree.
 *
 */
class GEOS_DLL SimpleSTRnode {

private:

    void *item;
    std::vector<SimpleSTRnode*> childNodes;
    geom::Envelope bounds;
    std::size_t level;

    void computeBounds();

public:

    /*
     * Constructs an AbstractNode at the given level in the tree
     */
    SimpleSTRnode(std::size_t newLevel, const geom::Envelope *itemEnv, void* item, size_t capacity = 10)
        : level(newLevel)
        , bounds()
        , item(item)
    {
        childNodes.reserve(capacity);
        if (itemEnv) {
            bounds = *itemEnv;
        }

    }

    SimpleSTRnode(std::size_t newLevel)
        : SimpleSTRnode(newLevel, nullptr, nullptr)
    {}

    void toString(std::ostream& os, int level) const;

    const std::vector<SimpleSTRnode*>&
    getChildNodes() const
    {
        return childNodes;
    }

    void* getItem() const {
        return item;
    }

    /**
     * Returns a representation of space that encloses this Node
     */
    const geom::Envelope& getBounds() {
        if(bounds.isNull()) {
            computeBounds();
        }
        return bounds;
    }

    /**
    * Returns 0 if this node is a leaf, 1 if a parent of a leaf, and so on; the
    * root node will have the highest level
    */
    std::size_t getLevel() const {
        return level;
    }

    std::size_t size() const {
        return childNodes.size();
    }

    /**
     * Adds either an AbstractNode, or if this is a leaf node, a data object
     * (wrapped in an ItemBoundable)
     */
    void addChildNode(SimpleSTRnode* childNode)
    {
        assert(bounds.isNull());
        childNodes.push_back(childNode);
    }

    bool isLeaf() const
    {
        return item != nullptr;
    }


};


} // namespace geos::index::strtree
} // namespace geos::index
} // namespace geos

#ifdef _MSC_VER
#pragma warning(pop)
#endif

