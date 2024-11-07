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

#include <geos/operation/relateng/NodeSection.h>
#include <geos/export.h>
#include <vector>
#include <memory>

// Forward declarations
namespace geos {
namespace operation {
namespace relateng {
// class NodeSection;
}
}
}


// using geos::geom::CoordinateXY;
// using geos::geom::Geometry;


namespace geos {      // geos.
namespace operation { // geos.operation
namespace relateng {  // geos.operation.relateng

/**
 * Converts the node sections at a polygon node where
 * a shell and one or more holes touch, or two or more holes touch.
 * This converts the node topological structure from
 * the OGC "touching-rings" (AKA "minimal-ring") model to the equivalent "self-touch"
 * (AKA "inverted/exverted ring" or "maximal ring") model.
 * In the "self-touch" model the converted NodeSection corners enclose areas
 * which all lies inside the polygon
 * (i.e. they does not enclose hole edges).
 * This allows RelateNode to use simple area-additive semantics
 * for adding edges and propagating edge locations.
 *
 * The input node sections are assumed to have canonical orientation
 * (CW shells and CCW holes).
 * The arrangement of shells and holes must be topologically valid.
 * Specifically, the node sections must not cross or be collinear.
 *
 * This supports multiple shell-shell touches
 * (including ones containing holes), and hole-hole touches,
 * This generalizes the relate algorithm to support
 * both the OGC model and the self-touch model.
 *
 * @author Martin Davis
 * @see RelateNode
 */
class GEOS_DLL PolygonNodeConverter {
    using CoordinateXY = geos::geom::CoordinateXY;

public:

    /**
    * Converts a list of sections of valid polygon rings
    * to have "self-touching" structure.
    * There are the same number of output sections as input ones.
    *
    * @param polySections the original sections
    * @return the converted sections
    */
    static std::vector<std::unique_ptr<NodeSection>> convert(
        std::vector<const NodeSection*>& polySections);


private:

    static std::size_t convertShellAndHoles(
        std::vector<const NodeSection*>& sections,
        std::size_t shellIndex,
        std::vector<std::unique_ptr<NodeSection>>& convertedSections);

    static std::vector<std::unique_ptr<NodeSection>> convertHoles(
        std::vector<const NodeSection*>& sections);

    static NodeSection* createSection(
        const NodeSection* ns,
        const CoordinateXY* v0,
        const CoordinateXY* v1);

    static std::vector<const NodeSection*> extractUnique(
        std::vector<const NodeSection*>& sections);

    static std::size_t next(
        std::vector<const NodeSection *>& ns, std::size_t i);

    static std::size_t findShell(
        std::vector<const NodeSection *>& polySections);


};

} // namespace geos.operation.relateng
} // namespace geos.operation
} // namespace geos

