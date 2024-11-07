/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2023 Martin Davis <mtnclimb@gmail.com>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: algorithm/construct/IndexedDistanceToPoint.java
 * https://github.com/locationtech/jts/commit/d92f783163d9440fcc10c729143787bf7b9fe8f9
 *
 **********************************************************************/

#pragma once

#include <geos/geom/Coordinate.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/Location.h>
#include <geos/index/strtree/TemplateSTRtree.h>
#include <geos/algorithm/locate/IndexedPointInAreaLocator.h>

namespace geos {
namespace algorithm { // geos::algorithm
namespace construct { // geos::algorithm::construct

/**
 * \brief Determines the location of a point in the polygonal elements of a geometry.
 * 
 * Uses spatial indexing to provide efficient performance.
 * 
 * \author Martin Davis
 */
class GEOS_DLL IndexedPointInPolygonsLocator {
    using Geometry = geos::geom::Geometry;
    using CoordinateXY = geos::geom::CoordinateXY;
    using Location = geos::geom::Location;
    template<typename ItemType>
    using TemplateSTRtree = geos::index::strtree::TemplateSTRtree<ItemType>;
    using IndexedPointInAreaLocator = geos::algorithm::locate::IndexedPointInAreaLocator;

public:
    /** 
     * \brief Creates an instance to locate a point in polygonal elements.
     * 
     * \param geom the geometry to locate in
     */
    IndexedPointInPolygonsLocator(const Geometry& geom);

    /** \brief
     * Determines the [Location](@ref geom::Location) of a point in 
     * the polygonal elements of a
     * [Geometry](@ref geom::Geometry).
     *
     * @param p the point to test
     * @return the location of the point in the geometry
     */
    Location locate(const CoordinateXY* /*const*/ p);

private:
    void init();

    // Declare type as noncopyable
    IndexedPointInPolygonsLocator(const IndexedPointInPolygonsLocator& other) = delete;
    IndexedPointInPolygonsLocator& operator=(const IndexedPointInPolygonsLocator& rhs) = delete;

    //-- members
    const Geometry& geom;
    bool isInitialized;
    TemplateSTRtree<IndexedPointInAreaLocator*> index;
    std::vector<std::unique_ptr<IndexedPointInAreaLocator>> locators;
};

}}}