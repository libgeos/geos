/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2020 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/geom/Point.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/LineString.h>
#include <geos/geom/Geometry.h>

#include <geos/export.h>

#include <vector>
#include <memory>

// Forward declarations
namespace geos {
namespace geom {
class Coordinate;
class CoordinateSequence;
class Envelope;
class GeometryFactory;
class PrecisionModel;
}
namespace operation {
namespace overlayng {
class InputGeometry;
class OverlayGraph;
}
}
}

namespace geos {      // geos.
namespace operation { // geos.operation
namespace overlayng { // geos.operation.overlayng

/**
 * Utility methods for overlay processing.
 *
 * @author mdavis
 *
 */
class GEOS_DLL OverlayUtil {
    using Geometry = geos::geom::Geometry;
    using Coordinate = geos::geom::Coordinate;
    using CoordinateSequence = geos::geom::CoordinateSequence;
    using Envelope = geos::geom::Envelope;
    using Point = geos::geom::Point;
    using LineString = geos::geom::LineString;
    using Polygon = geos::geom::Polygon;
    using GeometryFactory = geos::geom::GeometryFactory;
    using PrecisionModel = geos::geom::PrecisionModel;

private:

    static constexpr double SAFE_ENV_BUFFER_FACTOR = 0.1;
    static constexpr int SAFE_ENV_GRID_FACTOR = 3;
    static constexpr double AREA_HEURISTIC_TOLERANCE = 0.1;

    /**
    * Computes an envelope which covers the extent of the result of
    * a given overlay operation for given inputs.
    * The operations which have a result envelope smaller than the extent of the inputs
    * are:
    *
    * - INTERSECTION: result envelope is the intersection of the input envelopes
    * - DIFFERENCE: result envelope is the envelope of the A input geometry
    *
    * Otherwise, <code>null</code> is returned to indicate full extent.
    */
    static bool resultEnvelope(int opCode, const InputGeometry* inputGeom, const PrecisionModel* pm, Envelope& rsltEnvelope);
    static double safeExpandDistance(const Envelope* env, const PrecisionModel* pm);
    static bool safeEnv(const Envelope* env, const PrecisionModel* pm, Envelope& rsltEnvelope);

    static bool isEmpty(const Geometry* geom);

    /**
    * Tests for disjoint envelopes adjusting for rounding
    * caused by a fixed precision model.
    * Assumes envelopes are non-empty.
    */
    static bool isDisjoint(const Envelope* envA, const Envelope* envB, const PrecisionModel* pm);

    static bool isLess(double v1, double v2, double tol) {
        return v1 <= v2 * (1 + tol);
    };

    static bool isGreater(double v1, double v2, double tol) {
        return v1 >= v2 * (1 - tol);
    }


public:

    static bool isFloating(const PrecisionModel* pm);

    /**
    * Computes a clipping envelope for overlay input geometries.
    * The clipping envelope encloses all geometry line segments which
    * might participate in the overlay, with a buffer to
    * account for numerical precision
    * (in particular, rounding due to a precision model.
    * The clipping envelope is used in both the {@link RingClipper}
    * and in the {@link LineLimiter}.
    *
    * Some overlay operations (i.e. UNION and SYMDIFFERENCE
    * cannot use clipping as an optimization,
    * since the result envelope is the full extent of the two input geometries.
    * In this case the returned
    * envelope is null to indicate this.
    */
    static bool clippingEnvelope(int opCode, const InputGeometry* inputGeom, const PrecisionModel* pm, Envelope& rsltEnvelope);

    /**
    * Tests if the result can be determined to be empty
    * based on simple properties of the input geometries
    * (such as whether one or both are empty,
    * or their envelopes are disjoint).
    */
    static bool isEmptyResult(int opCode, const Geometry* a, const Geometry* b, const PrecisionModel* pm);

    /**
    * Tests if the geometry envelopes are disjoint, or empty.
    * The disjoint test must take into account the precision model
    * being used, since geometry coordinates may shift under rounding.
    */
    static bool isEnvDisjoint(const Geometry* a, const Geometry* b, const PrecisionModel* pm);

    /**
    * Creates an empty result geometry of the appropriate dimension and coordinate dimension,
    * based on the given overlay operation and the dimensions of the inputs.
    * The created geometry is an atomic geometry,
    * not a collection (unless the dimension is -1,
    * in which case a GEOMETRYCOLLECTION EMPTY is created.)
    */
    static std::unique_ptr<Geometry> createEmptyResult(int dim, size_t coordDim, const GeometryFactory* geomFact);

    /**
    * Computes the dimension of the result of
    * applying the given operation to inputs
    * with the given dimensions.
    * This assumes that complete collapse does not occur.
    *
    * The result dimension is computed according to the following rules:
    * - OverlayNG::INTERSECTION - result has the dimension of the lowest input dimension
    * - OverlayNG::UNION - result has the dimension of the highest input dimension
    * - OverlayNG::DIFFERENCE - result has the dimension of the left-hand input
    * - OverlayNG::SYMDIFFERENCE - result has the dimension of the highest input dimension
    * (since the Symmetric Difference is the Union of the Differences).
    */
    static int resultDimension(int opCode, int dim0, int dim1);

    /**
     * Computes the coordinate dimension of overlaying two geometries.
     * This is the smallest of the two coordinate dimensions 
     * (to avoid having to populate Z and M with unknown values).
     */
    static uint8_t resultCoordinateDimension(uint8_t coordDim0, uint8_t coordDim1);

    /**
    * Creates an overlay result geometry for homogeneous or mixed components.
    */
    static std::unique_ptr<Geometry> createResultGeometry(
        std::vector<std::unique_ptr<Polygon>>& resultPolyList,
        std::vector<std::unique_ptr<LineString>>& resultLineList,
        std::vector<std::unique_ptr<Point>>& resultPointList,
        const GeometryFactory* geometryFactory);

    static std::unique_ptr<Geometry> toLines(OverlayGraph* graph, bool isOutputEdges, const GeometryFactory* geomFact);

    /**
    * A heuristic check for overlay result correctness
    * comparing the areas of the input and result.
    * The heuristic is necessarily coarse, but it detects some obvious issues.
    * (e.g. https://github.com/locationtech/jts/issues/798)
    * <p>
    * <b>Note:</b> - this check is only safe if the precision model is floating.
    * It should also be safe for snapping noding if the distance tolerance is reasonably small.
    * (Fixed precision models can lead to collapse causing result area to expand.)
    *
    * @param geom0 input geometry 0
    * @param geom1 input geometry 1
    * @param opCode the overlay opcode
    * @param result the overlay result
    * @return true if the result area is consistent
    */
    static bool isResultAreaConsistent(
        const Geometry* geom0, const Geometry* geom1,
        int opCode, const Geometry* result);

    /**
    * Round the key point if precision model is fixed.
    * Note: return value is only copied if rounding is performed.
    */
    static bool round(const Point* pt, const PrecisionModel* pm, Coordinate& rsltCoord);

    template<typename T>
    static void moveGeometry(std::vector<std::unique_ptr<T>>& inGeoms, std::vector<std::unique_ptr<Geometry>>& outGeoms)
    {
        static_assert(std::is_base_of<Geometry, T>::value, "");
        for (auto& geom: inGeoms) {
            Geometry* outGeom = static_cast<Geometry*>(geom.release());
            outGeoms.emplace_back(outGeom);
        }
        return;
    }


};


} // namespace geos.operation.overlayng
} // namespace geos.operation
} // namespace geos

