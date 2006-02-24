/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#ifndef GEOS_UTIL_H
#define GEOS_UTIL_H

#include <memory>
#include <string>
#include <geos/platform.h>
#include <geos/geom.h>
#include <vector>

using namespace std;

namespace geos {

/**
 * \class GEOSException util.h geos.h
 *
 * \brief Base class for all GEOS exceptions.
 *
 * Exceptions are thrown as pointers to this type.
 * Use toString() to get a readable message.
 */
class GEOSException: public std::exception {

protected:
	string txt;
	string name;

public:
	virtual void setName(const string& nname) { name=nname; }
	virtual void setMessage(const string& msg) { txt=msg; }

	GEOSException()
	{
		setName("GEOSException");
		setMessage("unknown error");
	}

	GEOSException(const string& msg)
	{
		setName("GEOSException");
		setMessage(msg);
	}

	/// Create an exception of given type containing given message 
	GEOSException(const string& nname, const string& msg)
	{
		setName(nname);
		setMessage(msg);
	}

	virtual ~GEOSException() throw() {}

	/// Returns exception message
	virtual string toString() const;

	/// Implement std::exception.what()
	virtual const char* what() const throw() {
		return toString().c_str();
	}

};

/** \class AssertionFailedException util.h geos.h
 * \brief Indicates a bug in GEOS code.
 */
class AssertionFailedException: public GEOSException {

public:

	AssertionFailedException()
		:
		GEOSException("AssertionFailedException", "")
	{}

	AssertionFailedException(const string& msg)
		:
		GEOSException("AssertionFailedException", msg)
	{}

	~AssertionFailedException() throw() {}
};

/** \class IllegalArgumentException util.h geos.h
 * \brief Indicates one or more legal arguments.
 *
 * This exception is thrown - for example - when
 * trying to apply set-theoretic methods to a
 * GeometryCollection object.
 */
class IllegalArgumentException: public GEOSException {
public:
	IllegalArgumentException()
		:
		GEOSException("IllegalArgumentException", "")
	{}

	IllegalArgumentException(const string& msg)
		:
		GEOSException("IllegalArgumentException", msg)
	{}

	~IllegalArgumentException() throw() {};
};

/**
 * \class TopologyException util.h geos.h
 *
 * \brief
 * Indicates an invalid or inconsistent topological situation encountered
 * during processing
 */
class TopologyException: public GEOSException {
public:
	TopologyException()
		:
		GEOSException("TopologyException", "")
	{}

	TopologyException(const string& msg)
		:
		GEOSException("TopologyException", msg)
	{}

	TopologyException(const string& msg, const Coordinate *newPt)
		:
		GEOSException("TopologyException", msg+" "+newPt->toString()),
		pt(*newPt)
	{}

	~TopologyException() throw() {}
	Coordinate* getCoordinate() { return &pt; }
private:
	Coordinate pt;
};

/**
 * \class UnsupportedOperationException util.h geos.h
 *
 * \brief Indicates that the requested operation is unsopported.
 *
 * This exception is thrown - for example - when requesting the
 * X or Y member of an empty Point
 */
class UnsupportedOperationException: public GEOSException {
public:
	UnsupportedOperationException()
		:
		GEOSException("UnsupportedOperationException", "")
	{}

	UnsupportedOperationException(const string& msg)
		:
		GEOSException("UnsupportedOperationException", msg)
	{}

	~UnsupportedOperationException() throw() {};
};

class Coordinate;
class Assert {
public:

	static void isTrue(bool assertion, const string& message);

	static void isTrue(bool assertion) {
		isTrue(assertion, string());
	}


	static void equals(const Coordinate& expectedValue,
			const Coordinate& actualValue,
			const string& message);

	static void equals(const Coordinate& expectedValue,
			const Coordinate& actualValue)
	{
		equals(expectedValue, actualValue, string());
	}


	static void shouldNeverReachHere(const string& message);

	static void shouldNeverReachHere() { shouldNeverReachHere(string()); }
};

/**
 * A CoordinateFilter that adds read-only pointers
 * to every Coordinate in a Geometry to a given
 * vector.
 *
 * Last port: util/CoordinateArrayFilter.java rev. 1.15
 */
class CoordinateArrayFilter:public CoordinateFilter {
private:
	Coordinate::ConstVect &pts; // target vector reference
public:
	/**
	 * Constructs a CoordinateArrayFilter.
	 *
	 * @param  target   The destination vector. 
	 */
	CoordinateArrayFilter(Coordinate::ConstVect& target)
		:
		pts(target)
		{}

