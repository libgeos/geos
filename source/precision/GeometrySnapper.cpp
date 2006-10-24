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
 * Last port: precision/GeometrySnapper.java rev. 0
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

//#include <geos/geom/util/GeometryEditor.h>
//#include <geos/geom/util/CoordinateOperation.h>
//#include <geos/geom/PrecisionModel.h>
//#include <geos/geom/LineString.h>
//#include <geos/geom/LinearRing.h>

#include <vector>
#include <memory>
#include <algorithm>

//using namespace std;
using namespace geos::geom;

namespace geos {
namespace precision { // geos.precision

//const double GeometrySnapper::snapTol = 0.000001;
const double GeometrySnapper::snapPrecisionFactor = 10e-10;

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
	//return 0.000001; // testing: to compare with previous implementation
	const Envelope* env = g.getEnvelopeInternal();
        double minDimension = std::min(env->getHeight(), env->getWidth());
	double snapTol = minDimension * snapPrecisionFactor;
	return snapTol;
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

