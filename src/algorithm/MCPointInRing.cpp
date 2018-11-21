/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/algorithm/MCPointInRing.h>
#include <geos/algorithm/RobustDeterminant.h>
#include <geos/index/bintree/Bintree.h>
#include <geos/index/bintree/Interval.h>
#include <geos/geom/LineSegment.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/Envelope.h>
#include <geos/index/chain/MonotoneChain.h>
#include <geos/index/chain/MonotoneChainBuilder.h>

#include <vector>

using namespace std;
using namespace geos::geom;
using namespace geos::index;

namespace geos {
namespace algorithm { // geos.algorithm

MCPointInRing::MCSelecter::MCSelecter(const Coordinate& newP,
		MCPointInRing *prt)
	:
	MonotoneChainSelectAction()
{
	p=newP;
	parent=prt;
}

/* public overridden */
void
MCPointInRing::MCSelecter::select(const LineSegment& ls)
{
	parent->testLineSegment(p, ls);
}

MCPointInRing::MCPointInRing(const LinearRing *newRing)
	:
	ring(newRing),
	interval(),
	pts(nullptr),
	tree(nullptr),
	crossings(0)
{
	buildIndex();
}

MCPointInRing::~MCPointInRing() = default;

void
MCPointInRing::buildIndex()
{
	tree.reset(new bintree::Bintree());
	pts.reset(CoordinateSequence::removeRepeatedPoints(ring->getCoordinatesRO()));
	chains = chain::MonotoneChainBuilder::getChains(pts.get());

	for(const auto& mc : *chains)
	{
		const Envelope& mcEnv = mc->getEnvelope();
		interval.min = mcEnv.getMinY();
		interval.max = mcEnv.getMaxY();

		tree->insert(&interval, mc.get());
	}
}

bool
MCPointInRing::isInside(const Coordinate& pt)
{
	crossings=0;
	// test all segments intersected by ray from pt in positive x direction
	Envelope rayEnv(DoubleNegInfinity,DoubleInfinity,pt.y,pt.y);
	interval.min=pt.y;
	interval.max=pt.y;
	std::unique_ptr<vector<void*>> segs{tree->query(&interval)};

	MCSelecter mcSelecter(pt,this);
	for(auto& seg : *segs) {
		chain::MonotoneChain *mc= reinterpret_cast<chain::MonotoneChain*>(seg);
		testMonotoneChain(&rayEnv,&mcSelecter,mc);
	}

	/*
	 *  p is inside if number of crossings is odd.
	 */
	return crossings % 2 == 1;
}


void
MCPointInRing::testMonotoneChain(Envelope *rayEnv,
		MCSelecter *mcSelecter,
		chain::MonotoneChain *mc)
{
	mc->select(*rayEnv, *mcSelecter);
}

void
MCPointInRing::testLineSegment(const Coordinate& p, const LineSegment& seg)
{
	double xInt;  // x intersection of segment with ray
	double x1;    // translated coordinates
	double y1;
	double x2;
	double y2;

	/*
	 * Test if segment crosses ray from test point in positive x direction.
	 */
	const Coordinate& p1 = seg.p0;
	const Coordinate& p2 = seg.p1;
	x1 = p1.x - p.x;
	y1 = p1.y - p.y;
	x2 = p2.x - p.x;
	y2 = p2.y - p.y;

	if (((y1>0)&&(y2<=0)) || ((y2>0)&&(y1<=0)))
	{

		/*
		 *  segment straddles x axis, so compute intersection.
		 */
		xInt=RobustDeterminant::signOfDet2x2(x1,y1,x2,y2)/(y2-y1);

		/*
		 *  crosses ray if strictly positive intersection.
		 */
		if (0.0<xInt) {
			crossings++;
		}
	}
}

} // namespace geos.algorithm
} // namespace geos

