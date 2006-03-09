/**********************************************************************
 * $Id$
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

#ifndef GEOS_OPERATION_GEOMETRYGRAPHOPERATION_H
#define GEOS_OPERATION_GEOMETRYGRAPHOPERATION_H

#include <vector>

#include <geos/algorithm/LineIntersector.h> // for composition

// Forward declarations
namespace geos {
	namespace geom {
		class Geometry;
		class PrecisionModel;
	}
	namespace geomgraph {
		class GeometryGraph;
	}
}


namespace geos {
namespace operation { // geos.operation

/// The base class for operations that require GeometryGraph
class GeometryGraphOperation {
friend class Unload;
public:
	GeometryGraphOperation(const geom::Geometry *g0,const geom::Geometry *g1);
	GeometryGraphOperation(const geom::Geometry *g0);
	virtual ~GeometryGraphOperation();
	const geom::Geometry* getArgGeometry(int i) const;

protected:

	algorithm::LineIntersector li;

	const geom::PrecisionModel* resultPrecisionModel;

	/*
	 * The operation args into an array so they can be accessed by index
	 */
	std::vector<geomgraph::GeometryGraph*> arg;  // the arg(s) of the operation

	void setComputationPrecision(const geom::PrecisionModel* pm);
};

} // namespace geos.operation
} // namespace geos

#endif

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/

