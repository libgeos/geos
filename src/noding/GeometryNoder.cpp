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

#include <geos/algorithm/CircularArcIntersector.h>

#include <geos/geom/Geometry.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/CompoundCurve.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/CircularString.h>
#include <geos/geom/MultiCurve.h>
#include <geos/geom/LineString.h>

#include <geos/noding/ArcIntersectionAdder.h>
#include <geos/noding/GeometryNoder.h>
#include <geos/noding/IteratedNoder.h>
#include <geos/noding/MCIndexNoder.h>
#include <geos/noding/NodableArcString.h>
#include <geos/noding/NodedSegmentString.h>
#include <geos/noding/Noder.h>
#include <geos/noding/OrientedCoordinateArray.h>
#include <geos/noding/SegmentString.h>
#include <geos/noding/SimpleNoder.h>

#include <geos/util/Assert.h>

#include <memory> // for unique_ptr

using geos::geom::CoordinateXY;
using geos::geom::SimpleCurve;
using geos::geom::Geometry;


namespace geos {
namespace noding { // geos.noding

namespace {

/**
 * Add every linear element in a geometry into PathString vector
 */
class PathStringExtractor: public geom::GeometryComponentFilter {
public:
    PathStringExtractor(std::vector<std::unique_ptr<PathString>> & to,
                           bool constructZ,
                           bool constructM)
        : _to(to)
        , _constructZ(constructZ)
        , _constructM(constructM)
        , _foundCompoundCurve(false)
    {}

    bool foundCompoundCurve() const {
        return _foundCompoundCurve;
    }

    void
    filter_ro(const Geometry* g) override
    {
        if (g->isEmpty()) {
            return;
        }

        if(const auto* ls = dynamic_cast<const geom::LineString*>(g)) {
            auto coord = ls->getSharedCoordinates();
            auto ss = std::make_unique<NodedSegmentString>(coord, _constructZ, _constructM, ls);
            _to.push_back(std::move(ss));
        } else if (const auto* cs = dynamic_cast<const geom::CircularString*>(g)) {
            const auto& coords = cs->getSharedCoordinates();
            auto arcs = cs->getArcs();

            auto as = std::make_unique<NodableArcString>(std::move(arcs), coords, _constructZ, _constructM, cs);
            _to.push_back(std::move(as));
        } else if (const auto* cc = dynamic_cast<const geom::CompoundCurve*>(g)) {
            _foundCompoundCurve = true;
            for (std::size_t i = 0; i < cc->getNumCurves(); i++) {
                filter_ro(cc->getCurveN(i));
            }
        }
    }
private:
    std::vector<std::unique_ptr<PathString>>& _to;
    bool _constructZ;
    bool _constructM;
    bool _foundCompoundCurve;

    PathStringExtractor(PathStringExtractor const&); /*= delete*/
    PathStringExtractor& operator=(PathStringExtractor const&); /*= delete*/
};

/** Class to construct a geometry from a set of PathStrings, given the
 *  geometry from which they originated. This allows CompoundCurves to
 *  be reconstructed. Unlike CurveBuilder, PathStrings will not be
 *  joined into SimpleCurves.
 */
class CurveRebuilder {

public:
    explicit CurveRebuilder(const Geometry& g) : m_srcGeom(g) {}

    // Register a PathString with the rebuilder
    // The context of the PathString will be set to nullptr during computation.
    void add(PathString* path)
    {
        const auto& coords = path->getCoordinates();

        m_pathForStartPoint[coords->front<CoordinateXY>()].push_back(path);
        m_pathForContext[path->getData()].push_back(path);
    }

    // Prevent reconstruction of CompoundCurve across the starting
    // or ending nodes of a specified PathString.
    void disableNodes(const PathString* path)
    {
        const CoordinateXY& startPoint = path->getCoordinates()->front<CoordinateXY>();
        const CoordinateXY& endPoint = path->getCoordinates()->back<CoordinateXY>();

        doNotContinueFromNode(startPoint);
        doNotContinueFromNode(endPoint);
    }

    std::unique_ptr<Geometry> getGeometry()
    {
        collectGeoms(m_srcGeom);

        return m_srcGeom.getFactory()->buildGeometry(std::move(m_resultGeoms));
    }

private:
    void doNotContinueFromNode(const CoordinateXY& p) {
        m_pathForStartPoint.erase(p);
    }

