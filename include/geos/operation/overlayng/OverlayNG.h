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

#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/operation/overlay/OverlayOp.h>
#include <geos/operation/overlayng/OverlayGraph.h>
#include <geos/operation/overlayng/OverlayEdgeRing.h>
#include <geos/operation/overlayng/InputGeometry.h>
#include <geos/export.h>

// Forward declarations
namespace geos {
namespace geom {
class GeometryFactory;
class PrecisionModel;
}
namespace noding {
class Noder;
}
namespace operation {
namespace overlayng {
}
}
}

namespace geos {      // geos.
namespace operation { // geos.operation
namespace overlayng { // geos.operation.overlayng



class GEOS_DLL OverlayNG {

private:

    // Members
    const geom::PrecisionModel* pm;
    InputGeometry inputGeom;
    const geom::GeometryFactory* geomFact;
    int opCode;
    noding::Noder* noder;
    bool isOptimized;
    bool isOutputEdges;
    bool isOutputResultEdges;
    bool isOutputNodedEdges;

    // Methods
    std::unique_ptr<geom::Geometry> computeEdgeOverlay();
    void labelGraph(OverlayGraph* graph);

    /**
    * Extracts the result geometry components from the fully labelled topology graph.
    *
    * This method implements the semantic that the result of an
    * intersection operation is homogeneous with highest dimension.
    * In other words,
    * if an intersection has components of a given dimension
    * no lower-dimension components are output.
    * For example, if two polygons intersect in an area,
    * no linestrings or points are included in the result,
    * even if portions of the input do meet in lines or points.
    * This semantic choice makes more sense for typical usage,
    * in which only the highest dimension components are of interest.
    */
    std::unique_ptr<geom::Geometry> extractResult(int opCode, OverlayGraph* graph);
    std::unique_ptr<geom::Geometry> createEmptyResult();



public:

    static constexpr int INTERSECTION   = overlay::OverlayOp::opINTERSECTION;
    static constexpr int UNION          = overlay::OverlayOp::opUNION;
    static constexpr int DIFFERENCE     = overlay::OverlayOp::opDIFFERENCE;
    static constexpr int SYMDIFFERENCE  = overlay::OverlayOp::opSYMDIFFERENCE;

    /**
    * Creates an overlay operation on the given geometries,
    * with a defined precision model.
    * The noding strategy is determined by the precision model.
    */
    OverlayNG(const geom::Geometry* geom0, const geom::Geometry* geom1, const geom::PrecisionModel* p_pm, int p_opCode)
        : pm(p_pm)
        , inputGeom(geom0, geom1)
        , geomFact(geom0->getFactory())
        , opCode(p_opCode)
        , noder(nullptr)
        , isOptimized(true)
        , isOutputEdges(false)
        , isOutputResultEdges(false)
        , isOutputNodedEdges(false)
    {}

    /**
    * Creates an overlay operation on the given geometries
    * using the precision model of the geometries.
    *
    * The noder is chosen according to the precision model specified.
    *
    *  - For {@link PrecisionModel#FIXED}
    * a snap-rounding noder is used, and the computation is robust.
    *  - For {@link PrecisionModel#FLOATING}
    * a non-snapping noder is used,
    * and this computation may not be robust.
    * If errors occur a {@link TopologyException} is thrown.
    */
    OverlayNG(const geom::Geometry* geom0, const geom::Geometry* geom1, int p_opCode)
        : OverlayNG(geom0, geom1, geom0->getFactory()->getPrecisionModel(), p_opCode)
    {}

    OverlayNG(const geom::Geometry* geom0, const geom::PrecisionModel* p_pm)
        : OverlayNG(geom0, nullptr, p_pm, UNION)
    {}

    /**
    * Sets whether overlay processing optimizations are enabled.
    * It may be useful to disable optimizations
    * for testing purposes.
    * Default is TRUE (optimization enabled).
    *
    * @param isOptimized whether to optimize processing
    */
    void setOptimized(bool p_isOptimized) { isOptimized = p_isOptimized; }
    void setOutputEdges(bool p_isOutputEdges) { isOutputEdges = p_isOutputEdges; }
    void setOutputResultEdges(bool p_isOutputResultEdges) { isOutputResultEdges = p_isOutputResultEdges; }
    void setNoder(noding::Noder* p_noder) { noder = p_noder; }

    void setOutputNodedEdges(bool p_isOutputNodedEdges)
    {
        isOutputEdges = true;
        isOutputNodedEdges = p_isOutputNodedEdges;
    }

