/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2006 Refractions Research Inc.
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

#pragma once


#include <geos/export.h>
#include <geos/geom/Coordinate.h>

#include <string>
#include <vector>
#include <ostream> // for operator<<
#include <memory>
#include <cassert>
#include <algorithm>

namespace geos {
namespace geom { // geos::geom

class Envelope;

/// Output operator
GEOS_DLL std::ostream& operator<< (std::ostream& os, const Envelope& o);

class Coordinate;

/**
 * \class Envelope geom.h geos.h
 *
 * \brief
 * An Envelope defines a rectangulare region of the 2D coordinate plane.
 *
 * It is often used to represent the bounding box of a Geometry,
 * e.g. the minimum and maximum x and y values of the Coordinates.
 *
 * Note that Envelopes support infinite or half-infinite regions, by using
 * the values of `Double_POSITIVE_INFINITY` and `Double_NEGATIVE_INFINITY`.
 *
 * When Envelope objects are created or initialized, the supplies extent
 * values are automatically sorted into the correct order.
 *
 */
class GEOS_DLL Envelope {

public:

    friend std::ostream& operator<< (std::ostream& os, const Envelope& o);

    typedef std::unique_ptr<Envelope> Ptr;

    /** \brief
     * Creates a null Envelope.
     */
    Envelope()
        : minx(DoubleNotANumber)
        , maxx(DoubleNotANumber)
        , miny(DoubleNotANumber)
        , maxy(DoubleNotANumber)
        {};

    /** \brief
     * Creates an Envelope for a region defined by maximum and minimum values.
     *
     * @param  x1  the first x-value
     * @param  x2  the second x-value
     * @param  y1  the first y-value
     * @param  y2  the second y-value
     */
    Envelope(double x1, double x2, double y1, double y2)
    {
        init(x1, x2, y1, y2);
    }

    /** \brief
     * Creates an Envelope for a region defined by two Coordinates.
     *
     * @param  p1  the first Coordinate
     * @param  p2  the second Coordinate
     */
    Envelope(const CoordinateXY& p1, const CoordinateXY& p2)
    {
        init(p1, p2);
    }

    /** \brief
     * Creates an Envelope for a region defined by a single Coordinate.
     *
     * @param  p  the Coordinate
     */
    explicit Envelope(const CoordinateXY& p)
        : minx(p.x)
        , maxx(p.x)
        , miny(p.y)
        , maxy(p.y)
    {
    }

    /** \brief
     * Create an Envelope from an Envelope string representation produced
     * by Envelope::toString()
     */
    explicit Envelope(const std::string& str);

    /** \brief
     * Test the point `q` to see whether it intersects the Envelope
     * defined by `p1-p2`.
     *
     * @param p1 one extremal point of the envelope
     * @param p2 another extremal point of the envelope
     * @param q the point to test for intersection
     * @return `true` if q intersects the envelope p1-p2
     */
    static bool intersects(const CoordinateXY& p1, const CoordinateXY& p2,
                           const CoordinateXY& q);

    /** \brief
     * Test the envelope defined by `p1-p2` for intersection
     * with the envelope defined by `q1-q2`.
     *
     * @param p1 one extremal point of the envelope P
     * @param p2 another extremal point of the envelope P
     * @param q1 one extremal point of the envelope Q
     * @param q2 another extremal point of the envelope Q
     *
     * @return `true` if Q intersects P
     */
    static bool intersects(
        const CoordinateXY& p1, const CoordinateXY& p2,
        const CoordinateXY& q1, const CoordinateXY& q2)
    {
        double minq = std::min(q1.x, q2.x);
        double maxq = std::max(q1.x, q2.x);
        double minp = std::min(p1.x, p2.x);
        double maxp = std::max(p1.x, p2.x);
        if(minp > maxq) {
            return false;
        }
        if(maxp < minq) {
            return false;
        }
        minq = std::min(q1.y, q2.y);
        maxq = std::max(q1.y, q2.y);
        minp = std::min(p1.y, p2.y);
        maxp = std::max(p1.y, p2.y);
        if(minp > maxq) {
            return false;
        }
        if(maxp < minq) {
            return false;
        }
        return true;
    }