    void collectGeoms(const Geometry& g)
    {
        if (dynamic_cast<const geom::GeometryCollection*>(&g)) {
            for (std::size_t i = 0; i < g.getNumGeometries(); i++) {
                collectGeoms(*g.getGeometryN(i));
            }
            return;
        }

        const auto geomType = g.getGeometryTypeId();

        if (geomType == geom::GEOS_LINESTRING) {
            auto it = m_pathForContext.find(&g);
            util::Assert::isTrue(it != m_pathForContext.end());

            for (auto& path : it->second) {
                m_resultGeoms.push_back(getFactory().createLineString(path->getCoordinates()));
            }
        } else if (geomType == geom::GEOS_CIRCULARSTRING) {
            auto it = m_pathForContext.find(&g);
            util::Assert::isTrue(it != m_pathForContext.end());

            for (auto& path : it->second) {
                m_resultGeoms.push_back(getFactory().createCircularString(path->getCoordinates()));
            }
        } else if (geomType == geom::GEOS_COMPOUNDCURVE) {
            collectCompoundCurveGeoms(*detail::down_cast<const geom::CompoundCurve *>(&g));
        } else {
            throw util::UnsupportedOperationException("Unsupported geometry type: " + g.getGeometryType());
        }
    }

    void collectCompoundCurveGeoms(const geom::CompoundCurve& cc)
    {
        for (std::size_t i = 0; i < cc.getNumCurves(); i++) {
            std::vector<std::unique_ptr<SimpleCurve>> curves;

            const SimpleCurve* sc = cc.getCurveN(i);
            const CoordinateXY& originalEndPoint = sc->getCoordinatesRO()->back<CoordinateXY>();

            auto it = m_pathForContext.find(sc);
            util::Assert::isTrue(it != m_pathForContext.end());

            const auto& pathsForCurve = it->second;

            for (auto& path : pathsForCurve) {
                if (path->getData() == nullptr) {
                    continue;
                }
                path->setData(nullptr);

                const CoordinateXY& endPoint = path->getCoordinates()->back<CoordinateXY>();
                std::unique_ptr<SimpleCurve> geom = getPathGeometry(*path);

                if (endPoint == originalEndPoint) {
                    curves.push_back(std::move(geom));
                } else {
                    m_resultGeoms.push_back(std::move(geom));
                }
            }

            if (!curves.empty()) {
                auto it2 = m_pathForStartPoint.find(originalEndPoint);
                while (it2 != m_pathForStartPoint.end()) {
                    const auto& next = it2->second;
                    it2 = m_pathForStartPoint.end();

                    if (next.size() == 1) {
                        PathString* path = next[0];

                        auto nextIndex = i + curves.size();
                        if (nextIndex < cc.getNumCurves() && path->getData() == cc.getCurveN(nextIndex)) {
                            curves.push_back(getPathGeometry(*path));
                            path->setData(nullptr);

                            const CoordinateXY& endPoint = path->getCoordinates()->back<CoordinateXY>();
                            it2 = m_pathForStartPoint.find(endPoint);
                        }
                    }
                }

                if (curves.size() == 1) {
                    m_resultGeoms.push_back(std::move(curves[0]));
                } else {
                    m_resultGeoms.push_back(getFactory().createCompoundCurve(std::move(curves)));
                }
            }
        }
    }

    std::unique_ptr<SimpleCurve>
    getPathGeometry(const PathString& path) const
    {
        const bool isLinear = dynamic_cast<const SegmentString*>(&path);

        if (isLinear) {
            return getFactory().createLineString(path.getCoordinates());
        }

        return getFactory().createCircularString(path.getCoordinates());
    }

    const geom::GeometryFactory&
    getFactory() const
    {
        return *m_srcGeom.getFactory();
    }

    const Geometry& m_srcGeom;

    std::map<CoordinateXY, std::vector<PathString*>> m_pathForStartPoint;
    std::map<const void*, std::vector<PathString*>> m_pathForContext;