    std::unique_ptr<Geometry> getResult();

    /**
    * Tests whether a point with a given topological {@link Label}
    * relative to two geometries is contained in
    * the result of overlaying the geometries using
    * a given overlay operation.
    *
    * The method handles arguments of {@link Location#NONE} correctly
    */
    static bool isResultOfOpPoint(const OverlayLabel* label, int opCode);

    /**
    * Tests whether a point with given {@link Location}s
    * relative to two geometries would be contained in
    * the result of overlaying the geometries using
    * a given overlay operation.
    * This is used to determine whether components
    * computed during the overlay process should be
    * included in the result geometry.
    *
    * The method handles arguments of {@link Location#NONE} correctly.
    */
    static bool isResultOfOp(int overlayOpCode, Location loc0, Location loc1);

    /**
    * Computes an overlay operation for
    * the given geometry operands, with the
    * noding strategy determined by the precision model.
    *
    * @param geom0 the first geometry argument
    * @param geom1 the second geometry argument
    * @param opCode the code for the desired overlay operation
    * @param pm the precision model to use
    * @return the result of the overlay operation
    */
    static std::unique_ptr<Geometry>
    overlay(const Geometry* geom0, const Geometry* geom1,
            int opCode, const PrecisionModel* pm);


    /**
    * Computes an overlay operation on the given geometry operands,
    * using a supplied {@link Noder}.
    *
    * @param geom0 the first geometry argument
    * @param geom1 the second geometry argument
    * @param opCode the code for the desired overlay operation
    * @param pm the precision model to use (which may be null if the noder does not use one)
    * @param noder the noder to use
    * @return the result of the overlay operation
    */
    static std::unique_ptr<Geometry>
    overlay(const Geometry* geom0, const Geometry* geom1,
            int opCode, const PrecisionModel* pm, noding::Noder* noder);


    /**
    * Computes an overlay operation on the given geometry operands,
    * using a supplied {@link Noder}.
    *
    * @param geom0 the first geometry argument
    * @param geom1 the second geometry argument
    * @param opCode the code for the desired overlay operation
    * @param noder the noder to use
    * @return the result of the overlay operation
    */
    static std::unique_ptr<Geometry>
    overlay(const Geometry* geom0, const Geometry* geom1,
            int opCode, noding::Noder* noder);

    /**
    * Computes an overlay operation on
    * the given geometry operands,
    * using the precision model of the geometry.
    * and an appropriate noder.
    *
    * The noder is chosen according to the precision model specified.
    *
    *  - For {@link PrecisionModel#FIXED}
    *    a snap-rounding noder is used, and the computation is robust.
    *  - For {@link PrecisionModel#FLOATING}
    *    a non-snapping noder is used,
    *    and this computation may not be robust.
    * If errors occur a {@link TopologyException} is thrown.
    *
    * @param geom0 the first argument geometry
    * @param geom1 the second argument geometry
    * @param opCode the code for the desired overlay operation
    * @return the result of the overlay operation
    */
    static std::unique_ptr<Geometry>
    overlay(const Geometry* geom0, const Geometry* geom1, int opCode);


    /**
    * Computes a union operation on
    * the given geometry, with the supplied precision model.
    * The primary use for this is to perform precision reduction
    * (round the geometry to the supplied precision).
    *
    * The input must be a valid geometry.
    * Collections must be homogeneous.
    *
    * To union an overlapping set of polygons in a more performant way use {@link UnaryUnionNG}.
    * To union a polyonal coverage or linear network in a more performant way,
    * use {@link CoverageUnion}.
    *
    * @param geom0 the geometry
    * @param pm the precision model to use
    * @return the result of the union operation
    *
    * @see OverlayMixedPoints
    * @see PrecisionReducer
    * @see UnaryUnionNG
    * @see CoverageUnion
    */
    static std::unique_ptr<Geometry>
    geomunion(const Geometry* geom, const PrecisionModel* pm);


    /**
    * Computes a union of a single geometry using a custom noder.
    *
    * The primary use of this is to support coverage union.
    *
    * @param geom the geometry to union
    * @param pm the precision model to use (maybe be null)
    * @param noder the noder to use
    * @return the result geometry
    *
    * @see CoverageUnion
    */
    static std::unique_ptr<Geometry>
    geomunion(const Geometry* geom, const PrecisionModel* pm, noding::Noder* noder);




};


} // namespace geos.operation.overlayng
} // namespace geos.operation
} // namespace geos

