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

#include <geos/geom/Envelope.h>
#include <geos/geom/Geometry.h>

#include <geos/operation/grid/Side.h>
#include <geos/operation/grid/Traversal.h>

namespace geos::operation::grid {

/**
 * @brief The Cell class stores information about the spatial extent of a `Grid` cell and
 *        any cases where a line crosses that cell (recorded in a `Traversal`).
 */
class Cell
{

  public:
    Cell(double xmin, double ymin, double xmax, double ymax)
      : m_box{ xmin, ymin, xmax, ymax }
    {
    }

    explicit Cell(const geom::Envelope& b)
      : m_box{ b }
    {
    }

    /// Get all points that fall on the specified side of this cell.
    void getEdgePoints(Side s, std::vector<geom::CoordinateXY>& points) const;

    const geom::Envelope& box() const { return m_box; }

    double getWidth() const;

    double getHeight() const;

    double getArea() const;

    /// Force the last Coordinate processed (via `take`) to be considered as an
    /// exit point, provided that it lies on the boundary of this Cell.
    void forceExit();

    /// Returns whether the cell can be determined to be wholly or partially
    /// covered by a polygon.
    bool isDetermined() const;

    /// Return the total length of a linear geometry within this Cell
    double getTraversalLength() const;

    /// Return the fraction of this Cell that is covered by a polygon
    double getCoveredFraction() const;

    /// Return a newly constructed geometry representing the portion of this Cell
    /// that is covered by a polygon
    std::unique_ptr<geom::Geometry> getCoveredPolygons(const geom::GeometryFactory&) const;

    /// Return the last (most recent) traversal to which a coordinate has been
    /// added. The traversal may or may not be completed.
    Traversal& getLastTraversal();

    /**
     * Attempt to take a coordinate and add it to a Traversal in progress, or start a new Traversal
     *
     * @param c             Coordinate to process
     * @param prev_original The last *uninterpolated* coordinate preceding `c` in the
     *                      boundary being processed
     * @param exitOnBoundary Whether to exit the cell if `c` lies on the boundary. This is needed to ensure that
     *                       exit nodes are added to both polygons whose shared boundary is coincident with the
     *                       Cell boundary. If these nodes are not needed (for example, in the area-only case),
     *                       it is more efficient to _not_ exit the cell.
     * @param parentage an optional pointer indicating the source of the coordinate.
     *
     * @return `true` if the Coordinate was inside this cell, `false` otherwise
     */
    bool take(const geom::CoordinateXY& c, const geom::CoordinateXY* prev_original, bool exitOnBoundary, const void* parentage);

  private:
    std::vector<const Traversal*> getTraversals() const;

    enum class Location
    {
        INSIDE,
        OUTSIDE,
        BOUNDARY
    };

    geom::Envelope m_box;

    std::vector<Traversal> m_traversals;

    Side getSide(const geom::CoordinateXY& c) const;

    Location getLocation(const geom::CoordinateXY& c) const;

    /// If no Traversals have been started or the most recent Traversal has been completed,
    /// return a new Traversal. Otherwise, return the most recent Traversal.
    Traversal& traversal_in_progress();
};

}
