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
 * Last port: precision/SimpleGeometryPrecisionReducer.cpp rev. 1.10 (JTS-1.7)
 *
 **********************************************************************/

#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/CoordinateArraySequence.h>
#include <geos/geom/CoordinateSequenceFactory.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/LineString.h>
#include <geos/geom/LinearRing.h>
#include <geos/operation/overlayng/PrecisionReducer.h>
#include <geos/precision/PrecisionReducerTransformer.h>
#include <geos/util.h>

#include <vector>
#include <typeinfo>

using namespace geos::geom;
using namespace geos::geom::util;

namespace geos {
namespace precision { // geos.precision


class PrecisionReducerFilter : public CoordinateFilter {
    public:

        PrecisionReducerFilter(bool p_removeRepeated, const PrecisionModel& p_filterPM)
            : removeRepeated(p_removeRepeated)
            , filterPM(p_filterPM)
            {
                m_prev.setNull();
            }

        void filter_ro(const Coordinate* curr) override final {

            // round to precision model
            Coordinate coord = *curr;
            filterPM.makePrecise(coord);

            // skip duplicate point
            if (removeRepeated && !m_prev.isNull() && coord.equals2D(m_prev))
                return;

            m_coords.push_back(coord);
            m_prev = coord;
        }

        std::vector<Coordinate> getCoords() {
            return std::move(m_coords);
        }

    private:

        std::vector<Coordinate> m_coords;
        Coordinate m_prev;
        bool removeRepeated;
        const PrecisionModel& filterPM;
};


/* public static */
std::unique_ptr<Geometry>
PrecisionReducerTransformer::reduce(
    const Geometry& geom,
    const PrecisionModel& targetPM,
    bool isRemoveCollapsed)
{
    PrecisionReducerTransformer trans(targetPM, isRemoveCollapsed);
    return trans.transform(&geom);
}

/* private */
void
PrecisionReducerTransformer::extend(
    std::vector<Coordinate>& coords,
    std::size_t minLength)
{
    if (coords.size() >= minLength)
        return;

    while(coords.size() < minLength) {
        Coordinate endCoord = coords.back();
        coords.push_back(endCoord);
    }
}


std::unique_ptr<CoordinateSequence>
PrecisionReducerTransformer::transformCoordinates(
    const CoordinateSequence* coords,
    const Geometry* parent)
{
    if (coords->size() == 0)
        return nullptr;

    if (coords->isEmpty()) {
        return detail::make_unique<CoordinateArraySequence>(0u, coords->getDimension());
    }

    const bool removeRepeated = true;
    PrecisionReducerFilter filter(removeRepeated, targetPM);
    coords->apply_ro(&filter);
    std::vector<Coordinate> coordsReduce = filter.getCoords();
    // std::unique_ptr<CoordinateArraySequence> coordSeqReduced();

    /**
     * Check to see if the removal of repeated points collapsed the coordinate
     * list to an invalid length for the type of the parent geometry. It is not
     * necessary to check for Point collapses, since the coordinate list can
     * never collapse to less than one point. If the length is invalid, return
     * the full-length coordinate array first computed, or null if collapses are
     * being removed. (This may create an invalid geometry - the client must
     * handle this.)
     */
    std::size_t minLength = 0;
    if (parent->getGeometryTypeId() == GEOS_LINESTRING)
        minLength = 2;
    if (parent->getGeometryTypeId() == GEOS_LINEARRING)
        minLength = LinearRing::MINIMUM_VALID_SIZE;

    /**
    * Handle collapse. If specified return null so parent geometry is removed or empty,
    * otherwise extend to required length.
    */
    if (coordsReduce.size() < minLength) {
        if (isRemoveCollapsed)
            return nullptr;
        extend(coordsReduce, minLength);
    }

    CoordinateArraySequence* cas = new CoordinateArraySequence(std::move(coordsReduce));
    return std::unique_ptr<CoordinateSequence>(static_cast<CoordinateSequence*>(cas));
}


// protected
std::unique_ptr<Geometry>
PrecisionReducerTransformer::transformPolygon(
    const Polygon* geom, const Geometry* parent)
{
    (void)(parent); // ignore unused variable
    return reduceArea(geom);
}

// protected
std::unique_ptr<Geometry>
PrecisionReducerTransformer::transformMultiPolygon(
    const MultiPolygon* geom, const Geometry* parent)
{
    (void)(parent); // ignore unused variable
    return reduceArea(geom);
}

// private
std::unique_ptr<Geometry>
PrecisionReducerTransformer::reduceArea(const Geometry* geom)
{
    return operation::overlayng::PrecisionReducer::reducePrecision(geom, &targetPM);
}



} // namespace geos.precision
} // namespace geos

