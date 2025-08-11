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

#pragma once

#include <memory>

#include <geos/geom/Geometry.h>
#include <geos/operation/grid/Grid.h>
#include <geos/operation/grid/Matrix.h>

namespace geos::geom {
    class Geometry;
    class LineString;
}

namespace geos::operation::grid {

class Cell;

/**
 * @brief The GridIntersection class computes and stores information about intersection of a Grid and a Geometry.
 *        For a polygonal geometry, the fraction of each grid cell covered by polygon is stored.
 *        For a linear geometry, the length of the line in each grid cell is stored.
 *
 *        Intersection information returned as `Raster` with an equivalent resolution to the input `Grid`. The spatial
 *        extent may be reduced from the input `Grid.`
 */
class GEOS_DLL GridIntersection
{

  public:
    /**
     * @brief Compute the fraction of each cell in a rectangular grid that is covered by a Geometry.
     *        A matrix can be provided to which the fractions will be added.
     */
    GridIntersection(const Grid<bounded_extent>& raster_grid, const geom::Geometry& g, std::shared_ptr<Matrix<float>> cov = nullptr);

    /**
     * @brief Compute the fraction of each cell in a rectangular grid that is covered by an Envelope.
     *        A matrix can be provided to which the fractions will be added.
     */
    GridIntersection(const Grid<bounded_extent>& raster_grid, const geom::Envelope& box, std::shared_ptr<Matrix<float>> cov = nullptr);

    /**
     * @brief Return the intersection result matrix
     */
    std::shared_ptr<Matrix<float>> getResults() { return m_results; }

    /**
     * @brief Partition a polygonal geometry by a grid
     */
    static std::unique_ptr<geom::Geometry> subdividePolygon(const Grid<bounded_extent>& p_grid, const geom::Geometry& g, bool includeExterior);

    static std::shared_ptr<Matrix<float>>
    getIntersectionFractions(const Grid<bounded_extent>& raster_grid, const geom::Geometry& g);

    static std::shared_ptr<Matrix<float>>
    getIntersectionFractions(const Grid<bounded_extent>& raster_grid, const geom::Envelope& box);

    /**
     * @brief Determines the bounding box of the raster-vector intersection. Considers the bounding boxes
     *        of individual polygon components separately to avoid unnecessary computation for sparse
     *        multi-polygons.
     *
     * @param raster_extent Envelope representing the extent of the vector
     * @param geometry Geometry to be processed
     *
     * @return the portion of `raster_extent` that intersects one or more `component_boxes`
     */
    static geom::Envelope
    processingRegion(const geom::Envelope& raster_extent, const geom::Geometry& geometry);

  private:
    void process(const geom::Geometry& g);

    void processLine(const geom::LineString& ls, bool exterior_ring);

    void processRectangularRing(const geom::Envelope& box, bool exterior_ring);

    void addRingResults(size_t i0, size_t j0, const Matrix<float>& areas, bool exterior_ring);

    void setAreal(bool areal);

    static Matrix<float> collectAreas(const Matrix<std::unique_ptr<Cell>>& cells,
                                      const Grid<bounded_extent>& finite_ring_grid,
                                      const geom::Geometry& g);

    Grid<infinite_extent> m_geometry_grid;
    std::shared_ptr<Matrix<float>> m_results;
    bool m_first_geom;
    bool m_areal;
};

}
