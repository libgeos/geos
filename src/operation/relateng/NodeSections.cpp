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

#include <geos/operation/relateng/NodeSections.h>
#include <geos/operation/relateng/RelateNode.h>
#include <geos/operation/relateng/PolygonNodeConverter.h>


using geos::geom::CoordinateXY;
using geos::geom::Geometry;


namespace geos {      // geos
namespace operation { // geos.operation
namespace relateng {  // geos.operation.relateng

/* public */
const CoordinateXY*
NodeSections::getCoordinate() const
{
    return nodePt;
}


/* public */
void
NodeSections::addNodeSection(NodeSection* e)
{
    //System.out.println(e);
    sections.emplace_back(e);
}


/* public */
bool
NodeSections::hasInteractionAB() const
{
    bool isA = false;
    bool isB = false;
    for (const std::unique_ptr<NodeSection>& ns : sections) {
        if (ns->isA())
            isA = true;
        else
            isB = true;

        if (isA && isB)
            return true;
    }
    return false;
}


/* public */
const Geometry*
NodeSections::getPolygonal(bool isA) const
{
    for (const std::unique_ptr<NodeSection>& ns : sections) {
        if (ns->isA() == isA) {
            const Geometry* poly = ns->getPolygonal();
            if (poly != nullptr)
                return poly;
        }
    }
    return nullptr;
}


/* public */
std::unique_ptr<RelateNode>
NodeSections::createNode()
{
    prepareSections();

    std::unique_ptr<RelateNode> node(new RelateNode(nodePt));
    std::size_t i = 0;
    while (i < sections.size()) {
        const std::unique_ptr<NodeSection>& ns = sections[i];
        //-- if there multiple polygon sections incident at node convert them to maximal-ring structure
        if (ns->isArea() && hasMultiplePolygonSections(sections, i)) {
            std::vector<const NodeSection*> polySections = collectPolygonSections(sections, i);
            std::vector<std::unique_ptr<NodeSection>> nsConvert = PolygonNodeConverter::convert(polySections);
            node->addEdges(nsConvert);
            i += polySections.size();
        }
        else {
            //-- the most common case is a line or a single polygon ring section
            node->addEdges(ns.get());
            i += 1;
        }
    }
    return node;
}


/* private */
void
NodeSections::prepareSections()
{
    // Comparator lambda for sort support
    auto comparator = [](
        const std::unique_ptr<NodeSection>& a,
        const std::unique_ptr<NodeSection>& b)
    {
        return a->compareTo(*b) < 0;
    };

    std::sort(sections.begin(), sections.end(), comparator);
    //TODO: remove duplicate sections
}


/* private static */
bool
NodeSections::hasMultiplePolygonSections(
    std::vector<std::unique_ptr<NodeSection>>& sections,
    std::size_t i)
{
    //-- if last section can only be one
    if (i >= sections.size() - 1)
        return false;
    //-- check if there are at least two sections for same polygon
    std::unique_ptr<NodeSection>& ns = sections[i];
    std::unique_ptr<NodeSection>& nsNext = sections[i + 1];
    return ns->isSamePolygon(*nsNext);
}


/* private static */
std::vector<const NodeSection*>
NodeSections::collectPolygonSections(
    std::vector<std::unique_ptr<NodeSection>>& sections,
    std::size_t i)
{
    std::vector<const NodeSection*> polySections;
    //-- note ids are only unique to a geometry
    std::unique_ptr<NodeSection>& polySection = sections[i];
    while (i < sections.size() &&
        polySection->isSamePolygon(*(sections[i])))
    {
        polySections.push_back(sections[i].get());
        i++;
    }
    return polySections;
}


} // namespace geos.operation.overlayng
} // namespace geos.operation
} // namespace geos


