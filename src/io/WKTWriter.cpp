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
 **********************************************************************
 *
 * Last port: io/WKTWriter.java rev. 1.34 (JTS-1.7)
 *
 **********************************************************************/

#include <geos/io/WKTWriter.h>
#include <geos/io/Writer.h>
#include <geos/io/CLocalizer.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/Point.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/LineString.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/MultiPoint.h>
#include <geos/geom/MultiLineString.h>
#include <geos/geom/MultiPolygon.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/util/IllegalArgumentException.h>

#include <typeinfo>
#include <cstdio> // should avoid this
#include <string>
#include <sstream>
#include <cassert>
#include <cmath>

using namespace std;
using namespace geos::geom;

namespace geos {
namespace io { // geos.io

WKTWriter::WKTWriter():
	formatter("%f"),
	isFormatted(false),
	roundingPrecision(-1),
	trim(false),
	level(0),
	defaultOutputDimension(2),
	old3D(false)
{
}

/* public */
void
WKTWriter::setOutputDimension(int dims)
{
	if ( dims < 2 || dims > 3 )
		throw util::IllegalArgumentException("WKT output dimension must be 2 or 3");
	defaultOutputDimension = dims;
}

WKTWriter::~WKTWriter() {}

/*static*/
string
WKTWriter::toLineString(const CoordinateSequence& seq)
{
	stringstream buf(ios_base::in|ios_base::out);
    buf << "LINESTRING ";
	unsigned int npts = seq.getSize();
	if ( npts == 0 )
	{
		buf << "EMPTY";
	}
	else
	{
		buf << "(";
		for (unsigned int i=0; i<npts; ++i)
		{
			if (i) buf << ", ";
			buf << seq.getX(i) << " " << seq.getY(i);
#if PRINT_Z
			buf << seq.getZ(i);
#endif
		}
		buf << ")";
	}

	return buf.str();
}

/*static*/
string
WKTWriter::toLineString(const Coordinate& p0, const Coordinate& p1)
{
	stringstream ret(ios_base::in|ios_base::out);
	ret << "LINESTRING (" << p0.x << " " << p0.y;
#if PRINT_Z
	ret << " " << p0.z;
#endif
	ret << ", " << p1.x << " " << p1.y;
#if PRINT_Z
	ret << " " << p1.z;
#endif
    ret << ")";

	return ret.str();
}

/*static*/
string
WKTWriter::toPoint(const Coordinate& p0)
{
	stringstream ret(ios_base::in|ios_base::out);
    ret << "POINT (";
#if PRINT_Z
	ret << p0.x << " " << p0.y  << " " << p0.z << " )";
#else
	ret << p0.x << " " << p0.y  << " )";
#endif
	return ret.str();
}

void
WKTWriter::setRoundingPrecision(int p0)
{
	if(p0 < -1) {
		p0 = -1;
	}
	roundingPrecision = p0;
}

void
WKTWriter::setTrim(bool p0)
{
	trim = p0;
}

string
WKTWriter::createFormatter(const PrecisionModel* precisionModel, int overwritePrecision)
{
	// the default number of decimal places is 16, which is sufficient
	// to accomodate the maximum precision of a double.
	// if roundingPrecision is specified (not -1) it will be used instead
	int decimalPlaces = precisionModel->getMaximumSignificantDigits();
	if(overwritePrecision != -1) {
		decimalPlaces = overwritePrecision;
	}
    std::string fmt("%.");
    char buffer[255] = { 0 };
	sprintf(buffer,"%i",decimalPlaces);
	fmt.append(buffer);
	fmt.append("f");
	return fmt;
}

//string WKTWriter::stringOfChar(char ch, int count) {
	//string str="";
	//for (int i=0;i<count;i++) str+=ch;
	//return string(count, ch);
//}

string WKTWriter::write(const Geometry *geometry) {
	Writer sw;
//	try {
		writeFormatted(geometry,false,&sw);
//	} catch (const IOException ex) {
//		Assert::shouldNeverReachHere();
//	}
	string res=sw.toString();
	return res;
}

void WKTWriter::write(const Geometry *geometry, Writer *writer) {
	writeFormatted(geometry, false, writer);
}

string WKTWriter::writeFormatted(const Geometry *geometry) {
	Writer sw;
	writeFormatted(geometry, true, &sw);
	return sw.toString();
}

void WKTWriter::writeFormatted(const Geometry *geometry, Writer *writer) {
	writeFormatted(geometry, true, writer);
}

void
WKTWriter::writeFormatted(const Geometry *geometry, bool isFormatted,
                          Writer *writer)
{
        CLocalizer clocale;
	this->isFormatted=isFormatted;
	formatter=createFormatter(geometry->getPrecisionModel(), roundingPrecision);
	appendGeometryTaggedText(geometry, 0, writer);
}

void
WKTWriter::appendGeometryTaggedText(const Geometry *geometry, int level,
		Writer *writer)
{
  outputDimension = min( defaultOutputDimension,
                         geometry->getCoordinateDimension() );

  indent(level, writer);
  if ( const Point* point = dynamic_cast<const Point*>(geometry) )
  {
    appendPointTaggedText(point->getCoordinate(),level,writer);
  }
  else if ( const LinearRing* lr =
    dynamic_cast<const LinearRing*>(geometry) )
  {
    appendLinearRingTaggedText(lr, level, writer);
  }
  else if ( const LineString* ls =
    dynamic_cast<const LineString*>(geometry) )
  {
    appendLineStringTaggedText(ls, level, writer);
  }
  else if ( const Polygon* x =
    dynamic_cast<const Polygon*>(geometry) )
  {
    appendPolygonTaggedText(x, level, writer);
  }
  else if ( const MultiPoint* x =
    dynamic_cast<const MultiPoint*>(geometry) )
  {
    appendMultiPointTaggedText(x, level, writer);
  }
  else if ( const MultiLineString* x =
    dynamic_cast<const MultiLineString*>(geometry) )
  {
    appendMultiLineStringTaggedText(x, level, writer);
  }
  else if ( const MultiPolygon* x =
    dynamic_cast<const MultiPolygon*>(geometry) )
  {
    appendMultiPolygonTaggedText(x, level, writer);
  }
  else if ( const GeometryCollection* x =
    dynamic_cast<const GeometryCollection*>(geometry) )
  {
    appendGeometryCollectionTaggedText(x, level, writer);
  }
  else
  {
    assert(0); // Unsupported Geometry implementation
  }
}

/*protected*/
void
WKTWriter::appendPointTaggedText(const Coordinate* coordinate, int level,
		Writer *writer)
{
	writer->write("POINT ");
    if( outputDimension == 3 && !old3D && coordinate != NULL )
        writer->write( "Z " );

	appendPointText(coordinate, level, writer);
}

void
WKTWriter::appendLineStringTaggedText(const LineString *lineString, int level,
		Writer *writer)
{
	writer->write("LINESTRING ");
    if( outputDimension == 3 && !old3D && !lineString->isEmpty() )
        writer->write( "Z " );

	appendLineStringText(lineString, level, false, writer);
}

/*
 * Converts a <code>LinearRing</code> to &lt;LinearRing Tagged Text&gt;
 * format, then appends it to the writer.
 *
 * @param  linearRing  the <code>LinearRing</code> to process
 * @param  writer      the output writer to append to
 */
void WKTWriter::appendLinearRingTaggedText(const LinearRing* linearRing, int level, Writer *writer) {
	writer->write("LINEARRING ");
    if( outputDimension == 3 && !old3D && !linearRing->isEmpty() )
        writer->write( "Z " );
	appendLineStringText((LineString*)linearRing, level, false, writer);
}

void WKTWriter::appendPolygonTaggedText(const Polygon *polygon, int level, Writer *writer) {
	writer->write("POLYGON ");
    if( outputDimension == 3 && !old3D && !polygon->isEmpty())
        writer->write( "Z " );
	appendPolygonText(polygon, level, false, writer);
}

void WKTWriter::appendMultiPointTaggedText(const MultiPoint *multipoint, int level, Writer *writer) {
	writer->write("MULTIPOINT ");
    if( outputDimension == 3 && !old3D && !multipoint->isEmpty() )
        writer->write( "Z " );
	appendMultiPointText(multipoint, level, writer);
}

void WKTWriter::appendMultiLineStringTaggedText(const MultiLineString *multiLineString, int level,Writer *writer) {
	writer->write("MULTILINESTRING ");
    if( outputDimension == 3 && !old3D && !multiLineString->isEmpty() )
        writer->write( "Z " );
	appendMultiLineStringText(multiLineString, level, false, writer);
}

void WKTWriter::appendMultiPolygonTaggedText(const MultiPolygon *multiPolygon, int level, Writer *writer) {
	writer->write("MULTIPOLYGON ");
    if( outputDimension == 3 && !old3D && !multiPolygon->isEmpty() )
        writer->write( "Z " );
	appendMultiPolygonText(multiPolygon, level, writer);
}

void WKTWriter::appendGeometryCollectionTaggedText(const GeometryCollection *geometryCollection, int level,Writer *writer) {
	writer->write("GEOMETRYCOLLECTION ");
    if( outputDimension == 3 && !old3D && !geometryCollection->isEmpty() )
        writer->write( "Z " );
	appendGeometryCollectionText(geometryCollection, level, writer);
}

void
WKTWriter::appendPointText(const Coordinate* coordinate, int /*level*/,
		Writer *writer)
{
	if (coordinate==NULL) {
		writer->write("EMPTY");
	} else {
		writer->write("(");
		appendCoordinate(coordinate, writer);
		writer->write(")");
	}
}

void
WKTWriter::appendCoordinate(const Coordinate* coordinate,
		Writer *writer)
{
	string out="";
	out+=writeNumber(coordinate->x);
	out+=" ";
	out+=writeNumber(coordinate->y);
    if( outputDimension == 3 )
    {
        out+=" ";
        if( ISNAN(coordinate->z) )
            out+=writeNumber(0.0);
        else
            out+=writeNumber(coordinate->z);
    }
	writer->write(out);
}

string WKTWriter::writeNumber(double d) {
    char buffer[255] = { 0 };
	sprintf(buffer,formatter.c_str(),d);
	//If we've set trim to true, do it here
	if(trim)	{
		bool roundNines = false;
		int i = 0, j;

		//count the length of the string
		while(buffer[i] != '\0') {
			i++;
		}
		//arrays go to length-1 of course
		i--;

		/*
		 * Is there a rounding error at the end?
		 */
		if(i > 6 &&
			buffer[i] != '0' &&
			buffer[i-1] == '0' &&
			buffer[i-2] == '0' &&
			buffer[i-3] == '0' &&
			buffer[i-4] == '0' &&
			buffer[i-5] == '0') {
			buffer[i--] = '\0';
		}
		/*
		 * Do we need to round 9's?
		 */
		if(i > 6 &&
			buffer[i] == '9' &&
			buffer[i-1] == '9' &&
			buffer[i-2] == '9' &&
			buffer[i-3] == '9' &&
			buffer[i-4] == '9' &&
			buffer[i-5] == '9') {
			roundNines = true;
		}

		/*
		 * Now let's format the string
		 */
		for(j = i; j >= 0; j--) {
			if(roundNines) {
				if(buffer[j] == '9') {
					buffer[j] = '\0';
				}
				else {
					buffer[j]++;
					roundNines = false;
				}
			}
			else if(buffer[j] == '0') {
				buffer[j] = '\0';
			}
			else {
				//remove period if no decimals
				if(buffer[j] == '.' && buffer[j+1] == '\0') {
					buffer[j] = '\0';
				}
				//and we're done
				break;
			}
		}
	}
	std::string out(buffer);
	out.append("");
	return out;
}

void
WKTWriter::appendLineStringText(const LineString *lineString, int level,
		bool doIndent, Writer *writer)
{
	if (lineString->isEmpty()) {
		writer->write("EMPTY");
	} else {
		if (doIndent) indent(level, writer);
		writer->write("(");
		for(size_t i=0, n=lineString->getNumPoints(); i<n; ++i)
		{
			if (i>0) {
				writer->write(", ");
				if (i%10==0) indent(level + 2, writer);
			}
			appendCoordinate(&(lineString->getCoordinateN(i)), writer);
		}
		writer->write(")");
	}
}

void
WKTWriter::appendPolygonText(const Polygon *polygon, int /*level*/,
		bool indentFirst, Writer *writer)
{
	if (polygon->isEmpty()) {
		writer->write("EMPTY");
	} else {
		if (indentFirst) indent(level, writer);
		writer->write("(");
		appendLineStringText(polygon->getExteriorRing(), level, false, writer);
		for (size_t i=0, n=polygon->getNumInteriorRing(); i<n; ++i)
		{
			writer->write(", ");
			const LineString *ls=polygon->getInteriorRingN(i);
			appendLineStringText(ls, level + 1, true, writer);
		}
		writer->write(")");
	}
}

void
WKTWriter::appendMultiPointText(const MultiPoint *multiPoint,
		int /*level*/, Writer *writer)
{
	if (multiPoint->isEmpty()) {
		writer->write("EMPTY");
	} else {
		writer->write("(");
		for (unsigned int i=0, n=multiPoint->getNumGeometries();
				i<n; i++)
		{
			if (i > 0)
			{
				writer->write(", ");
			}
			appendCoordinate(
				dynamic_cast<const Point*>(multiPoint->getGeometryN(i))->getCoordinate(),
				writer);
		}
		writer->write(")");
	}
}

void WKTWriter::appendMultiLineStringText(const MultiLineString *multiLineString, int level, bool indentFirst,
											Writer *writer) {
	if (multiLineString->isEmpty()) {
		writer->write("EMPTY");
	} else {
		int level2=level;
		bool doIndent=indentFirst;
		writer->write("(");
		for (unsigned int i=0, n=multiLineString->getNumGeometries();
				i<n; i++)
		{
			if (i>0) {
				writer->write(", ");
				level2=level+1;
				doIndent=true;
			}
			const LineString* ls = dynamic_cast<const LineString *>(
			  multiLineString->getGeometryN(i)
			);
			appendLineStringText(ls, level2, doIndent, writer);
		}
		writer->write(")");
	}
}

void WKTWriter::appendMultiPolygonText(const MultiPolygon *multiPolygon, int level, Writer *writer) {
	if (multiPolygon->isEmpty()) {
		writer->write("EMPTY");
	} else {
		int level2=level;
		bool doIndent=false;
		writer->write("(");
		for (unsigned int i=0, n=multiPolygon->getNumGeometries();
				i < n; i++)
		{
			if (i>0) {
				writer->write(", ");
				level2=level+1;
				doIndent=true;
			}
			const Polygon *p = dynamic_cast<const Polygon *>(
			  multiPolygon->getGeometryN(i)
			);
			appendPolygonText(p, level2, doIndent, writer);
		}
		writer->write(")");
	}
}

void
WKTWriter::appendGeometryCollectionText(
		const GeometryCollection *geometryCollection,
		int level,
		Writer *writer)
{
	if (geometryCollection->isEmpty()) {
		writer->write("EMPTY");
	} else {
		int level2=level;
		writer->write("(");
		for (unsigned int i=0, n=geometryCollection->getNumGeometries();
				i < n ; ++i)
		{
			if (i>0) {
				writer->write(", ");
				level2=level+1;
			}
			appendGeometryTaggedText(geometryCollection->getGeometryN(i),level2,writer);
		}
		writer->write(")");
	}
}

void WKTWriter::indent(int level, Writer *writer) {
	if (!isFormatted || level<=0) return;
	writer->write("\n");
	//writer->write(stringOfChar(' ', INDENT * level));
	writer->write(string(INDENT * level, ' '));
}

} // namespace geos.io
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.35  2006/06/12 16:55:53  strk
 * fixed compiler warnings, fixed some methods to omit unused parameters.
 *
 * Revision 1.34  2006/06/08 17:58:57  strk
 * Polygon::getNumInteriorRing() return size_t, Polygon::interiorRingN() takes size_t.
 *
 * Revision 1.33  2006/06/01 11:49:36  strk
 * Reduced installed headers form geomgraph namespace
 *
 * Revision 1.32  2006/04/28 11:12:31  strk
 * removed warnings related to change in getNumPoints() return type.
 *
 * Revision 1.31  2006/04/07 09:54:30  strk
 * Geometry::getNumGeometries() changed to return 'unsigned int'
 * rather then 'int'
 *
 * Revision 1.30  2006/03/22 16:58:35  strk
 * Removed (almost) all inclusions of geom.h.
 * Removed obsoleted .cpp files.
 * Fixed a bug in WKTReader not using the provided CoordinateSequence
 * implementation, optimized out some memory allocations.
 *
 * Revision 1.29  2006/03/20 18:18:15  strk
 * io.h header split
 *
 * Revision 1.28  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 * Revision 1.27  2006/03/06 19:40:47  strk
 * geos::util namespace. New GeometryCollection::iterator interface, many cleanups.
 *
 * Revision 1.26  2006/03/06 15:23:14  strk
 * geos::io namespace
 *
 * Revision 1.25  2006/03/03 10:46:21  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.24  2006/02/09 15:52:47  strk
 * GEOSException derived from std::exception; always thrown and cought by const ref.
 *
 * Revision 1.23  2006/02/08 17:18:28  strk
 * - New WKTWriter::toLineString and ::toPoint convenience methods
 * - New IsValidOp::setSelfTouchingRingFormingHoleValid method
 * - New Envelope::centre()
 * - New Envelope::intersection(Envelope)
 * - New Envelope::expandBy(distance, [ydistance])
 * - New LineString::reverse()
 * - New MultiLineString::reverse()
 * - New Geometry::buffer(distance, quadSeg, endCapStyle)
 * - Obsoleted toInternalGeometry/fromInternalGeometry
 * - More const-correctness in Buffer "package"
 *
 * Revision 1.22  2006/01/18 17:46:57  strk
 * Fixed leak in ::writeFormatted(Geometry *)
 *
 * Revision 1.21  2004/12/08 13:54:43  strk
 * gcc warnings checked and fixed, general cleanups.
 *
 * Revision 1.20  2004/10/21 22:29:54  strk
 * Indentation changes and some more COMPUTE_Z rules
 *
 * Revision 1.19  2004/10/20 17:32:14  strk
 * Initial approach to 2.5d intersection()
 *
 * Revision 1.18  2004/07/19 13:19:31  strk
 * Documentation fixes
 *
 * Revision 1.17  2004/07/07 09:38:12  strk
 * Dropped WKTWriter::stringOfChars (implemented by std::string).
 * Dropped WKTWriter default constructor (internally created GeometryFactory).
 * Updated XMLTester to respect the changes.
 * Main documentation page made nicer.
 *
 * Revision 1.16  2004/07/02 13:28:27  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.15  2004/03/18 10:42:44  ybychkov
 * "IO" and "Util" upgraded to JTS 1.4
 * "Geometry" partially upgraded.
 *
 * Revision 1.14  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/


