/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2020 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/noding/Noder.h>
#include <geos/noding/ValidatingNoder.h>
#include <geos/noding/SegmentString.h>
#include <geos/noding/NodedSegmentString.h>
#include <geos/noding/snapround/SnapRoundingNoder.h>
#include <geos/io/WKTReader.h>
#include <geos/io/WKTWriter.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/geom/util/LinearComponentExtracter.h>

// std
#include <memory>


namespace geos {

class NodingTestUtil {

    private:

        // Methods
        static std::unique_ptr<geom::Geometry>
        toLines(const std::vector<noding::SegmentString*>* nodedList, const geom::GeometryFactory* geomFact);

        static std::vector<noding::SegmentString*>
        toSegmentStrings(std::vector<const geom::LineString*>& lines);

    public:

        // Methods
        static std::unique_ptr<geom::Geometry>
        nodeValidated(const geom::Geometry* geom1, const geom::Geometry* geom2, noding::Noder* noder);

};


} // geos




