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

#include <geos/operation/grid/Grid.h>

using geos::geom::Envelope;

namespace geos::operation::grid {

template<>
Envelope
Grid<bounded_extent>::getCellEnvelope(size_t row, size_t col) const
{
    // The ternary clauses below are used to make sure that the cells along
    // the right and bottom edges of our grid are slightly larger than m_dx, m_dy
    // if needed to make sure that we capture our whole extent. This is necessary
    // because xmin + nx*m_dx may be less than xmax because of floating point
    // errors.
    return {
        m_xOrig + static_cast<double>(col + m_skipCols) * dx(),
        col == (getNumCols() - 1) ? xmax() : (m_xOrig + static_cast<double>(col + m_skipCols + 1) * dx()),
        row == (getNumRows() - 1) ? ymin() : (m_yOrig - static_cast<double>(row + m_skipRows + 1) * dy()),
        m_yOrig - static_cast<double>(row + m_skipRows) * dy()
    };
}

template<>
Envelope
Grid<infinite_extent>::getCellEnvelope(size_t row, size_t col) const
{
    double cell_xmin, cell_xmax, cell_ymin, cell_ymax;

    constexpr double PADDED_CELL_SIZE = 1.0;

    if (col == 0) {
        // Left-side padding
        cell_xmin = std::min(xmin() - PADDED_CELL_SIZE, m_domain.getMinX());
    } else if (col == getNumCols() - 1) {
        // Right-side padding
        cell_xmin = xmax(); // because rightmost col of regular may have different width from others
    } else {
        // Internal
        cell_xmin = m_xOrig + static_cast<double>(m_skipCols + col - 1) * dx();
    }

    switch (getNumCols() - col) {
        case 1:
            // Right-side padding
            cell_xmax = std::max(xmax() + PADDED_CELL_SIZE, m_domain.getMaxX());
            break;
        case 2:
            // Rightmost internal cell
            cell_xmax = xmax();
            break;
        default:
            cell_xmax = m_xOrig + static_cast<double>(m_skipCols + col) * dx();
    }

    if (row == 0) {
        // Top row padding
        cell_ymax = std::max(ymax() + PADDED_CELL_SIZE, m_domain.getMaxY());
    } else if (row == getNumRows() - 1) {
        // Bottom row padding
        cell_ymax = ymin(); // because bottom row of regular may have different height from others
    } else {
        cell_ymax = m_yOrig - static_cast<double>(m_skipRows + row - 1) * dy();
    }

    switch (getNumRows() - row) {
        case 1:
            // Bottom row padding
            cell_ymin = std::min(xmin() - PADDED_CELL_SIZE, m_domain.getMinY());
            break;
        case 2:
            // Bottom-most internal row
            cell_ymin = ymin();
            break;
        default:
            cell_ymin = m_yOrig - static_cast<double>(m_skipRows + row) * dy();
    }

    Envelope box{ cell_xmin, cell_xmax, cell_ymin, cell_ymax };
    return box;
}

Grid<infinite_extent>
make_infinite(const Grid<bounded_extent>& grid, const Envelope& domain)
{
    Grid<infinite_extent> ret{ grid.getExtent(), grid.dx(), grid.dy() , domain };
    ret.m_xOrig = grid.m_xOrig;
    ret.m_yOrig = grid.m_yOrig;
    ret.m_skipCols = grid.m_skipCols;
    ret.m_skipRows = grid.m_skipRows;
    return ret;
}

Grid<bounded_extent>
make_finite(const Grid<infinite_extent>& grid)
{
    Grid<bounded_extent> ret{ grid.getExtent(), grid.dx(), grid.dy() };
    ret.m_xOrig = grid.m_xOrig;
    ret.m_yOrig = grid.m_yOrig;
    ret.m_skipCols = grid.m_skipCols;
    ret.m_skipRows = grid.m_skipRows;
    return ret;
}

}
