/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#ifndef GEOS_GEOM_ENVELOPE_H
#define GEOS_GEOM_ENVELOPE_H

#include <string>
#include <vector>

#include <geos/inline.h>

namespace geos {
namespace geom { // geos::geom

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
 * the values of <code>Double_POSITIVE_INFINITY</code> and
 * <code>Double_NEGATIVE_INFINITY</code>.
 *
 * When Envelope objects are created or initialized,
 * the supplies extent values are automatically sorted into the correct order.
 *
 */
class Envelope {
public:
	Envelope(void);
	Envelope(double x1, double x2, double y1, double y2);
	Envelope(const Coordinate& p1, const Coordinate& p2);
	Envelope(const Coordinate& p);
	Envelope(const Envelope &env);
	Envelope(const std::string &str);
	~Envelope(void);

	static bool intersects(const Coordinate& p1, const Coordinate& p2,
			const Coordinate& q);

	static bool intersects(const Coordinate& p1, const Coordinate& p2,
			const Coordinate& q1, const Coordinate& q2);

	void init(void);
	void init(double x1, double x2, double y1, double y2);
	void init(const Coordinate& p1, const Coordinate& p2);
	void init(const Coordinate& p);
	void init(Envelope env);
	void setToNull(void);

	bool isNull(void) const;

	double getWidth(void) const;
	double getHeight(void) const;

	/**
	 *  Returns the Envelope maximum y-value. min y > max y
	 *  indicates that this is a null Envelope.
	 */
	double getMaxY() const;

	/**
	 *  Returns the Envelope maximum x-value. min x > max x
	 *  indicates that this is a null Envelope.
	 */
	double getMaxX() const;

	/**
	 *  Returns the Envelope minimum y-value. min y > max y
	 *  indicates that this is a null Envelope.
	 */
	double getMinY() const;

	/**
	 *  Returns the Envelope minimum x-value. min x > max x
	 *  indicates that this is a null Envelope.
	 */
	double getMinX() const;

	/**
	 * Computes the coordinate of the centre of this envelope
	 * (as long as it is non-null)
	 *
	 * @param centre The coordinate to write results into
	 * @return NULL is the center could not be found
	 * (null envelope).
	 */
	bool centre(Coordinate& centre) const;

	/**
	 * Computes the intersection of two {@link Envelopes}
	 *
	 * @param env the envelope to intersect with
	 * @param result the envelope representing the intersection of
	 *               the envelopes (this will be the null envelope
	 *               if either argument is null, or they do not intersect)
	 * @return false if not intersection is found
	 */
	bool intersection(const Envelope& env, Envelope& result);

	/**
	 * Translates this envelope by given amounts in the X and Y direction.
	 *
	 * @param transX the amount to translate along the X axis
	 * @param transY the amount to translate along the Y axis
	 */
	void translate(double transX, double transY);

	/**
	 * Expands this envelope by a given distance in all directions.
	 * Both positive and negative distances are supported.
	 *
	 * @param deltaX the distance to expand the envelope along 
	 *               the X axis
	 * @param deltaY the distance to expand the envelope along
	 *               the Y axis
	 */
	void expandBy(double deltaX, double deltaY);

	/**
	 * Expands this envelope by a given distance in all directions.
	 * Both positive and negative distances are supported.
	 *
	 * @param distance the distance to expand the envelope
	 * @return this envelope
	 */
	void expandBy(double distance) { expandBy(distance, distance); }

	void expandToInclude(const Coordinate& p);
	void expandToInclude(double x, double y);
	void expandToInclude(const Envelope* other);
	bool contains(const Coordinate& p) const;
	bool contains(double x, double y) const;
	bool contains(const Envelope* other) const;
	bool contains(const Envelope& other) const { return contains(&other); }

	/**
	 * Check if the point p
	 * intersects (lies inside) the region of this Envelope.
	 *
	 * @param  p  	the Coordinate to be tested
	 * @return true if the point intersects this Envelope
	 */
	bool intersects(const Coordinate& p) const;

	/**
	 *  Check if the point (x, y)
	 *  intersects (lies inside) the region of this Envelope.
	 *
	 * @param  x  the x-ordinate of the point
	 * @param  y  the y-ordinate of the point
	 * @return <code>true</code> if the point intersects this Envelope
	 */
	bool intersects(double x, double y) const;

	/**
	 * Check if the region defined by other Envelope
	 * intersects (intersects) the region of this Envelope.
	 *
	 * @param other the Envelope which this Envelope is
	 *              being checked for intersection
	 *
	 * @return      true if the Envelopes intersects
	 */
	bool intersects(const Envelope* other) const;

	bool intersects(const Envelope& other) const;

	bool equals(const Envelope* other) const;
	std::string toString(void) const;
	double distance(const Envelope* env) const;
	int hashCode() const;

private:
	std::vector<std::string> split(const std::string &str, const std::string &delimiters = " ");
	static double distance(double x0,double y0,double x1,double y1);
	double minx;	/// the minimum x-coordinate
	double maxx;	/// the maximum x-coordinate
	double miny;	/// the minimum y-coordinate
	double maxy;	/// the maximum y-coordinate
};

/// Checks if two Envelopes are equal (2D only check)
bool operator==(const Envelope& a, const Envelope& b);

} // namespace geos::geom
} // namespace geos

#ifdef GEOS_INLINE
# include "geos/geom/Envelope.inl"
#endif

#endif // ndef GEOS_GEOM_ENVELOPE_H

/**********************************************************************
 * $Log$
 * Revision 1.2  2006/03/24 09:52:41  strk
 * USE_INLINE => GEOS_INLINE
 *
 * Revision 1.1  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/