	virtual ~CoordinateArrayFilter() {}

	virtual void filter_ro(const Coordinate *coord)
	{
		pts.push_back(coord);
	}

	virtual void filter_rw(Coordinate *coord) const
	{
		// Unsupported
		throw  UnsupportedOperationException("CoordinateArrayFilter"
			" is a read-only filter");
	}
};

/*
 *  A CoordinateFilter that fills a vector of Coordinate const pointers.
 *  The set of coordinates contains no duplicate points.
 *
 *  Last port: util/UniqueCoordinateArrayFilter.java rev. 1.17
 */
class UniqueCoordinateArrayFilter:public CoordinateFilter {

private:
	Coordinate::ConstVect &pts;	// target set reference
	Coordinate::ConstSet uniqPts; 	// unique points set

public:
	/**
	 * Constructs a CoordinateArrayFilter.
	 *
	 * @param  target   The destination set. 
	 */
	UniqueCoordinateArrayFilter(Coordinate::ConstVect &target)
		:
		pts(target)
		{}

	virtual ~UniqueCoordinateArrayFilter() {}

	virtual void filter_ro(const Coordinate *coord)
	{
		if ( uniqPts.insert(coord).second )
			pts.push_back(coord);
	}

	virtual void filter_rw(Coordinate *coord) const
	{
		 // Unsupported
		throw  UnsupportedOperationException(
			"UniqueCoordinateArrayFilter is a read-only filter");
	}
};


/**
 * \class GeometricShapeFactory util.h geos.h
 *
 * \brief
 * Computes various kinds of common geometric shapes.
 * Allows various ways of specifying the location and extent of the shapes,
 * as well as number of line segments used to form them.
 *
 */
class GeometricShapeFactory {
private:
	class Dimensions {
	public:
		Dimensions();
		Coordinate base;
		Coordinate centre;
		double width;
		double height;
		void setBase(const Coordinate& newBase);
		void setCentre(const Coordinate& newCentre);
		void setSize(double size);
		void setWidth(double nWidth);
		void setHeight(double nHeight);
		Envelope* getEnvelope();
	};
	const GeometryFactory* geomFact;
	Dimensions dim;
	int nPts;
public:
	/**
	* \brief
	* Create a shape factory which will create shapes using the given
	* GeometryFactory.
	*
	* @param factory the factory to use. You need to keep the
	*	factory alive for the whole GeometricShapeFactory
	*	life time.
	* 
	*/
	GeometricShapeFactory(const GeometryFactory *factory);

	~GeometricShapeFactory();

	/**
	* \brief Creates a elliptical arc, as a LineString.
	*
	* @return an elliptical arc
	*/
	LineString* createArc(double startAng,double endAng);

	/**
	* \brief Creates a circular Polygon.
	*
	* @return a circle
	*/
	Polygon* createCircle();

	/**
	* \brief Creates a rectangular Polygon.
	*
	* @return a rectangular Polygon
	*/
	Polygon* createRectangle();

	/**
	* \brief
	* Sets the location of the shape by specifying the base coordinate
	* (which in most cases is the * lower left point of the envelope
	* containing the shape).
	*
	* @param base the base coordinate of the shape
	*/
	void setBase(const Coordinate& base);

	/**
	* \brief
	* Sets the location of the shape by specifying the centre of
	* the shape's bounding box
	*
	* @param centre the centre coordinate of the shape
	*/
	void setCentre(const Coordinate& centre);

	/**
	* \brief Sets the height of the shape.
	*
	* @param height the height of the shape
	*/
	void setHeight(double height);

	/**
	* \brief Sets the total number of points in the created Geometry
	*/
	void setNumPoints(int nNPts);

	/**
	* \brief
	* Sets the size of the extent of the shape in both x and y directions.
	*
	* @param size the size of the shape's extent
	*/
	void setSize(double size);

	/**
	* \brief Sets the width of the shape.
	*
	* @param width the width of the shape
	*/
	void setWidth(double width);

};

namespace util { // geos.util

 	/// Symmetric Rounding Algorithm
	double sym_round(double val);

} // namespace geos.util

} // namespace geos

#endif // GEOS_UTIL_H

