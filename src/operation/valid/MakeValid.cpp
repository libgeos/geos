/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright 2009-2010 Sandro Santilli <strk@kbt.io>
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

#include <geos/operation/valid/MakeValid.h>
#include <geos/operation/valid/IsValidOp.h>
#include <geos/operation/polygonize/BuildArea.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryCollection.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/LineString.h>
#include <geos/geom/Point.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/MultiLineString.h>
#include <geos/geom/MultiPolygon.h>
#include <geos/util/UniqueCoordinateArrayFilter.h>
#include <geos/util/UnsupportedOperationException.h>

// std
#include <cassert>
#include <algorithm>
#include <utility>
#include <vector>

#ifdef _MSC_VER
#pragma warning(disable:4355)
#endif

using namespace geos::geom;

namespace geos {
namespace operation { // geos.operation
namespace valid { // geos.operation.valid

/*
 * Fully node given linework
 */
static std::unique_ptr<geom::Geometry>
nodeLineWithFirstCoordinate(const geom::Geometry* geom)
{
  /*
   * Union with first geometry point, obtaining full noding
   * and dissolving of duplicated repeated points
   *
   * TODO: substitute this with UnaryUnion?
   */

  if( geom->isEmpty() )
      return nullptr;

  const auto geomType = geom->getGeometryTypeId();
  assert( geomType == GEOS_LINESTRING || geomType == GEOS_MULTILINESTRING );

  geom::Geometry* point;
  if( geomType == GEOS_LINESTRING ) {
      auto line = dynamic_cast<const geom::LineString*>(geom);
      assert(line);
      point = line->getPointN(0);
  } else {
      auto mls = dynamic_cast<const geom::MultiLineString*>(geom);
      assert(mls);
      auto line = dynamic_cast<const geom::LineString*>(mls->getGeometryN(0));
      assert(line);
      point = line->getPointN(0);
  }
  auto noded = geom->Union(point);
  delete point;

  return std::unique_ptr<geom::Geometry>(noded);
}


static std::unique_ptr<geom::Geometry> MakeValidLine(const geom::LineString* line)
{
    return nodeLineWithFirstCoordinate(line);
}

static std::unique_ptr<geom::Geometry> MakeValidMultiLine(const geom::MultiLineString* mls)
{

    std::vector<std::unique_ptr<geom::Geometry>> points;
    std::vector<std::unique_ptr<geom::Geometry>> lines;

    for(const auto subgeom: *mls) {
        auto line = dynamic_cast<const geom::LineString*>(subgeom);
        assert(line);
        auto validSubGeom = MakeValidLine(line);
        if( !validSubGeom || validSubGeom->isEmpty() ) {
            continue;
        }
        auto validLineType = validSubGeom->getGeometryTypeId();
        if( validLineType == GEOS_POINT ) {
            points.emplace_back(std::move(validSubGeom));
        }
        else if( validLineType == GEOS_LINESTRING ) {
            lines.emplace_back(std::move(validSubGeom));
        } else if( validLineType == GEOS_MULTILINESTRING ) {
            auto mlsValid = dynamic_cast<const geom::MultiLineString*>(validSubGeom.get());
            for(const auto subgeomMlsValid: *mlsValid) {
                lines.emplace_back(subgeomMlsValid->clone());
            }
        } else {
            throw util::UnsupportedOperationException();
        }
    }

    std::unique_ptr<geom::Geometry> pointsRet;
    if( !points.empty() ) {
        if( points.size() > 1 ) {
            auto pointsRawPtr = new std::vector<geom::Geometry*>(points.size());
            for( size_t i = 0; i < points.size(); i++ ) {
                auto pointMoved = std::move(points[i]);
                (*pointsRawPtr)[i] = pointMoved.release();
            }
            pointsRet.reset(
                mls->getFactory()->createMultiPoint(pointsRawPtr));
        } else {
            pointsRet = std::move(points[0]);
        }
    }

    std::unique_ptr<geom::Geometry> linesRet;
    if( !lines.empty() ) {
        if( lines.size() > 1 ) {
            auto linesRawPtr = new std::vector<geom::Geometry*>(lines.size());
            for( size_t i = 0; i < lines.size(); i++ ) {
                auto lineMoved = std::move(lines[i]);
                (*linesRawPtr)[i] = lineMoved.release();
            }
            linesRet.reset(
                mls->getFactory()->createMultiLineString(linesRawPtr));
        } else {
            linesRet = std::move(lines[0]);
        }
    }

    if( pointsRet && linesRet ) {
        auto geoms = new std::vector<geom::Geometry*>(2);
        (*geoms)[0] = pointsRet.release();
        (*geoms)[1] = linesRet.release();
        return std::unique_ptr<geom::Geometry>(
                    mls->getFactory()->createGeometryCollection(geoms));
    } else if( pointsRet ) {
        return pointsRet;
    } else if( linesRet ) {
        return linesRet;
    }

    return nullptr;
}

static std::unique_ptr<geom::Geometry> extractUniquePoints(const geom::Geometry* geom)
{

    // Code taken from GEOSGeom_extractUniquePoints_r()

    /* 1: extract points */
    std::vector<const geom::Coordinate*> coords;
    geos::util::UniqueCoordinateArrayFilter filter(coords);
    geom->apply_ro(&filter);

    /* 2: for each point, create a geometry and put into a vector */
    auto points = new std::vector<geom::Geometry*>();
    points->reserve(coords.size());
    const GeometryFactory* factory = geom->getFactory();
    for(std::vector<const geom::Coordinate*>::iterator it = coords.begin(),
            itE = coords.end();
            it != itE; ++it) {
        auto point = factory->createPoint(*(*it));
        points->push_back(point);
    }

    /* 3: create a multipoint */
    return std::unique_ptr<geom::Geometry>(factory->createMultiPoint(points));
}

static std::unique_ptr<geom::Geometry> MakeValidPoly(const geom::Geometry* geom)
{
    assert( geom->getGeometryTypeId() == GEOS_POLYGON ||
            geom->getGeometryTypeId() == GEOS_MULTIPOLYGON );

    std::unique_ptr<geom::Geometry> bound(geom->getBoundary());
    if( !bound )
        return nullptr;

    /* Use noded boundaries as initial "cut" edges */
    auto cut_edges = nodeLineWithFirstCoordinate(bound.get());
    if( !cut_edges )
        return nullptr;

    /* NOTE: the noding process may drop lines collapsing to points.
    *       We want to retrieve any of those */
    auto pi = extractUniquePoints(bound.get());
    auto po = extractUniquePoints(cut_edges.get());
    std::unique_ptr<geom::Geometry> collapse_points(pi->difference(po.get()));
    assert(collapse_points);
    pi.reset();
    po.reset();

    /* And use an empty geometry as initial "area" */
    const GeometryFactory* factory = geom->getFactory();
    std::unique_ptr<geom::Geometry> area(factory->createPolygon());
    assert(area);

    /*
    * See if an area can be build with the remaining edges
    * and if it can, symdifference with the original area.
    * Iterate this until no more polygons can be created
    * with left-over edges.
    */
    while( cut_edges->getNumGeometries() ) {

        // ASSUMPTION: cut_edges should already be fully noded
        auto new_area = geos::operation::polygonize::BuildArea().build(cut_edges.get());
        assert(new_area); // never return nullptr, but exception
        if( new_area->isEmpty() ) {
            /* no more rings can be build with thes edges */
            break;
        }

        // We succeeded in building a ring !
        // Save the new ring boundaries first (to compute further cut edges later)
        std::unique_ptr<geom::Geometry> new_area_bound(new_area->getBoundary());
        assert(new_area_bound);

        // Now symdif new and old area
        std::unique_ptr<geom::Geometry> symdif(area->symDifference(new_area.get()));
        assert(symdif);

        area = std::move(symdif);

        /*
        * Now let's re-set cut_edges with what's left
        * from the original boundary.
        * ASSUMPTION: only the previous cut-edges can be
        *             left, so we don't need to reconsider
        *             the whole original boundaries
        *
        * NOTE: this is an expensive operation.
        *
        */
        std::unique_ptr<geom::Geometry> new_cut_edges(cut_edges->difference(new_area_bound.get()));
        assert(new_cut_edges);

        cut_edges = std::move(new_cut_edges);
    }

    auto vgeoms = new std::vector<Geometry*>(3);
    unsigned int nvgeoms=0;

    if( !area->isEmpty() ) {
        (*vgeoms)[nvgeoms++] = area.release();
    }
    if( !cut_edges->isEmpty() ) {
        (*vgeoms)[nvgeoms++] = cut_edges.release();
    }
    if( !collapse_points->isEmpty() ) {
        (*vgeoms)[nvgeoms++] = collapse_points.release();
    }

    if( nvgeoms == 1 ) {
        /* Return cut edges */
        auto ret = std::unique_ptr<geom::Geometry>((*vgeoms)[0]);
        delete vgeoms;
        return ret;
    }

    /* Collect areas and lines (if any line) */
    vgeoms->resize(nvgeoms);
    return std::unique_ptr<geom::Geometry>(
                    factory->createGeometryCollection(vgeoms));
}

static std::unique_ptr<geom::Geometry> MakeValidCollection(const geom::GeometryCollection* coll)
{
    auto validGeoms = new std::vector<geom::Geometry*>();
    try {
        for( auto geom: *coll )
        {
            validGeoms->push_back(MakeValid().build(geom).release());
        }
        return std::unique_ptr<geom::Geometry>(
            GeometryFactory::create()->createGeometryCollection(validGeoms));
    }
    catch( ... ) {
        for( auto geom: *validGeoms ) {
            delete geom;
        }
        delete validGeoms;
        throw;
    }
}

/** Return a valid version of the input geometry. */
std::unique_ptr<geom::Geometry> MakeValid::build(const geom::Geometry* geom)
{

    IsValidOp ivo(geom);
    if( ivo.getValidationError() == nullptr ) {
        return std::unique_ptr<geom::Geometry>(geom->clone());
    }

    auto typeId = geom->getGeometryTypeId();
    if( typeId == GEOS_LINESTRING ) {
        auto lineString = dynamic_cast<const LineString*>(geom);
        return MakeValidLine(lineString);
    }
    if( typeId == GEOS_MULTILINESTRING ) {
        auto mls = dynamic_cast<const MultiLineString*>(geom);
        return MakeValidMultiLine(mls);
    }
    if( typeId == GEOS_POLYGON ||
        typeId == GEOS_MULTIPOLYGON ) {
        return MakeValidPoly(geom);
    }
    if( typeId == GEOS_GEOMETRYCOLLECTION ) {
        auto coll = dynamic_cast<const GeometryCollection*>(geom);
        return MakeValidCollection(coll);
    }

    throw util::UnsupportedOperationException();
}

} // namespace geos.operation.valid
} // namespace geos.operation
} // namespace geos

