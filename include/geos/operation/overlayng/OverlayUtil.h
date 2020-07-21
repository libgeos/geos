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

#include <geos/export.h>

#include <vector>
#include <memory>

// Forward declarations
namespace geos {
namespace geom {
class Coordinate;
class CoordinateSequence;
class Envelope;
class Geometry;
class GeometryFactory;
class LineString;
class Point;
class Polygon;
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

using namespace geos::geom;

/**
 * Utility methods for overlay processing.
 *
 * @author mdavis
 *
 */
class GEOS_DLL OverlayUtil {

private:

    static constexpr double SAFE_ENV_BUFFER_FACTOR = 0.1;
    static constexpr int SAFE_ENV_GRID_FACTOR = 3;

    static bool overlapEnvelope(int opCode, const InputGeometry* inputGeom, const PrecisionModel* pm, Envelope& rsltEnvelope);
    static double safeExpandDistance(const Envelope* env, const PrecisionModel* pm);
    static bool safeEnv(const Envelope* env, const PrecisionModel* pm, Envelope& rsltEnvelope);

    static bool isEmpty(const Geometry* geom);

    /**
    * Tests for disjoint envelopes adjusting for rounding
    * caused by a fixed precision model.
    * Assumes envelopes are non-empty.
    */
    static bool isDisjoint(const Envelope* envA, const Envelope* envB, const PrecisionModel* pm);


public:

    static bool isFloating(const PrecisionModel* pm);
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
    * Creates an empty result geometry of the appropriate dimension,
    * based on the given overlay operation and the dimensions of the inputs.
    * The created geometry is an atomic geometry,
    * not a collection (unless the dimension is -1,
    * in which case a GEOMETRYCOLLECTION EMPTY is created.)
    */
    static std::unique_ptr<Geometry> createEmptyResult(int dim, const GeometryFactory* geomFact);

    /**
    * Computes the dimension of the result of
    * applying the given operation to inputs
    * with the given dimensions.
    * This assumes that complete collapse does not occur.
    *
    * The result dimension is computed according to the following rules:
    * - {@link OverlayNG#INTERSECTION} - result has the dimension of the lowest input dimension
    * - {@link OverlayNG#UNION} - result has the dimension of the highest input dimension
    * - {@link OverlayNG#DIFFERENCE} - result has the dimension of the left-hand input
    * - {@link OverlayNG#SYMDIFFERENCE} - result has the dimension of the highest input dimension
    * (since the Symmetric Difference is the Union of the Differences).
    */
    static int resultDimension(int opCode, int dim0, int dim1);

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

