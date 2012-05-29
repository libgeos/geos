/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: simplify/DouglasPeuckerSimplifier.java rev. 1.5 (JTS-1.7)
 *
 **********************************************************************/

#include <geos/simplify/DouglasPeuckerSimplifier.h>
#include <geos/simplify/DouglasPeuckerLineSimplifier.h>
#include <geos/geom/Geometry.h> // for AutoPtr typedefs
#include <geos/geom/MultiPolygon.h> 
#include <geos/geom/CoordinateSequence.h> // for AutoPtr typedefs
#include <geos/geom/GeometryFactory.h> 
#include <geos/geom/CoordinateSequenceFactory.h> 
#include <geos/geom/util/GeometryTransformer.h> // for DPTransformer inheritance
#include <geos/util/IllegalArgumentException.h>
#include <geos/util.h>

#include <memory> // for auto_ptr
#include <cassert>

#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif

#ifdef GEOS_DEBUG
#include <iostream>
#endif

using namespace geos::geom;

namespace geos {
namespace simplify { // geos::simplify

class DPTransformer: public geom::util::GeometryTransformer {

public:

	DPTransformer(double tolerance);

protected:

	CoordinateSequence::AutoPtr transformCoordinates(
			const CoordinateSequence* coords,
			const Geometry* parent);

	Geometry::AutoPtr transformPolygon(
			const Polygon* geom,
			const Geometry* parent);

	Geometry::AutoPtr transformMultiPolygon(
			const MultiPolygon* geom,
			const Geometry* parent);

private:

	/*
	 * Creates a valid area geometry from one that possibly has
	 * bad topology (i.e. self-intersections).
	 * Since buffer can handle invalid topology, but always returns
	 * valid geometry, constructing a 0-width buffer "corrects" the
	 * topology.
	 * Note this only works for area geometries, since buffer always returns
	 * areas.  This also may return empty geometries, if the input
	 * has no actual area.
	 *
	 * @param roughAreaGeom an area geometry possibly containing
	 *        self-intersections
	 * @return a valid area geometry
	 */
	Geometry::AutoPtr createValidArea(const Geometry* roughAreaGeom);

	double distanceTolerance;

};

DPTransformer::DPTransformer(double t)
	:
	distanceTolerance(t)
{
}

Geometry::AutoPtr
DPTransformer::createValidArea(const Geometry* roughAreaGeom)
{
	return Geometry::AutoPtr(roughAreaGeom->buffer(0.0));
}

CoordinateSequence::AutoPtr
DPTransformer::transformCoordinates(
		const CoordinateSequence* coords,
		const Geometry* parent)
{
    ::geos::ignore_unused_variable_warning(parent);

	const Coordinate::Vect* inputPts = coords->toVector();
	assert(inputPts);

	std::auto_ptr<Coordinate::Vect> newPts =
			DouglasPeuckerLineSimplifier::simplify(*inputPts,
				distanceTolerance);

	return CoordinateSequence::AutoPtr(
		factory->getCoordinateSequenceFactory()->create(
			newPts.release()
		));
}

Geometry::AutoPtr
DPTransformer::transformPolygon(
		const Polygon* geom,
		const Geometry* parent)
{

#if GEOS_DEBUG
	std::cerr << "DPTransformer::transformPolygon(Polygon " << geom << ", Geometry " << parent << ");" << std::endl;
#endif

	Geometry::AutoPtr roughGeom(GeometryTransformer::transformPolygon(geom, parent));

        // don't try and correct if the parent is going to do this
	if ( dynamic_cast<const MultiPolygon*>(parent) )
	{
		return roughGeom;
	}

	return createValidArea(roughGeom.get());
}

Geometry::AutoPtr
DPTransformer::transformMultiPolygon(
		const MultiPolygon* geom,
		const Geometry* parent)
{
#if GEOS_DEBUG
	std::cerr << "DPTransformer::transformMultiPolygon(MultiPolygon " << geom << ", Geometry " << parent << ");" << std::endl;
#endif
	Geometry::AutoPtr roughGeom(GeometryTransformer::transformMultiPolygon(geom, parent));
        return createValidArea(roughGeom.get());
}

/************************************************************************/



//DouglasPeuckerSimplifier::

/*public static*/
Geometry::AutoPtr
DouglasPeuckerSimplifier::simplify(const Geometry* geom,
		double tolerance)
{
	DouglasPeuckerSimplifier tss(geom);
	tss.setDistanceTolerance(tolerance);
	return tss.getResultGeometry();
}

/*public*/
DouglasPeuckerSimplifier::DouglasPeuckerSimplifier(const Geometry* geom)
	:
	inputGeom(geom)
{
}

/*public*/
void
DouglasPeuckerSimplifier::setDistanceTolerance(double tol)
{
	if (tol < 0.0)
		throw util::IllegalArgumentException("Tolerance must be non-negative");
	distanceTolerance = tol;
}

Geometry::AutoPtr
DouglasPeuckerSimplifier::getResultGeometry()
{
	DPTransformer t(distanceTolerance);
	return t.transform(inputGeom);

}

} // namespace geos::simplify
} // namespace geos
