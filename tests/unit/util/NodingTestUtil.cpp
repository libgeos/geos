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

#include <util/NodingTestUtil.h>


using namespace geos::geom;
using namespace geos::noding;
using geos::noding::Noder;
using geos::io::WKTReader;
using geos::io::WKTWriter;
using geos::geom::util::LinearComponentExtracter;

namespace geos {

/*private static*/
std::unique_ptr<Geometry>
NodingTestUtil::toLines(const std::vector<SegmentString*>* nodedList, const GeometryFactory* geomFact)
{
    std::vector<std::unique_ptr<Geometry>> lines;

    for (auto nss : *nodedList) {
      CoordinateSequence* pts = nss->getCoordinates();
      // pts is owned by nss, so we make a copy to build the line
      // on top of. Lines are 100% self-contained and own all their parts.
      // Input nodedList can be freed.
      lines.emplace_back(geomFact->createLineString(pts->clone()));
    }
    if (lines.size() == 1) return std::move(lines[0]);

    // move the lines to pass ownership to the multiLineString
    return geomFact->createMultiLineString(std::move(lines));
}


/*private static*/
std::vector<SegmentString*>
NodingTestUtil::toSegmentStrings(std::vector<const LineString*>& lines)
{
    std::vector<SegmentString*> nssList;
    for (auto line : lines) {
        // line->getCoordinates() clones CoordinateSequence
        // into a unique_ptr<> which we have to release() to the
        // NodedSegmentString constructor, so
        // nss now owns nss->pts
        NodedSegmentString* nss = new NodedSegmentString(line->getCoordinates().release(), line);
        nssList.push_back(nss);
    }
    return nssList;
}

/*public static*/
std::unique_ptr<Geometry>
NodingTestUtil::nodeValidated(const Geometry* geom1, const Geometry* geom2, Noder* noder)
{
    std::vector<const LineString*> lines;
    // lines are const* to linear components of geom1, geom1 still
    // owns all coordinates, etc
    LinearComponentExtracter::getLines(*geom1, lines);
    if (geom2 != nullptr) {
        LinearComponentExtracter::getLines(*geom2, lines);
    }

    // ssList needs to be disposed after noder is done working
    std::vector<SegmentString*> ssList = toSegmentStrings(lines);

    ValidatingNoder noderValid(*noder);
    // computeNotes might alster ssList, but ssList still
    // holds all memory
    noderValid.computeNodes(&ssList);

    // getNodedSubstrings calls NodedSegmentString::getNodedSubStrings()
    // which creates new NodedSegmentString and new pts member, so complete
    // new copy of data. Can be disposed of after geometries are constructed
    // std::vector<SegmentString*>* nodedList = noderValid.getNodedSubstrings();
    std::vector<SegmentString*>* nodedList = noderValid.getNodedSubstrings();

    // Dispose of ssList
    for (auto ss: ssList) {
        delete ss;
    }

    std::unique_ptr<Geometry> lineGeom = toLines(nodedList, geom1->getFactory());

    // Dispose of nodedList
    for (auto nss: *nodedList) {
        delete nss;
    }
    delete nodedList;

    return lineGeom;
}

} // geos

