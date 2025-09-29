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
#include <iomanip>
#include <optional>
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

#include "geos/operation/valid/RepeatedPointRemover.h"

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

static bool
containsProperly(const Envelope& e, const std::vector<CoordinateXY>& coords)
{
    return std::all_of(coords.begin(), coords.end(), [&e](const auto& c) {
        return e.containsProperly(c);
    });
}

static std::vector<CoordinateXY>
scrollRingToStartOnEdge(const Envelope& box, const std::vector<CoordinateXY>& coords)
{
    for (std::size_t i = 0; i < coords.size(); i++)
    {
        // Potentially this ring as an ordinary Traversal.
        if (!box.containsProperly(coords[i])) {
            // First, modify the ring so that its start point lies on the boundary.
            std::vector<CoordinateXY> mod;
            mod.reserve(coords.size());

            for (std::size_t j = i; j < coords.size(); j++) {
                if (mod.empty() || mod.back() != coords[j]) {
                    mod.push_back(coords[j]);
                }
            }
            for (std::size_t j = 0; j < i; j++) {
                if (mod.empty() || mod.back() != coords[j]) {
                    mod.push_back(coords[j]);
                }
            }
            mod.push_back(mod.front()); // close ring

            return mod;
        }
    }

    throw std::runtime_error("Ring does not touch box");
}

