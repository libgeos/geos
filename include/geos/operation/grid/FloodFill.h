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

#include <geos/operation/grid/Grid.h>
#include <geos/operation/grid/Matrix.h>

#include <queue>

namespace geos::geom {
    class Geometry;
}

namespace geos::algorithm::locate {
    class PointOnGeometryLocator;
}

namespace geos::operation::grid {

template<typename T>
struct fill_values
{
    static T EXTERIOR;
};

template<>
struct fill_values<float>
{
    static constexpr float EXTERIOR{ 0.0f };  // Cell is known to be entirely outside the polygon
    static constexpr float INTERIOR{ 1.0f };  // Cell is known to be entirely within the polygon
    static constexpr float FILLABLE{ -1.0f }; // Cell location relative to polygon unknown, but
                                              // can be determined by fill.
    static constexpr float UNKNOWN{ -2.0f };  // Cell location relative to polygon unknown
                                              // and cannot be determined from a flood fill
                                              // (must be explicitly tested)
};

class FloodFill
{

  public:
    FloodFill(const geom::Geometry& g, const Grid<bounded_extent>& extent);

    ~FloodFill();

    template<typename T>
    void flood(Matrix<T>& arr) const;

    bool cellIsInside(size_t i, size_t j) const;

  private:
    Grid<bounded_extent> m_extent;
    const geom::Geometry& m_g;
    std::unique_ptr<algorithm::locate::PointOnGeometryLocator> m_loc;
};

template<typename T>
void
floodFromPixel(Matrix<T>& arr, size_t i, size_t j, T fill_value)
{
    std::queue<std::pair<size_t, size_t>> locations;

    locations.emplace(i, j);

    while (!locations.empty()) {
        i = locations.front().first;
        j = locations.front().second;
        locations.pop();

        if (arr(i, j) == fill_value) {
            continue;
        }

        // Left
        if (j > 0 && arr(i, j - 1) == fill_values<T>::FILLABLE) {
            locations.emplace(i, j - 1);
        }

        auto j0 = j;

        // Fill along this row until we hit something
        for (; j < arr.getNumCols() && arr(i, j) == fill_values<T>::FILLABLE; j++) {
            arr(i, j) = fill_value;
        }

        auto j1 = j;

        // Initiate scanlines above our current row
        if (i > 0) {
            for (j = j0; j < j1; j++) {
                // Up
                if (arr(i - 1, j) == fill_values<T>::FILLABLE) {
                    locations.emplace(i - 1, j);
                }
            }
        }

        // Initiate scanlines below our current row
        if (i < arr.getNumRows() - 1) {
            for (j = j0; j < j1; j++) {
                // Down
                if (arr(i + 1, j) == fill_values<T>::FILLABLE) {
                    locations.emplace(i + 1, j);
                }
            }
        }
    }
}

template<typename T>
void
FloodFill::flood(Matrix<T>& arr) const
{

    for (size_t i = 0; i < arr.getNumRows(); i++) {
        for (size_t j = 0; j < arr.getNumCols(); j++) {
            if (arr(i, j) == fill_values<T>::UNKNOWN) {
                throw std::runtime_error("Cell with unknown position encountered.");
            } else if (arr(i, j) == fill_values<T>::FILLABLE) {
                // Cell position relative to polygon is unknown but can
                // be determined from adjacent cells.
                if (cellIsInside(i, j)) {
                    floodFromPixel(arr, i, j, fill_values<T>::INTERIOR);
                } else {
                    floodFromPixel(arr, i, j, fill_values<T>::EXTERIOR);
                }
            }
        }
    }
}

}
