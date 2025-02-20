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

#include <stdexcept>

#include <geos/algorithm/Area.h>
#include <geos/algorithm/Length.h>
#include <geos/algorithm/Orientation.h>
#include <geos/operation/grid/Cell.h>
#include <geos/operation/grid/FloodFill.h>
#include <geos/operation/grid/GridIntersection.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/GeometryFactory.h>

#include "geos/operation/overlayng/CoverageUnion.h"

using geos::geom::Geometry;
using geos::geom::LineString;
using geos::geom::Polygon;
using geos::geom::Envelope;
using geos::geom::CoordinateXY;

namespace geos::operation::grid {

std::shared_ptr<Matrix<float>>
GridIntersection::grid_intersection(const Grid<bounded_extent>& raster_grid, const Geometry& g)
{
    GridIntersection rci(raster_grid, g);
    return rci.results();
}

std::shared_ptr<Matrix<float>>
GridIntersection::grid_intersection(const Grid<bounded_extent>& raster_grid, const Envelope& box)
{
    GridIntersection rci(raster_grid, box);
    return rci.results();
}

static Cell*
get_cell(Matrix<std::unique_ptr<Cell>>& cells, const Grid<infinite_extent>& ex, size_t row, size_t col)
{
    if (cells(row, col) == nullptr) {
        cells(row, col) = std::make_unique<Cell>(ex.grid_cell(row, col));
    }

    return cells(row, col).get();
}

Envelope
GridIntersection::processing_region(const Envelope& raster_extent, const Geometry& g)
{
    Envelope ret;

    auto ngeoms = g.getNumGeometries();
    for (size_t i = 0; i < ngeoms; i++) {
        if (ret == raster_extent) {
            // No more expansion is possible
            return ret;
        }

        const Envelope& box = *g.getGeometryN(i)->getEnvelopeInternal();

        Envelope isect = raster_extent.intersection(box);
        if (ret.isNull()) {
            ret = isect;
        } else if (!ret.contains(isect)) {
            ret.expandToInclude(isect);
        }
    }

    return ret;
}

GridIntersection::GridIntersection(const Grid<bounded_extent>& raster_grid, const Geometry& g, std::shared_ptr<Matrix<float>> cov)
  : m_geometry_grid{ make_infinite(raster_grid, *g.getEnvelopeInternal()) }
  , m_results{ cov ? cov : std::make_shared<Matrix<float>>(raster_grid.rows(), raster_grid.cols()) }
  , m_first_geom{ true }
  , m_areal{ false }
{
    if (g.getDimension() == 0) {
        throw std::invalid_argument("Unsupported geometry type.");
    }

    if (!m_geometry_grid.empty()) {
        process(g);
    }
}

GridIntersection::GridIntersection(const Grid<bounded_extent>& raster_grid, const Envelope& box, std::shared_ptr<Matrix<float>> cov)
  : m_geometry_grid{ make_infinite(raster_grid, box) }
  , m_results{ cov ? cov : std::make_shared<Matrix<float>>(raster_grid.rows(), raster_grid.cols()) }
{
    if (!m_geometry_grid.empty()) {
        process_rectangular_ring(box, true);
    }
}

void
GridIntersection::process(const Geometry& g)
{
    auto type = g.getGeometryTypeId();

    if (type == geom::GEOS_POLYGON) {
        set_areal(true);

        const Polygon& p = static_cast<const Polygon&>(g);

        process_line(*p.getExteriorRing(), true);
        auto nrings = p.getNumInteriorRing();
        for (std::size_t i = 0; i < nrings; i++) {
            process_line(*p.getInteriorRingN(i), false);
        }
    } else if (type == geom::GEOS_LINESTRING || type == geom::GEOS_LINEARRING) {
        set_areal(false);

        process_line(static_cast<const LineString&>(g), true);
    } else if (type == geom::GEOS_GEOMETRYCOLLECTION || type == geom::GEOS_MULTILINESTRING || type == geom::GEOS_MULTIPOLYGON) {
        auto ngeoms = g.getNumGeometries();
        for (std::size_t i = 0; i < ngeoms; i++) {
            process(*g.getGeometryN(i));
        }
    } else {
        throw std::invalid_argument("Unsupported geometry type.");
    }
}

static Grid<infinite_extent>
get_box_grid(const Envelope& box, const Grid<infinite_extent>& geometry_grid)
{
    Envelope cropped_ring_extent = geometry_grid.extent().intersection(box);
    return geometry_grid.shrink_to_fit(cropped_ring_extent);
}

static Grid<infinite_extent>
get_ring_grid(const Geometry& ls, const Grid<infinite_extent>& geometry_grid)
{
    return get_box_grid(*ls.getEnvelopeInternal(), geometry_grid);
}

void
GridIntersection::process_rectangular_ring(const Envelope& box, bool exterior_ring)
{
    if (!box.intersects(m_geometry_grid.extent())) {
        return;
    }

    auto ring_grid = get_box_grid(box, m_geometry_grid);

    auto row_min = ring_grid.get_row(box.getMaxY());
    auto row_max = ring_grid.get_row(box.getMinY());
    auto col_min = ring_grid.get_column(box.getMinX());
    auto col_max = ring_grid.get_column(box.getMaxX());

    Matrix<float> areas(ring_grid.rows() - 2, ring_grid.cols() - 2);

    // upper-left
    if (row_min > 0 && col_min > 0) {
        auto ul = ring_grid.grid_cell(row_min, col_min);
        areas(row_min - 1, col_min - 1) = static_cast<float>(ul.intersection(box).getArea() / ul.getArea());
    }

    // upper-right
    if (row_min > 0 && col_max < ring_grid.cols() - 1) {
        auto ur = ring_grid.grid_cell( row_min, col_max);
        auto frac = ur.intersection(box).getArea() / ur.getArea();
        areas(row_min - 1, col_max - 1) = static_cast<float>(frac);
    }

    // lower-left
    if (row_max < ring_grid.rows() - 1 && col_min > 0) {
        auto ll = ring_grid.grid_cell(row_max, col_min);
        areas(row_max - 1, col_min - 1) = static_cast<float>(ll.intersection(box).getArea() / ll.getArea());
    }

    // lower-right
    if (row_max < ring_grid.rows() - 1 && col_max < ring_grid.cols() - 1) {
        auto lr = ring_grid.grid_cell(row_max, col_max);
        areas(row_max - 1, col_max - 1) = static_cast<float>(lr.intersection(box).getArea() / lr.getArea());
    }

    // left
    if (col_min > 0) {
        auto left = ring_grid.grid_cell(row_min + 1, col_min);
        auto frac = left.intersection(box).getArea() / left.getArea();
        for (size_t row = row_min + 1; row < row_max; row++) {
            areas(row - 1, col_min - 1) = static_cast<float>(frac);
        }
    }

    // right
    if (col_max < ring_grid.cols() - 1) {
        auto right = ring_grid.grid_cell(row_min + 1, col_max);
        auto frac = right.intersection(box).getArea() / right.getArea();
        for (size_t row = row_min + 1; row < row_max; row++) {
            areas(row - 1, col_max - 1) = static_cast<float>(frac);
        }
    }

    // top
    if (row_min > 0) {
        auto top = ring_grid.grid_cell(row_min, col_min + 1);
        auto frac = top.intersection(box).getArea() / top.getArea();
        for (size_t col = col_min + 1; col < col_max; col++) {
            areas(row_min - 1, col - 1) = static_cast<float>(frac);
        }
    }

    // bottom
    if (row_max < ring_grid.rows() - 1) {
        auto bottom = ring_grid.grid_cell(row_max, col_min + 1);
        auto frac = bottom.intersection(box).getArea() / bottom.getArea();
        for (size_t col = col_min + 1; col < col_max; col++) {
            areas(row_max - 1, col - 1) = static_cast<float>(frac);
        }
    }

    // interior
    for (size_t row = row_min + 1; row < row_max; row++) {
        for (size_t col = col_min + 1; col < col_max; col++) {
            areas(row - 1, col - 1) = 1.0f;
        }
    }

    // Transfer these areas to our global set
    size_t i0 = ring_grid.row_offset(m_geometry_grid);
    size_t j0 = ring_grid.col_offset(m_geometry_grid);

    add_ring_results(i0, j0, areas, exterior_ring);
}

void
GridIntersection::set_areal(bool areal)
{
    if (m_first_geom) {
        m_first_geom = false;
        m_areal = areal;
    } else {
        if (m_areal != areal) {
            throw std::runtime_error("Mixed-type geometries not supported.");
        }
    }
}

Matrix<float>
GridIntersection::collect_areas(const Matrix<std::unique_ptr<Cell>>& cells,
                                      const Grid<bounded_extent>& finite_ring_grid,
                                      const Geometry& g)
{

    // Create a matrix of areas using the FILLABLE placeholder value, which means that
    // a known state (interior/exterior) can be propated from an adjacent cell.
    Matrix<float> areas(cells.rows() - 2,
                        cells.cols() - 2,
                        fill_values<float>::FILLABLE);

    FloodFill ff(g, finite_ring_grid);

    // Mark all cells that have been traversed as being either INTERIOR or EXTERIOR.
    for (size_t i = 1; i <= areas.rows(); i++) {
        for (size_t j = 1; j <= areas.cols(); j++) {
            if (cells(i, j) != nullptr) {
                // When we encounter a cell that has been processed (ie, it is not nullptr)
                // but whose traversals contains no linear segments, we have no way to know
                // if it is inside of the polygon. So we perform point-in-polygon test and set
                // the covered fraction to 1.0 if needed.

                if (!cells(i, j)->determined()) {
                    areas(i - 1, j - 1) = ff.cell_is_inside(i - 1, j - 1) ? fill_values<float>::INTERIOR : fill_values<float>::EXTERIOR;
                } else {
                    areas(i - 1, j - 1) = static_cast<float>(cells(i, j)->covered_fraction());
                }
            }
        }
    }

    // Propagate INTERIOR and EXTERIOR values to all remaining FILLABLE cells
    ff.flood(areas);

    return areas;
}

Matrix<float>
collect_lengths(const Matrix<std::unique_ptr<Cell>>& cells)
{
    Matrix<float> lengths(cells.rows() - 2,
                          cells.cols() - 2,
                          fill_values<float>::EXTERIOR);

    for (size_t i = 1; i <= lengths.rows(); i++) {
        for (size_t j = 1; j <= lengths.cols(); j++) {
            if (cells(i, j) != nullptr) {
                lengths(i - 1, j - 1) = static_cast<float>(cells(i, j)->traversal_length());
            }
        }
    }

    return lengths;
}

void
traverse_cells(Matrix<std::unique_ptr<Cell>>& cells, std::vector<CoordinateXY>& coords, const Grid<infinite_extent>& ring_grid, bool areal)
{
    size_t pos = 0;
    size_t row = ring_grid.get_row(coords.front().y);
    size_t col = ring_grid.get_column(coords.front().x);
    const CoordinateXY* last_exit = nullptr;

    while (pos < coords.size()) {
        Cell& cell = *get_cell(cells, ring_grid, row, col);

        while (pos < coords.size()) {
            const CoordinateXY* next_coord = last_exit ? last_exit : &coords[pos];
            const CoordinateXY* prev_coord = pos > 0 ? &coords[pos - 1] : nullptr;

            cell.take(*next_coord, prev_coord);

            if (cell.last_traversal().exited()) {
                // Only push our exit coordinate if it's not same as the
                // coordinate we just took. This covers the case where
                // the next coordinate in the stack falls exactly on
                // the cell boundary.
                const CoordinateXY& exc = cell.last_traversal().exit_coordinate();
                if (exc != *next_coord) {
                    last_exit = &exc;
                }
                break;
            } else {
                if (last_exit) {
                    last_exit = nullptr;
                } else {
                    pos++;
                }
            }
        }

        cell.force_exit();

        if (cell.last_traversal().exited()) {
            // When we start in the middle of a cell, for a polygon (areal) calculation,
            // we need to save the coordinates from our incomplete traversal and reprocess
            // them at the end of the line. The effect is just to restructure the polygon
            // so that the start/end coordinate falls on a cell boundary.
            if (areal && !cell.last_traversal().traversed()) {
                for (const auto& coord : cell.last_traversal().coords()) {
                    coords.push_back(coord);
                }
            }

            switch (cell.last_traversal().exit_side()) {
                case Side::TOP:
                    row--;
                    break;
                case Side::BOTTOM:
                    row++;
                    break;
                case Side::LEFT:
                    col--;
                    break;
                case Side::RIGHT:
                    col++;
                    break;
                default:
                    throw std::runtime_error("Invalid traversal");
            }
        }
    }
}

void
GridIntersection::process_line(const LineString& ls, bool exterior_ring)
{
    const Envelope& geom_box = *ls.getEnvelopeInternal();

    const Envelope intersection = geom_box.intersection(m_geometry_grid.extent());
    if (intersection.getArea() == 0) {
        return;
    }

    const geom::CoordinateSequence& coords = *ls.getCoordinatesRO();

    if (m_areal) {
        if (coords.size() < 4) {
            // Component cannot have any area, just skip it.
            return;
        }

        if (coords.size() == 5 && algorithm::Area::ofRing(&coords) == geom_box.getArea()) {
            process_rectangular_ring(geom_box, exterior_ring);
            return;
        }
    }

    Grid<infinite_extent> ring_grid = get_ring_grid(ls, m_geometry_grid);

    size_t rows = ring_grid.rows();
    size_t cols = ring_grid.cols();

    // Short circuit for small geometries that are entirely contained within a single grid cell.
    if (rows == (1 + 2 * infinite_extent::padding) &&
        cols == (1 + 2 * infinite_extent::padding) &&
        ring_grid.grid_cell(1, 1).contains(geom_box)) {

        size_t i0 = ring_grid.row_offset(m_geometry_grid);
        size_t j0 = ring_grid.col_offset(m_geometry_grid);

        if (m_areal) {
            auto ring_area = static_cast<float>(algorithm::Area::ofRing(&coords) / ring_grid.grid_cell(1, 1).getArea());

            if (exterior_ring) {
                m_results->increment(i0, j0, ring_area);
            } else {
                m_results->increment(i0, j0, -1 * ring_area);
            }
        } else {
            m_results->increment(i0, j0, static_cast<float>(algorithm::Length::ofLine(&coords)));
        }

        return;
    }

    std::vector<CoordinateXY> coordsVec;
    coords.toVector(coordsVec);

    if (m_areal && !algorithm::Orientation::isCCW(&coords)) {
        std::reverse(coordsVec.begin(), coordsVec.end());
    }

    Matrix<std::unique_ptr<Cell>> cells(ring_grid.rows(), ring_grid.cols());
    traverse_cells(cells, coordsVec, ring_grid, m_areal);

    // Compute the fraction covered for all cells and assign it to
    // the area matrix
    // TODO avoid copying matrix when geometry has only one polygon, and polygon has only one ring
    Matrix<float> areas = m_areal ? collect_areas(cells, make_finite(ring_grid), ls) : collect_lengths(cells);

    // Transfer these areas to our global set
    size_t i0 = ring_grid.row_offset(m_geometry_grid);
    size_t j0 = ring_grid.col_offset(m_geometry_grid);

    add_ring_results(i0, j0, areas, exterior_ring || !m_areal);
}

void
GridIntersection::add_ring_results(size_t i0, size_t j0, const Matrix<float>& areas, bool exterior_ring)
{
    float factor = exterior_ring ? 1.0f : -1.0f;

    for (size_t i = 0; i < areas.rows(); i++) {
        for (size_t j = 0; j < areas.cols(); j++) {
            m_results->increment(i0 + i, j0 + j, factor * areas(i, j));
        }
    }
}

void
traverse_ring(Matrix<std::unique_ptr<Cell>>& cells, const Grid<infinite_extent>& grid, const LineString& g, bool want_ccw)
{
    const geom::CoordinateSequence& seq = *g.getCoordinatesRO();

    std::vector<CoordinateXY> coords;
    seq.toVector(coords);

    if (want_ccw != algorithm::Orientation::isCCW(&seq)) {
      std::reverse(coords.begin(), coords.end());
   }
    traverse_cells(cells, coords, grid, true);
}

std::unique_ptr<Geometry>
GridIntersection::subdivide_polygon(const Grid<bounded_extent>& p_grid, const Geometry& g, bool includeExterior)
{
     const geom::GeometryFactory& gfact = *g.getFactory();

    Grid<infinite_extent> grid = make_infinite(p_grid, *g.getEnvelopeInternal());
    Matrix<std::unique_ptr<Cell>> cells(grid.rows(), grid.cols());

    std::size_t ngeoms = g.getNumGeometries();
    for (std::size_t i = 0; i < ngeoms; i++) {
        const Polygon& gi = static_cast<const Polygon&>(*g.getGeometryN(i));
        const LineString& shell = *gi.getExteriorRing();
        traverse_ring(cells, grid, shell, true);

        std::size_t nrings = gi.getNumInteriorRing();
        for (std::size_t j = 0; j < nrings; j++) {
            traverse_ring(cells, grid, *gi.getInteriorRingN(j), false);
        }
    }

    auto areas = collect_areas(cells, p_grid, g);

    std::vector<std::unique_ptr<Geometry>> geoms;
    std::vector<std::unique_ptr<Geometry>> edge_geoms;

    for (std::size_t i = 0; i < cells.rows(); i++) {
        bool row_edge = i == 0 || i == (cells.rows() - 1);

        for (std::size_t j = 0; j < cells.cols(); j++) {
            bool edge = row_edge || (j == 0 || j == (cells.cols() - 1));

            if (cells(i, j) != nullptr) {
                if (edge) {
                    if (includeExterior) {
                        edge_geoms.push_back(cells(i, j)->covered_polygons(gfact));
                    }
                } else if (areas(i - 1, j - 1) == fill_values<float>::INTERIOR) {
                    // Cell is completely covered by polygon
                    Envelope env = cells(i, j)->box();
                    geoms.push_back(gfact.toGeometry(&env));
                } else if (areas(i - 1, j - 1) != fill_values<float>::EXTERIOR) {
                    // Cell is partly covered by polygon
                    geoms.push_back(cells(i, j)->covered_polygons(gfact));
                }
            } else if (!edge && areas(i - 1, j - 1) == fill_values<float>::INTERIOR) {
                // Cell is entirely covered by polygon
                Envelope env = grid.grid_cell(i, j);
                geoms.push_back(gfact.toGeometry(&env));
            }
        }
    }

    if (!edge_geoms.empty()) {
        auto edge_coll = gfact.createGeometryCollection(std::move(edge_geoms));
        geoms.push_back(overlayng::CoverageUnion::geomunion(edge_coll.get()));
    }

    return gfact.createGeometryCollection(std::move(geoms));
}

}
