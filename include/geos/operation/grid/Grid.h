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

#include <numeric>
#include <stdexcept>
#include <vector>

#include <geos/geom/Envelope.h>

namespace geos::operation::grid {
struct infinite_extent
{
    static constexpr size_t padding = 1;
};

struct bounded_extent
{
    static constexpr size_t padding = 0;
};

/**
 * @brief The Grid class represents a grid of constant-size rectangular cells that covers a specified envelope.
 *        The width of the cells may be different from the height. If the Grid has an "infinite" extent rather than
 *        a "bounded" extent, then an extra row and column will be added on all side of the grid. The size of the cells
 *        in these columns may be larger than those in the primary grid, such that the extended grid covers a "domain"
 *        that is larger than the extent of the regular grid.
 */
template<typename extent_tag>
class GEOS_DLL Grid
{

  public:
    /// Construct a bounded grid covering a specified extent.
    Grid(const geom::Envelope& extent, double dx, double dy)
      : m_extent{ extent }
      , m_domain{}
      , m_dx{ dx }
      , m_dy{ dy }
      , m_xOrig { extent.getMinX() }
      , m_yOrig { extent.getMaxY() }
      , m_num_rows{ 2 * extent_tag::padding + (extent.getMaxY() > extent.getMinY() ? static_cast<size_t>(std::round(extent.getHeight() / dy)) : 0) }
      , m_num_cols{ 2 * extent_tag::padding + (extent.getMaxX() > extent.getMinX() ? static_cast<size_t>(std::round(extent.getWidth() / dx)) : 0) } {
        static_assert(std::is_same_v<extent_tag, bounded_extent>);
    }

    /// Construct an infinite grid covering a specified extent with regularly-sized cells, and adding a row and column
    /// of variably-sized cells to each edge of the primary grid such that the specified domain is covered.
    Grid(const geom::Envelope& extent, double dx, double dy, const geom::Envelope& domain)
      : m_extent{ extent }
      , m_domain{ domain }
      , m_dx{ dx }
      , m_dy{ dy }
      , m_xOrig { extent.getMinX() }
      , m_yOrig { extent.getMaxY() }
      , m_num_rows{ 2 * extent_tag::padding + (extent.getMaxY() > extent.getMinY() ? static_cast<size_t>(std::round(extent.getHeight() / dy)) : 0) }
      , m_num_cols{ 2 * extent_tag::padding + (extent.getMaxX() > extent.getMinX() ? static_cast<size_t>(std::round(extent.getWidth() / dx)) : 0) }
    {
        static_assert(std::is_same_v<extent_tag, infinite_extent>);
    }


    static Grid makeEmpty()
    {
        if constexpr (std::is_same_v<extent_tag, bounded_extent>) {
            return Grid({ 0, 0, 0, 0 }, 0, 0);
        } else {
            return Grid({ 0, 0, 0, 0 }, 0, 0, geom::Envelope());
        }
    }

    /// Get the column in which the specified x coordinate would fall.
    size_t getColumn(double x) const
    {
        if (extent_tag::padding) {
            if (x < m_extent.getMinX())
                return 0;
            if (x > m_extent.getMaxX())
                return m_num_cols - 1;
            if (x == m_extent.getMaxX()) // special case, returning the cell for which xmax is the right
                return m_num_cols - 2;
        } else {
            if (x < m_extent.getMinX() || x > m_extent.getMaxX())
                throw std::out_of_range("x");

            if (x == m_extent.getMaxX())
                return m_num_cols - 1;
        }

        // Since we've already range-checked our x value, make sure that
        // the computed column index is no greater than the column index
        // associated with xmax.
        return std::min(
          extent_tag::padding + static_cast<size_t>(std::floor((x - m_extent.getMinX()) / m_dx)),
          getColumn(m_extent.getMaxX()));
    }

    /// Get the row in which the specified y coordinate would fall.
    size_t getRow(double y) const
    {
        if (extent_tag::padding) {
            if (y > m_extent.getMaxY())
                return 0;
            if (y < m_extent.getMinY())
                return m_num_rows - 1;
            if (y == m_extent.getMinY()) // special case, returning the cell for which ymin is the bottom
                return m_num_rows - 2;
        } else {
            if (y < m_extent.getMinY() || y > m_extent.getMaxY())
                throw std::out_of_range("y");

            if (y == m_extent.getMinY())
                return m_num_rows - 1;
        }

        // Since we've already range-checked our y value, make sure that
        // the computed row index is no greater than the column index
        // associated with ymin.
        return std::min(
          extent_tag::padding + static_cast<size_t>(std::floor((m_extent.getMaxY() - y) / m_dy)),
          getRow(m_extent.getMinY()));
    }

