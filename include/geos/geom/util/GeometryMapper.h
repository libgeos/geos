/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2021 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: geom/util/GeometryMapper.java
 *
 **********************************************************************/

#pragma once

#include <memory>
#include <vector>
#include <functional>

#include <geos/export.h>

// Forward declarations
namespace geos {
namespace geom {
class Geometry;
class GeometryCollection;
class GeometryFactory;
}
}

namespace geos {
namespace geom { // geos.geom
namespace util { // geos.geom.util


/**
 * Methods to map various collections
 * of {@link Geometry}s
 * via defined mapping functions.
 *
 * @author Martin Davis
 *
 */
class GEOS_DLL GeometryMapper {

public:

    /**
    * An interface for geometry functions that map a geometry input to a geometry output.
    * The output may be nullptr if there is no valid output value for
    * the given input value.
    */
    typedef std::function<std::unique_ptr<Geometry>(const Geometry&)> mapOp;

    /**
    * Maps the members of a {@link Geometry}
    * (which may be atomic or composite)
    * into another Geometry of most specific type.
    * null results are skipped.
    * In the case of hierarchical {@link GeometryCollection}s,
    * only the first level of members are mapped.
    *
    * @param geom the input atomic or composite geometry
    * @param op the mapping operation
    * @return a result collection or geometry of most specific type
    */
    static std::unique_ptr<Geometry> map(
        const Geometry& geom,
        mapOp op);

    /**
    * Maps the atomic elements of a {@link Geometry}
    * (which may be atomic or composite)
    * using a mapOp mapping operation
    * into an atomic Geometry or a flat collection
    * of the most specific type.
    * null and empty values returned from the mapping operation
    * are discarded.
    *
    * @param geom the geometry to map
    * @param emptyDim the dimension of empty geometry to create
    * @param op the mapping operation
    * @return the mapped result
    */
    static std::unique_ptr<Geometry> flatMap(
        const Geometry& geom,
        int emptyDim,
        mapOp op);



private:

    static void flatMap(
        const Geometry& geom,
        mapOp op,
        std::vector<std::unique_ptr<Geometry>>& mapped);

    static void addFlat(
        std::unique_ptr<Geometry>& geom,
        std::vector<std::unique_ptr<Geometry>>& geomList);



};

} // namespace geos.geom.util
} // namespace geos.geom
} // namespace geos

