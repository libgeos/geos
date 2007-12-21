/**********************************************************************
 * $Id
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
 *
 **********************************************************************/

#ifndef GEOS_ALGORITHM_LOCATE_INDEXEDPOINTINAREALOCATOR_H
#define GEOS_ALGORITHM_LOCATE_INDEXEDPOINTINAREALOCATOR_H

#include <geos/algorithm/locate/PointOnGeometryLocator.h> // inherited
#include <geos/index/ItemVisitor.h> // inherited

namespace geos {
	namespace algorithm {
		class RayCrossingCounter;
	}
	namespace geom {
		class Geometry;
		class Coordinate; 
		class CoordinateSequence; 
	}
	namespace index {
		namespace intervalrtree {
			class SortedPackedIntervalRTree;
		}
	}
}

namespace geos {
namespace algorithm { // geos::algorithm
namespace locate { // geos::algorithm::locate

class IndexedPointInAreaLocator : public PointOnGeometryLocator 
{
private:
	class IntervalIndexedGeometry
	{
	private:
		index::intervalrtree::SortedPackedIntervalRTree * index;

		void init( const geom::Geometry & g);
		void addLine( geom::CoordinateSequence * pts);

	public:
		IntervalIndexedGeometry( const geom::Geometry & g);
		~IntervalIndexedGeometry();

		void query(double min, double max, index::ItemVisitor * visitor);
	};


	class SegmentVisitor : public index::ItemVisitor
	{
	private:
		algorithm::RayCrossingCounter * counter;

	public:
		SegmentVisitor( algorithm::RayCrossingCounter * counter) 
		:	counter( counter)
		{ }
		
		~SegmentVisitor() 
		{ }

		void visitItem( void * item);
	};


	const geom::Geometry & areaGeom;
	IntervalIndexedGeometry * index;

	void buildIndex( const geom::Geometry & g);


protected:
public:
	/**
	 * Creates a new locator for a given {@link Geometry}
	 * @param g the Geometry to locate in
	 */
	IndexedPointInAreaLocator( const geom::Geometry & g);

	~IndexedPointInAreaLocator();
    
	/**
	 * Determines the {@link Location} of a point in an areal {@link Geometry}.
	 * 
	 * @param p the point to test
	 * @return the location of the point in the geometry  
	 */
	int locate( const geom::Coordinate * /*const*/ p);
    
};

} // geos::algorithm::locate
} // geos::algorithm
} // geos

#endif // GEOS_ALGORITHM_LOCATE_INDEXEDPOINTINAREALOCATOR_H
/**********************************************************************
 * $Log$
 **********************************************************************/