    /// Get the cell index in which the specified x and y values would fall. Cells are indexed from left-to-right,
    /// then top-to-bottom.
    std::size_t getCell(double x, double y) const
    {
        return getRow(y) * getNumCols() + getColumn(x);
    }

    bool isEmpty() const { return m_num_rows <= 2 * extent_tag::padding && m_num_cols <= 2 * extent_tag::padding; }

    size_t getNumRows() const { return m_num_rows; }

    size_t getNumCols() const { return m_num_cols; }

    size_t getSize() const { return getNumRows() * getNumCols(); }

    double xmin() const { return m_extent.getMinX(); }

    double xmax() const { return m_extent.getMaxX(); }

    double ymin() const { return m_extent.getMinY(); }

    double ymax() const { return m_extent.getMaxY(); }

    double dx() const { return m_dx; }

    double dy() const { return m_dy; }

    const geom::Envelope& getExtent() const { return m_extent; }

    /// Return the number of rows by which another grid is offset from this Grid. It is assumed that the two
    /// grids have the same resolution, and that the maximum y value of the other grid is less than or equal
    /// to the maximum y value of this grid.
    size_t getRowOffset(const Grid& other) const { return static_cast<size_t>(std::round(std::abs(other.m_extent.getMaxY() - m_extent.getMaxY()) / m_dy)); }

    /// Return the number of columns by which another grid is offset from this Grid. It is assumed that the two
    /// grids have the same resolution, and that the minimum x value of the other grid is greater than or equal
    /// to the minimum x value of this grid.
    size_t getColOffset(const Grid& other) const { return static_cast<size_t>(std::round(std::abs(m_extent.getMinX() - other.m_extent.getMinX()) / m_dx)); }

    /// Get the x coordinate at the center of the specified column.
    double getColX(size_t col) const { return m_extent.getMinX() + (static_cast<double>(col - extent_tag::padding) + 0.5) * m_dx; }

    /// Get the y coordinate at the center of the specified row.
    double getRowY(size_t row) const { return m_extent.getMaxY() - (static_cast<double>(row - extent_tag::padding) + 0.5) * m_dy; }

    Grid crop(const geom::Envelope& e) const
    {
        if (m_extent.intersects(e)) {
            return shrinkToFit(e.intersection(m_extent));
        } else {
            return makeEmpty();
        }
    }