/**********************************************************************
 * $Log$
 * Revision 1.14  2006/02/24 16:20:15  strk
 * Added Mateusz implementation of round() in a new math.cpp file
 * named sym_round(). Changed use of rint_vc to sym_round in PrecisionModel.
 * Moved rint_vc to math.cpp (geos::util namespace), to be renamed
 * to something more meaningful
 *
 * Revision 1.13  2006/02/23 11:54:20  strk
 * - MCIndexPointSnapper
 * - MCIndexSnapRounder
 * - SnapRounding BufferOp
 * - ScaledNoder
 * - GEOSException hierarchy cleanups
 * - SpatialIndex memory-friendly query interface
 * - GeometryGraph::getBoundaryNodes memory-friendly
 * - NodeMap::getBoundaryNodes memory-friendly
 * - Cleanups in geomgraph::Edge
 * - Added an XML test for snaprounding buffer (shows leaks, working on it)
 *
 * Revision 1.12  2006/02/09 15:52:47  strk
 * GEOSException derived from std::exception; always thrown and cought by const ref.
 *
 * Revision 1.11  2006/01/31 19:07:34  strk
 * - Renamed DefaultCoordinateSequence to CoordinateArraySequence.
 * - Moved GetNumGeometries() and GetGeometryN() interfaces
 *   from GeometryCollection to Geometry class.
 * - Added getAt(int pos, Coordinate &to) funtion to CoordinateSequence class.
 * - Reworked automake scripts to produce a static lib for each subdir and
 *   then link all subsystem's libs togheter
 * - Moved C-API in it's own top-level dir capi/
 * - Moved source/bigtest and source/test to tests/bigtest and test/xmltester
 * - Fixed PointLocator handling of LinearRings
 * - Changed CoordinateArrayFilter to reduce memory copies
 * - Changed UniqueCoordinateArrayFilter to reduce memory copies
 * - Added CGAlgorithms::isPointInRing() version working with
 *   Coordinate::ConstVect type (faster!)
 * - Ported JTS-1.7 version of ConvexHull with big attention to
 *   memory usage optimizations.
 * - Improved XMLTester output and user interface
 * - geos::geom::util namespace used for geom/util stuff
 * - Improved memory use in geos::geom::util::PolygonExtractor
 * - New ShortCircuitedGeometryVisitor class
 * - New operation/predicate package
 *
 * Revision 1.10  2005/12/08 14:14:07  strk
 * ElevationMatrixFilter used for both elevation and Matrix fill,
 * thus removing CoordinateSequence copy in ElevetaionMatrix::add(Geometry *).
 * Changed CoordinateFilter::filter_rw to be a const method: updated
 * all apply_rw() methods to take a const CoordinateFilter.
 *
 * Revision 1.9  2005/11/10 10:24:08  strk
 * Fixed virtual overload of CoordinateArrayFilter::filter*
 *
 * Revision 1.8  2004/11/01 16:43:04  strk
 * Added Profiler code.
 * Temporarly patched a bug in DoubleBits (must check drawbacks).
 * Various cleanups and speedups.
 *
 * Revision 1.7  2004/07/16 10:28:41  strk
 * Dimesions object allocated on the heap
 *
 * Revision 1.6  2004/07/14 21:19:35  strk
 * GeometricShapeFactory first pass of bug fixes
 *
 * Revision 1.5  2004/07/08 19:34:49  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added DefaultCoordinateSequenceFactory::instance() function.
 *
 * Revision 1.4  2004/07/07 10:29:54  strk
 * Adjusted exceptions documentation.
 *
 * Revision 1.3  2004/07/05 14:23:03  strk
 * More documentation cleanups.
 *
 * Revision 1.2  2004/07/05 10:50:21  strk
 * deep-dopy construction taken out of Geometry and implemented only
 * in GeometryFactory.
 * Deep-copy geometry construction takes care of cleaning up copies
 * on exception.
 * Implemented clone() method for CoordinateSequence
 * Changed createMultiPoint(CoordinateSequence) signature to reflect
 * copy semantic (by-ref instead of by-pointer).
 * Cleaned up documentation.
 *
 * Revision 1.1  2004/07/02 13:20:42  strk
 * Header files moved under geos/ dir.
 *
 * Revision 1.14  2004/04/10 22:41:25  ybychkov
 * "precision" upgraded to JTS 1.4
 *
 * Revision 1.13  2004/03/18 10:42:44  ybychkov
 * "IO" and "Util" upgraded to JTS 1.4
 * "Geometry" partially upgraded.
 *
 * Revision 1.12  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/

