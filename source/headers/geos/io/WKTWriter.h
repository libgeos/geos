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

#ifndef GEOS_IO_WKTWRITER_H
#define GEOS_IO_WKTWRITER_H

// Forward declarations
namespace geos {
	namespace geom {

		class Coordinate;
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

	namespace io {

		class Writer;

	} // namespace io

} // namespace geos


namespace geos {
namespace io {

/**
 * \class WKTWriter io.h geos.h
 *
 * \brief Outputs the textual representation of a Geometry.
 * See also WKTReader.
 * 
 * The WKTWriter outputs coordinates rounded to the precision
 * model. No more than the maximum number of necessary decimal places will be
 * output.
 * 
 * The Well-known Text format is defined in the <A
 * HREF="http://www.opengis.org/techno/specs.htm">OpenGIS Simple Features
 * Specification for SQL</A>.
 * 
 * A non-standard "LINEARRING" tag is used for LinearRings. The WKT spec does
 * not define a special tag for LinearRings. The standard tag to use is
 * "LINESTRING".
 *
 * See WKTReader for parsing.
 *
 */
class WKTWriter {
public:
	WKTWriter();
	~WKTWriter();

	//string(count, ch) can be used for this
	//static string stringOfChar(char ch, int count);

	/// Returns WKT string for the given Geometry
	std::string write(const geom::Geometry *geometry);

	// Send Geometry's WKT to the given Writer
	void write(const geom::Geometry *geometry, Writer *writer);

	std::string writeFormatted(const geom::Geometry *geometry);

	void writeFormatted(const geom::Geometry *geometry, Writer *writer);

	/**
	 * Generates the WKT for a N-point <code>LineString</code>.
	 *
	 * @param seq the sequence to outpout
	 *
	 * @return the WKT
	 */
	static std::string toLineString(const geom::CoordinateSequence& seq); 

	/**
	 * Generates the WKT for a 2-point <code>LineString</code>.
	 *
	 * @param p0 the first coordinate
	 * @param p1 the second coordinate
	 *
	 * @return the WKT
	 */
	static std::string toLineString(const geom::Coordinate& p0, const geom::Coordinate& p1);

	/**
	 * Generates the WKT for a <code>Point</code>.
	 *
	 * @param p0 the point coordinate
	 *
	 * @return the WKT
	 */
	static std::string toPoint(const geom::Coordinate& p0);
 

protected:
	std::string formatter;
	void appendGeometryTaggedText(const geom::Geometry *geometry, int level, Writer *writer);
	void appendPointTaggedText(const geom::Coordinate* coordinate, int level, Writer *writer, const geom::PrecisionModel* precisionModel);
	void appendLineStringTaggedText(const geom::LineString *lineString, int level, Writer *writer);
	void appendLinearRingTaggedText(const geom::LinearRing *lineString, int level, Writer *writer);
	void appendPolygonTaggedText(const geom::Polygon *polygon, int level, Writer *writer);
	void appendMultiPointTaggedText(const geom::MultiPoint *multipoint, int level, Writer *writer);
	void appendMultiLineStringTaggedText(const geom::MultiLineString *multiLineString, int level,Writer *writer);
	void appendMultiPolygonTaggedText(const geom::MultiPolygon *multiPolygon, int level, Writer *writer);
	void appendGeometryCollectionTaggedText(const geom::GeometryCollection *geometryCollection, int level,Writer *writer);
	void appendPointText(const geom::Coordinate* coordinate, int level, Writer *writer, const geom::PrecisionModel* precisionModel);
	void appendCoordinate(const geom::Coordinate* coordinate, Writer *writer, const geom::PrecisionModel* precisionModel);
	std::string writeNumber(double d);
	void appendLineStringText(const geom::LineString *lineString, int level, bool doIndent, Writer *writer);
	void appendPolygonText(const geom::Polygon *polygon, int level, bool indentFirst, Writer *writer);
	void appendMultiPointText(const geom::MultiPoint *multiPoint, int level, Writer *writer);
	void appendMultiLineStringText(const geom::MultiLineString *multiLineString, int level, bool indentFirst,Writer *writer);
	void appendMultiPolygonText(const geom::MultiPolygon *multiPolygon, int level, Writer *writer);
	void appendGeometryCollectionText(const geom::GeometryCollection *geometryCollection, int level,Writer *writer);
private:
	enum {
		INDENT = 2
	};
//	static const int INDENT = 2;
	static std::string createFormatter(const geom::PrecisionModel* precisionModel);
	bool isFormatted;
	int level;
	void writeFormatted(const geom::Geometry *geometry, bool isFormatted, Writer *writer);
	void indent(int level, Writer *writer);
};

} // namespace geos::io
} // namespace geos

#endif // #ifndef GEOS_IO_WKTWRITER_H

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/03/20 18:18:15  strk
 * io.h header split
 *
 **********************************************************************/
