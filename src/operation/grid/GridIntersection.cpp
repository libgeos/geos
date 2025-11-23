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
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Polygon.h>
#include <geos/operation/grid/Cell.h>
#include <geos/operation/grid/FloodFill.h>
#include <geos/operation/grid/GridIntersection.h>
#include <geos/operation/grid/PerimeterDistance.h>
#include <geos/operation/overlayng/CoverageUnion.h>
#include <geos/util.h>

using geos::geom::Geometry;
using geos::geom::LineString;
using geos::geom::Polygon;
using geos::geom::Envelope;
using geos::geom::CoordinateXY;

namespace geos::operation::grid {

std::shared_ptr<Matrix<float>>
GridIntersection::getIntersectionFractions(const Grid<bounded_extent>& raster_grid, const Geometry& g)
{
    GridIntersection rci(raster_grid, g);
    return rci.getResults();
}

std::shared_ptr<Matrix<float>>
GridIntersection::getIntersectionFractions(const Grid<bounded_extent>& raster_grid, const Envelope& box)
{
    GridIntersection rci(raster_grid, box);
    return rci.getResults();
}

static Cell*
get_cell(Matrix<std::unique_ptr<Cell>>& cells, const Grid<infinite_extent>& ex, size_t row, size_t col)
{
    assert(row < cells.getNumRows());
    assert(col < cells.getNumCols());
    if (cells(row, col) == nullptr) {
        cells(row, col) = std::make_unique<Cell>(ex.getCellEnvelope(row, col));
    }

    return cells(row, col).get();
}

Envelope
GridIntersection::processingRegion(const Envelope& raster_extent, const Geometry& g)
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
  , m_results{ cov ? cov : std::make_shared<Matrix<float>>(raster_grid.getNumRows(), raster_grid.getNumCols()) }
  , m_first_geom{ true }
  , m_areal{ false }
{
    if (g.getDimension() == 0) {
        throw std::invalid_argument("Unsupported geometry type.");
    }

    if (!m_geometry_grid.isEmpty()) {
        process(g);
    }
}

GridIntersection::GridIntersection(const Grid<bounded_extent>& raster_grid, const Envelope& box, std::shared_ptr<Matrix<float>> cov)
  : m_geometry_grid{ make_infinite(raster_grid, box) }
  , m_results{ cov ? cov : std::make_shared<Matrix<float>>(raster_grid.getNumRows(), raster_grid.getNumCols()) }
{
    if (!m_geometry_grid.isEmpty()) {
        processRectangularRing(box, true);
    }
}

void
GridIntersection::process(const Geometry& g)
{
    auto type = g.getGeometryTypeId();

    if (type == geom::GEOS_POLYGON) {
        setAreal(true);

        const Polygon& p = static_cast<const Polygon&>(g);

        processLine(*p.getExteriorRing(), true);
        auto nrings = p.getNumInteriorRing();
        for (std::size_t i = 0; i < nrings; i++) {
            processLine(*p.getInteriorRingN(i), false);
        }
    } else if (type == geom::GEOS_LINESTRING || type == geom::GEOS_LINEARRING) {
        setAreal(false);

        processLine(static_cast<const LineString&>(g), true);
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
    Envelope cropped_ring_extent = geometry_grid.getExtent().intersection(box);
    return geometry_grid.shrinkToFit(cropped_ring_extent);
}

static Grid<infinite_extent>
get_ring_grid(const Geometry& ls, const Grid<infinite_extent>& geometry_grid)
{
    return get_box_grid(*ls.getEnvelopeInternal(), geometry_grid);
}

void
GridIntersection::processRectangularRing(const Envelope& box, bool exterior_ring)
{
    if (!box.intersects(m_geometry_grid.getExtent())) {
        return;
    }

    auto ring_grid = get_box_grid(box, m_geometry_grid);

    auto row_min = ring_grid.getRow(box.getMaxY());
    auto row_max = ring_grid.getRow(box.getMinY());
    auto col_min = ring_grid.getColumn(box.getMinX());
    auto col_max = ring_grid.getColumn(box.getMaxX());

    Matrix<float> areas(ring_grid.getNumRows() - 2, ring_grid.getNumCols() - 2);

    // upper-left
    if (row_min > 0 && col_min > 0) {
        auto ul = ring_grid.getCellEnvelope(row_min, col_min);
        areas(row_min - 1, col_min - 1) = static_cast<float>(ul.intersection(box).getArea() / ul.getArea());
    }

    // upper-right
    if (row_min > 0 && col_max < ring_grid.getNumCols() - 1) {
        auto ur = ring_grid.getCellEnvelope( row_min, col_max);
        auto frac = ur.intersection(box).getArea() / ur.getArea();
        areas(row_min - 1, col_max - 1) = static_cast<float>(frac);
    }

    // lower-left
    if (row_max < ring_grid.getNumRows() - 1 && col_min > 0) {
        auto ll = ring_grid.getCellEnvelope(row_max, col_min);
        areas(row_max - 1, col_min - 1) = static_cast<float>(ll.intersection(box).getArea() / ll.getArea());
    }

    // lower-right
    if (row_max < ring_grid.getNumRows() - 1 && col_max < ring_grid.getNumCols() - 1) {
        auto lr = ring_grid.getCellEnvelope(row_max, col_max);
        areas(row_max - 1, col_max - 1) = static_cast<float>(lr.intersection(box).getArea() / lr.getArea());
    }

    // left
    if (col_min > 0) {
        auto left = ring_grid.getCellEnvelope(row_min + 1, col_min);
        auto frac = left.intersection(box).getArea() / left.getArea();
        for (size_t row = row_min + 1; row < row_max; row++) {
            areas(row - 1, col_min - 1) = static_cast<float>(frac);
        }
    }

    // right
    if (col_max < ring_grid.getNumCols() - 1) {
        auto right = ring_grid.getCellEnvelope(row_min + 1, col_max);
        auto frac = right.intersection(box).getArea() / right.getArea();
        for (size_t row = row_min + 1; row < row_max; row++) {
            areas(row - 1, col_max - 1) = static_cast<float>(frac);
        }
    }

    // top
    if (row_min > 0) {
        auto top = ring_grid.getCellEnvelope(row_min, col_min + 1);
        auto frac = top.intersection(box).getArea() / top.getArea();
        for (size_t col = col_min + 1; col < col_max; col++) {
            areas(row_min - 1, col - 1) = static_cast<float>(frac);
        }
    }

    // bottom
    if (row_max < ring_grid.getNumRows() - 1) {
        auto bottom = ring_grid.getCellEnvelope(row_max, col_min + 1);
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
    size_t i0 = ring_grid.getRowOffset(m_geometry_grid);
    size_t j0 = ring_grid.getColOffset(m_geometry_grid);

    addRingResults(i0, j0, areas, exterior_ring);
}

void
GridIntersection::setAreal(bool areal)
{
    if (m_first_geom) {
        m_first_geom = false;
        m_areal = areal;
    } else {
        if (m_areal != areal) {
            throw util::GEOSException("Mixed-type geometries not supported.");
        }
    }
}

Matrix<float>
GridIntersection::collectAreas(const Matrix<std::unique_ptr<Cell>>& cells,
                                      const Grid<bounded_extent>& finite_ring_grid,
                                      const Geometry& g)
{

    // Create a matrix of areas using the FILLABLE placeholder value, which means that
    // a known state (interior/exterior) can be propated from an adjacent cell.
    Matrix<float> areas(cells.getNumRows() - 2,
                        cells.getNumCols() - 2,
                        fill_values<float>::FILLABLE);

    FloodFill ff(g, finite_ring_grid);

    // Mark all cells that have been traversed as being either INTERIOR or EXTERIOR.
    for (size_t i = 1; i <= areas.getNumRows(); i++) {
        for (size_t j = 1; j <= areas.getNumCols(); j++) {
            if (cells(i, j) != nullptr) {
                // When we encounter a cell that has been processed (ie, it is not nullptr)
                // but whose traversals contains no linear segments, we have no way to know
                // if it is inside of the polygon. So we perform point-in-polygon test and set
                // the covered fraction to 1.0 if needed.

                if (!cells(i, j)->isDetermined()) {
                    areas(i - 1, j - 1) = ff.cellIsInside(i - 1, j - 1) ? fill_values<float>::INTERIOR : fill_values<float>::EXTERIOR;
                } else {
                    areas(i - 1, j - 1) = static_cast<float>(cells(i, j)->getCoveredFraction());
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
    Matrix<float> lengths(cells.getNumRows() - 2,
                          cells.getNumCols() - 2,
                          fill_values<float>::EXTERIOR);

    for (size_t i = 1; i <= lengths.getNumRows(); i++) {
        for (size_t j = 1; j <= lengths.getNumCols(); j++) {
            if (cells(i, j) != nullptr) {
                lengths(i - 1, j - 1) = static_cast<float>(cells(i, j)->getTraversalLength());
            }
        }
    }

    return lengths;
}

static void
traverseCells(Matrix<std::unique_ptr<Cell>>& cells, std::vector<CoordinateXY>& coords, const Grid<infinite_extent>& ring_grid, bool areal, bool exitOnBoundary, const void* parentage)
{
    size_t pos = 0;
    size_t row = ring_grid.getRow(coords.front().y);
    size_t col = ring_grid.getColumn(coords.front().x);
    const CoordinateXY* last_exit = nullptr;

    while (pos < coords.size()) {
        Cell& cell = *get_cell(cells, ring_grid, row, col);

        while (pos < coords.size()) {
            const CoordinateXY* next_coord = last_exit ? last_exit : &coords[pos];
            const CoordinateXY* prev_coord = pos > 0 ? &coords[pos - 1] : nullptr;

            cell.take(*next_coord, prev_coord, exitOnBoundary, parentage);

            if (cell.getLastTraversal().isExited()) {
                // Only push our exit coordinate if it's not same as the
                // coordinate we just took. This covers the case where
                // the next coordinate in the stack falls exactly on
                // the cell boundary.
                const CoordinateXY& exc = cell.getLastTraversal().getExitCoordinate();
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

        cell.forceExit();

        if (cell.getLastTraversal().isExited()) {
            // When we start in the middle of a cell, for a polygon (areal) calculation,
            // we need to save the coordinates from our incomplete traversal and reprocess
            // them at the end of the line. The effect is just to restructure the polygon
            // so that the start/end coordinate falls on a cell boundary.
            if (areal && !cell.getLastTraversal().isTraversed()) {
                for (const auto& coord : cell.getLastTraversal().getCoordinates()) {
                    coords.push_back(coord);
                }
            }

            switch (cell.getLastTraversal().getExitSide()) {
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
                    throw util::GEOSException("Invalid traversal");
            }
        }
    }
}

void
GridIntersection::processLine(const LineString& ls, bool exterior_ring)
{
    const Envelope& geom_box = *ls.getEnvelopeInternal();

    const Envelope intersection = geom_box.intersection(m_geometry_grid.getExtent());
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
            processRectangularRing(geom_box, exterior_ring);
            return;
        }
    }

    Grid<infinite_extent> ring_grid = get_ring_grid(ls, m_geometry_grid);

    size_t rows = ring_grid.getNumRows();
    size_t cols = ring_grid.getNumCols();

    // Short circuit for small geometries that are entirely contained within a single grid cell.
    if (rows == (1 + 2 * infinite_extent::padding) &&
        cols == (1 + 2 * infinite_extent::padding) &&
        ring_grid.getCellEnvelope(1, 1).contains(geom_box)) {

        size_t i0 = ring_grid.getRowOffset(m_geometry_grid);
        size_t j0 = ring_grid.getColOffset(m_geometry_grid);

        if (m_areal) {
            auto ring_area = static_cast<float>(algorithm::Area::ofRing(&coords) / ring_grid.getCellEnvelope(1, 1).getArea());

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

    Matrix<std::unique_ptr<Cell>> cells(ring_grid.getNumRows(), ring_grid.getNumCols());
    traverseCells(cells, coordsVec, ring_grid, m_areal, false, &ls);

    // Compute the fraction covered for all cells and assign it to
    // the area matrix
    // TODO avoid copying matrix when geometry has only one polygon, and polygon has only one ring
    Matrix<float> areas = m_areal ? collectAreas(cells, make_finite(ring_grid), ls) : collect_lengths(cells);

    // Transfer these areas to our global set
    size_t i0 = ring_grid.getRowOffset(m_geometry_grid);
    size_t j0 = ring_grid.getColOffset(m_geometry_grid);

    addRingResults(i0, j0, areas, exterior_ring || !m_areal);
}

void
GridIntersection::addRingResults(size_t i0, size_t j0, const Matrix<float>& areas, bool exterior_ring)
{
    float factor = exterior_ring ? 1.0f : -1.0f;

    for (size_t i = 0; i < areas.getNumRows(); i++) {
        for (size_t j = 0; j < areas.getNumCols(); j++) {
            m_results->increment(i0 + i, j0 + j, factor * areas(i, j));
        }
    }
}

static void
traverseRing(Matrix<std::unique_ptr<Cell>>& cells, const Grid<infinite_extent>& grid, const LineString& g, bool want_ccw, bool exitOnBoundary)
{
    const geom::CoordinateSequence& seq = *g.getCoordinatesRO();

    std::vector<CoordinateXY> coords;
    seq.toVector(coords);

    if (want_ccw != algorithm::Orientation::isCCW(&seq)) {
      std::reverse(coords.begin(), coords.end());
    }
    traverseCells(cells, coords, grid, true, exitOnBoundary, &g);
}

static void
traversePolygons(Matrix<std::unique_ptr<Cell>>& cells, const Grid<infinite_extent>& grid, const Geometry& g, bool exitOnBoundary)
{
    using geom::GeometryTypeId;

    std::size_t ngeoms = g.getNumGeometries();
    for (std::size_t i = 0; i < ngeoms; i++) {
        const Geometry& gi = *g.getGeometryN(i);
        auto typ = gi.getGeometryTypeId();

        if (typ == GeometryTypeId::GEOS_POLYGON) {
            const Polygon& poly = static_cast<const Polygon&>(gi);
            const LineString& shell = *poly.getExteriorRing();
            traverseRing(cells, grid, shell, true, exitOnBoundary);

            std::size_t nrings = poly.getNumInteriorRing();
            for (std::size_t j = 0; j < nrings; j++) {
                traverseRing(cells, grid, *poly.getInteriorRingN(j), false, exitOnBoundary);
            }
        } else if (typ == GeometryTypeId::GEOS_MULTIPOLYGON || typ == GeometryTypeId::GEOS_GEOMETRYCOLLECTION) {
            traversePolygons(cells, grid, gi, exitOnBoundary);
        } else {
            throw util::GEOSException("Unsupported geometry type");
        }
    }
}

// Create a rectangular polygon from a set of points lying on the boundary
// of a specified envelope. The provided points do not need to include the
// corners of the envelope itself.
static std::unique_ptr<Geometry>
createRectangle(const geom::GeometryFactory& gfact, const Envelope& env, std::vector<CoordinateXY>& points)
{
    if (points.empty()) {
        return gfact.toGeometry(&env);
    } else {
        auto perimeterDistanceCmp = [&env](const CoordinateXY& a, const CoordinateXY& b) {
            return PerimeterDistance::getPerimeterDistance(env, a) <
                PerimeterDistance::getPerimeterDistance(env, b);
        };

        points.emplace_back(env.getMinX(), env.getMinY());
        points.emplace_back(env.getMinX(), env.getMaxY());
        points.emplace_back(env.getMaxX(), env.getMinY());
        points.emplace_back(env.getMaxX(), env.getMaxY());

        std::sort(points.begin(), points.end(), perimeterDistanceCmp);
        points.push_back(points.front());
        auto seq = std::make_unique<geom::CoordinateSequence>(0, false, false);
        seq->reserve(points.size());
        for (const auto& c : points) {
            seq->add(c, false);
        }

        auto ring = gfact.createLinearRing(std::move(seq));
        return gfact.createPolygon(std::move(ring));
    }
}

// Get any points from adjacent cells that are also on the boundary of this cell.
static std::vector<CoordinateXY>
getExtraNodes(const Matrix<std::unique_ptr<Cell> > &cells, size_t i, size_t j) {
    std::vector<CoordinateXY> points;

    if (const Cell *above = cells(i - 1, j).get()) {
        above->getEdgePoints(Side::BOTTOM, points);
    }
    if (const Cell *below = cells(i + 1, j).get()) {
        below->getEdgePoints(Side::TOP, points);
    }
    if (const Cell *left = cells(i, j - 1).get()) {
        left->getEdgePoints(Side::RIGHT, points);
    }
    if (const Cell *right = cells(i, j + 1).get()) {
        right->getEdgePoints(Side::LEFT, points);
    }

    return points;
}

std::unique_ptr<Geometry>
GridIntersection::subdividePolygon(const Grid<bounded_extent>& p_grid, const Geometry& g, bool includeExterior)
{
    util::ensureNoCurvedComponents(g);

    const geom::GeometryFactory& gfact = *g.getFactory();

    const auto cropped_grid = p_grid.shrinkToFit(p_grid.getExtent().intersection(*g.getEnvelopeInternal()), false);
    //const auto cropped_grid = p_grid;

    geom::Envelope gridExtent = *g.getEnvelopeInternal();
    gridExtent.expandBy(1);

    const Grid<infinite_extent> cell_grid = make_infinite(cropped_grid, gridExtent);
    Matrix<std::unique_ptr<Cell>> cells(cell_grid.getNumRows(), cell_grid.getNumCols());

    traversePolygons(cells, cell_grid, g, true);

    const auto areas = collectAreas(cells, cropped_grid, g);

    std::vector<std::unique_ptr<Geometry>> geoms;
    std::vector<std::unique_ptr<Geometry>> edge_geoms;

    for (std::size_t i = 0; i < cells.getNumRows(); i++) {
        const bool row_edge = i == 0 || i == (cells.getNumRows() - 1);

        for (std::size_t j = 0; j < cells.getNumCols(); j++) {
            const bool col_edge = j == 0 || j == cells.getNumCols() - 1;
            const bool edge = row_edge || col_edge;

            const Cell* cell = cells(i, j).get();

            if (cell != nullptr && cell->isDetermined()) {
                // It is possible for the area to equal the cell area when the polygon is not the same as the
                // cell area. (See GridIntersectionTest test #46). So, we only compare the area to
                // fill_values<float>::INTERIOR for cells that have not been traversed.
                if (edge) {
                    if (includeExterior) {
                        edge_geoms.push_back(cell->getCoveredPolygons(gfact));
                    }
                } else if (areas(i - 1, j - 1) != fill_values<float>::EXTERIOR) {
                    // Cell is partly covered by polygon
                    geoms.push_back(cell->getCoveredPolygons(gfact));
                }
            } else if (!edge && areas(i - 1, j - 1) == fill_values<float>::INTERIOR) {
                // Cell is entirely covered by polygon
                // In order to have the outputs forms a properly noded coverage,
                // we need to add nodes from adjacent polygons.
                Envelope env = cell_grid.getCellEnvelope(i, j);
                std::vector<CoordinateXY> points = getExtraNodes(cells, i, j);
                geoms.push_back(createRectangle(gfact, env, points));
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
