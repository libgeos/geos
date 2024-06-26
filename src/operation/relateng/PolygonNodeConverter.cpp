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

#include <geos/operation/relateng/PolygonNodeConverter.h>

#include <geos/algorithm/PolygonNodeTopology.h>
// #include <geos/operation/relateng/RelateEdge.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/Dimension.h>
// #include <geos/geom/Position.h>
// #include <geos/io/WKTWriter.h>
#include <geos/constants.h>


using geos::algorithm::PolygonNodeTopology;
using geos::geom::CoordinateXY;
using geos::geom::Dimension;
// using geos::geom::Location;
// using geos::geom::Position;


namespace geos {      // geos
namespace operation { // geos.operation
namespace relateng {  // geos.operation.relateng


/* public static */
std::vector<std::unique_ptr<NodeSection>>
PolygonNodeConverter::convert(std::vector<const NodeSection*>& polySections)
{
    auto comparator = [](
        const NodeSection* ns1,
        const NodeSection* ns2)
    {
        return PolygonNodeTopology::compareAngle(
            ns1->nodePt(),
            ns1->getVertex(0),
            ns2->getVertex(0));
    };

    std::sort(polySections.begin(), polySections.end(), comparator);
    // polySections.sort(new NodeSection.EdgeAngleComparator());

    //TODO: move uniquing up to caller
    std::vector<const NodeSection*> sections = extractUnique(polySections);
    if (sections.size() == 1) {
        std::vector<std::unique_ptr<NodeSection>> nss;
        nss.emplace_back(new NodeSection(sections[0]));
        return nss;
    }

    //-- find shell section index
    std::size_t shellIndex = findShell(sections);
    if (shellIndex == INDEX_UNKNOWN) {
        return convertHoles(sections);
    }
    //-- at least one shell is present.  Handle multiple ones if present
    std::vector<std::unique_ptr<NodeSection>> convertedSections;
    std::size_t nextShellIndex = shellIndex;
    do {
        nextShellIndex = convertShellAndHoles(
            sections, nextShellIndex, convertedSections);
    } while (nextShellIndex != shellIndex);

    return convertedSections;
}


/* private static */
std::size_t
PolygonNodeConverter::convertShellAndHoles(
    std::vector<const NodeSection*>& sections,
    std::size_t shellIndex,
    std::vector<std::unique_ptr<NodeSection>>& convertedSections)
{
    const NodeSection* shellSection = sections[shellIndex];
    const CoordinateXY* inVertex = shellSection->getVertex(0);
    std::size_t i = next(sections, shellIndex);
    const NodeSection* holeSection = nullptr;
    while (! sections[i]->isShell()) {
        holeSection = sections[i];
        // Assert: holeSection.isShell() = false
        const CoordinateXY* outVertex = holeSection->getVertex(1);
        NodeSection* ns = createSection(shellSection, inVertex, outVertex);
        convertedSections.emplace_back(ns);
        inVertex = holeSection->getVertex(0);
        i = next(sections, i);
    }
    //-- create final section for corner from last hole to shell
    const CoordinateXY* outVertex = shellSection->getVertex(1);
    NodeSection* ns = createSection(shellSection, inVertex, outVertex);
    convertedSections.emplace_back(ns);
    return i;
}


/* private static */
std::vector<std::unique_ptr<NodeSection>>
PolygonNodeConverter::convertHoles(std::vector<const NodeSection*>& sections)
{
    std::vector<std::unique_ptr<NodeSection>> convertedSections;
    const NodeSection* copySection = sections[0];
    for (std::size_t i = 0; i < sections.size(); i++) {
        std::size_t inext = next(sections, i);
        const CoordinateXY* inVertex = sections[i]->getVertex(0);
        const CoordinateXY* outVertex = sections[inext]->getVertex(1);
        NodeSection* ns = createSection(copySection, inVertex, outVertex);
        convertedSections.emplace_back(ns);
    }
    return convertedSections;
}


/* private static */
NodeSection*
PolygonNodeConverter::createSection(
    const NodeSection* ns,
    const CoordinateXY* v0,
    const CoordinateXY* v1)
{
    return new NodeSection(
        ns->isA(),
        Dimension::A,
        ns->id(), 0,
        ns->getPolygonal(),
        ns->isNodeAtVertex(),
        v0, ns->nodePt(), v1);
}



/* private static */
std::vector<const NodeSection*>
PolygonNodeConverter::extractUnique(std::vector<const NodeSection*>& sections)
{
    std::vector<const NodeSection*> uniqueSections;
    const NodeSection* lastUnique = sections[0];
    uniqueSections.push_back(lastUnique);
    for (const NodeSection* ns : sections) {
        if (0 != lastUnique->compareTo(ns)) {
            uniqueSections.push_back(ns);
            lastUnique = ns;
        }
    }
    return uniqueSections;
}


/* private static */
std::size_t
PolygonNodeConverter::next(std::vector<const NodeSection *>& ns, std::size_t i)
{
    std::size_t next = i;
    if (next == INDEX_UNKNOWN)
        next = 0;
    else
        next = i + 1;

    if (next >= ns.size())
        next = 0;

    return next;
}


/* private static */
std::size_t
PolygonNodeConverter::findShell(std::vector<const NodeSection *>& polySections)
{
    for (std::size_t i = 0; i < polySections.size(); i++) {
        if (polySections[i]->isShell())
            return i;
    }
    return INDEX_UNKNOWN;
}


} // namespace geos.operation.overlayng
} // namespace geos.operation
} // namespace geos

