/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2012 Excensus LLC.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: triangulate/VoronoiDiagramBuilder.java rev. r524
 *
 **********************************************************************/

#include <geos/triangulate/VoronoiDiagramBuilder.h>

#include <algorithm>
#include <cmath>
#include <vector>
#include <iostream>
#include <sstream>
#include <unordered_map>

#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/Envelope.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/triangulate/IncrementalDelaunayTriangulator.h>
#include <geos/triangulate/DelaunayTriangulationBuilder.h>
#include <geos/triangulate/quadedge/QuadEdgeSubdivision.h>
#include <geos/operation/valid/RepeatedPointRemover.h>
#include <geos/util.h>

using geos::detail::make_unique;

namespace geos {
namespace triangulate { //geos.triangulate

using namespace geos::geom;


VoronoiDiagramBuilder::VoronoiDiagramBuilder() :
    tolerance(0.0), clipEnv(nullptr), inputGeom(nullptr), inputSeq(nullptr), isOrdered(false)
{
}

void
VoronoiDiagramBuilder::setSites(const geom::Geometry& geom)
{
    siteCoords = DelaunayTriangulationBuilder::extractUniqueCoordinates(geom);
    inputGeom = &geom;
}

void
VoronoiDiagramBuilder::setSites(const geom::CoordinateSequence& coords)
{
    siteCoords = DelaunayTriangulationBuilder::unique(&coords);
    inputSeq = &coords;
}

void
VoronoiDiagramBuilder::setClipEnvelope(const geom::Envelope* nClipEnv)
{
    clipEnv = nClipEnv;
}

void
VoronoiDiagramBuilder::setOrdered(bool p_isOrdered)
{
    isOrdered = p_isOrdered;
}

void
VoronoiDiagramBuilder::setTolerance(double nTolerance)
{
    tolerance = nTolerance;
}

void
VoronoiDiagramBuilder::create()
{
    if(subdiv) {
        return;
    }

    if (siteCoords->isEmpty()) {
        return;
    }

    diagramEnv = siteCoords->getEnvelope();
    //adding buffer around the final envelope
    double expandBy = std::max(diagramEnv.getWidth(), diagramEnv.getHeight());
    diagramEnv.expandBy(expandBy);
    if(clipEnv) {
        diagramEnv.expandToInclude(clipEnv);
    }

    auto vertices = DelaunayTriangulationBuilder::toVertices(*siteCoords);
    std::sort(vertices.begin(), vertices.end()); // Best performance from locator when inserting points near each other

    subdiv.reset(new quadedge::QuadEdgeSubdivision(diagramEnv, tolerance));
    IncrementalDelaunayTriangulator triangulator(subdiv.get());
    /**
     * Avoid creating very narrow triangles along triangulation boundary.
     * These otherwise can cause malformed Voronoi cells.
     */
    triangulator.forceConvex(false);
    triangulator.insertSites(vertices);
}

std::unique_ptr<quadedge::QuadEdgeSubdivision>
VoronoiDiagramBuilder::getSubdivision()
{
    create();
    // NOTE: Apparently, this is 'source' method giving up the object resource.
    return std::move(subdiv);
}

std::size_t
VoronoiDiagramBuilder::getNumInputPoints() const {
    if (inputGeom) {
        return inputGeom->getNumPoints();
    } else {
        return inputSeq->getSize();
    }
}

std::unique_ptr<geom::GeometryCollection>
VoronoiDiagramBuilder::getDiagram(const geom::GeometryFactory& geomFact)
{
    create();

    std::unique_ptr<GeometryCollection> ret;
    if (subdiv) {
        auto polys = subdiv->getVoronoiCellPolygons(geomFact);

        if (isOrdered) {
            reorderCellsToInput(polys);
        }

        for (auto& p : polys) {
            // Don't let references to Vertex objects
            // owned by the QuadEdgeSubdivision escape
            p->setUserData(nullptr);
        }

        ret = clipGeometryCollection(polys, diagramEnv);
    }

    if (ret == nullptr) {
        return std::unique_ptr<geom::GeometryCollection>(geomFact.createGeometryCollection());
    }

    return ret;
}

std::unique_ptr<MultiLineString>
VoronoiDiagramBuilder::getDiagramEdges(const geom::GeometryFactory& geomFact)
{
    create();

    if (!subdiv) {
        return geomFact.createMultiLineString();
    }

    auto edges = subdiv->getVoronoiDiagramEdges(geomFact);

    if(edges->isEmpty()) {
        return edges;
    }

    std::unique_ptr<geom::Geometry> clipPoly(geomFact.toGeometry(&diagramEnv));
    std::unique_ptr<Geometry> clipped(clipPoly->intersection(edges.get()));

    switch (clipped->getGeometryTypeId()) {
        case GEOS_LINESTRING: {
            std::vector<std::unique_ptr<LineString>> lines;
            lines.emplace_back(static_cast<LineString*>(clipped.release()));
            return geomFact.createMultiLineString(std::move(lines));
        }
        case GEOS_MULTILINESTRING: {
            std::unique_ptr<MultiLineString> mls(static_cast<MultiLineString*>(clipped.release()));
            return mls;
        }
        default: {
            throw util::GEOSException("Unknown state");
        }
    }
}

std::unique_ptr<geom::GeometryCollection>
VoronoiDiagramBuilder::clipGeometryCollection(std::vector<std::unique_ptr<Geometry>> & geoms, const geom::Envelope& clipEnv)
{
    if (geoms.empty()) {
        return nullptr;
    }

    auto gfact = geoms[0]->getFactory();

    std::unique_ptr<geom::Geometry> clipPoly(gfact->toGeometry(&clipEnv));
    std::vector<std::unique_ptr<Geometry>> clipped;

    for(auto& g : geoms) {
        // don't clip unless necessary
        if(clipEnv.contains(g->getEnvelopeInternal())) {
            clipped.push_back(std::move(g));
        } else if(clipEnv.intersects(g->getEnvelopeInternal())) {
            auto result = clipPoly->intersection(g.get());
            if (!result->isEmpty()) {
                clipped.push_back(std::move(result));
            }
        }
    }

    return gfact->createGeometryCollection(std::move(clipped));
}

void
VoronoiDiagramBuilder::addCellsForCoordinates(CoordinateCellMap& cellMap,
                                              const CoordinateSequence& seq,
                                              std::vector<std::unique_ptr<Geometry>> & polys) {
    for (const CoordinateXY& c : seq.items<CoordinateXY>()) {
        auto cell = cellMap.find(c);

        if (cell == cellMap.end()) {
            std::stringstream ss;
            ss << "No cell found for input coordinate " << c;
            throw util::GEOSException(ss.str());
        }

        if (cell->second == nullptr) {
            std::stringstream ss;
            ss << "Multiple input coordinates in cell at " << c;
            throw util::GEOSException(ss.str());
        }

        polys.push_back(std::move(cell->second));
    }
}

void
VoronoiDiagramBuilder::addCellsForCoordinates(CoordinateCellMap& cellMap,
                                              const Geometry& g,
                                              std::vector<std::unique_ptr<Geometry>> & polys) {
    auto typ = g.getGeometryTypeId();

    if (typ == GEOS_LINESTRING) {
        const auto& seq = *static_cast<const LineString&>(g).getCoordinatesRO();
        addCellsForCoordinates(cellMap, seq, polys);
    } else if (typ == GEOS_POINT) {
        const auto& seq = *static_cast<const Point&>(g).getCoordinatesRO();
        addCellsForCoordinates(cellMap, seq, polys);
    } else {
        for (std::size_t i = 0; i < g.getNumGeometries(); i++) {
            addCellsForCoordinates(cellMap, *g.getGeometryN(i), polys);
        }
    }
}

void
VoronoiDiagramBuilder::reorderCellsToInput(std::vector<std::unique_ptr<Geometry>> & polys) const
{
    CoordinateCellMap cellMap;
    for (auto& p : polys) {
        const CoordinateXY* c = reinterpret_cast<const Coordinate*>(p->getUserData());
        cellMap.emplace(*c, std::move(p));
    }

    auto npts = getNumInputPoints();
    std::vector<std::unique_ptr<Geometry>> reorderedPolys;
    reorderedPolys.reserve(npts);

    if (inputSeq) {
        addCellsForCoordinates(cellMap, *inputSeq, reorderedPolys);
    } else {
        addCellsForCoordinates(cellMap, *inputGeom, reorderedPolys);
    }

    polys = std::move(reorderedPolys);
}

} //namespace geos.triangulate
} //namespace geos
