/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2018-2025 ISciences, LLC
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <limits>
#include <vector>

#include <geos/algorithm/Area.h>
#include <geos/algorithm/Orientation.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/operation/grid/PerimeterDistance.h>

#include <geos/operation/polygonize/Polygonizer.h>
#include <geos/operation/grid/TraversalAreas.h>

using geos::geom::CoordinateSequence;
using geos::geom::CoordinateXY;
using geos::geom::Geometry;
using geos::geom::GeometryFactory;
using geos::geom::Envelope;

#define DEBUG_TRAVERSAL_AREAS 0

namespace geos::operation::grid {

/// A CoordinateChain stores a set of coordinates whose start and end points lie on
/// the envelope.
struct CoordinateChain
{
    const double start; // perimeter distance value of the first coordinate
    const double stop;  // perimeter distance value of the last coordinate
    bool visited;

    CoordinateChain(double p_start, double p_stop, const std::vector<CoordinateXY>& p_coords)
      : start{ p_start }
      , stop{ p_stop }
      , visited{ false }
      , m_start{ p_coords.begin() }
      , m_stop{ p_coords.end() }
    {
    }

    CoordinateChain(double p_start, double p_stop, const std::vector<CoordinateXY>& p_coords, size_t from, size_t to)
      : start{ p_start }
    , stop{ p_stop }
    , visited{ false }
    , m_start{ std::next(p_coords.begin(), static_cast<std::ptrdiff_t>(from)) }
    , m_stop{ std::next(p_coords.begin(), static_cast<std::ptrdiff_t>(to + 1)) }
    {
    }

    CoordinateChain(double pDistStart, double pDistStop,
        const std::vector<CoordinateXY>::const_iterator& itStart,
        const std::vector<CoordinateXY>::const_iterator& itStop)
      : start{ pDistStart }
    , stop{ pDistStop }
    , visited{ false }
    , m_start{ itStart }
    , m_stop{ itStop}
    {
    }

    auto begin() const {
        return m_start;
    }

    auto end() const {
        return m_stop;
    }

