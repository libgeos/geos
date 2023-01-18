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

#include <geos/noding/GeometryNoder.h>
#include <geos/noding/SegmentString.h>
#include <geos/noding/NodedSegmentString.h>
#include <geos/noding/OrientedCoordinateArray.h>
#include <geos/noding/Noder.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/LineString.h>

#include <geos/noding/IteratedNoder.h>

#include <geos/algorithm/LineIntersector.h>
#include <geos/noding/IntersectionAdder.h>
#include <geos/noding/MCIndexNoder.h>

#include <geos/noding/snapround/MCIndexSnapRounder.h>

#include <memory> // for unique_ptr
#include <iostream>

namespace geos {
namespace noding { // geos.noding

namespace {

/**
 * Add every linear element in a geometry into SegmentString vector
 */
class SegmentStringExtractor: public geom::GeometryComponentFilter {
public:
    SegmentStringExtractor(SegmentString::NonConstVect& to,
                           bool constructZ,
                           bool constructM)
        : _to(to)
        , _constructZ(constructZ)
        , _constructM(constructM)
    {}

    void
    filter_ro(const geom::Geometry* g) override
    {
        const geom::LineString* ls = dynamic_cast<const geom::LineString*>(g);
        if(ls) {
            auto coord = ls->getCoordinates();
            // coord ownership transferred to SegmentString
            SegmentString* ss = new NodedSegmentString(coord.release(), _constructZ, _constructM, nullptr);
            _to.push_back(ss);
        }
    }
private:
    SegmentString::NonConstVect& _to;
    bool _constructZ;
    bool _constructM;

    SegmentStringExtractor(SegmentStringExtractor const&); /*= delete*/
    SegmentStringExtractor& operator=(SegmentStringExtractor const&); /*= delete*/
};

}


/* public static */
std::unique_ptr<geom::Geometry>
GeometryNoder::node(const geom::Geometry& geom)
{
    GeometryNoder noder(geom);
    return noder.getNoded();
}

/* public */
GeometryNoder::GeometryNoder(const geom::Geometry& g)
    :
    argGeom(g)
{
}

/* private */
std::unique_ptr<geom::Geometry>
GeometryNoder::toGeometry(SegmentString::NonConstVect& nodedEdges)
{
    const geom::GeometryFactory* geomFact = argGeom.getFactory();

    std::set< OrientedCoordinateArray > ocas;

    // Create a geometry out of the noded substrings.
    std::vector<std::unique_ptr<geom::Geometry>> lines;
    lines.reserve(nodedEdges.size());
    for(auto& ss :  nodedEdges) {
        const geom::CoordinateSequence* coords = ss->getCoordinates();

        // Check if an equivalent edge is known
        OrientedCoordinateArray oca1(*coords);
        if(ocas.insert(oca1).second) {
            lines.push_back(geomFact->createLineString(coords->clone()));
        }
    }

    return geomFact->createMultiLineString(std::move(lines));
}

/* public */
std::unique_ptr<geom::Geometry>
GeometryNoder::getNoded()
{
    SegmentString::NonConstVect p_lineList;
    if (argGeom.isEmpty())
        return argGeom.clone();

    extractSegmentStrings(argGeom, p_lineList);

    Noder& p_noder = getNoder();
    SegmentString::NonConstVect* nodedEdges = nullptr;

    try {
        p_noder.computeNodes(&p_lineList);
        nodedEdges = p_noder.getNodedSubstrings();
    }
    catch(const std::exception&) {
        for(std::size_t i = 0, n = p_lineList.size(); i < n; ++i) {
            delete p_lineList[i];
        }
        throw;
    }

    std::unique_ptr<geom::Geometry> noded = toGeometry(*nodedEdges);

    for(auto* elem : (*nodedEdges)) {
        delete elem;
    }
    delete nodedEdges;

    for(auto* elem : p_lineList) {
        delete elem;
    }

    return noded;
}

/* private static */
void
GeometryNoder::extractSegmentStrings(const geom::Geometry& g,
                                     SegmentString::NonConstVect& to)
{
    SegmentStringExtractor ex(to, g.hasZ(), g.hasM());
    g.apply_ro(&ex);
}

/* private */
Noder&
GeometryNoder::getNoder()
{
    if(! noder.get()) {
        const geom::PrecisionModel* pm = argGeom.getFactory()->getPrecisionModel();
        IteratedNoder* in = new IteratedNoder(pm);
        noder.reset(in);
    }
    return *noder;
}


} // namespace geos.noding
} // namespace geos
