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

#include <memory> // for unique_ptr

// Forward declarations
namespace geos {
namespace algorithm {
class CircularArcIntersector;
}
namespace geom {
class Geometry;
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

    GeometryNoder(const geom::Geometry& g);

    GeometryNoder(const geom::Geometry& g1, const geom::Geometry& g2);

    ~GeometryNoder();

    std::unique_ptr<geom::Geometry> getNoded();

    void setOnlyFirstGeomEdges(bool onlyFirstGeomEdges);

    void setPreserveCompoundCurves(bool preserve);

    // Declare type as noncopyable
    GeometryNoder(GeometryNoder const&) = delete;
    GeometryNoder& operator=(GeometryNoder const&) = delete;

private:

    bool isInResult(const PathString& ps) const;

    const geom::Geometry* argGeom1;
    const geom::Geometry* argGeom2;
    const bool argGeomHasCurves;
    bool argGeomHasCompoundCurves;
    bool onlyFirstGeomEdges;
    bool preserveCompoundCurves;

    std::unique_ptr<Noder> noder;
    std::unique_ptr<algorithm::CircularArcIntersector> m_cai;
    std::unique_ptr<ArcIntersectionAdder> m_aia;

    void extractPathStrings(const geom::Geometry& g, std::vector<std::unique_ptr<PathString>>& to);

    Noder& getNoder();

    std::unique_ptr<geom::Geometry> toGeometry(std::vector<std::unique_ptr<PathString>>& noded) const;

};

} // namespace geos.noding
} // namespace geos