    /** \brief
     * Check if the extent defined by two extremal points intersects
     * the extent of this Envelope.
     *
     * @param a a point
     * @param b another point
     * @return `true` if the extents intersect
     */
    bool intersects(const CoordinateXY& a, const CoordinateXY& b) const;

    /** \brief
     *  Initialize to a null Envelope.
     */
    void init()
    {
        setToNull();
    };

    /** \brief
     * Initialize an Envelope for a region defined by maximum and minimum values.
     *
     * @param  x1  the first x-value
     * @param  x2  the second x-value
     * @param  y1  the first y-value
     * @param  y2  the second y-value
     */
    void init(double x1, double x2, double y1, double y2)
    {
        if(x1 < x2) {
            minx = x1;
            maxx = x2;
        }
        else {
            minx = x2;
            maxx = x1;
        }
        if(y1 < y2) {
            miny = y1;
            maxy = y2;
        }
        else {
            miny = y2;
            maxy = y1;
        }
    };

    /** \brief
     * Initialize an Envelope to a region defined by two Coordinates.
     *
     * @param  p1  the first Coordinate
     * @param  p2  the second Coordinate
     */
    void init(const CoordinateXY& p1, const CoordinateXY& p2)
    {
        init(p1.x, p2.x, p1.y, p2.y);
    };

    /** \brief
     * Initialize an Envelope to a region defined by a single Coordinate.
     *
     * @param  p  the Coordinate
     */
    void init(const CoordinateXY& p)
    {
        init(p.x, p.x, p.y, p.y);
    };

    /** \brief
     * Makes this `Envelope` a "null" envelope, that is, the envelope
     * of the empty geometry.
     */
    void setToNull()
    {
        minx = maxx = miny = maxy = DoubleNotANumber;
    };

    /** \brief
     * Returns `true` if this Envelope is a "null" envelope.
     *
     * @return `true` if this Envelope is uninitialized or is the
     *                envelope of the empty geometry.
     */
    bool isNull(void) const
    {
        return std::isnan(maxx);
    };

    /** \brief
     * Returns the difference between the maximum and minimum x values.
     *
     * @return  `max x - min x`, or 0 if this is a null Envelope
     */
    double getWidth() const
    {
        if(isNull()) {
            return 0;
        }
        return maxx - minx;
    }

    /** \brief
     * Returns the difference between the maximum and minimum y values.
     *
     * @return `max y - min y`, or 0 if this is a null Envelope
     */
    double getHeight() const
    {
        if(isNull()) {
            return 0;
        }
        return maxy - miny;
    }

    /** \brief
     * Gets the area of this envelope.
     *
     * @return the area of the envelope
     * @return 0.0 if the envelope is null
     */
    double
    getArea() const
    {
        return getWidth() * getHeight();
    }

    /** \brief
     * Returns true if this Envelope covers a finite region
     */
    bool
    isFinite() const
    {
        return std::isfinite(getArea());
    }

    /** \brief
     * Returns the Envelope maximum y-value. `min y > max y` indicates
     * that this is a null Envelope.
     */
    double getMaxY() const
    {
        assert(!isNull());
        return maxy;
    };

    /** \brief
     * Returns the Envelope maximum x-value. `min x > max x` indicates
     * that this is a null Envelope.
     */
    double getMaxX() const
    {
        assert(!isNull());
        return maxx;
    };

    /** \brief
     * Returns the Envelope minimum y-value. `min y > max y` indicates
     * that this is a null Envelope.
     */
    double getMinY() const
    {
        assert(!isNull());
        return miny;
    };

    /** \brief
     * Returns the Envelope minimum x-value. `min x > max x` indicates
     * that this is a null Envelope.
     */
    double getMinX() const
    {
        assert(!isNull());
        return minx;
    };

    /**
     * Gets the length of the diameter (diagonal) of the envelope.
     *
     * @return the diameter length
     */
    double getDiameter() const
    {
        if (isNull()) {
            return 0.0;
        }
        double w = getWidth();
        double h = getHeight();
        return std::sqrt(w*w + h*h);
    }

    /** \brief
     * Computes the coordinate of the centre of this envelope
     * (as long as it is non-null).
     *
     * @param centre The coordinate to write results into
     * @return `false` if the center could not be found (null envelope).
     */
    bool centre(CoordinateXY& centre) const;

