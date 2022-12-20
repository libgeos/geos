/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2020 Sandro Santilli <strk@kbt.io>
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 *
 **********************************************************************
 *
 * Last port: geom/prep/PreparedPolygon.java rev 1.7 (JTS-1.10)
 *
 **********************************************************************/

#pragma once

#include <geos/geom/prep/BasicPreparedGeometry.h> // for inheritance
#include <geos/noding/SegmentString.h>
#include <geos/operation/distance/IndexedFacetDistance.h>

#include <memory>

namespace geos {
namespace noding {
class FastSegmentSetIntersectionFinder;
}
namespace algorithm {
namespace locate {
class PointOnGeometryLocator;
}
}
}

namespace geos {
namespace geom { // geos::geom
namespace prep { // geos::geom::prep

/**
 * \brief
 * A prepared version of {@link Polygon} or {@link MultiPolygon} geometries.
 *
 * @author mbdavis
 *
 */
class PreparedPolygon : public BasicPreparedGeometry {
private:
    bool isRectangle;
    mutable std::unique_ptr<noding::FastSegmentSetIntersectionFinder> segIntFinder;
    mutable std::unique_ptr<algorithm::locate::PointOnGeometryLocator> ptOnGeomLoc;
    mutable std::unique_ptr<algorithm::locate::PointOnGeometryLocator> indexedPtOnGeomLoc;
    mutable noding::SegmentString::ConstVect segStrings;
    mutable std::unique_ptr<operation::distance::IndexedFacetDistance> indexedDistance;

protected:
public:
    PreparedPolygon(const geom::Geometry* geom);
    ~PreparedPolygon() override;

    noding::FastSegmentSetIntersectionFinder* getIntersectionFinder() const;
    algorithm::locate::PointOnGeometryLocator* getPointLocator() const;
    operation::distance::IndexedFacetDistance* getIndexedFacetDistance() const;

    bool contains(const geom::Geometry* g) const override;
    bool containsProperly(const geom::Geometry* g) const override;
    bool covers(const geom::Geometry* g) const override;
    bool intersects(const geom::Geometry* g) const override;
    double distance(const geom::Geometry* g) const override;
    bool isWithinDistance(const geom::Geometry* g, double d) const override;

};

} // namespace geos::geom::prep
} // namespace geos::geom
} // namespace geos

