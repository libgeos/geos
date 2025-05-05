/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (c) 2025 Martin Davis
 * Copyright (C) 2025 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/coverage/CleanCoverage.h>

#include <geos/geom/Envelope.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Polygon.h>
#include <geos/index/quadtree/Quadtree.h>
#include <geos/operation/overlayng/OverlayNG.h>
#include <geos/operation/overlayng/OverlayNGRobust.h>
#include <geos/operation/relateng/IntersectionMatrixPattern.h>
#include <geos/operation/relateng/RelateNG.h>


using geos::geom::Envelope;
using geos::geom::Geometry;
using geos::geom::GeometryFactory;
using geos::geom::Polygon;
using geos::index::quadtree::Quadtree;
using geos::operation::overlayng::OverlayNG;
using geos::operation::overlayng::OverlayNGRobust;
using geos::operation::relateng::IntersectionMatrixPattern;
using geos::operation::relateng::RelateNG;


namespace geos {     // geos
namespace coverage { // geos.coverage







} // namespace geos.coverage
} // namespace geos


