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
 **********************************************************************
 *
 * Last port: linearref/LinearGeometryBuilder.java rev. 1.1
 *
 **********************************************************************/

#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/LineString.h>
#include <geos/linearref/ExtractLineByLocation.h>
#include <geos/linearref/LengthIndexedLine.h>
#include <geos/linearref/LinearLocation.h>
#include <geos/linearref/LengthLocationMap.h>
#include <geos/linearref/LengthIndexOfPoint.h>
#include <geos/linearref/LinearGeometryBuilder.h>
#include <geos/util/IllegalArgumentException.h>
#include <geos/util.h>

#include <cassert>



using namespace geos::geom;

namespace geos {

namespace linearref { // geos.linearref

/* public */
LinearGeometryBuilder::LinearGeometryBuilder(const GeometryFactory* p_geomFact) :
    geomFact(p_geomFact),
    ignoreInvalidLines(false),
    fixInvalidLines(false),
    coordList(nullptr) {}

/* public */
void
LinearGeometryBuilder::setIgnoreInvalidLines(bool p_ignoreInvalidLines)
{
    this->ignoreInvalidLines = p_ignoreInvalidLines;
}

/* public */
void
LinearGeometryBuilder::setFixInvalidLines(bool p_fixInvalidLines)
{
    this->fixInvalidLines = p_fixInvalidLines;
}

/* public */
void
LinearGeometryBuilder::add(const Coordinate& pt)
{
    add(pt, true);
}

/* public */
void
LinearGeometryBuilder::add(const Coordinate& pt, bool allowRepeatedPoints)
{
    if(!coordList) {
        coordList = detail::make_unique<CoordinateSequence>();
    }
    coordList->add(pt, allowRepeatedPoints);
    lastPt = pt;
}

/* public */
Coordinate
LinearGeometryBuilder::getLastCoordinate() const
{
    return lastPt;
}

/* public */
void
LinearGeometryBuilder::endLine()
{
    if(!coordList) {
        return;
    }
    if(coordList->size() < 2) {
        if(ignoreInvalidLines) {
            if(coordList) {
                coordList.reset();
            }
            return;
        }
        else if(fixInvalidLines) {
            assert(!coordList->isEmpty()); // just to be sure

            // NOTE: we copy the point cause reallocation of
            //       vector memory will invalidate the reference
            //       to one of its elements.
            //
            //       We wouldn't have such problems with a vector
            //       of pointers (as used in JTS)...
            //
            Coordinate firstPoint = (*coordList)[0];
            add(firstPoint);
        }
    }

    std::unique_ptr<LineString> line;
    try {
        line = geomFact->createLineString(std::move(coordList));
    }
    catch(util::IllegalArgumentException & ex) {
        // exception is due to too few points in line.
        // only propagate if not ignoring short lines
        ::geos::ignore_unused_variable_warning(ex);
        if(! ignoreInvalidLines) {
            throw;
        }
    }

    if(line) {
        lines.push_back(std::move(line));
    }
}

/* public */
std::unique_ptr<Geometry>
LinearGeometryBuilder::getGeometry()
{
    // end last line in case it was not done by user
    endLine();

    // NOTE: lines elements are cloned
    return geomFact->buildGeometry(std::move(lines));
}

/* public */
LinearGeometryBuilder::~LinearGeometryBuilder()
{
}

} // namespace geos.linearref
} // namespace geos
