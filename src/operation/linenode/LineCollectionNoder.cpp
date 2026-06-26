/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2025 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/operation/linenode/LineCollectionNoder.h>

#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryComponentFilter.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/LineString.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/noding/IteratedNoder.h>
#include <geos/noding/NodedSegmentString.h>
#include <geos/noding/Noder.h>
#include <geos/noding/OrientedCoordinateArray.h>
#include <geos/noding/SegmentString.h>
#include <geos/noding/snapround/SnapRoundingNoder.h>

#include <memory>
#include <set>
#include <vector>

using geos::geom::Geometry;
using geos::geom::GeometryComponentFilter;
using geos::geom::GeometryFactory;
using geos::geom::LinearRing;
using geos::geom::LineString;
using geos::geom::PrecisionModel;
using geos::noding::IteratedNoder;
using geos::noding::NodedSegmentString;
using geos::noding::Noder;
using geos::noding::EdgeDeduplicator;
using geos::noding::SegmentString;
using geos::noding::snapround::SnapRoundingNoder;

namespace geos {
namespace operation {
namespace linenode {

namespace {

class LineStringExtractor : public GeometryComponentFilter {
public:
    LineStringExtractor(std::size_t p_index,
                        std::vector<std::unique_ptr<NodedSegmentString>>& p_segs)
        : m_index(p_index)
        , m_segs(p_segs)
    {}

    void filter_ro(const Geometry* g) override
    {
        // Skip polygon rings — only standalone LineStrings
        if (dynamic_cast<const LinearRing*>(g)) return;
        const auto* ls = dynamic_cast<const LineString*>(g);
        if (!ls || ls->isEmpty()) return;

        auto coords = ls->getSharedCoordinates();
        auto nss = std::make_unique<NodedSegmentString>(
            coords,
            ls->hasZ(),
            ls->hasM(),
            reinterpret_cast<const void*>(static_cast<uintptr_t>(m_index)));
        m_segs.push_back(std::move(nss));
    }

private:
    std::size_t m_index;
    std::vector<std::unique_ptr<NodedSegmentString>>& m_segs;
};

} // anonymous namespace


LineCollectionNoder::LineCollectionNoder(const std::vector<const Geometry*>& collection)
    : m_input(collection)
    , m_geomFactory(collection.empty() ? nullptr : collection[0]->getFactory())
{}


/* static */
void
LineCollectionNoder::extractSegments(
    const Geometry& g,
    std::size_t index,
    std::vector<std::unique_ptr<NodedSegmentString>>& segments)
{
    LineStringExtractor ex(index, segments);
    g.apply_ro(&ex);
}


/* private */
std::unique_ptr<Geometry>
LineCollectionNoder::buildResult(
    const std::vector<std::unique_ptr<SegmentString>>& nodedSegs,
    std::size_t index) const
{
    const void* tag = reinterpret_cast<const void*>(static_cast<uintptr_t>(index));

    EdgeDeduplicator dedup;
    std::vector<std::unique_ptr<Geometry>> lines;

    for (const auto& ss : nodedSegs) {
        if (ss->getData() != tag) continue;
        const auto& coords = ss->getCoordinates();
        if (dedup.add(*coords)) {
            lines.push_back(m_geomFactory->createLineString(coords));
        }
    }

    return m_geomFactory->createMultiLineString(std::move(lines));
}


/* public */
std::vector<std::unique_ptr<Geometry>>
LineCollectionNoder::node(double gridSize)
{
    const std::size_t n = m_input.size();
    std::vector<std::unique_ptr<Geometry>> result;
    result.reserve(n);

    if (n == 0) return result;

    // Extract all LinearString segments from all inputs, tagged with source index
    std::vector<std::unique_ptr<NodedSegmentString>> segments;
    for (std::size_t i = 0; i < n; ++i) {
        extractSegments(*m_input[i], i, segments);
    }

    if (segments.empty()) {
        for (std::size_t i = 0; i < n; ++i)
            result.push_back(m_geomFactory->createMultiLineString());
        return result;
    }

    // Build raw pointer vector for noder API
    std::vector<SegmentString*> rawSegs;
    rawSegs.reserve(segments.size());
    for (auto& ss : segments)
        rawSegs.push_back(ss.get());

    // Select noder based on gridSize
    std::unique_ptr<PrecisionModel> pm;
    std::unique_ptr<Noder> noder;
    if (gridSize > 0.0) {
        pm = std::make_unique<PrecisionModel>(1.0 / gridSize);
        noder = std::make_unique<SnapRoundingNoder>(pm.get());
    } else {
        noder = std::make_unique<IteratedNoder>(m_geomFactory->getPrecisionModel());
    }

    noder->computeNodes(rawSegs);
    auto nodedSegs = noder->getNodedSubstrings();

    // Build one noded MultiLineString per input geometry
    for (std::size_t i = 0; i < n; ++i)
        result.push_back(buildResult(nodedSegs, i));

    return result;
}


/* static */
std::vector<std::unique_ptr<Geometry>>
LineCollectionNoder::node(
    std::vector<const Geometry*>& collection,
    double gridSize)
{
    LineCollectionNoder lcn(collection);
    return lcn.node(gridSize);
}


/* static */
std::vector<std::unique_ptr<Geometry>>
LineCollectionNoder::node(
    const std::vector<std::unique_ptr<Geometry>>& collection,
    double gridSize)
{
    std::vector<const Geometry*> raw;
    raw.reserve(collection.size());
    for (const auto& g : collection)
        raw.push_back(g.get());
    LineCollectionNoder lcn(raw);
    return lcn.node(gridSize);
}

} // geos::operation::linenode
} // geos::operation
} // geos
