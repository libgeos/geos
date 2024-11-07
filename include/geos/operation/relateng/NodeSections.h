/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (c) 2024 Martin Davis
 * Copyright (C) 2024 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <vector>
#include <memory>
#include <geos/operation/relateng/NodeSection.h>
#include <geos/export.h>


// Forward declarations
namespace geos {
namespace operation {
namespace relateng {
class RelateNode;
// class NodeSection;
}
}
namespace geom {
    class CoordinateXY;
    class Geometry;
}
}

namespace geos {      // geos.
namespace operation { // geos.operation
namespace relateng { // geos.operation.relateng


class GEOS_DLL NodeSections {
    using CoordinateXY = geos::geom::CoordinateXY;
    using Geometry = geos::geom::Geometry;

private:

    // Members
    const CoordinateXY* nodePt;
    std::vector<std::unique_ptr<NodeSection>> sections;

    // Methods

    /**
    * Sorts the sections so that:
    *  * lines are before areas
    *  * edges from the same polygon are contiguous
    */
    void prepareSections();

    static bool hasMultiplePolygonSections(
        std::vector<std::unique_ptr<NodeSection>>& sections,
        std::size_t i);

    static std::vector<const NodeSection*> collectPolygonSections(
        std::vector<std::unique_ptr<NodeSection>>& sections,
        std::size_t i);


public:

    NodeSections(const CoordinateXY* pt)
        : nodePt(pt)
        {};

    const CoordinateXY* getCoordinate() const;

    void addNodeSection(NodeSection* e);

    bool hasInteractionAB() const;

    const Geometry* getPolygonal(bool isA) const;

    std::unique_ptr<RelateNode> createNode();

    /**
     * Disable copy construction and assignment. Apparently needed to make this
     * class compile under MSVC. (See https://stackoverflow.com/q/29565299)
     */
    NodeSections(const NodeSections&) = delete;
    NodeSections& operator=(const NodeSections&) = delete;

};

} // namespace geos.operation.relateng
} // namespace geos.operation
} // namespace geos

