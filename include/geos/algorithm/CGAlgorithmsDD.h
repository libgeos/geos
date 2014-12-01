/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2014 Mateusz Loskot <mateusz@loskot.net>
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: algorithm/CGAlgorithmsDD.java r789 (JTS-1.14)
 *
 **********************************************************************/

#ifndef GEOS_ALGORITHM_CGALGORITHMDD_H
#define GEOS_ALGORITHM_CGALGORITHMDD_H

#include <geos/export.h>

// Forward declarations
namespace geos {
	namespace geom {
		class Coordinate;
		class CoordinateSequence;
	}
}

namespace geos {
namespace algorithm { // geos::algorithm

/**
 * \brief
 * Implements basic computational geometry algorithms using extended precision float-point arithmetic.
 */
class GEOS_DLL CGAlgorithmsDD {

public:
  
  /**
   * Returns the index of the direction of the point <code>q</code> relative to
   * a vector specified by <code>p1-p2</code>.
   * 
   * @param p1 the origin point of the vector
   * @param p2 the final point of the vector
   * @param q the point to compute the direction to
   * 
   * @return 1 if q is counter-clockwise (left) from p1-p2
   * @return -1 if q is clockwise (right) from p1-p2
   * @return 0 if q is collinear with p1-p2
   */
    static int orientationIndex(const geom::Coordinate& p1,
        const geom::Coordinate& p2,
        const geom::Coordinate& q);

  /**
   * A filter for computing the orientation index of three coordinates.
   * <p>
   * If the orientation can be computed safely using standard DP
   * arithmetic, this routine returns the orientation index.
   * Otherwise, a value i > 1 is returned.
   * In this case the orientation index must 
   * be computed using some other more robust method.
   * The filter is fast to compute, so can be used to 
   * avoid the use of slower robust methods except when they are really needed,
   * thus providing better average performance.
   * <p>
   * Uses an approach due to Jonathan Shewchuk, which is in the public domain.
   * 
   * @param pa a coordinate
   * @param pb a coordinate
   * @param pc a coordinate
   * @return the orientation index if it can be computed safely
   * @return i > 1 if the orientation index cannot be computed safely
   */
    static int orientationIndexFilter(const geom::Coordinate& pa,
        const geom::Coordinate& pb,
        const geom::Coordinate& pc);

    static int signum(double x) {
        if (x > 0) return 1;
        if (x < 0) return -1;
        return 0;
    }
};

} // namespace geos::algorithm
} // namespace geos

#endif // GEOS_ALGORITHM_CGALGORITHM_H
