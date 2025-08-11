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

namespace geos::operation::grid {

struct CoordinateChain
{
    double start; // perimeter distance value of the first coordinate
    double stop;  // perimeter distance value of the last coordinate
    const std::vector<CoordinateXY>* coordinates;
    bool visited;

    CoordinateChain(double p_start, double p_stop, const std::vector<CoordinateXY>* p_coords)
      : start{ p_start }
      , stop{ p_stop }
      , coordinates{ p_coords }
      , visited{ false }
    {
    }
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

    for (CoordinateChain& candidate : chains) {
        if (candidate.visited && std::addressof(candidate) != kill) {
            continue;
        }

        double distance = exit_to_entry_perimeter_distance_ccw(*chain, candidate, perimeter);
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

    for (const auto& coords : coord_lists) {
        if (!hasMultipleUniqueCoordinates(*coords)) {
            continue;
        }

        if (coords->front() == coords->back() && hasMultipleUniqueCoordinates(*coords)) {
            // Closed ring. Check orientation.

            // TODO: Remove copy
            CoordinateSequence seq(0, false, false);
            seq.setPoints(*coords);
            bool is_ccw = algorithm::Orientation::isCCW(&seq);
            visitor(*coords, is_ccw);
        } else {
            double start = PerimeterDistance::getPerimeterDistance(box, coords->front());
            double stop = PerimeterDistance::getPerimeterDistance(box, coords->back());

            chains.emplace_back(start, stop, coords);
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
    chains.emplace_back(0.0, 0.0, &bottom_left);
    chains.emplace_back(height, height, &top_left);
    chains.emplace_back(height + width, height + width, &top_right);
    chains.emplace_back(2 * height + width, 2 * height + width, &bottom_right);

    std::vector<CoordinateXY> coords;
    for (auto& chain_ref : chains) {
        if (chain_ref.visited || chain_ref.coordinates->size() == 1) {
            continue;
        }

        coords.clear();

        CoordinateChain* chain = std::addressof(chain_ref);
        CoordinateChain* first_chain = chain;
        do {
            chain->visited = true;
            coords.insert(coords.end(), chain->coordinates->cbegin(), chain->coordinates->cend());
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

    visitRings(box, coord_lists, [&gfact, &shells, &holes, &found_a_ring](const std::vector<CoordinateXY>& coords, bool is_ccw) {
        found_a_ring = true;

        // finding a collapsed ring is sufficient to determine whether the cell interior is inside our outside,
        // but we don't want to actually construct the ring.
        if (algorithm::Area::ofRing(coords) == 0) {
            return;
        }

        CoordinateSequence seq(0, false, false);
        seq.setPoints(coords);
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
