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

#include <geos/noding/GeometryNoder.h>
#include <geos/noding/SegmentString.h>
#include <geos/noding/NodedSegmentString.h>
#include <geos/noding/OrientedCoordinateArray.h>
#include <geos/noding/Noder.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/CircularString.h>
#include <geos/geom/MultiCurve.h>
#include <geos/geom/LineString.h>

#include <geos/noding/IteratedNoder.h>
#include <geos/noding/NodableArcString.h>
#include <geos/noding/SimpleNoder.h>

#include <geos/algorithm/LineIntersector.h>
#include <geos/noding/IntersectionAdder.h>
#include <geos/noding/MCIndexNoder.h>

#include <geos/noding/snapround/MCIndexSnapRounder.h>

#include <memory> // for unique_ptr
#include <iostream>

namespace geos {
namespace noding { // geos.noding

namespace {

/**
 * Add every linear element in a geometry into SegmentString vector
 */
class PathStringExtractor: public geom::GeometryComponentFilter {
public:
    PathStringExtractor(std::vector<std::unique_ptr<PathString>> & to,
                           bool constructZ,
                           bool constructM)
        : _to(to)
        , _constructZ(constructZ)
        , _constructM(constructM)
    {}

    void
    filter_ro(const geom::Geometry* g) override
    {
        if(const auto* ls = dynamic_cast<const geom::LineString*>(g)) {
            auto coord = ls->getSharedCoordinates();
            // coord ownership transferred to SegmentString
            auto ss = std::make_unique<NodedSegmentString>(coord, _constructZ, _constructM, nullptr);
            _to.push_back(std::move(ss));
        } else if (const auto* cs = dynamic_cast<const geom::CircularString*>(g)) {
            auto coords = cs->getCoordinates();

            // TODO: Store this vector in the CircularString ?
            std::vector<geom::CircularArc> arcs;
            for (std::size_t i = 0; i < coords->getSize() - 2; i += 2) {
                arcs.emplace_back(*coords, i);
            }

            auto as = std::make_unique<NodableArcString>(std::move(arcs), std::move(coords), _constructZ, _constructM, nullptr);
            _to.push_back(std::move(as));
        }
    }
private:
    std::vector<std::unique_ptr<PathString>>& _to;
    bool _constructZ;
    bool _constructM;

    PathStringExtractor(PathStringExtractor const&); /*= delete*/
    PathStringExtractor& operator=(PathStringExtractor const&); /*= delete*/
};

}


/* public static */
std::unique_ptr<geom::Geometry>
GeometryNoder::node(const geom::Geometry& geom)
{
    GeometryNoder noder(geom);
    return noder.getNoded();
}

/* public */
GeometryNoder::GeometryNoder(const geom::Geometry& g)
    :
    argGeom(g),
    argGeomHasCurves(g.hasCurvedComponents())
{}

GeometryNoder::~GeometryNoder() = default;

/* private */
std::unique_ptr<geom::Geometry>
GeometryNoder::toGeometry(std::vector<std::unique_ptr<PathString>>& nodedEdges) const
{
    const geom::GeometryFactory* geomFact = argGeom.getFactory();

    std::set< OrientedCoordinateArray > ocas;

    // Create a geometry out of the noded substrings.
    std::vector<std::unique_ptr<geom::Geometry>> lines;
    lines.reserve(nodedEdges.size());

    bool resultArcs = false;
    for(auto& path :  nodedEdges) {
        if (const auto* ss = dynamic_cast<SegmentString*>(path.get())) {
            const auto& coords = ss->getCoordinates();

            // Check if an equivalent edge is known
            OrientedCoordinateArray oca1(*coords);
            if(ocas.insert(oca1).second) {
                lines.push_back(geomFact->createLineString(coords));
            }
        } else {
            resultArcs = true;
            auto* as = dynamic_cast<ArcString*>(path.get());
            // FIXME: check for duplicates
            lines.push_back(geomFact->createCircularString(as->releaseCoordinates()));
        }
    }

    if (resultArcs) {
        return geomFact->createMultiCurve(std::move(lines));
    } else {
        return geomFact->createMultiLineString(std::move(lines));
    }
}

/* public */
std::unique_ptr<geom::Geometry>
GeometryNoder::getNoded()
{
    if (argGeom.isEmpty())
        return argGeom.clone();

    std::vector<std::unique_ptr<PathString>> p_lineList;
    extractPathStrings(argGeom, p_lineList);

    ArcNoder& p_noder = getNoder();
    std::vector<std::unique_ptr<PathString>> nodedEdges;

    try {
        p_noder.computePathNodes(PathString::toRawPointerVector(p_lineList));
        nodedEdges = p_noder.getNodedPaths();
    }
    catch(const std::exception&) {
        throw;
    }

    std::unique_ptr<geom::Geometry> noded = toGeometry(nodedEdges);

    return noded;
}

/* private static */
void
GeometryNoder::extractPathStrings(const geom::Geometry& g,
                                  std::vector<std::unique_ptr<PathString>>& to)
{
    PathStringExtractor ex(to, g.hasZ(), g.hasM());
    g.apply_ro(&ex);
}

/* private */
ArcNoder&
GeometryNoder::getNoder()
{
    if(! noder.get()) {
        const geom::PrecisionModel* pm = argGeom.getFactory()->getPrecisionModel();
        if (argGeomHasCurves) {
            noder = std::make_unique<IteratedNoder>(pm, []() { return std::make_unique<SimpleNoder>(); });
        } else {
            noder = std::make_unique<IteratedNoder>(pm);
        }
    }
    return *noder;
}


} // namespace geos.noding
} // namespace geos
