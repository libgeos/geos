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
 **********************************************************************/

#ifndef GEOS_GEOM_TRIANGLE_H
#define GEOS_GEOM_TRIANGLE_H

#include <geos/export.h>
#include <geos/geom/Coordinate.h>

#include <geos/inline.h>

namespace geos {
namespace geom { // geos::geom

/**
 * \brief
 * Represents a planar triangle, and provides methods for calculating various
 * properties of triangles.
 */
class GEOS_DLL Triangle {
public:
    Coordinate p0, p1, p2;

    Triangle(const Coordinate& nP0, const Coordinate& nP1, const Coordinate& nP2)
        : p0(nP0)
        , p1(nP1)
        , p2(nP2) {}


    /** \brief
     * The inCentre of a triangle is the point which is equidistant
     * from the sides of the triangle.
     *
     * This is also the point at which the bisectors of the angles meet.
     *
     * @param resultPoint the point into which to write the inCentre of the triangle
     */
    void inCentre(Coordinate& resultPoint);

    /** \brief
     * Computes the circumcentre of a triangle.
     *
     * The circumcentre is the centre of the circumcircle, the smallest circle
     * which encloses the triangle. It is also the common intersection point of
     * the perpendicular bisectors of the sides of the triangle, and is the only
     * point which has equal distance to all three vertices of the triangle.
     *
     * The circumcentre does not necessarily lie within the triangle. For example,
     * the circumcentre of an obtuse isoceles triangle lies outside the triangle.
     *
     * This method uses an algorithm due to J.R.Shewchuk which uses normalization
     * to the origin to improve the accuracy of computation. (See *Lecture Notes
     * on Geometric Robustness*, Jonathan Richard Shewchuk, 1999).
     *
     * @param resultPoint the point into which to write the inCentre of the triangle
     */
    void circumcentre(Coordinate& resultPoint);
    void circumcentreDD(Coordinate& resultPoint);

    /** Computes the circumcentre of a triangle. */
    static const Coordinate circumcentre(
        const Coordinate& p0, const Coordinate& p1, const Coordinate& p2);

    bool isIsoceles();

    /**
    * Tests whether a triangle is acute. A triangle is acute if all interior
    * angles are acute. This is a strict test - right triangles will return
    * <tt>false</tt>. A triangle which is not acute is either right or obtuse.
    * <p>
    * Note: this implementation is not robust for angles very close to 90
    * degrees.
    *
    * @param a a vertex of the triangle
    * @param b a vertex of the triangle
    * @param c a vertex of the triangle
    * @return true if the triangle is acute
    */
    static bool isAcute(const Coordinate& a, const Coordinate& b, const Coordinate& c);

    /**
    * Tests whether a triangle is oriented counter-clockwise.
    *
    * @param a a vertex of the triangle
    * @param b a vertex of the triangle
    * @param c a vertex of the triangle
    * @return true if the triangle orientation is counter-clockwise
    */
    static bool isCCW(const Coordinate& a, const Coordinate& b, const Coordinate& c);


    /**
    * Tests whether a triangle intersects a point.
    *
    * @param a a vertex of the triangle
    * @param b a vertex of the triangle
    * @param c a vertex of the triangle
    * @param p the point to test
    * @return true if the triangle intersects the point
    */
    static bool intersects(const Coordinate& a, const Coordinate& b, const Coordinate& c,
        const Coordinate& p);


    /**
    * Tests whether a triangle intersects a point.
    * @param p the point to test
    * @return true if the triangle intersects the point
    */
    bool intersects(const Coordinate& p) { return intersects(p0, p1, p2, p); };

    /**
    * Tests whether this triangle is oriented counter-clockwise.
    * @return true if the triangle orientation is counter-clockwise
    */
    bool isCCW() { return isCCW(p0, p1, p2); };

    /**
    * Tests whether this triangle is acute.
    * @return true if this triangle is acute
    */
    bool isAcute() { return isAcute(p0, p1, p2); };



private:

    /**
     * Computes the determinant of a 2x2 matrix. Uses standard double-precision
     * arithmetic, so is susceptible to round-off error.
     *
     * @param m00
     *          the [0,0] entry of the matrix
     * @param m01
     *          the [0,1] entry of the matrix
     * @param m10
     *          the [1,0] entry of the matrix
     * @param m11
     *          the [1,1] entry of the matrix
     * @return the determinant
     */
    double det(double m00, double m01, double m10, double m11) const;

};


} // namespace geos::geom
} // namespace geos

//#ifdef GEOS_INLINE
//# include "geos/geom/Triangle.inl"
//#endif

#endif // ndef GEOS_GEOM_TRIANGLE_H
