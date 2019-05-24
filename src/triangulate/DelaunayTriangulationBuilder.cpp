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
 * Last port: triangulate/DelaunayTriangulationBuilder.java rev. r524
 *
 **********************************************************************/

#include <geos/triangulate/DelaunayTriangulationBuilder.h>

#include <algorithm>

#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateArraySequenceFactory.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/operation/valid/RepeatedPointRemover.h>
#include <geos/triangulate/IncrementalDelaunayTriangulator.h>
#include <geos/triangulate/quadedge/QuadEdgeSubdivision.h>
#include <geos/operation/valid/RepeatedPointRemover.h>
#include <geos/operation/valid/RepeatedPointTester.h>

namespace geos {
namespace triangulate { //geos.triangulate

using namespace geos::geom;

std::unique_ptr<CoordinateSequence>
DelaunayTriangulationBuilder::extractUniqueCoordinates(
    const Geometry& geom)
{
    std::unique_ptr<CoordinateSequence> seq(geom.getCoordinates());
    return unique(seq.get());
}

std::unique_ptr<CoordinateSequence>
DelaunayTriangulationBuilder::unique(const CoordinateSequence* seq) {
    auto seqFactory = CoordinateArraySequenceFactory::instance();
    auto dim = seq->getDimension();

    std::unique_ptr<std::vector<Coordinate>> coords(new std::vector<Coordinate>());
    seq->toVector(*(coords.get()));
    std::sort(coords->begin(), coords->end(), geos::geom::CoordinateLessThen());

    std::unique_ptr<CoordinateSequence> sortedSeq(seqFactory->create(coords.release(), dim));

    operation::valid::RepeatedPointTester rpt;
    if (rpt.hasRepeatedPoint(sortedSeq.get())) {
        return operation::valid::RepeatedPointRemover::removeRepeatedPoints(sortedSeq.get());
    } else {
        return sortedSeq;
    }
}

IncrementalDelaunayTriangulator::VertexList*
DelaunayTriangulationBuilder::toVertices(
    const CoordinateSequence& coords)
{
    IncrementalDelaunayTriangulator::VertexList* vertexList =
        new IncrementalDelaunayTriangulator::VertexList();

    for(size_t iter = 0; iter < coords.size(); ++iter) {
        vertexList->push_back(quadedge::Vertex(coords.getAt(iter)));
    }
    return vertexList;
}

DelaunayTriangulationBuilder::DelaunayTriangulationBuilder() :
    siteCoords(nullptr), tolerance(0.0), subdiv(nullptr)
{
}

DelaunayTriangulationBuilder::~DelaunayTriangulationBuilder()
{
    if(subdiv) {
        delete subdiv;
    }
}

void
DelaunayTriangulationBuilder::setSites(const Geometry& geom)
{
    // remove any duplicate points (they will cause the triangulation to fail)
    siteCoords = extractUniqueCoordinates(geom);
}

void
DelaunayTriangulationBuilder::setSites(const CoordinateSequence& coords)
{
    // remove any duplicate points (they will cause the triangulation to fail)
    siteCoords = operation::valid::RepeatedPointRemover::removeRepeatedPoints(&coords);
}

void
DelaunayTriangulationBuilder::create()
{
    if(subdiv != nullptr || siteCoords == nullptr) {
        return;
    }

    Envelope siteEnv;
    siteCoords ->expandEnvelope(siteEnv);
    IncrementalDelaunayTriangulator::VertexList* vertices = toVertices(*siteCoords);
    subdiv = new quadedge::QuadEdgeSubdivision(siteEnv, tolerance);
    IncrementalDelaunayTriangulator triangulator = IncrementalDelaunayTriangulator(subdiv);
    triangulator.insertSites(*vertices);
    delete vertices;
}

quadedge::QuadEdgeSubdivision&
DelaunayTriangulationBuilder::getSubdivision()
{
    create();
    return *subdiv;
}

std::unique_ptr<MultiLineString>
DelaunayTriangulationBuilder::getEdges(
    const GeometryFactory& geomFact)
{
    create();
    return subdiv->getEdges(geomFact);
}

std::unique_ptr<geom::GeometryCollection>
DelaunayTriangulationBuilder::getTriangles(
    const geom::GeometryFactory& geomFact)
{
    create();
    return subdiv->getTriangles(geomFact);
}

geom::Envelope
DelaunayTriangulationBuilder::envelope(const geom::CoordinateSequence& coords)
{
    Envelope env;
    std::vector<Coordinate> coord_vector;
    coords.toVector(coord_vector);
    for(std::vector<Coordinate>::iterator it = coord_vector.begin() ; it != coord_vector.end() ; ++it) {
        const Coordinate& coord = *it;
        env.expandToInclude(coord);
    }
    return env;
}


} //namespace geos.triangulate
} //namespace goes

