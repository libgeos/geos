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

#ifndef GEOS_IO_WKBWRITER_H
#define GEOS_IO_WKBWRITER_H

#include <geos/platform.h> // for getMachineByteOrder
#include <iosfwd>

// Forward declarations
namespace geos {
	namespace geom {

		class CoordinateSequence;
		class Geometry;
		class GeometryCollection;
		class Point;
		class LineString;
		class LinearRing;
		class Polygon;
		class MultiPoint;
		class MultiLineString;
		class MultiPolygon;
		class PrecisionModel;

	} // namespace geom
} // namespace geos

namespace geos {
namespace io {

/**
 *
 * \class WKBWriter io.h geos.h
 *
 * \brief Writes a Geometry into Well-Known Binary format.
 *
 * The WKB format is specified in the OGC Simple Features for SQL specification.
 * This implementation supports the extended WKB standard for representing
 * 3-dimensional coordinates.  The presence of 3D coordinates is signified
 * by setting the high bit of the wkbType word.
 *
 * Empty Points cannot be represented in WKB; an
 * IllegalArgumentException will be thrown if one is
 * written. The WKB specification does not support representing LinearRing
 * they will be written as LineString
 * 
 * This class is designed to support reuse of a single instance to read multiple
 * geometries. This class is not thread-safe; each thread should create its own
 * instance.
 *
 * @see WKBReader
 */
class WKBWriter {

public:

	WKBWriter(int dims=2, int bo=getMachineByteOrder());

	/**
	 * \brief Write a Geometry to an ostream.
	 *
	 * @param g the geometry to write
	 * @param os the output stream
	 * @throws IOException
	 */
	void write(const geom::Geometry &g, std::ostream &os);
		// throws IOException, ParseException

	/**
	 * \brief Write a Geometry to an ostream in binary hex format.
	 *
	 * @param g the geometry to write
	 * @param os the output stream
	 * @throws IOException
	 */
	void writeHEX(const geom::Geometry &g, std::ostream &os);
		// throws IOException, ParseException

private:

	int outputDimension;

	int byteOrder;

	std::ostream *outStream;

	unsigned char buf[8];

	void writePoint(const geom::Point &p);
		// throws IOException

	void writeLineString(const geom::LineString &ls);
		// throws IOException

	void writePolygon(const geom::Polygon &p);
		// throws IOException

	void writeGeometryCollection(const geom::GeometryCollection &c, int wkbtype);
		// throws IOException, ParseException

	void writeCoordinateSequence(const geom::CoordinateSequence &cs, bool sized);
		// throws IOException

	void writeCoordinate(const geom::CoordinateSequence &cs, int idx, bool is3d);
		// throws IOException

	void writeGeometryType(int geometryType);
		// throws IOException

	void writeByteOrder();
		// throws IOException

	void writeInt(int intValue);
		// throws IOException

};

} // namespace io
} // namespace geos

#endif // #ifndef GEOS_IO_WKBWRITER_H

/**********************************************************************
 * $Log$
 * Revision 1.2  2006/03/28 11:26:13  strk
 * ByteOrderDataInStream inlines moved to .inl file, updated
 * implementation files includes.
 *
 * Revision 1.1  2006/03/20 18:18:14  strk
 * io.h header split
 *
 **********************************************************************/
