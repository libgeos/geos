/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2009  Sandro Santilli <strk@keybit.net>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 ***********************************************************************
 *
 * Last port: operation/overlay/snap/GeometrySnapper.java rev 1.8 (JTS-1.10)
 *
 **********************************************************************/

#include <geos/operation/overlay/snap/GeometrySnapper.h>
#include <geos/operation/overlay/snap/LineStringSnapper.h>
#include <geos/geom/PrecisionModel.h> 
#include <geos/geom/util/GeometryTransformer.h> // SnapTransformer inheritance
#include <geos/geom/CoordinateSequence.h> 
#include <geos/util/UniqueCoordinateArrayFilter.h> 

#include <cassert>
#include <limits> // for numeric_limits
#include <memory> // for auto_ptr

#include <algorithm> // for std::min

#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif

using namespace std;
using namespace geos::geom;

namespace geos {
namespace operation { // geos.operation
namespace overlay { // geos.operation.overlay
namespace snap { // geos.operation.overlay.snap

namespace { // anonymous

class SnapTransformer : public geom::util::GeometryTransformer
{
public:

	double snapTolerance;
	const vector<const Coordinate*>& snapPts;

	SnapTransformer(double nSnapTolerance,
			const vector<const Coordinate*>& nSnapPts)
		:
		snapTolerance(nSnapTolerance),
		snapPts(nSnapPts)
	{
	}

protected:

	virtual CoordinateSequence::AutoPtr transformCoordinates(
		const CoordinateSequence* coords,
		const Geometry* parent)
	{
		return snapLine(*coords, snapPts);
	}

private:

	auto_ptr< CoordinateSequence >
	snapLine(const CoordinateSequence& srcPts,
	         const vector<const Coordinate*>& snapPts)
	{
		// TODO: make the LineStringSnapper a private member...
		LineStringSnapper snapper(srcPts, snapTolerance);

		return snapper.snapTo(snapPts);
	}
};

} // anonymous namespace

/* private static */
const double GeometrySnapper::SNAP_PRECISION_FACTOR = 10e-10;


/* public static */
double
GeometrySnapper::computeOverlaySnapTolerance(const Geometry& g)
{
	double snapTolerance = computeSizeBasedSnapTolerance(g);

	/**
	 * Overlay is carried out in the precision model
	 * of the two inputs.
	 * If this precision model is of type FIXED, then the snap tolerance
	 * must reflect the precision grid size.
	 * Specifically, the snap tolerance should be at least
	 * the distance from a corner of a precision grid cell
	 * to the centre point of the cell.
	 */
	const PrecisionModel& pm = *(g.getPrecisionModel());
	if (pm.getType() == PrecisionModel::FIXED) {
		double fixedSnapTol = (1 / pm.getScale()) * 2 / 1.415;
		if (fixedSnapTol > snapTolerance)
			snapTolerance = fixedSnapTol;
	}
	return snapTolerance;
}

/* public static */
double
GeometrySnapper::computeSizeBasedSnapTolerance(const geom::Geometry& g)
{
	const Envelope& env = *(g.getEnvelopeInternal());
	double minDimension = std::min(env.getHeight(), env.getWidth());
	double snapTol = minDimension * SNAP_PRECISION_FACTOR;
	return snapTol;
}

/* public static */
double
GeometrySnapper::computeOverlaySnapTolerance(const geom::Geometry& g0,
	                                     const geom::Geometry& g1)
{
	return std::min(computeOverlaySnapTolerance(g0),
			computeOverlaySnapTolerance(g1));
}

/* public static */
void
GeometrySnapper::snap(const geom::Geometry& g0,
                      const geom::Geometry& g1,
                      double snapTolerance,
                      GeometrySnapper::GeomPtrPair& snapGeom)
{
	GeometrySnapper snapper0(g0);
	snapGeom.first = snapper0.snapTo(g1, snapTolerance);

	GeometrySnapper snapper1(g1);

	/**
	 * Snap the second geometry to the snapped first geometry
	 * (this strategy minimizes the number of possible different
	 * points in the result)
	 */
	snapGeom.second = snapper1.snapTo(*snapGeom.first, snapTolerance);

//	cout << *snapGeom.first << endl;
//	cout << *snapGeom.second << endl;

	//return snapGeom;
}

/*public*/
GeometrySnapper::GeomPtr
GeometrySnapper::snapTo(const geom::Geometry& snapGeom, double snapTolerance)
{
	vector<const Coordinate*> snapPts;
	extractTargetCoordinates(snapGeom, snapPts);

	SnapTransformer snapTrans(snapTolerance, snapPts);
	return snapTrans.transform(&srcGeom);
}

/*public*/
void
GeometrySnapper::extractTargetCoordinates(const Geometry& g,
	  vector<const Coordinate*>& target)
{
	util::UniqueCoordinateArrayFilter filter(target);
	g.apply_ro(&filter);
}


} // namespace geos.operation.snap
} // namespace geos.operation.overlay
} // namespace geos.operation
} // namespace geos

