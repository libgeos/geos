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
#include <geos/operation/grid/Traversal.h>
#include <geos/geom/util/GeometryFixer.h>

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
    const void* parentage;
    bool visited;

    CoordinateChain(double p_start, double p_stop, const std::vector<CoordinateXY>& p_coords)
      : start{ p_start }
      , stop{ p_stop }
      , parentage{ nullptr }
      , visited{ false }
      , m_start{ p_coords.begin() }
      , m_stop{ p_coords.end() }
    {
    }

    CoordinateChain(double p_start, double p_stop, const std::vector<CoordinateXY>& p_coords, size_t from, size_t to, const void* p_parentage)
      : start{ p_start }
    , stop{ p_stop }
    , parentage{ p_parentage }
    , visited{ false }
    , m_start{ std::next(p_coords.begin(), static_cast<std::ptrdiff_t>(from)) }
    , m_stop{ std::next(p_coords.begin(), static_cast<std::ptrdiff_t>(to + 1)) }
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
visitRings(const Envelope& box, const std::vector<const Traversal*>& traversals, F&& visitor)
{
    std::vector<CoordinateChain> chains;
    chains.reserve(traversals.size() + 4);

    for (const auto& traversal : traversals) {
        if (!traversal->hasMultipleUniqueCoordinates()) {
            continue;
        }

        const auto& coords = traversal->getCoordinates();

        if (isRing(coords)) {
            // Closed ring. Check orientation.

            // TODO: Remove copy
            CoordinateSequence seq(0, false, false);
            seq.setPoints(coords);
            const bool isCCW = algorithm::Orientation::isCCW(&seq);
            constexpr bool hasMultipleParents = false;
            visitor(coords, isCCW, hasMultipleParents);
        } else {
            // Split coordinates into separate chains when they touch an edge
            // This prevents the creation of self-touching rings.
            // For area calculations, this doesn't matter, and the step can be skipped.
            size_t from = 0;
            for (size_t to = 1; to < coords.size(); to++) {
                const CoordinateXY& c = coords[to];
                const bool ptIsOnEdge = c.x == box.getMinX() || c.x == box.getMaxX() || c.y == box.getMinY() || c.y == box.getMaxY();
                if (ptIsOnEdge)  {
                    double start = PerimeterDistance::getPerimeterDistance(box, coords[from]);
                    double stop = PerimeterDistance::getPerimeterDistance(box, coords[to]);
                    chains.emplace_back(start, stop, coords, from, to, traversal->getParentage());
                    from = to;
                }
            }
        }
    }

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
    std::cout << std::endl << std::fixed;
    std::cout << "Identifying rings in box " << box.getMinX() << "-" << box.getMaxX() << ", " << box.getMinY() << "-" << box.getMaxY() << std::endl;
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
        bool hasMultipleParents = false;
        constexpr bool isCCW = true;
        do {
            chain->visited = true;
            coords.insert(coords.end(), chain->begin(), chain->end());

            if (chain->parentage != nullptr && chain->parentage != first_chain->parentage) {
                hasMultipleParents = true;
            }

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
            visitor(coords, isCCW, hasMultipleParents);
        }
    }
}

