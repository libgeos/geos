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
 ***********************************************************************
 *
 * Last port: operation/overlay/snap/GeometrySnapper.java rev. 1.1
 * (we should move in GEOS too, probably)
 *
 **********************************************************************/

#include <geos/precision/GeometrySnapper.h>
#include <geos/precision/LineStringSnapper.h>
#include <geos/geom/util/GeometryTransformer.h> // inherit. of SnapTransformer
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/Coordinate.h>
#include <geos/util/UniqueCoordinateArrayFilter.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/CoordinateSequenceFactory.h>
#include <geos/geom/PrecisionModel.h>

#include <vector>
#include <memory>
#include <algorithm>

//using namespace std;
using namespace geos::geom;

namespace geos {
namespace precision { // geos.precision

//const double GeometrySnapper::snapTol = 0.000001;
const double GeometrySnapper::snapPrecisionFactor = 10e-10; // 10e-8 in JTS!

class SnapTransformer: public geos::geom::util::GeometryTransformer {

private:

	double snapTol;

	const Coordinate::ConstVect& snapPts;

	CoordinateSequence::AutoPtr snapLine(
			const CoordinateSequence* srcPts)
	{
		using std::auto_ptr;

		assert(srcPts);
		assert(srcPts->toVector());
		LineStringSnapper snapper(*(srcPts->toVector()), snapTol);
		auto_ptr<Coordinate::Vect> newPts = snapper.snapTo(snapPts);

		const CoordinateSequenceFactory* cfact = factory->getCoordinateSequenceFactory();
		return auto_ptr<CoordinateSequence>(cfact->create(newPts.release()));
	}

public:

	SnapTransformer(double nSnapTol,
			const Coordinate::ConstVect& nSnapPts)
		:
		snapTol(nSnapTol),
		snapPts(nSnapPts)
	{
	}

	CoordinateSequence::AutoPtr transformCoordinates(
			const CoordinateSequence* coords,
			const Geometry* parent)
	{
		return snapLine(coords);
	}


};

/*private*/
std::auto_ptr<Coordinate::ConstVect>
GeometrySnapper::extractTargetCoordinates(const Geometry& g)
{
	std::auto_ptr<Coordinate::ConstVect> snapPts(new Coordinate::ConstVect());
	util::UniqueCoordinateArrayFilter filter(*snapPts);
	g.apply_ro(&filter);
	// integrity check
	assert( snapPts->size() <= g.getNumPoints() );
	return snapPts;
}

/*public*/
std::auto_ptr<geom::Geometry>
GeometrySnapper::snapTo(const geom::Geometry& g, double snapTolerance)
{

	using std::auto_ptr;
	using geom::util::GeometryTransformer;
	
	// Get snap points
	auto_ptr<Coordinate::ConstVect> snapPts=extractTargetCoordinates(g);

	// Apply a SnapTransformer to source geometry
	// (we need a pointer for dynamic polymorphism)
	auto_ptr<GeometryTransformer> snapTrans(new SnapTransformer(snapTolerance, *snapPts));
	return snapTrans->transform(&srcGeom);
}

/*public static*/
double
GeometrySnapper::computeSnapTolerance(const geom::Geometry& g)
{
	// If precision model is fixed, then the snap tolerance
	// must reflect the precision model.
	// Precisely, the snap tolerance should be at least
	// the distance from a corner of a precision grid cell
	// to the centre point of the cell.
	assert(g.getPrecisionModel());
	const PrecisionModel& pm = *(g.getPrecisionModel());
	if (pm.getType() == PrecisionModel::FIXED)
	{
		return (1 / pm.getScale()) * 2 / 1.415;
	}
	else
	{
		const Envelope* env = g.getEnvelopeInternal();
		double minDimension = std::min(env->getHeight(), env->getWidth());
		double snapTol = minDimension * snapPrecisionFactor;
		return snapTol;
	}
}

/*public static*/
double
GeometrySnapper::computeSnapTolerance(const geom::Geometry& g1,
		const geom::Geometry& g2)
{
        return std::min(computeSnapTolerance(g1), computeSnapTolerance(g2));
}


} // namespace geos.precision
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/07/21 17:09:14  strk
 * Added new precision::LineStringSnapper class + test
 * and precision::GeometrySnapper (w/out test)
 *
 **********************************************************************/

