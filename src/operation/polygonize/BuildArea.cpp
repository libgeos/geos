/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright 2011-2014 Sandro Santilli <strk@kbt.io>
 * Copyright (C) 2019 Even Rouault <even.rouault@spatialys.com>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 **********************************************************************
 *
 * Ported from rtgeom_geos.c from
 *   rttopo - topology library
 *   http://git.osgeo.org/gitea/rttopo/librttopo
 * with relicensing from GPL to LGPL with Copyright holder permission.
 *
 **********************************************************************/

#include <geos/operation/polygonize/Polygonizer.h>
#include <geos/operation/polygonize/BuildArea.h>
#include <geos/operation/union/CascadedPolygonUnion.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryCollection.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/LineString.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/MultiPolygon.h>

//#define DUMP_GEOM
#ifdef DUMP_GEOM
#include <geos/io/WKTWriter.h>
#endif

// std
#include <algorithm>
#include <vector>

#ifdef _MSC_VER
#pragma warning(disable:4355)
#endif

using namespace std;
using namespace geos::geom;

namespace geos {
namespace operation { // geos.operation
namespace polygonize { // geos.operation.polygonize

struct Face {
    const geom::Polygon* poly = nullptr;
    std::unique_ptr<geom::Geometry> env; // envelope
    double envarea = 0.0; // envelope area
    Face* parent = nullptr; /* if this face is an hole of another one, or NULL */

    size_t countParents() const {
        const Face* f = this;
        size_t pcount = 0;
        while ( f->parent ) {
            ++pcount;
            f = f->parent;
        }
        return pcount;
    }
};

static std::unique_ptr<Face> newFace(const geom::Polygon* p) {
    auto f = std::unique_ptr<Face>(new Face());
    f->poly = p;
    f->env = p->getEnvelope();
    f->envarea = f->env->getArea();
    return f;
}

struct CompareByEnvarea {

    bool operator()(const std::unique_ptr<Face> &a,
                    const std::unique_ptr<Face> &b) const {
        return a->envarea > b->envarea;
    }
};

/* Find holes of each face */
static void findFaceHoles(std::vector<std::unique_ptr<Face>>& faces) {

    /* We sort by decreasing envelope area so that we know holes are only
     * after their shells */
    std::sort(faces.begin(), faces.end(), CompareByEnvarea());

    const size_t nfaces = faces.size();
    for( size_t i = 0; i < nfaces; ++i ) {
        auto& f = faces[i];
        const size_t nholes = f->poly->getNumInteriorRing();
        for( size_t h = 0; h < nholes; h++ ) {
            const auto hole = f->poly->getInteriorRingN(h);
            for( auto j=i+1; j < nfaces; ++j ) {
                auto& f2 = faces[j];
                if( f2->parent ) {
                    continue; /* hole already assigned */
                }
                const auto f2er = f2->poly->getExteriorRing();
                /* TODO: can be optimized as the ring would have the
                *       same vertices, possibly in different order.
                *       maybe comparing number of points could already be
                *       useful.
                */
                if( f2er->equals(hole) ) {
                    f2->parent = f.get();
                    break;
                }
            }
        }
    }
}

static std::unique_ptr<geom::MultiPolygon> collectFacesWithEvenAncestors(
    std::vector<std::unique_ptr<Face>>& faces) {
    std::vector<std::unique_ptr<geom::Geometry>> geoms;
    for( auto& face: faces ) {
        if( face->countParents() % 2 ) {
            continue; /* we skip odd parents geoms */
        }
        geoms.push_back(face->poly->clone());
    }
    // TODO don't create new GeometryFactory here
    return GeometryFactory::create()->createMultiPolygon(std::move(geoms));
}

#ifdef DUMP_GEOM

static void dumpGeometry(const geom::Geometry* geom)
{
  geos::io::WKTWriter oWriter;
  std::cerr << oWriter.write(geom) << std::endl;
}
#endif

/** Return the area built from the constituent linework of the input geometry. */
unique_ptr<geom::Geometry> BuildArea::build(const geom::Geometry* geom) {
    Polygonizer polygonizer;
    polygonizer.add(geom);
    auto polys = polygonizer.getPolygons();

    // No geometries in collection, early out
    if( polys->empty() ) {
        // TODO don't create new GeometryFactory here
        auto emptyGeomCollection = unique_ptr<geom::Geometry>(
            GeometryFactory::create()->createGeometryCollection());
        emptyGeomCollection->setSRID(geom->getSRID());
        return emptyGeomCollection;
    }

    // Return first geometry if we only have one in collection
    if( polys->size() == 1 ) {
        auto ret = std::unique_ptr<geom::Geometry>((*polys)[0].release());
        ret->setSRID(geom->getSRID());
        return ret;
    }

    /*
    * Polygonizer returns a polygon for each face in the built topology.
    *
    * This means that for any face with holes we'll have other faces
    * representing each hole. We can imagine a parent-child relationship
    * between these faces.
    *
    * In order to maximize the number of visible rings in output we
    * only use those faces which have an even number of parents.
    *
    * Example:
    *
    *   +---------------+
    *   |     L0        |  L0 has no parents
    *   |  +---------+  |
    *   |  |   L1    |  |  L1 is an hole of L0
    *   |  |  +---+  |  |
    *   |  |  |L2 |  |  |  L2 is an hole of L1 (which is an hole of L0)
    *   |  |  |   |  |  |
    *   |  |  +---+  |  |
    *   |  +---------+  |
    *   |               |
    *   +---------------+
    *
    * See http://trac.osgeo.org/postgis/ticket/1806
    *
    */

    /* Prepare face structures for later analysis */
    std::vector<std::unique_ptr<Face>> faces;
    for(auto& poly: *polys) {
        faces.emplace_back(newFace(poly.get()));
    }

    /* Find faces representing other faces holes */
    findFaceHoles(faces);

    /* Build a MultiPolygon composed only by faces with an
    * even number of ancestors */
    auto tmp = collectFacesWithEvenAncestors(faces);

#ifdef DUMP_GEOM
    std::cerr << "after collectFacesWithEvenAncestors:" << std::endl;
    dumpGeometry(tmp.get());
#endif

    /* Run a single overlay operation to dissolve shared edges */
    auto shp = std::unique_ptr<geom::Geometry>(
        geos::operation::geounion::CascadedPolygonUnion::CascadedPolygonUnion::Union(tmp.get()));
    if( shp ) {
        shp->setSRID(geom->getSRID());
    }

#ifdef DUMP_GEOM
    std::cerr << "after CascadedPolygonUnion:" << std::endl;
    dumpGeometry(shp.get());
#endif

    return shp;
}

} // namespace geos.operation.polygonize
} // namespace geos.operation
} // namespace geos