    std::vector<std::unique_ptr<Geometry>> m_resultGeoms;
};

}


/* public static */
std::unique_ptr<geom::Geometry>
GeometryNoder::node(const geom::Geometry& geom)
{
    GeometryNoder noder(geom);
    return noder.getNoded();
}

std::unique_ptr<geom::Geometry>
GeometryNoder::node(const geom::Geometry& geom1, const geom::Geometry& geom2)
{
    GeometryNoder noder(geom1, geom2);
    return noder.getNoded();
}

/* public */
GeometryNoder::GeometryNoder(const geom::Geometry& g)
    :
    argGeom1(&g),
    argGeom2(nullptr),
    argGeomHasCurves(g.hasCurvedComponents()),
    argGeomHasCompoundCurves(false),
    onlyFirstGeomEdges(false),
    preserveCompoundCurves(false)
{}

GeometryNoder::GeometryNoder(const geom::Geometry& g1, const geom::Geometry& g2)
    :
    argGeom1(&g1),
    argGeom2(&g2),
    argGeomHasCurves(g1.hasCurvedComponents() || g2.hasCurvedComponents()),
    argGeomHasCompoundCurves(false),
    onlyFirstGeomEdges(false),
    preserveCompoundCurves(false)
{}

GeometryNoder::~GeometryNoder() = default;

bool
GeometryNoder::isInResult(const PathString& ps) const
{
    return !onlyFirstGeomEdges || ps.getData() != nullptr;
}

/* private */
std::unique_ptr<geom::Geometry>
GeometryNoder::toGeometry(std::vector<std::unique_ptr<PathString>>& nodedEdges) const
{
    const geom::GeometryFactory* geomFact = argGeom1->getFactory();

    std::set< OrientedCoordinateArray > ocas;

    std::vector<PathString*> pathsToKeep;



    for(auto& path : nodedEdges) {
        if (!isInResult(*path)) {
            continue;
        }

        const auto& coords = path->getCoordinates();
        OrientedCoordinateArray oca1(*coords);

        // Check if an equivalent edge is known
        if(ocas.insert(oca1).second) {
            pathsToKeep.push_back(path.get());
        }
    }

    if (preserveCompoundCurves && argGeomHasCompoundCurves) {
        util::Assert::isTrue(onlyFirstGeomEdges);

        CurveRebuilder rebuilder(*argGeom1);

        for (auto& path : pathsToKeep) {
            rebuilder.add(path);
        }

        // Any edge that begins at a point where a geomB edge starts/ends is not
        // eligible for merging.
        for (auto& path : nodedEdges) {
            if (!isInResult(*path)) {
                rebuilder.disableNodes(path.get());
            }
        }

        return rebuilder.getGeometry();
    }

    // Create a geometry out of the noded substrings.
    std::vector<std::unique_ptr<geom::Geometry>> lines;
    lines.reserve(nodedEdges.size());
    bool resultArcs = false;

    for (const auto& path : pathsToKeep) {
        const auto& coords = path->getCoordinates();

        const bool isLinear = dynamic_cast<const SegmentString*>(path);

        if (isLinear) {
            lines.push_back(geomFact->createLineString(coords));
        } else {
            resultArcs = true;
            lines.push_back(geomFact->createCircularString(coords));
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
    if (argGeom1->isEmpty() && (argGeom2 == nullptr || argGeom2->isEmpty()))
        return argGeom1->clone();

    std::vector<std::unique_ptr<PathString>> lineList;

    extractPathStrings(*argGeom1, lineList);
    if (argGeom2 != nullptr) {
        std::vector<std::unique_ptr<PathString>> lineList2;
        extractPathStrings(*argGeom2, lineList2);
        for (auto& path : lineList2) {
            path->setData(nullptr); // prevent from appearing in result
            lineList.push_back(std::move(path));
        }
    }

    Noder& p_noder = getNoder();
    p_noder.computePathNodes(PathString::toRawPointerVector(lineList));
    auto nodedEdges = p_noder.getNodedPaths();

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
    argGeomHasCompoundCurves |= ex.foundCompoundCurve();
}

/* private */
Noder&
GeometryNoder::getNoder()
{
    if(!noder) {
        const geom::PrecisionModel* pm = argGeom1->getFactory()->getPrecisionModel();
        if (argGeomHasCurves) {
            noder = std::make_unique<SimpleNoder>();

            m_cai = std::make_unique<algorithm::CircularArcIntersector>(argGeom1->getPrecisionModel());
            m_aia = std::make_unique<ArcIntersectionAdder>(*m_cai);
            detail::down_cast<SimpleNoder*>(noder.get())->setArcIntersector(*m_aia);
        } else {
            noder = std::make_unique<IteratedNoder>(pm);
        }
    }
    return *noder;
}

void
GeometryNoder::setOnlyFirstGeomEdges(bool p_onlyFirstGeomEdges)
{
    onlyFirstGeomEdges = p_onlyFirstGeomEdges;
}

void
GeometryNoder::setPreserveCompoundCurves(bool preserve)
{
    preserveCompoundCurves = preserve;
}

} // namespace geos.noding
} // namespace geos
