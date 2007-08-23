/**********************************************************************
 * $Id: OffsetCurveBuilder.h 1820 2006-09-06 16:54:23Z mloskot $
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2007 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: operation/buffer/OffsetCurveVertexList.java rev. 1.1
 *
 **********************************************************************/

#ifndef GEOS_OP_BUFFER_OFFSETCURVEVERTEXLIST_H
#define GEOS_OP_BUFFER_OFFSETCURVEVERTEXLIST_H

#include <geos/geom/Coordinate.h> // for inlines
#include <geos/geom/CoordinateSequence.h> // for inlines
#include <geos/geom/CoordinateArraySequence.h> // for composition
#include <geos/geom/PrecisionModel.h> // for inlines

#include <vector>
#include <memory>

// Forward declarations
namespace geos {
	namespace geom {
		//class CoordinateSequence;
		//class PrecisionModel;
	}
}

namespace geos {
namespace operation { // geos.operation
namespace buffer { // geos.operation.buffer

// ---------------------------------------------
// OffsetCurveVertexList
// ---------------------------------------------

/// A list of the vertices in a constructed offset curve.
//
/// Automatically removes close adjacent vertices.
///
class OffsetCurveVertexList 
{

private:

	geom::CoordinateSequence* ptList;
	bool ptListConsumed;

	const geom::PrecisionModel* precisionModel;
  
	/** \brief
	 * The distance below which two adjacent points on the curve 
	 * are considered to be coincident.
	 *
	 * This is chosen to be a small fraction of the offset distance.
	 */
	double minimumVertexDistance;

	/** \brief
	 * Tests whether the given point duplicates the previous
	 * point in the list (up to tolerance)
	 * 
	 * @param pt
	 * @return true if the point duplicates the previous point
	 */
	bool isDuplicate(const geom::Coordinate& pt)
	{
		if (ptList->size() < 1)
			return false;
		const geom::Coordinate& lastPt = ptList->back();
		double ptDist = pt.distance(lastPt);
		if (ptDist < minimumVertexDistance)
			return true;
		return false;
	}
	

public:
	
	OffsetCurveVertexList()
		:
		ptList(new geom::CoordinateArraySequence()),
		ptListConsumed(false),
		precisionModel(NULL),
		minimumVertexDistance (0.0)
	{
	}

	~OffsetCurveVertexList()
	{
		if (!ptListConsumed)
			delete ptList;
	}
	
	void setPrecisionModel(const geom::PrecisionModel* nPrecisionModel)
	{
		precisionModel = nPrecisionModel;
	}
	
	void setMinimumVertexDistance(double nMinVertexDistance)
	{
		minimumVertexDistance = nMinVertexDistance;
	}
	
	void addPt(const geom::Coordinate& pt)
	{
		assert(precisionModel);

		geom::Coordinate bufPt = pt;
		precisionModel->makePrecise(bufPt);
		// don't add duplicate (or near-duplicate) points
		if (isDuplicate(bufPt))
		{
			return;
		}
		// we ask to allow repeated as we checked this ourself
		// (JTS uses a vector for ptList, not a CoordinateSequence,
		// we should do the same)
		ptList->add(bufPt, true);
		//ptList.push_back(bufPt);
	}
	
	// check that points are a ring - add the startpoint again if they are not
	void closeRing()
	{
		if (ptList->size() < 1) return;
		const geom::Coordinate& startPt = ptList->front();
		const geom::Coordinate& lastPt = ptList->back();
		if (startPt.equals(lastPt)) return;
		// we ask to allow repeated as we checked this ourself
		ptList->add(startPt, true);
	}

	geom::CoordinateSequence* getCoordinates()
	{
		closeRing();
		ptListConsumed = true;
		return ptList;
	}

};

} // namespace geos.operation.buffer
} // namespace geos.operation
} // namespace geos


#endif // ndef GEOS_OP_BUFFER_OFFSETCURVEVERTEXLIST_H

/**********************************************************************
 * $Log$
 **********************************************************************/