    /** \brief
     * Computes the intersection of two [Envelopes](@ref Envelope).
     *
     * @param env the envelope to intersect with
     * @param result the envelope representing the intersection of
     *               the envelopes (this will be the null envelope
     *               if either argument is null, or they do not intersect)
     * @return false if not intersection is found
     */
    bool intersection(const Envelope& env, Envelope& result) const;

    /** \brief
     * Translates this envelope by given amounts in the X and Y direction.
     *
     * @param transX the amount to translate along the X axis
     * @param transY the amount to translate along the Y axis
     */
    void translate(double transX, double transY);

    /** \brief
     * Expands this envelope by a given distance in all directions.
     * Both positive and negative distances are supported.
     *
     * @param deltaX the distance to expand the envelope along the X axis
     * @param deltaY the distance to expand the envelope along the Y axis
     */
    void expandBy(double deltaX, double deltaY);

    /** \brief
     * Expands this envelope by a given distance in all directions.
     *
     * Both positive and negative distances are supported.
     *
     * @param p_distance the distance to expand the envelope
     */
    void
    expandBy(double p_distance)
    {
        expandBy(p_distance, p_distance);
    };

    /** \brief
     * Enlarges the boundary of the Envelope so that it contains p. Does
     * nothing if p is already on or within the boundaries.
     *
     * @param  p the Coordinate to include
     */
    void expandToInclude(const CoordinateXY& p)
    {
        expandToInclude(p.x, p.y);
    };

    /** \brief
     * Enlarges the boundary of the Envelope so that it contains (x,y).
     *
     * Does nothing if (x,y) is already on or within the boundaries.
     *
     * @param  x  the value to lower the minimum x
     *            to or to raise the maximum x to
     * @param  y  the value to lower the minimum y
     *            to or to raise the maximum y to
     */
    void expandToInclude(double x, double y)
    {
        if(isNull()) {
            minx = x;
            maxx = x;
            miny = y;
            maxy = y;
        }
        else {
            if(x < minx) {
                minx = x;
            }
            if(x > maxx) {
                maxx = x;
            }
            if(y < miny) {
                miny = y;
            }
            if(y > maxy) {
                maxy = y;
            }
        }
    };

    /** \brief
     * Enlarges the boundary of the Envelope so that it contains `other`.
     *
     * Does nothing if other is wholly on or within the boundaries.
     *
     * @param other the Envelope to merge with
     */
    void expandToInclude(const Envelope* other)
    {
        if(isNull()) {
            minx = other->minx;
            maxx = other->maxx;
            miny = other->miny;
            maxy = other->maxy;
        }
        else {
            if(std::isless(other->minx, minx)) {
                minx = other->minx;
            }
            if(std::isgreater(other->maxx, maxx)) {
                maxx = other->maxx;
            }
            if(std::isless(other->miny, miny)) {
                miny = other->miny;
            }
            if(std::isgreater(other->maxy, maxy)) {
                maxy = other->maxy;
            }
        }
    };

    void expandToInclude(const Envelope& other)
    {
        return expandToInclude(&other);
    };

    /** \brief
     * Tests if the Envelope `other` lies wholly inside this Envelope
     * (inclusive of the boundary).
     *
     * Note that this is **not** the same definition as the SFS `contains`,
     * which would exclude the envelope boundary.
     *
     * @param other the Envelope to check
     * @return `true` if `other` is contained in this Envelope
     *
     * @see covers(Envelope)
     */
    bool
    contains(const Envelope& other) const
    {
        return covers(other);
    }

    bool
    contains(const Envelope* other) const
    {
        return contains(*other);
    }

    /** \brief
     * Returns `true` if the given point lies in or on the envelope.
     *
     * @param p the point which this Envelope is being checked for containing
     * @return `true` if the point lies in the interior or on the boundary
     *         of this Envelope.
     */
    bool
    contains(const CoordinateXY& p) const
    {
        return covers(p.x, p.y);
    }

    /** \brief
     * Returns `true` if the given point lies in or on the envelope.
     *
     * @param x the x-coordinate of the point which this Envelope is
     *          being checked for containing
     * @param y the y-coordinate of the point which this Envelope is being
     *          checked for containing
     * @return `true` if `(x, y)` lies in the interior or on the boundary
     *         of this Envelope.
     */
    bool
    contains(double x, double y) const
    {
        return covers(x, y);
    }