    /// Reduce the size of the grid to contain only the provided Envelope
    /// If calcExtentFromNewGrid is true, then the xmax and ymin of the new
    /// grid will be calculated relative to the origin point of the original grid
    /// rather than the newly cropped grid.
    Grid<extent_tag> shrinkToFit(const geom::Envelope& b, bool calcExtentFromNewGrid=true) const
    {
        if (b.getArea() == 0) {
            return makeEmpty();
        }

        if (b.getMinX() < m_extent.getMinX() || b.getMinY() < m_extent.getMinY() || b.getMaxX() > m_extent.getMaxX() || b.getMaxY() > m_extent.getMaxY()) {
            throw std::range_error("Cannot shrink extent to bounds larger than original.");
        }

        size_t col0 = getColumn(b.getMinX());
        size_t row1 = getRow(b.getMaxY());

        // Shrink xmin and ymax to fit the upper-left corner of the supplied extent
        double snapped_xmin = m_extent.getMinX() + static_cast<double>(col0 - extent_tag::padding) * m_dx;
        double snapped_ymax = m_extent.getMaxY() - static_cast<double>(row1 - extent_tag::padding) * m_dy;

        // Make sure snapped_xmin and snapped_ymax are within the reduced extent.
        // Because of floating point round-off errors, this is not always the case.
        if (b.getMinX() < snapped_xmin) {
            snapped_xmin -= m_dx;
            col0--;
        }

        if (b.getMaxY() > snapped_ymax) {
            snapped_ymax += m_dy;
            row1--;
        }

        size_t col1 = getColumn(b.getMaxX());
        size_t row0 = getRow(b.getMinY());

        size_t num_rows = 1 + (row0 - row1);
        size_t num_cols = 1 + (col1 - col0);

        // If xmax or ymin falls cleanly on a cell boundary, we don't
        // need as many rows or columns as we otherwise would, because
        // we assume that the rightmost cell of the grid is a closed
        // interval in x, and the lowermost cell of the grid is a
        // closed interval in y.
        if (num_rows > 2 && (snapped_ymax - static_cast<double>(num_rows - 1) * m_dy <= b.getMinY())) {
            num_rows--;
        }
        if (num_cols > 2 && (snapped_xmin + static_cast<double>(num_cols - 1) * m_dx >= b.getMaxX())) {
            num_cols--;
        }

        const double snapped_xmax = m_extent.getMinX() + static_cast<double>(col0 + num_cols - extent_tag::padding) * m_dx;
        const double snapped_ymin = m_extent.getMaxY() - static_cast<double>(row1 + num_rows - extent_tag::padding) * m_dy;

        // Perform offsets relative to the new xmin, ymax origin
        // points. If this is not done, then floating point roundoff
        // error can cause progressive shrink() calls with the same
        // inputs to produce different results.
        geom::Envelope reduced_box = {
            snapped_xmin,
            calcExtentFromNewGrid ? std::max(snapped_xmin + static_cast<double>(num_cols) * m_dx, b.getMaxX()) : snapped_xmax,
            calcExtentFromNewGrid ? std::min(snapped_ymax - static_cast<double>(num_rows) * m_dy, b.getMinY()) : snapped_ymin,
            snapped_ymax
        };

        // Fudge computed xmax and ymin, if needed, to prevent extent
        // from growing during a shrink operation.
        if (reduced_box.getMaxX() > m_extent.getMaxX()) {
            if (std::round(reduced_box.getWidth() / m_dx) ==
                std::round((m_extent.getMaxX() - reduced_box.getMinX()) / m_dx)) {
                reduced_box = geom::Envelope(reduced_box.getMinX(), m_extent.getMaxX(), reduced_box.getMinY(), reduced_box.getMaxY());
            } else {
                throw std::runtime_error("Shrink operation failed.");
            }
        }
        if (reduced_box.getMinY() < m_extent.getMinY()) {
            if (std::round(reduced_box.getHeight() / m_dy) ==
                std::round((reduced_box.getMaxY() - m_extent.getMinY()) / m_dy)) {
                reduced_box = geom::Envelope(reduced_box.getMinX(), reduced_box.getMaxX(), m_extent.getMinY(), reduced_box.getMaxY());
            } else {
                throw std::runtime_error("Shrink operation failed.");
            }
        }

        if constexpr (std::is_same_v<extent_tag, bounded_extent>) {
            Grid<extent_tag> reduced{reduced_box, m_dx, m_dy};

            if (!calcExtentFromNewGrid) {
                reduced.m_xOrig = m_xOrig;
                reduced.m_yOrig = m_yOrig;
                reduced.m_skipRows = reduced.getRowOffset(*this);
                reduced.m_skipCols = reduced.getColOffset(*this);
            } else if (!reduced.getExtent().contains(b)) {
                throw std::runtime_error("Shrink operation failed.");
            }
            return reduced;
        } else {
            Grid<extent_tag> reduced{reduced_box, m_dx, m_dy, m_domain};

            if (!calcExtentFromNewGrid) {
                reduced.m_xOrig = m_xOrig;
                reduced.m_yOrig = m_yOrig;
                reduced.m_skipRows = reduced.getRowOffset(*this);
                reduced.m_skipCols = reduced.getColOffset(*this);
            }else if (!reduced.getExtent().contains(b)) {
                throw std::runtime_error("Shrink operation failed.");
            }
            return reduced;
        }
    }

    bool operator==(const Grid<extent_tag>& b) const
    {
        return m_extent == b.m_extent &&
               m_dx == b.m_dx &&
               m_dy == b.m_dy;
    }

    bool operator!=(const Grid<extent_tag>& b) const
    {
        return !(*this == b);
    }

    geom::Envelope getCellEnvelope(size_t row, size_t col) const;

  private:
    geom::Envelope m_extent;
    geom::Envelope m_domain;

    double m_dx;
    double m_dy;

    double m_xOrig; // origin point that is distinct from xmin of m_extent. Used to allow a subgrid to calculate
                    // sub-envelopes that exactly match those of the parent grid
    double m_yOrig;

    size_t m_skipRows{0}; // number of rows to skip when computing a cell envelope using m_yOrig
    size_t m_skipCols{0}; // number of cols to skip when computing a cell envelope using m_xOrig

    size_t m_num_rows;
    size_t m_num_cols;

    friend Grid<infinite_extent> make_infinite(const Grid<bounded_extent>&, const geom::Envelope&);
    friend Grid<bounded_extent> make_finite(const Grid<infinite_extent>&);
};

Grid<infinite_extent>
make_infinite(const Grid<bounded_extent>& grid, const geom::Envelope& domain);
Grid<bounded_extent>
make_finite(const Grid<infinite_extent>& grid);

}