static bool
hasEdgeSegment(const Envelope& box, const std::vector<CoordinateXY>& coords)
{
    for (size_t i = 1; i < coords.size(); i++) {
        const CoordinateXY& p0 = coords[i - 1];
        const CoordinateXY& p1 = coords[i];

        if (p0.x == p1.x && (p0.x == box.getMinX() || p0.x == box.getMaxX())) {
            return true;
        }

        if (p0.y == p1.y && (p0.y == box.getMinY() || p0.y == box.getMaxY())) {
            return true;
        }
    }

    return false;
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
    std::deque<std::vector<CoordinateXY>> coordinateStore;

    std::vector<CoordinateXY> boxPoints;

    double areaCCW = 0;
    double areaCW = 0;
    std::optional<bool> isInterior;

    for (const auto& traversal : traversals) {
        if (!traversal->hasMultipleUniqueCoordinates()) {
            const auto& coords = traversal->getCoordinates();
            double m = PerimeterDistance::getPerimeterDistance(box, coords.front());
            chains.emplace_back(m, m, coords);
            boxPoints.push_back(coords.front());
            continue;
        }

        const std::vector<CoordinateXY>* coords = &traversal->getCoordinates();

        if (isRing(*coords)) {
            if (containsProperly(box, *coords) || !hasEdgeSegment(box, *coords)) {
                // TODO: Remove copy
                CoordinateSequence seq(0, false, false);
                seq.setPoints(*coords);
                const bool isCCW = algorithm::Orientation::isCCW(&seq);

                // Add nodes to box edges
                for (const auto& c : *coords) {
                    if (!box.containsProperly(c)) {
                        boxPoints.push_back(c);
                        coordinateStore.emplace_back(std::vector<CoordinateXY>{c});
                        double m = PerimeterDistance::getPerimeterDistance(box, c);
                        chains.emplace_back(m, m, coordinateStore.back());
                    }
                }

                constexpr bool hasMultipleParents = false;
                visitor(*coords, isCCW, hasMultipleParents);

                if (isCCW) {
                    areaCCW += algorithm::Area::ofRing(*coords);
                } else {
                    areaCW += algorithm::Area::ofRing(*coords);
                }

                continue;
            }
            // TODO add node on edge.

            auto modCoords = scrollRingToStartOnEdge(box, *coords);
            coordinateStore.push_back(std::move(modCoords));
            coords = &coordinateStore.back();
        }

        // Split coordinates into separate chains when they touch an edge
        // This prevents the creation of self-touching rings.
        // For area calculations, this doesn't matter, and the step can be skipped.
        size_t from = 0;
        size_t uniquePoints = 1;

        for (size_t to = 1; to < coords->size(); to++) {
            const CoordinateXY& prev = (*coords)[from];
            const CoordinateXY& c = (*coords)[to];

            if (!c.equals2D(prev)) {
                uniquePoints++;
            }

            const bool ptOnTop = c.y == box.getMaxY();
            const bool ptOnBottom = c.y == box.getMinY();
            const bool ptOnLeft = c.x == box.getMinX();
            const bool ptOnRight = c.x == box.getMaxX();

            if (ptOnTop || ptOnBottom || ptOnLeft || ptOnRight)  {
                bool isEdgeSegment = false;
                if (uniquePoints == 2) {
                    if (ptOnTop && prev.y == c.y) {
                        isEdgeSegment = true;
                        isInterior = c.x < prev.x;
                    } else if (ptOnBottom && prev.y == c.y) {
                        isEdgeSegment = true;
                        isInterior = c.x > prev.x;
                    } else if (ptOnLeft && prev.x == c.x) {
                        isEdgeSegment = true;
                        isInterior = c.y < prev.y;
                    } else if (ptOnRight && prev.x == c.x) {
                        isEdgeSegment = true;
                        isInterior = c.y > prev.y;
                    }
                }

                if (isEdgeSegment) {
                    //if (isInterior) {
                        boxPoints.push_back(c);
                        boxPoints.push_back(prev);
                    //}
                    coordinateStore.emplace_back(std::vector<CoordinateXY>{c});
                    double m = PerimeterDistance::getPerimeterDistance(box, c);
                    chains.emplace_back(m, m, coordinateStore.back());

                    coordinateStore.emplace_back(std::vector<CoordinateXY>{prev});
                    m = PerimeterDistance::getPerimeterDistance(box, prev);
                    chains.emplace_back(m, m, coordinateStore.back());

                } else {
                    double start = PerimeterDistance::getPerimeterDistance(box, (*coords)[from]);
                    double stop = PerimeterDistance::getPerimeterDistance(box, (*coords)[to]);
                    chains.emplace_back(start, stop, *coords, from, to, traversal->getParentage());
                }
                from = to;
                uniquePoints = 1;
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
    std::cout << std::setprecision(1);
    std::cout << "Identifying rings in box " << box.getMinX() << ":" << box.getMaxX() << "," << box.getMinY() << ":" << box.getMaxY() << std::endl;
    std::cout << std::setprecision(18);
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
#if DEBUG_TRAVERSAL_AREAS
        std::cout << "Completed chain ";
        for (const auto& c : coords) {
            std::cout << c << ", ";
        }
        std::cout<< std::endl;
#endif

        if (hasMultipleUniqueCoordinates(coords)) {
            visitor(coords, isCCW, hasMultipleParents);
            double area = algorithm::Area::ofRing(coords);
            areaCCW += area;
            if (area == 0) {
                isInterior = false;
            }
        }
    }

    if (areaCCW == 0 && areaCW == 0 && !isInterior.has_value()) {
        throw util::GEOSException("Cannot determine coverage fraction (it is either 0 or 100%)");
    }

    if (areaCW > areaCCW || (areaCCW == 0 && areaCW == 0 && isInterior.value())) {
        bool sortIsNeeded = !boxPoints.empty();

        boxPoints.emplace_back(box.getMinX(), box.getMinY());
        boxPoints.emplace_back(box.getMaxX(), box.getMinY());
        boxPoints.emplace_back(box.getMaxX(), box.getMaxY());
        boxPoints.emplace_back(box.getMinX(), box.getMaxY());

        if (sortIsNeeded) {
            std::sort(boxPoints.begin(), boxPoints.end(), [&box](const CoordinateXY& lhs, const CoordinateXY& rhs) {
                return PerimeterDistance::getPerimeterDistance(box, lhs) <
                    PerimeterDistance::getPerimeterDistance(box, rhs);
            });
        }

        boxPoints.push_back(boxPoints.front());

        visitor(boxPoints, true, false);
    }
}

double
TraversalAreas::getLeftHandArea(const Envelope& box, const std::vector<const Traversal*>& coord_lists)
{
    double ccw_sum = 0;
    double cw_sum = 0;

    visitRings(box, coord_lists, [&cw_sum, &ccw_sum](const std::vector<CoordinateXY>& coords, bool isCCW, bool) {
        if (isCCW) {
            ccw_sum += algorithm::Area::ofRing(coords);
        } else {
            cw_sum += algorithm::Area::ofRing(coords);
        }
    });

    return ccw_sum - cw_sum;
}

std::unique_ptr<Geometry>
TraversalAreas::getLeftHandRings(const GeometryFactory& gfact, const Envelope& box, const std::vector<const Traversal*>& coord_lists) {
    using geom::LinearRing;

    std::vector<std::unique_ptr<LinearRing>> shells;
    std::vector<std::unique_ptr<LinearRing>> holes;

    bool checkValidity = false;

    visitRings(box, coord_lists, [&gfact, &shells, &holes, &checkValidity](const std::vector<CoordinateXY>& coords, bool isCCW, bool hasMultipleParents) {
        // If a given ring was created from traversals from both a ring and shell, for example, it is possible for
        // the ring to self-intersect. Rather than try and detect self-intersections o the fly (rare?) we check and
        // repair validity in this limited case.
        checkValidity |= hasMultipleParents;

        // finding a collapsed ring is sufficient to determine whether the cell interior is inside or outside,
        // but we don't want to actually construct the ring.
        if (algorithm::Area::ofRing(coords) == 0) {
            return;
        }

        auto seq = std::make_unique<CoordinateSequence>(0, false, false);
        seq->reserve(coords.size());
        seq->setPoints(coords);
        if (seq->hasRepeatedPoints()) {
            seq = valid::RepeatedPointRemover::removeRepeatedPoints(seq.get());
        }
        //for (const auto& coord : coords) {
        //    seq->add(coord, false);
        //}
        auto ring = gfact.createLinearRing(std::move(seq));

        if (isCCW) {
            shells.push_back(std::move(ring));
        } else {
            holes.push_back(std::move(ring));
        }

    });

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