    /** \brief
     * Check if the point p intersects (lies inside) the region of this Envelope.
     *
     * @param other the Coordinate to be tested
     * @return true if the point intersects this Envelope
     */
    bool intersects(const CoordinateXY& other) const
    {
        return (std::islessequal(other.x, maxx) && std::isgreaterequal(other.x, minx) &&
                std::islessequal(other.y, maxy) && std::isgreaterequal(other.y,  miny));
    }

    /** \brief
     *  Check if the point (x, y) intersects (lies inside) the region of this Envelope.
     *
     * @param x the x-ordinate of the point
     * @param y the y-ordinate of the point
     * @return `true` if the point intersects this Envelope
     */
    bool intersects(double x, double y) const
    {
        return std::islessequal(x, maxx) &&
               std::isgreaterequal(x, minx) &&
               std::islessequal(y, maxy) &&
               std::isgreaterequal(y, miny);
    }

    /** \brief
     * Check if the region defined by other Envelope intersects the region of this Envelope.
     *
     * @param other the Envelope which this Envelope is being checked for intersection
     * @return true if the Envelopes intersects
     */
    bool intersects(const Envelope* other) const
    {
        return std::islessequal(other->minx, maxx) &&
               std::isgreaterequal(other->maxx, minx) &&
               std::islessequal(other->miny, maxy) &&
               std::isgreaterequal(other->maxy, miny);
    }

    bool intersects(const Envelope& other) const
    {
        return intersects(&other);
    }

    /**
    * Tests if the region defined by other
    * is disjoint from the region of this Envelope
    *
    * @param other  the Envelope being checked for disjointness
    * @return true if the Envelopes are disjoint
    */
    bool disjoint(const Envelope& other) const
    {
        return !intersects(other);
    }

    bool disjoint(const Envelope* other) const
    {
        return !intersects(other);
    }

    /** \brief
     * Tests if the given point lies in or on the envelope.
     *
     * @param x the x-coordinate of the point which this Envelope is being checked for containing
     * @param y the y-coordinate of the point which this Envelope is being checked for containing
     * @return `true` if `(x, y)` lies in the interior or on the boundary of this Envelope.
     */
    bool covers(double x, double y) const {
        return std::isgreaterequal(x,  minx) &&
               std::islessequal(x, maxx) &&
               std::isgreaterequal(y, miny) &&
               std::islessequal(y,  maxy);
    }

    /** \brief
     * Tests if the given point lies in or on the envelope.
     *
     * @param p the point which this Envelope is being checked for containing
     * @return `true` if the point lies in the interior or on the boundary of this Envelope.
     */
    bool covers(const CoordinateXY* p) const
    {
        return covers(p->x, p->y);
    }

    /** \brief
     * Tests if the Envelope `other` lies wholly inside this Envelope (inclusive of the boundary).
     *
     * @param other the Envelope to check
     * @return true if this Envelope covers the `other`
     */
    bool covers(const Envelope& other) const;

    bool
    covers(const Envelope* other) const
    {
        return covers(*other);
    }


    /** \brief
     * Returns `true` if the Envelope `other` spatially equals this Envelope.
     *
     * @param  other the Envelope which this Envelope is being checked for equality
     * @return `true` if this and `other` Envelope objects are spatially equal
     */
    bool equals(const Envelope* other) const;

    /**
     * Returns `true` if all the extents of the Envelope are finite and defined (not NaN)
     *
     * @return `true` if envelope has only finite/valid extents, `false` otherwise
     */
    bool isfinite() const;

    /** \brief
     * Returns a `string` of the form `Env[minx:maxx,miny:maxy]`.
     *
     * @return a `string` of the form `Env[minx:maxx,miny:maxy]`
     */
    std::string toString() const;

    /** \brief
     * Computes the distance between this and another Envelope.
     *
     * The distance between overlapping Envelopes is 0. Otherwise, the
     * distance is the Euclidean distance between the closest points.
     */
    double distance(const Envelope& env) const
    {
        return std::sqrt(distanceSquared(env));
    }

    /** \brief
     * Computes the maximum distance between points in this and another Envelope.
     */
    double maxDistance(const Envelope& other) const
    {
        Coordinate p(std::min(minx, other.minx), std::min(miny, other.miny));
        Coordinate q(std::max(maxx, other.maxx), std::max(maxy, other.maxy));
        return p.distance(q);
    }

