/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2012  Sandro Santilli <strk@kbt.io>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * NOTE: this is not in JTS. JTS has a snapround/GeometryNoder though
 *
 **********************************************************************/

#pragma once

#include <geos/export.h>
#include <geos/noding/SegmentString.h> // for NonConstVect

#include <cstddef>
#include <map>
#include <memory> // for unique_ptr
#include <vector>

// Forward declarations
namespace geos {
namespace algorithm {
class CircularArcIntersector;
}
namespace geom {
class Geometry;
class PrecisionModel;
}
namespace noding {
class ArcIntersectionAdder;
class Noder;
}
}

namespace geos {
namespace noding { // geos.noding

class GEOS_DLL GeometryNoder {
public:

    static std::unique_ptr<geom::Geometry> node(const geom::Geometry& geom);

    static std::unique_ptr<geom::Geometry> node(const geom::Geometry& geom1, const geom::Geometry& geom2);

    /**
     * Nodes a collection of linear (or curved) geometries against each
     * other, returning a collection of the same size with a 1:1
     * relationship to the input: output element `i` is the noded form
     * of input element `i`.
     *
     * Unlike node(const geom::Geometry&), which collects all input
     * edges into a single flattened result, this preserves the identity
     * of each input member. Linework that is shared (or nearly shared)
     * between members is not dissolved — every member that touches a
     * node is split there.
     *
     * Each output element is a MultiLineString (or a MultiCurve, if the
     * corresponding input contributed curved components). Consistent with
     * node(const geom::Geometry&), areal members are reduced to their
     * boundary linework: a polygon's exterior and interior rings are noded
     * like any other lines (and participate in noding the other members),
     * so its slot returns the noded boundary as a MultiLineString. Point
     * members — and any member with no linear or areal component — add
     * nothing to the noding and yield an empty MultiLineString in their slot.
     *
     * When `gridSize <= 0.0` (the default is 0.0) exact noding is used
     * (IteratedNoder for linear input, SimpleNoder with arc support when
     * curves are present). When `gridSize > 0.0` a SnapRoundingNoder is
     * used for robust output on near-coincident coordinates
     * (see https://github.com/libgeos/geos/issues/877). Snap-rounding is
     * linear-only: if the input contains curved components, `gridSize`
     * is ignored and exact arc noding is used.
     *
     * @param geoms input members; the caller retains ownership and the
     *   vector (and the geometries it points at) must outlive the call.
     *   Members must be distinct geometry objects (no aliasing).
     * @param gridSize snap-rounding grid size when > 0; any value <= 0.0
     *   selects exact noding
     * @return one noded geometry per input member, in input order
     */
    static std::vector<std::unique_ptr<geom::Geometry>> nodeCollection(
        const std::vector<const geom::Geometry*>& geoms,
        double gridSize = 0.0);

    GeometryNoder(const geom::Geometry& g);

    GeometryNoder(const geom::Geometry& g1, const geom::Geometry& g2);

    GeometryNoder(const std::vector<const geom::Geometry*>& geoms, double gridSize = 0.0);

    ~GeometryNoder();

    std::unique_ptr<geom::Geometry> getNoded();

    std::vector<std::unique_ptr<geom::Geometry>> getNodedCollection();

    void setOnlyFirstGeomEdges(bool onlyFirstGeomEdges);

    void setPreserveCompoundCurves(bool preserve);

    // Declare type as noncopyable
    GeometryNoder(GeometryNoder const&) = delete;
    GeometryNoder& operator=(GeometryNoder const&) = delete;

private:

    bool isInResult(const PathString& ps) const;

    static bool collectionHasCurves(const std::vector<const geom::Geometry*>& geoms);

    const geom::Geometry* argGeom1;
    const geom::Geometry* argGeom2;
    const bool argGeomHasCurves;
    bool argGeomHasCompoundCurves;
    bool onlyFirstGeomEdges;
    bool preserveCompoundCurves;

    // Collection-noding state (null/empty for single/two-geometry modes)
    const std::vector<const geom::Geometry*>* argColl = nullptr;
    double m_gridSize = 0.0;
    std::map<const void*, std::size_t> m_contextToMember;

    std::unique_ptr<Noder> noder;
    std::unique_ptr<geom::PrecisionModel> m_pm;
    std::unique_ptr<algorithm::CircularArcIntersector> m_cai;
    std::unique_ptr<ArcIntersectionAdder> m_aia;

    void extractPathStrings(const geom::Geometry& g, std::vector<std::unique_ptr<PathString>>& to);

    Noder& getNoder();

    std::unique_ptr<geom::Geometry> toGeometry(std::vector<std::unique_ptr<PathString>>& noded) const;

    std::vector<std::unique_ptr<geom::Geometry>> toGeometryCollection(
        std::vector<std::unique_ptr<PathString>>& noded) const;

    /**
     * Builds one output geometry for a single source geometry slot from
     * the noded paths selected for it. `selected` are the candidate
     * paths (deduplicated here); `blockers` are paths from other slots
     * whose endpoints must not be merged across when reconstructing
     * compound curves. `srcGeom` is the originating geometry (used for
     * its factory and, when preserving compound curves, its structure).
     */
    std::unique_ptr<geom::Geometry> buildSlot(
        const std::vector<PathString*>& selected,
        const std::vector<PathString*>& blockers,
        const geom::Geometry& srcGeom) const;

};

} // namespace geos.noding
} // namespace geos
