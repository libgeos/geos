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
 * Last port: geom/Envelope.java rev 1.46 (JTS-1.10)
 *
 **********************************************************************/

#include <geos/geom/Envelope.h>
#include <geos/geom/Coordinate.h>

#include <algorithm>
#include <sstream>
#include <cmath>


#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif

#if GEOS_DEBUG
#include <iostream>
#endif

namespace geos {
namespace geom { // geos::geom

/*public*/
bool
Envelope::intersects(const CoordinateXY& p1, const CoordinateXY& p2,
                     const CoordinateXY& q)
{
    //OptimizeIt shows that Math#min and Math#max here are a bottleneck.
    //Replace with direct comparisons. [Jon Aquino]
    if(((q.x >= (p1.x < p2.x ? p1.x : p2.x)) && (q.x <= (p1.x > p2.x ? p1.x : p2.x))) &&
            ((q.y >= (p1.y < p2.y ? p1.y : p2.y)) && (q.y <= (p1.y > p2.y ? p1.y : p2.y)))) {
        return true;
    }
    return false;
}

/*public*/
bool
Envelope::intersects(const CoordinateXY& a, const CoordinateXY& b) const
{
    // These comparisons look redundant, but an alternative using
    // std::minmax performs no better and compiles down to more
    // instructions.
    double envminx = (a.x < b.x) ? a.x : b.x;
    if(!std::isgreaterequal(maxx, envminx)) { // awkward comparison catches cases where this->isNull()
        return false;
    }

    double envmaxx = (a.x > b.x) ? a.x : b.x;
    if(std::isless(envmaxx, minx)) {
        return false;
    }

    double envminy = (a.y < b.y) ? a.y : b.y;
    if(std::isgreater(envminy, maxy)) {
        return false;
    }

    double envmaxy = (a.y > b.y) ? a.y : b.y;
    if(std::isless(envmaxy, miny)) {
        return false;
    }

    return true;
}

/*public*/
Envelope::Envelope(const std::string& str)
{
    // The string should be in the format:
    // Env[7.2:2.3,7.1:8.2]

    // extract out the values between the [ and ] characters
    std::string::size_type index = str.find('[');
    std::string coordString = str.substr(index + 1, str.size() - 1 - 1);

    // now split apart the string on : and , characters
    std::vector<std::string> values = split(coordString, ":,");

    // create a new envelope
    init(strtod(values[0].c_str(), nullptr),
         strtod(values[1].c_str(), nullptr),
         strtod(values[2].c_str(), nullptr),
         strtod(values[3].c_str(), nullptr));
}

/*public*/
bool
Envelope::covers(const Envelope& other) const
{
    return
        std::isgreaterequal(other.minx,  minx) &&
        std::islessequal(other.maxx,  maxx) &&
        std::isgreaterequal(other.miny, miny) &&
        std::islessequal(other.maxy,  maxy);
}

/*public*/
bool
Envelope::equals(const Envelope* other) const
{
    if(isNull()) {
        return other->isNull();
    }
    return  other->minx == minx &&
            other->maxx == maxx &&
            other->miny == miny &&
            other->maxy == maxy;
}

bool
Envelope::isfinite() const
{
    return std::isfinite(minx) && std::isfinite(maxx) &&
           std::isfinite(miny) && std::isfinite(maxy);
}

/* public */
std::ostream&
operator<< (std::ostream& os, const Envelope& o)
{
    os << "Env[" << o.minx << ":" << o.maxx << ","
       << o.miny << ":" << o.maxy << "]";
    return os;
}


/*public*/
std::string
Envelope::toString() const
{
    std::ostringstream s;
    s << *this;
    return s.str();
}

/*public static*/
std::vector<std::string>
Envelope::split(const std::string& str, const std::string& delimiters)
{
    std::vector<std::string> tokens;

    // Find first "non-delimiter".
    std::string::size_type lastPos = 0;
    std::string::size_type pos = str.find_first_of(delimiters, lastPos);

    while(std::string::npos != pos || std::string::npos != lastPos) {
        // Found a token, add it to the vector.
        tokens.push_back(str.substr(lastPos, pos - lastPos));

        // Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of(delimiters, pos);

        // Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
    }

    return tokens;
}

/*public*/
bool
Envelope::centre(CoordinateXY& p_centre) const
{
    if(isNull()) {
        return false;
    }
    p_centre.x = (getMinX() + getMaxX()) / 2.0;
    p_centre.y = (getMinY() + getMaxY()) / 2.0;
    return true;
}

/*public*/
bool
Envelope::intersection(const Envelope& env, Envelope& result) const
{
    if(isNull() || env.isNull() || ! intersects(env)) {
        return false;
    }

    double intMinX = minx > env.minx ? minx : env.minx;
    double intMinY = miny > env.miny ? miny : env.miny;
    double intMaxX = maxx < env.maxx ? maxx : env.maxx;
    double intMaxY = maxy < env.maxy ? maxy : env.maxy;
    result.init(intMinX, intMaxX, intMinY, intMaxY);
    return true;
}

/*public*/
void
Envelope::translate(double transX, double transY)
{
    if(isNull()) {
        return;
    }
    init(getMinX() + transX, getMaxX() + transX,
         getMinY() + transY, getMaxY() + transY);
}


/*public*/
void
Envelope::expandBy(double deltaX, double deltaY)
{
    minx -= deltaX;
    maxx += deltaX;
    miny -= deltaY;
    maxy += deltaY;

    // check for envelope disappearing
    if(std::isgreater(minx, maxx) || std::isgreater(miny, maxy)) {
        setToNull();
    }
}


bool
operator< (const Envelope& a, const Envelope& b)
{
    /*
    * Compares two envelopes using lexicographic ordering.
    * The ordering comparison is based on the usual numerical
    * comparison between the sequence of ordinates.
    * Null envelopes are less than all non-null envelopes.
    */
    if (a.isNull()) {
        // null == null
        if (b.isNull())
            return false;
        // null < notnull
        else
            return true;
    }
    // notnull > null
    if (b.isNull())
        return false;

    // compare based on numerical ordering of ordinates
    if (a.getMinX() < b.getMinX()) return true;
    if (a.getMinX() > b.getMinX()) return false;
    if (a.getMinY() < b.getMinY()) return true;
    if (a.getMinY() > b.getMinY()) return false;
    if (a.getMaxX() < b.getMaxX()) return true;
    if (a.getMaxX() > b.getMaxX()) return false;
    if (a.getMaxY() < b.getMaxY()) return true;
    if (a.getMaxY() > b.getMaxY()) return false;
    return false; // == is not strictly <
}



} // namespace geos::geom
} // namespace geos