    /** \brief
     * Computes the square of the distance between this and another Envelope.
     *
     * The distance between overlapping Envelopes is 0. Otherwise, the
     * distance is the Euclidean distance between the closest points.
     */
    double distanceSquared(const Envelope& env) const
    {
        double dx = std::max(0.0,
            std::max(maxx, env.maxx) - std::min(minx, env.minx) - (maxx - minx) -
            (env.maxx - env.minx));
        double dy = std::max(0.0,
            std::max(maxy, env.maxy) - std::min(miny, env.miny) - (maxy - miny) -
            (env.maxy - env.miny));

        return dx * dx + dy * dy;
    };

    /** \brief
     * Computes the distance between one Coordinate and an Envelope
     * defined by two other Coordinates. The order of the Coordinates
     * used to define the envelope is not significant.
     *
     * @param c the coordinate to from which distance should be found
     * @param p0 first coordinate defining an envelope
     * @param p1 second coordinate defining an envelope.
     */
    static double distanceToCoordinate(
        const CoordinateXY& c,
        const CoordinateXY& p0,
        const CoordinateXY& p1)
    {
        return std::sqrt(distanceSquaredToCoordinate(c, p0, p1));
    };

    /** \brief
     * Computes the squared distance between one Coordinate and an Envelope
     * defined by two other Coordinates. The order of the Coordinates
     * used to define the envelope is not significant.
     *
     * @param c the coordinate to from which distance should be found
     * @param p0 first coordinate defining an envelope
     * @param p1 second coordinate defining an envelope.
     */
    static double distanceSquaredToCoordinate(
        const CoordinateXY& c,
        const CoordinateXY& p0,
        const CoordinateXY& p1)
    {
        double xa = c.x - p0.x;
        double xb = c.x - p1.x;
        double ya = c.y - p0.y;
        double yb = c.y - p1.y;

        // If sign of a and b are not the same, then Envelope spans c and distance is zero.
        double dx = (std::signbit(xa) == std::signbit(xb)) * std::min(std::abs(xa), std::abs(xb));
        double dy = (std::signbit(ya) == std::signbit(yb)) * std::min(std::abs(ya), std::abs(yb));

        return dx*dx + dy*dy;
    }

    std::size_t hashCode() const
    {
        auto hash = std::hash<double>{};

        //Algorithm from Effective Java by Joshua Bloch [Jon Aquino]
        std::size_t result = 17;
        result = 37 * result + hash(minx);
        result = 37 * result + hash(maxx);
        result = 37 * result + hash(miny);
        result = 37 * result + hash(maxy);
        return result;
    }

    struct GEOS_DLL HashCode
    {
        std::size_t operator()(const Envelope& e) const
        {
            return e.hashCode();
        };
    };

    /// Checks if two Envelopes are equal (2D only check)
    // GEOS_DLL bool operator==(const Envelope& a, const Envelope& b);
    GEOS_DLL friend bool
    operator==(const Envelope& a, const Envelope& b)
    {
        return a.equals(&b);
    }

    // GEOS_DLL bool operator!=(const Envelope& a, const Envelope& b);
    GEOS_DLL friend bool
    operator!=(const Envelope& a, const Envelope& b)
    {
        return !(a == b);
    }

    /// Strict weak ordering operator for Envelope
    /// This is the C++ equivalent of JTS's compareTo
    GEOS_DLL friend bool
    operator< (const Envelope& a, const Envelope& b);

private:

    /** \brief
     * Splits a string into parts based on the supplied delimiters.
     *
     * This is a generic function that really belongs in a utility
     * file somewhere
     */
    static std::vector<std::string> split(const std::string& str,
                                   const std::string& delimiters = " ");

    static double distance(double x0, double y0, double x1, double y1)
    {
        double dx = x1 - x0;
        double dy = y1 - y0;
        return std::sqrt(dx * dx + dy * dy);
    }

    /// the minimum x-coordinate
    double minx;

    /// the maximum x-coordinate
    double maxx;

    /// the minimum y-coordinate
    double miny;

    /// the maximum y-coordinate
    double maxy;
};




} // namespace geos::geom
} // namespace geos
