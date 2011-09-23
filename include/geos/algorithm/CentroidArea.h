/**********************************************************************
 * $Id: CentroidArea.h 2556 2009-06-06 22:22:28Z strk $
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2005-2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#ifndef GEOS_ALGORITHM_CENTROIDAREA_H
#define GEOS_ALGORITHM_CENTROIDAREA_H


#include <geos/export.h>
#include <geos/geom/Coordinate.h>

// Forward declarations
namespace geos {
	namespace geom {
		class CoordinateSequence;
		class Geometry;
		class Polygon;
	}
}

namespace geos {
namespace algorithm { // geos::algorithm

/**
 * \class CentroidArea geosAlgorithm.h geos/geosAlgorithm.h
 *
 * \brief Computes the centroid of an area geometry.
 *
 * Algorithm:
 *
 * Based on the usual algorithm for calculating
 * the centroid as a weighted sum of the centroids
 * of a decomposition of the area into (possibly overlapping) triangles.
 * The algorithm has been extended to handle holes and multi-polygons.
 * See <code>http://www.faqs.org/faqs/graphics/algorithms-faq/</code>
 * for further details of the basic approach.
 */
class GEOS_DLL CentroidArea {

public:

	CentroidArea()
		:
		basePt(0.0, 0.0),
		areasum2(0)
	{}

	~CentroidArea() {}

	/**
	 * Adds the area defined by a Geometry to the centroid total.
	 * If the geometry has no area it does not contribute to the centroid.
	 *
	 * @param geom the geometry to add
	 */
	void add(const geom::Geometry *geom);

	/**
	 * Adds the area defined by an array of
	 * coordinates.  The array must be a ring;
	 * i.e. end with the same coordinate as it starts with.
	 * @param ring an array of {@link Coordinate}s
	 */
	void add(const geom::CoordinateSequence *ring);

	geom::Coordinate* getCentroid() const;

	/// Return false if a centroid couldn't be computed
	bool getCentroid(geom::Coordinate& ret) const;

private:

	/// the point all triangles are based at
	geom::Coordinate basePt;

	// temporary variable to hold centroid of triangle
	geom::Coordinate triangleCent3;

	/// Partial area sum 
	double areasum2;       

	/// partial centroid sum
	geom::Coordinate cg3;

	void setBasePoint(const geom::Coordinate &newbasePt);

	void add(const geom::Polygon *poly);

	void addShell(const geom::CoordinateSequence *pts);

	void addHole(const geom::CoordinateSequence *pts);
	
	void addTriangle(const geom::Coordinate &p0, const geom::Coordinate &p1,
			const geom::Coordinate &p2,bool isPositiveArea);

	static void centroid3(const geom::Coordinate &p1, const geom::Coordinate &p2,
			const geom::Coordinate &p3, geom::Coordinate &c);

	static double area2(const geom::Coordinate &p1, const geom::Coordinate &p2,
			const geom::Coordinate &p3);

};

} // namespace geos::algorithm
} // namespace geos


#endif // GEOS_ALGORITHM_CENTROIDAREA_H

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/03/09 16:46:48  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/

