/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2005-2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 ***********************************************************************
 *
 * Last port: precision/GeometryPrecisionReducer.cpp rev. 1.10 (JTS-1.7)
 *
 **********************************************************************/

#include <geos/precision/GeometryPrecisionReducer.h>
#include <geos/precision/PrecisionReducerCoordinateOperation.h>
#include <geos/geom/util/GeometryEditor.h>
#include <geos/geom/util/NoOpGeometryOperation.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/LineString.h>
#include <geos/geom/LinearRing.h>
#include <geos/operation/overlayng/PrecisionReducer.h>
#include <geos/precision/PrecisionReducerTransformer.h>
#include <geos/precision/PointwisePrecisionReducerTransformer.h>

#include <vector>
#include <typeinfo>

using namespace geos::geom;
using namespace geos::geom::util;

namespace geos {
namespace precision { // geos.precision


/* public */
std::unique_ptr<Geometry>
GeometryPrecisionReducer::reduce(const Geometry& geom)
{
    std::unique_ptr<Geometry> reduced;
    if (isPointwise) {
        reduced = PointwisePrecisionReducerTransformer::reduce(geom, targetPM);
    }
    else {
        reduced = PrecisionReducerTransformer::reduce(geom, targetPM, removeCollapsed);
    }

    // Match the collection level of the output to the input
    // if necessary
    if (geom.isCollection()
        && ! reduced->isCollection()
        && (geom.getCoordinateDimension() == reduced->getCoordinateDimension()))
    {
        reduced = geom.getFactory()->createMulti(std::move(reduced));
    }

    // TODO: incorporate this in the Transformer above
    if (changePrecisionModel &&
        (&targetPM != geom.getFactory()->getPrecisionModel()))
    {
         return changePM(reduced.get(), targetPM);
    }

    return reduced;
}


/* private */
std::unique_ptr<Geometry>
GeometryPrecisionReducer::changePM(const Geometry* geom, const geom::PrecisionModel& newPM)
{
    const GeometryFactory* previousFactory = geom->getFactory();
    GeometryFactory::Ptr changedFactory = createFactory(*previousFactory, newPM);
    GeometryEditor geomEdit(changedFactory.get());

    // this operation changes the PM for the entire geometry tree
    NoOpGeometryOperation noop;
    return geomEdit.edit(geom, &noop);
}


/* public static */
std::unique_ptr<Geometry>
GeometryPrecisionReducer::reducePointwise(const Geometry& g, const geom::PrecisionModel& precModel)
{
    GeometryPrecisionReducer reducer(precModel);
    reducer.setPointwise(true);
    return reducer.reduce(g);
}

/* public static */
std::unique_ptr<Geometry>
GeometryPrecisionReducer::reduceKeepCollapsed(const Geometry& g, const geom::PrecisionModel& precModel)
{
    GeometryPrecisionReducer reducer(precModel);
    reducer.setRemoveCollapsedComponents(false);
    return reducer.reduce(g);
}


/* public static */
std::unique_ptr<geom::Geometry>
GeometryPrecisionReducer::reduce(const geom::Geometry& g, const geom::PrecisionModel& precModel)
{
    GeometryPrecisionReducer reducer(precModel);
    return reducer.reduce(g);
}


/* private */
std::unique_ptr<Geometry>
GeometryPrecisionReducer::fixPolygonalTopology(const Geometry& geom)
{
    /*
     * If precision model was *not* changed, need to flip
     * geometry to targetPM, buffer in that model, then flip back
     */
    std::unique_ptr<geom::Geometry> tmp;
    GeometryFactory::Ptr tmpFactory;

    const Geometry* geomToBuffer = &geom;

    if(! newFactory) {
        tmpFactory = createFactory(*geom.getFactory(), targetPM);
        tmp = tmpFactory->createGeometry(&geom);
        geomToBuffer = tmp.get();
    }

    std::unique_ptr<Geometry> bufGeom(geomToBuffer->buffer(0));

    if(! newFactory) {
        // a slick way to copy the geometry with the original precision factory
        bufGeom = geom.getFactory()->createGeometry(bufGeom.get());
    }

    return bufGeom;
}

/* private */
GeometryFactory::Ptr
GeometryPrecisionReducer::createFactory(const GeometryFactory& oldGF,
                                        const PrecisionModel& newPM)
{
    GeometryFactory::Ptr p_newFactory(
        GeometryFactory::create(&newPM,
                                oldGF.getSRID())
    );
    return p_newFactory;
}



} // namespace geos.precision
} // namespace geos
