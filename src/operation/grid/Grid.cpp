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
    // the right and bottom edges of our grid are slightly larger than m_dx,dy
    // if needed to make sure that we capture our whole extent. This is necessary
    // because xmin + nx*m_dx may be less than xmax because of floating point
    // errors.
    return {
        xmin() + static_cast<double>(col) * dx(),
        row == (getNumRows() - 1) ? ymin() : (ymax() - static_cast<double>(row + 1) * dy()),
        col == (getNumCols() - 1) ? xmax() : (xmin() + static_cast<double>(col + 1) * dx()),
        ymax() - static_cast<double>(row) * dy()
    };
}

template<>
Envelope
Grid<infinite_extent>::getCellEnvelope(size_t row, size_t col) const
{
    double cell_xmin, cell_xmax, cell_ymin, cell_ymax;

    constexpr double PADDED_CELL_SIZE = 1.0;

    if (col == 0) {
        cell_xmin = std::min(xmin() - PADDED_CELL_SIZE, m_domain.getMinX());
    } else if (col == getNumCols() - 1) {
        cell_xmin = xmax(); // because rightmost col of regular may have different width from others
    } else {
        cell_xmin = xmin() + static_cast<double>(col - 1) * dx();
    }

    switch (getNumCols() - col) {
        case 1:
            cell_xmax = std::max(xmax() + PADDED_CELL_SIZE, m_domain.getMaxX());
            break;
        case 2:
            cell_xmax = xmax();
            break;
        default:
            cell_xmax = xmin() + static_cast<double>(col) * dx();
    }

    if (row == 0) {
        cell_ymax = std::max(xmax() + PADDED_CELL_SIZE, m_domain.getMaxX());
    } else if (row == getNumRows() - 1) {
        cell_ymax = ymin(); // because bottom row of regular may have different height from others
    } else {
        cell_ymax = ymax() - static_cast<double>(row - 1) * dy();
    }

    switch (getNumRows() - row) {
        case 1:
            cell_ymin = std::min(xmin() - PADDED_CELL_SIZE, m_domain.getMinY());
            break;
        case 2:
            cell_ymin = ymin();
            break;
        default:
            cell_ymin = ymax() - static_cast<double>(row) * dy();
    }

    return { cell_xmin, cell_xmax, cell_ymin, cell_ymax };
}

Grid<infinite_extent>
make_infinite(const Grid<bounded_extent>& grid, const Envelope& domain)
{
    return { grid.getExtent(), grid.dx(), grid.dy() , domain };
}

Grid<bounded_extent>
make_finite(const Grid<infinite_extent>& grid)
{
    return { grid.getExtent(), grid.dx(), grid.dy() };
}

}