    auto getSize() const {
        return m_stop - m_start;
    }

#if 0
    /// Tests whether this chain intersects the interior of the box.
    bool crossesInterior(const Envelope& box) const
    {
        if (getSize() < 2) {
            return false;
        }

        const CoordinateXY& origin = *begin();
        for (const auto& coord: *this) {
            if (coord.x != origin.x && coord.y != origin.y) {
                // Both X and Y have changed, we cannot be on the
                // edge where we started.
                return true;
            }
            if (box.containsProperly(coord)) {
                return true;
            }
        }

        return false;
    }
#endif

private:
    const std::vector<CoordinateXY>::const_iterator m_start;
    const std::vector<CoordinateXY>::const_iterator m_stop;
};

static double
exit_to_entry_perimeter_distance_ccw(const CoordinateChain& c1, const CoordinateChain& c2, double perimeter)
{
    return PerimeterDistance::getPerimeterDistanceCCW(c1.stop, c2.start, perimeter);
}

static CoordinateChain*
getNextChain(std::vector<CoordinateChain>& chains,
             const CoordinateChain* chain,
             const CoordinateChain* kill,
             double perimeter)
{

    CoordinateChain* min = nullptr;
    double min_distance = std::numeric_limits<double>::max();

#if DEBUG_TRAVERSAL_AREAS
    std::cout << "Acceptable range: " << chain->stop << " " << kill->start << std::endl;
#endif

    for (CoordinateChain& candidate : chains) {
        if (candidate.visited && std::addressof(candidate) != kill) {
            continue;
        }

        double distance = exit_to_entry_perimeter_distance_ccw(*chain, candidate, perimeter);

#if DEBUG_TRAVERSAL_AREAS
        std::cout << "Distance " << distance << ", start " << candidate.start << ", stop " << candidate.stop << ": ";
        for (const auto& c : candidate) {
            std::cout << c << ", ";
        }
        std::cout<< std::endl;
#endif

        if (distance == 0 && std::addressof(candidate) != kill && !PerimeterDistance::isBetweenCCW(candidate.stop, chain->stop, kill->start)) {
           // Make sure the candidate chain closes the ring instead of starting a new ring.
            continue;
        }

        if (distance < min_distance) {
            min_distance = distance;
            min = std::addressof(candidate);
        }
    }

    if (min == nullptr) {
        throw std::runtime_error("Failed to find next CoordinateChain");
    }

    return min;
}

template<typename T>
bool
hasMultipleUniqueCoordinates(const T& vec)
{
    for (std::size_t i = 1; i < vec.size(); i++) {
        if (vec[i] != vec[0]) {
            return true;
        }
    }

    return false;
}

static bool
isRing(const std::vector<CoordinateXY>& coords)
{
    return coords.front() == coords.back() && hasMultipleUniqueCoordinates(coords);
}

/**
 * @brief Identify counter-clockwise rings formed by the supplied coordinate vectors and the boundary of this box.
 *
 * @param box Box to be included in rings
 * @param coord_lists A list of coordinate vectors, with each vector representing a traversal of `box` or a closed ring.
 * @param visitor Function be applied to each ring identified. Because `coord_lists` may include both clockwise and
 *                counter-clockwise closed rings, `visitor` will be provided with the orientation of each ring as an
 *                argument.
 */
template<typename F>
void
visitRings(const Envelope& box, const std::vector<const std::vector<CoordinateXY>*>& coord_lists, F&& visitor)
{
    std::vector<CoordinateChain> chains;
    chains.reserve(coord_lists.size() + 4);

    bool validPolygons = true;

    for (const auto& coords : coord_lists) {
        if (!hasMultipleUniqueCoordinates(*coords)) {
            continue;
        }

        if (isRing(*coords)) {
            // Closed ring. Check orientation.

            // TODO: Remove copy
            CoordinateSequence seq(0, false, false);
            seq.setPoints(*coords);
            bool is_ccw = algorithm::Orientation::isCCW(&seq);
            visitor(*coords, is_ccw);
        } else if (validPolygons) {
            // Split coordinates into separate chains when they touch an edge
            // This prevents the creation of self-touching rings.
            // For area calculations, this doesn't matter, and the step can be skipped.
            size_t from = 0;
            for (size_t to = 1; to < coords->size(); to++) {
                const CoordinateXY& c = (*coords)[to];
                const bool ptIsOnEdge = c.x == box.getMinX() || c.x == box.getMaxX() || c.y == box.getMinY() || c.y == box.getMaxY();
                if (ptIsOnEdge)  {
                    double start = PerimeterDistance::getPerimeterDistance(box, (*coords)[from]);
                    double stop = PerimeterDistance::getPerimeterDistance(box, (*coords)[to]);
                    chains.emplace_back(start, stop, *coords, from, to);
                    from = to;
                }
            }
        } else {
            auto from = coords->begin();
            auto to = coords->end();
            double start = PerimeterDistance::getPerimeterDistance(box, *from);
            double stop = PerimeterDistance::getPerimeterDistance(box, *std::prev(to));
            chains.emplace_back(start, stop, from, to);
        }
    }

#if 0
    if (validPolygons) {
        const bool crossesInterior = std::any_of(chains.begin(), chains.end(), [&box](const auto& chain) {
            return chain.crossesInterior(box);
        });

        if (crossesInterior) {
            for (auto& chain_ref : chains) {
                // Pre-visit all edge segments, and do nothing with them.
                if (chain_ref.getSize() == 2) {
                    const CoordinateXY& from = *chain_ref.begin();
                    const CoordinateXY& to = *std::next(chain_ref.begin());

                    if (from.x == to.x &&(from.x == box.getMinX() || from.x == box.getMaxX())) {
                        chain_ref.visited = true;
                    }
                    else if (from.y == to.y && (from.y == box.getMinY() || from.y == box.getMaxY())) {
                        chain_ref.visited = true;
                    }
                }
            }
        }
    }
#endif

    double height{ box.getHeight() };
    double width{ box.getWidth() };
    double perimeter{ box.getPerimeter() };

    // create coordinate lists for corners
    std::vector<CoordinateXY> bottom_left = { CoordinateXY(box.getMinX(), box.getMinY()) };
    std::vector<CoordinateXY> top_left = { CoordinateXY(box.getMinX(), box.getMaxY()) };
    std::vector<CoordinateXY> top_right = { CoordinateXY(box.getMaxX(), box.getMaxY()) };
    std::vector<CoordinateXY> bottom_right = { CoordinateXY(box.getMaxX(), box.getMinY()) };

    // Add chains for corners
    chains.emplace_back(0.0, 0.0, bottom_left);
    chains.emplace_back(height, height, top_left);
    chains.emplace_back(height + width, height + width, top_right);
    chains.emplace_back(2 * height + width, 2 * height + width, bottom_right);

    std::vector<CoordinateXY> coords;

#if DEBUG_TRAVERSAL_AREAS
    std::cout << "Identifying rings in box " << box << std::endl;
    std::cout << "Available chains:" << std::endl;
    for (const auto& chain : chains) {
        for (const auto& coord : chain) {
            std:: cout << coord << ", ";
        }
        std::cout << std::endl;
    }
#endif

    for (auto& chain_ref : chains) {
        if (chain_ref.visited || chain_ref.getSize() == 1) {
            continue;
        }

        coords.clear();
#if DEBUG_TRAVERSAL_AREAS
        std::cout << "New ring." << std::endl;
#endif
        CoordinateChain* chain = std::addressof(chain_ref);
        const CoordinateChain* first_chain = chain;
        do {
            chain->visited = true;
            coords.insert(coords.end(), chain->begin(), chain->end());

#if DEBUG_TRAVERSAL_AREAS
            std::cout << "Added chain ";
            for (const auto& c : *chain) {
                std::cout << c << ", ";
            }
            std::cout<< std::endl;
#endif

            chain = getNextChain(chains, chain, first_chain, perimeter);
        } while (chain != first_chain);

        coords.push_back(coords[0]);

        if (hasMultipleUniqueCoordinates(coords)) {
            visitor(coords, true);
        }
    }
}

double
TraversalAreas::getLeftHandArea(const Envelope& box, const std::vector<const std::vector<CoordinateXY>*>& coord_lists)
{
    double ccw_sum = 0;
    double cw_sum = 0;
    bool found_a_ring = false;

    visitRings(box, coord_lists, [&cw_sum, &ccw_sum, &found_a_ring](const std::vector<CoordinateXY>& coords, bool is_ccw) {
        found_a_ring = true;

        if (is_ccw) {
            ccw_sum += algorithm::Area::ofRing(coords);
        } else {
            cw_sum += algorithm::Area::ofRing(coords);
        }
    });

    if (!found_a_ring) {
        throw std::runtime_error("Cannot determine coverage fraction (it is either 0 or 100%)");
    }

    // If this box has only clockwise rings (holes) then the area
    // of those holes should be subtracted from the complete box area.
    if (ccw_sum < cw_sum) {
        ccw_sum += box.getArea();
    }

    return ccw_sum - cw_sum;
}

std::unique_ptr<Geometry>
TraversalAreas::getLeftHandRings(const GeometryFactory& gfact, const Envelope& box, const std::vector<const std::vector<CoordinateXY>*>& coord_lists)
{
    using geom::LinearRing;

    std::vector<std::unique_ptr<LinearRing>> shells;
    std::vector<std::unique_ptr<LinearRing>> holes;

    bool found_a_ring = false;

    visitRings(box, coord_lists, [&gfact, &box, &shells, &holes, &found_a_ring](const std::vector<CoordinateXY>& coords, bool is_ccw) {
        found_a_ring = true;

        // finding a collapsed ring is sufficient to determine whether the cell interior is inside or outside,
        // but we don't want to actually construct the ring.
        if (algorithm::Area::ofRing(coords) == 0) {
            return;
        }

        auto seq = std::make_unique<CoordinateSequence>(0, false, false);
        seq->reserve(coords.size());
        for (const auto& coord : coords) {
#if 0
            if (!seq.isEmpty()) {
                const CoordinateXY& prev = seq.back();

                const bool isHorizontal = coord.y == prev.y;
                const bool isVertical = coord.x == prev.x;

                // When an input geometry linework follows grid cell boundaries, we can end up
                // with duplicate points or zero-area spikes in rings. These don't affect the
                // area calculations, but we need to remove them before constructing the geometry.

                // Skip duplicate points
                if (isHorizontal && isVertical) {
                    continue;
                }

                // Skip rightward along top edge
                if (isHorizontal && coord.y == box.getMaxY() && coord.x > prev.x) {
                    continue;
                }
                // Skip leftward along bottom edge
                if (isHorizontal && coord.y == box.getMinY() && coord.x < prev.x) {
                    continue;
                }

                // Skip downward along right edge
                if (isVertical && coord.x == box.getMaxX() && coord.y < prev.y) {
                    continue;
                }
                // Skip upward along left edge
                if (isVertical && coord.x == box.getMinX() && coord.y > prev.y) {
                    continue;
                }
            }
#endif
            seq->add(coord, false);
        }
        auto ring = gfact.createLinearRing(std::move(seq));

        if (is_ccw) {
            shells.push_back(std::move(ring));
        } else {
            holes.push_back(std::move(ring));
        }
    });

    if (!found_a_ring) {
        throw std::runtime_error("Cannot determine coverage fraction (it is either 0 or 100%)");
    }

    if (shells.empty() && holes.empty()) {
        return gfact.createPolygon();
    }

    if (shells.empty() && !holes.empty()) {
        CoordinateSequence seq(5, false, false);
        seq.setAt(CoordinateXY{box.getMinX(), box.getMinY()}, 0);
        seq.setAt(CoordinateXY{box.getMaxX(), box.getMinY()}, 1);
        seq.setAt(CoordinateXY{box.getMaxX(), box.getMaxY()}, 2);
        seq.setAt(CoordinateXY{box.getMinX(), box.getMaxY()}, 3);
        seq.setAt(CoordinateXY{box.getMinX(), box.getMinY()}, 4);

        shells.push_back(gfact.createLinearRing(std::move(seq)));
    }

    if (holes.empty()) {
        std::vector<std::unique_ptr<Geometry>> polygons;
        for (auto& shell : shells) {
            polygons.push_back(gfact.createPolygon(std::move(shell)));
        }

        if (polygons.size() == 1) {
            return std::move(polygons.front());
        } else {
            return gfact.createMultiPolygon(std::move(polygons));
        }
    }

    if (shells.size() == 1) {
        return gfact.createPolygon(std::move(shells.front()), std::move(holes));
    }

    polygonize::Polygonizer polygonizer(true);

    for (const auto& shell : shells) {
      polygonizer.add(static_cast<const Geometry*>(shell.get()));
    }
    for (const auto& hole : holes) {
      polygonizer.add(static_cast<const Geometry*>(hole.get()));
    }

    auto polygonized = polygonizer.getPolygons();
    return gfact.createMultiPolygon(std::move(polygonized));
}

}