double
TraversalAreas::getLeftHandArea(const Envelope& box, const std::vector<const Traversal*>& coord_lists)
{
    double ccw_sum = 0;
    double cw_sum = 0;
    bool found_a_ring = false;

    visitRings(box, coord_lists, [&cw_sum, &ccw_sum, &found_a_ring](const std::vector<CoordinateXY>& coords, bool isCCW, bool) {
        found_a_ring = true;

        if (isCCW) {
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
TraversalAreas::getLeftHandRings(const GeometryFactory& gfact, const Envelope& box, const std::vector<const Traversal*>& coord_lists) {
    using geom::LinearRing;

    std::vector<std::unique_ptr<LinearRing>> shells;
    std::vector<std::unique_ptr<LinearRing>> holes;

    bool foundARing = false;
    bool checkValidity = false;

    visitRings(box, coord_lists, [&gfact, &shells, &holes, &foundARing, &checkValidity](const std::vector<CoordinateXY>& coords, bool isCCW, bool hasMultipleParents) {
        // If a given ring was created from traversals from both a ring and shell, for example, it is possible for
        // the ring to self-intersect. Rather than try and detect self-intersections o the fly (rare?) we check and
        // repair validity in this limited case.
        checkValidity |= hasMultipleParents;
        foundARing = true;

        // finding a collapsed ring is sufficient to determine whether the cell interior is inside or outside,
        // but we don't want to actually construct the ring.
        if (algorithm::Area::ofRing(coords) == 0) {
            return;
        }

        auto seq = std::make_unique<CoordinateSequence>(0, false, false);
        seq->reserve(coords.size());
        for (const auto& coord : coords) {
            seq->add(coord, false);
        }
        auto ring = gfact.createLinearRing(std::move(seq));

        if (isCCW) {
            shells.push_back(std::move(ring));
        } else {
            holes.push_back(std::move(ring));
        }

    });

    if (!foundARing) {
        throw std::runtime_error("Cannot determine coverage fraction (it is either 0 or 100%)");
    }

#if DEBUG_TRAVERSAL_AREAS
    std::cout << "SHELLS" << std::endl;
    for (const auto& shell: shells) {
        std::cout << shell->toString() << std::endl;
    }
    std::cout << "HOLES" << std::endl;
    for (const auto& hole: holes) {
        std::cout << hole->toString() << std::endl;
    }
#endif

    if (shells.empty() && holes.empty()) {
        return gfact.createPolygon();
    }

    if (shells.empty() && !holes.empty()) {
        auto seq = std::make_unique<CoordinateSequence>(5, false, false);
        seq->setAt(CoordinateXY{box.getMinX(), box.getMinY()}, 0);
        seq->setAt(CoordinateXY{box.getMaxX(), box.getMinY()}, 1);
        seq->setAt(CoordinateXY{box.getMaxX(), box.getMaxY()}, 2);
        seq->setAt(CoordinateXY{box.getMinX(), box.getMaxY()}, 3);
        seq->setAt(CoordinateXY{box.getMinX(), box.getMinY()}, 4);

        shells.push_back(gfact.createLinearRing(std::move(seq)));
    }

    std::unique_ptr<Geometry> result;

    if (holes.empty()) {
        std::vector<std::unique_ptr<Geometry>> polygons;
        for (auto& shell : shells) {
            polygons.push_back(gfact.createPolygon(std::move(shell)));
        }

        if (polygons.size() == 1) {
            result = std::move(polygons.front());
        } else {
            result = gfact.createMultiPolygon(std::move(polygons));
        }
    } else if (shells.size() == 1) {
        result = gfact.createPolygon(std::move(shells.front()), std::move(holes));
    } else {
        polygonize::Polygonizer polygonizer(true);

        std::vector<std::unique_ptr<Geometry>> holder;

        for (const auto& shell : shells) {
            if (checkValidity && !shell->isSimple()) {
                holder.push_back(shell->Union());
                polygonizer.add(holder.back().get());
            } else {
                polygonizer.add(static_cast<const Geometry*>(shell.get()));
            }
        }
        for (const auto& hole : holes) {
            if (checkValidity && !hole->isSimple()) {
                holder.push_back(hole->Union());
                polygonizer.add(holder.back().get());
            } else {
                polygonizer.add(static_cast<const Geometry*>(hole.get()));
            }
        }

        auto polygonized = polygonizer.getPolygons();
        result = gfact.createMultiPolygon(std::move(polygonized));
        checkValidity = false;
    }

    if (checkValidity && !result->isValid()) {
        result = geom::util::GeometryFixer::fix(result.get());
    }

    return result;
}

}
