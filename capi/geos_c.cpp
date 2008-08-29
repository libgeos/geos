/************************************************************************
 *
 * $Id$
 *
 * C-Wrapper for GEOS library
 *
 * Copyright (C) 2005-2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 * Author: Sandro Santilli <strk@refractions.net>
 *
 ***********************************************************************/

#include <geos/geom/Geometry.h> 
#include <geos/geom/prep/PreparedGeometry.h> 
#include <geos/geom/prep/PreparedGeometryFactory.h> 
#include <geos/geom/GeometryCollection.h> 
#include <geos/geom/Polygon.h> 
#include <geos/geom/Point.h> 
#include <geos/geom/MultiPoint.h> 
#include <geos/geom/MultiLineString.h> 
#include <geos/geom/MultiPolygon.h> 
#include <geos/geom/LinearRing.h> 
#include <geos/geom/LineString.h> 
#include <geos/geom/PrecisionModel.h> 
#include <geos/geom/GeometryFactory.h> 
#include <geos/geom/CoordinateSequenceFactory.h> 
#include <geos/geom/IntersectionMatrix.h> 
#include <geos/io/WKTReader.h>
#include <geos/io/WKBReader.h>
#include <geos/io/WKTWriter.h>
#include <geos/io/WKBWriter.h>
#include <geos/io/CLocalizer.h>
#include <geos/simplify/DouglasPeuckerSimplifier.h>
#include <geos/simplify/TopologyPreservingSimplifier.h>
#include <geos/operation/valid/IsValidOp.h>
#include <geos/operation/polygonize/Polygonizer.h>
#include <geos/operation/linemerge/LineMerger.h>
#include <geos/operation/overlay/OverlayOp.h>
#include <geos/geom/BinaryOp.h>
#include <geos/version.h> 

// This should go away
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <memory>

// Some extra magic to make type declarations in geos_c.h work - for cross-checking of types in header.
#define GEOSGeometry geos::geom::Geometry
#define GEOSPreparedGeometry geos::geom::prep::PreparedGeometry
#define GEOSCoordSequence geos::geom::CoordinateSequence
#define GEOSWKTReader_t geos::io::WKTReader
#define GEOSWKTWriter_t geos::io::WKTWriter
#define GEOSWKBReader_t geos::io::WKBReader
#define GEOSWKBWriter_t geos::io::WKBWriter

#include "geos_c.h"

/// Define this if you want operations triggering Exceptions to
/// be printed (will use the NOTIFY channel - only implemented for GEOSUnion so far)
///
#undef VERBOSE_EXCEPTIONS

#if defined(_MSC_VER)
#  define GEOS_DLL     __declspec(dllexport)
#else
#  define GEOS_DLL
#endif

// import the most frequently used definitions globally
using geos::geom::Geometry;
using geos::geom::LineString;
using geos::geom::Polygon;
using geos::geom::CoordinateSequence;
using geos::geom::GeometryFactory;

using geos::io::WKTReader;
using geos::io::WKTWriter;
using geos::io::WKBReader;
using geos::io::WKBWriter;
using geos::io::CLocalizer;

using geos::operation::overlay::OverlayOp;
using geos::operation::overlay::overlayOp;

typedef std::auto_ptr<Geometry> GeomAutoPtr;

//## PROTOTYPES #############################################

extern "C" const char GEOS_DLL *GEOSjtsport();
extern "C" char GEOS_DLL *GEOSasText(Geometry *g1);

//## GLOBALS ################################################

// NOTE: SRID will have to be changed after geometry creation
static const GeometryFactory *geomFactory = 
	GeometryFactory::getDefaultInstance();

static GEOSMessageHandler NOTICE_MESSAGE;
static GEOSMessageHandler ERROR_MESSAGE;
static int WKBOutputDims = 2;
static int WKBByteOrder = getMachineByteOrder();

extern "C" {

void
initGEOS (GEOSMessageHandler nf, GEOSMessageHandler ef)
{
	NOTICE_MESSAGE = nf;
	ERROR_MESSAGE = ef;
}

void
finishGEOS ()
{
	// Nothing to do
	//delete geomFactory;
}

//-----------------------------------------------------------
// relate()-related functions
//  return 0 = false, 1 = true, 2 = error occured
//-----------------------------------------------------------

char
GEOSDisjoint(const Geometry *g1, const Geometry *g2)
{
	try {
		bool result;
		result = g1->disjoint(g2);
		return result;
	}
	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
		return 2;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return 2;
	}
}

char
GEOSTouches(const Geometry *g1, const Geometry *g2)
{
	try {
		bool result;
		result =  g1->touches(g2);
		return result;
	}

	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
		return 2;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return 2;
	}
}

char
GEOSIntersects(const Geometry *g1, const Geometry *g2)
{
	try {
		bool result;
		result = g1->intersects(g2);
		return result;
	}

	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
		return 2;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return 2;
	}
}

char
GEOSCrosses(const Geometry *g1, const Geometry *g2)
{
	try {
		bool result;
		result = g1->crosses(g2);
		return result;
	}

	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
		return 2;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return 2;
	}
}

char
GEOSWithin(const Geometry *g1, const Geometry *g2)
{
	try {
		bool result;
		result = g1->within(g2);
		return result;
	}

	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
		return 2;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return 2;
	}
}

// call g1->contains(g2)
// returns 0 = false
//         1 = true
//         2 = error was trapped
char
GEOSContains(const Geometry *g1, const Geometry *g2)
{
	try {
		bool result;
		result = g1->contains(g2);
		return result;
	}

	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
		return 2;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return 2;
	}
}

char
GEOSOverlaps(const Geometry *g1, const Geometry *g2)
{
	try {
		bool result;
		result = g1->overlaps(g2);
		return result;
	}

	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
		return 2;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return 2;
	}
}


//-------------------------------------------------------------------
// low-level relate functions
//------------------------------------------------------------------

char
GEOSRelatePattern(const Geometry *g1, const Geometry *g2, const char *pat)
{
	try {
		bool result;
		std::string s = pat;
		result = g1->relate(g2,s);
		return result;
	}

	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
		return 2;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return 2;
	}
}

char *
GEOSRelate(const Geometry *g1, const Geometry *g2)
{
	using geos::geom::IntersectionMatrix;
	try {

		IntersectionMatrix *im = g1->relate(g2);
		if (im == NULL)
				return NULL;
		
        std::string s(im->toString());
		char *result = NULL;
		result = (char*) std::malloc( s.length() + 1);
		std::strcpy(result, s.c_str() );
		delete im;

		return result;
	}
	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
	}
	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
	}
    
    return NULL;
}



//-----------------------------------------------------------------
// isValid
//-----------------------------------------------------------------


char
GEOSisValid(const Geometry *g1)
{
	using geos::operation::valid::IsValidOp;
	using geos::operation::valid::TopologyValidationError;
	IsValidOp ivo(g1);
	bool result;
	try {
		result = ivo.isValid();
		if ( result == 0 )
		{
			TopologyValidationError *err = ivo.getValidationError();
			if ( err ) {
				std::string errmsg = err->toString();
				NOTICE_MESSAGE("%s", errmsg.c_str());
			}
		}
		return result;
	}

	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
		return 2;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return 2;
	}

}


//-----------------------------------------------------------------
// general purpose
//-----------------------------------------------------------------

char
GEOSEquals(const Geometry *g1, const Geometry *g2)
{
	try {
		bool result;
		result = g1->equals(g2);
		return result;
	}

	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
		return 2;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return 2;
	}
}

char
GEOSEqualsExact(const Geometry *g1, const Geometry *g2, double tolerance)
{
	try {
		bool result;
		result = g1->equalsExact(g2, tolerance);
		return result;
	}

	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
		return 2;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return 2;
	}
}

int
GEOSDistance(const Geometry *g1, const Geometry *g2, double *dist)
{
	try {
		*dist = g1->distance(g2);
		return 1;
	}

	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
		return 0;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return 0;
	}
}

int
GEOSArea(const Geometry *g, double *area)
{
	try {
		*area = g->getArea();
		return 1;
	}

	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
		return 0;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return 0;
	}
}

int
GEOSLength(const Geometry *g, double *length)
{
	try {
		*length = g->getLength();
		return 1;
	}

	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
		return 0;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return 0;
	}
}

Geometry *
GEOSGeomFromWKT(const char *wkt)
{
    CLocalizer clocale;
	try
	{
		WKTReader r(geomFactory);
		const std::string wktstring = std::string(wkt);
		Geometry *g = r.read(wktstring);
		return g;
	}
	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
		return NULL;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}

char *
GEOSGeomToWKT(const Geometry *g1)
{
    CLocalizer clocale;
	try
	{
		std::string s = g1->toString();
		char *result;
		result = (char*) std::malloc( s.length() + 1);
		std::strcpy(result, s.c_str() );
        return result;
	}
	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
		return NULL;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}

// Remember to free the result!
unsigned char *
GEOSGeomToWKB_buf(const Geometry *g, size_t *size)
{
	using geos::io::WKBWriter;
	try
	{
		int byteOrder = (int) WKBByteOrder;
		WKBWriter w(WKBOutputDims, byteOrder);
		std::ostringstream s(std::ios_base::binary);
		w.write(*g, s);
		std::string wkbstring = s.str();
		size_t len = wkbstring.length();

		unsigned char *result;
		result = (unsigned char*) std::malloc(len);
        std::memcpy(result, wkbstring.c_str(), len);
		*size = len;
		return result;
	}
	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
		return NULL;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}

Geometry *
GEOSGeomFromWKB_buf(const unsigned char *wkb, size_t size)
{
	using geos::io::WKBReader;
	try
	{
		std::string wkbstring = std::string((const char*)wkb, size); // make it binary !
		WKBReader r(*geomFactory);
		std::istringstream s(std::ios_base::binary);
		s.str(wkbstring);

		s.seekg(0, std::ios::beg); // rewind reader pointer
		Geometry *g = r.read(s);
		return g;
	}
	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
		return NULL;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}

/* Read/write wkb hex values.  Returned geometries are
   owned by the caller.*/
unsigned char *
GEOSGeomToHEX_buf(const Geometry *g, size_t *size)
{
	using geos::io::WKBWriter;
	try
	{
        int byteOrder = (int) WKBByteOrder;
		WKBWriter w(WKBOutputDims, byteOrder);
		std::ostringstream s(std::ios_base::binary);
		w.writeHEX(*g, s);
		std::string hexstring = s.str();
		size_t len = hexstring.length();

		char *result;
		result = (char*) std::malloc(len);
        std::memcpy(result, hexstring.c_str(), len);
		*size = len;
		return (unsigned char*) result;
	}
	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
		return NULL;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}

Geometry *
GEOSGeomFromHEX_buf(const unsigned char *hex, size_t size)
{
	using geos::io::WKBReader;
	try
	{
		std::string hexstring = std::string((const char*)hex, size); 
		WKBReader r(*geomFactory);
		std::istringstream s(std::ios_base::binary);
		s.str(hexstring);

		s.seekg(0, std::ios::beg); // rewind reader pointer
		Geometry *g = r.readHEX(s);
		return g;
	}
	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
		return NULL;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}

char
GEOSisEmpty(const Geometry *g1)
{
	try
	{
		return g1->isEmpty();
	}
	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
		return 2;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return 2;
	}
}

char
GEOSisSimple(const Geometry *g1)
{
	try
	{
		return g1->isSimple();
	}
	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
		return 2;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return 2;
	}
}

char
GEOSisRing(const Geometry *g)
{
	try
	{
		const LineString *ls = dynamic_cast<const LineString *>(g);
		if ( ls ) {
			return (ls->isRing());
		} else {
			return 0;
		}
	}
	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
		return 2;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return 2;
	}
}



//free the result of this
char *
GEOSGeomType(const Geometry *g1)
{
	try
	{
		std::string s = g1->getGeometryType();

		char *result;
		result = (char*) std::malloc( s.length() + 1);
		std::strcpy(result, s.c_str() );
		return result;
	}
	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
		return NULL;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}

// Return postgis geometry type index
int
GEOSGeomTypeId(const Geometry *g1)
{
	try
	{
		return g1->getGeometryTypeId();
	}
	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
		return -1;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return -1;
	}
}




//-------------------------------------------------------------------
// GEOS functions that return geometries
//-------------------------------------------------------------------

Geometry *
GEOSEnvelope(const Geometry *g1)
{
	try
	{
		Geometry *g3 = g1->getEnvelope();
		return g3;
	}
	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
		return NULL;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}

Geometry *
GEOSIntersection(const Geometry *g1, const Geometry *g2)
{
	try
	{
		GeomAutoPtr g3 = BinaryOp(g1, g2, overlayOp(OverlayOp::opINTERSECTION));
		return g3.release();
		//Geometry *g3 = g1->intersection(g2);
		//return g3;
	}
	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
		return NULL;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}

Geometry *
GEOSBuffer(const Geometry *g1, double width, int quadrantsegments)
{
	try
	{
		Geometry *g3 = g1->buffer(width, quadrantsegments);
		return g3;
	}
	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
		return NULL;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}

Geometry *
GEOSConvexHull(const Geometry *g1)
{
	try
	{
		Geometry *g3 = g1->convexHull();
		return g3;
	}
	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
		return NULL;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}

Geometry *
GEOSDifference(const Geometry *g1, const Geometry *g2)
{
	try
	{
		GeomAutoPtr g3 = BinaryOp(g1, g2, overlayOp(OverlayOp::opDIFFERENCE));
		return g3.release();
		//Geometry *g3 = g1->difference(g2);
		//return g3;
	}
	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
		return NULL;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}

Geometry *
GEOSBoundary(const Geometry *g1)
{
	try
	{
		Geometry *g3 = g1->getBoundary();
		return g3;
	}
	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
		return NULL;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}

Geometry *
GEOSSymDifference(const Geometry *g1, const Geometry *g2)
{
	try
	{
		GeomAutoPtr g3 = BinaryOp(g1, g2, overlayOp(OverlayOp::opSYMDIFFERENCE));
		return g3.release();
		//Geometry *g3 = g1->symDifference(g2);
		//return g3;
	}
	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
		return NULL;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}

Geometry *
GEOSUnion(const Geometry *g1, const Geometry *g2)
{
	try
	{
		GeomAutoPtr g3 = BinaryOp(g1, g2, overlayOp(OverlayOp::opUNION));
		return g3.release();
		//Geometry *g3 = g1->Union(g2);
		//return g3;
	}
	catch (const std::exception &e)
	{
#if VERBOSE_EXCEPTIONS
		std::ostringstream s; 
		s << "Exception on GEOSUnion with following inputs:" << std::endl;
		s << "A: "<<g1->toString() << std::endl;
		s << "B: "<<g2->toString() << std::endl;
		NOTICE_MESSAGE("%s", s.str().c_str());
#endif // VERBOSE_EXCEPTIONS
		ERROR_MESSAGE("%s", e.what());
		return NULL;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}


Geometry *
GEOSPointOnSurface(const Geometry *g1)
{
	try
	{
		Geometry *ret = g1->getInteriorPoint();
		if ( ! ret ) return geomFactory->createGeometryCollection();
		return ret;
	}
	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
		return NULL;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}





//-------------------------------------------------------------------
// memory management functions
//------------------------------------------------------------------


void
GEOSGeom_destroy(Geometry *a)
{
	try{
		delete a;
	}
	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
	}
}

void
GEOSSetSRID(Geometry *g, int SRID)
{
	g->setSRID(SRID);
}

/*
void
GEOSdeleteChar(char *a)
{
	try{
	   free(a);
	}
	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
	}
}
*/


int
GEOSGetNumCoordinates(const Geometry *g1)
{
	try{
		return g1->getNumPoints();
	}
	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
		return -1;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return -1;
	}
}

/*
 * Return -1 on exception, 0 otherwise. 
 * Converts Geometry to normal form (or canonical form).
 */
int
GEOSNormalize(Geometry *g1)
{
	try{
		g1->normalize();
	}
	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
		return -1;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return -1;
	}
    return 0;
}

int
GEOSGetNumInteriorRings(const Geometry *g1)
{
	try{
		const Polygon *p = dynamic_cast<const Polygon *>(g1);
                if ( ! p )
                {
                        ERROR_MESSAGE("Argument is not a Polygon");
                        return -1;
                }
		return p->getNumInteriorRing();
	}
	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
		return -1;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return -1;
	}
}


// returns -1 on error and 1 for non-multi geometries
int
GEOSGetNumGeometries(const Geometry *g1)
{
	try{
		return g1->getNumGeometries();
	}
	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
		return -1;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return -1;
	}
}


/*
 * Call only on GEOMETRYCOLLECTION or MULTI*.
 * Return a pointer to the internal Geometry.
 */
const Geometry *
GEOSGetGeometryN(const Geometry *g1, int n)
{
	using geos::geom::GeometryCollection;
	try{
		const GeometryCollection *gc = dynamic_cast<const GeometryCollection *>(g1);
		if ( ! gc )
		{
			ERROR_MESSAGE("Argument is not a GeometryCollection");
			return NULL;
		}
		return gc->getGeometryN(n);
	}
	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
		return NULL;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}


/*
 * Call only on polygon
 * Return a copy of the internal Geometry.
 */
const Geometry *
GEOSGetExteriorRing(const Geometry *g1)
{
	try{
		const Polygon *p = dynamic_cast<const Polygon *>(g1);
		if ( ! p ) 
		{
			ERROR_MESSAGE("Invalid argument (must be a Polygon)");
			return NULL;
		}
		return p->getExteriorRing();
	}
	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
		return NULL;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}

/*
 * Call only on polygon
 * Return a pointer to internal storage, do not destroy it.
 */
const Geometry *
GEOSGetInteriorRingN(const Geometry *g1, int n)
{
	try{
		const Polygon *p = dynamic_cast<const Polygon *>(g1);
		if ( ! p ) 
		{
			ERROR_MESSAGE("Invalid argument (must be a Polygon)");
			return NULL;
		}
		return p->getInteriorRingN(n);
	}
	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
		return NULL;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}

Geometry *
GEOSGetCentroid(const Geometry *g)
{
	try{
		Geometry *ret = g->getCentroid();
		if ( ! ret ) return geomFactory->createGeometryCollection();
		return ret;
	}
	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
		return NULL;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}

Geometry *
GEOSGeom_createCollection(int type, Geometry **geoms, unsigned int ngeoms)
{
#ifdef GEOS_DEBUG
	char buf[256];
	sprintf(buf, "PostGIS2GEOS_collection: requested type %d, ngeoms: %d",
			type, ngeoms);
	NOTICE_MESSAGE("%s", buf);// TODO: Can NOTICE_MESSAGE format that directly? 
#endif

	try
	{
		Geometry *g;
		std::vector<Geometry *> *vgeoms = new std::vector<Geometry *>(geoms, geoms+ngeoms);

		switch (type)
		{
			case GEOS_GEOMETRYCOLLECTION:
				g = geomFactory->createGeometryCollection(vgeoms);
				break;
			case GEOS_MULTIPOINT:
				g = geomFactory->createMultiPoint(vgeoms);
				break;
			case GEOS_MULTILINESTRING:
				g = geomFactory->createMultiLineString(vgeoms);
				break;
			case GEOS_MULTIPOLYGON:
				g = geomFactory->createMultiPolygon(vgeoms);
				break;
			default:
				ERROR_MESSAGE("Unsupported type request for PostGIS2GEOS_collection");
				g = NULL;
				
		}
		if (g==NULL) return NULL;
		return g;
	}
	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
		return NULL;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}

Geometry *
GEOSPolygonize(const Geometry * const * g, unsigned int ngeoms)
{
	using geos::operation::polygonize::Polygonizer;
	unsigned int i;
	Geometry *out = NULL;

	try{
		// Polygonize
		Polygonizer plgnzr;
		for (i=0; i<ngeoms; i++) plgnzr.add(g[i]);
#if GEOS_DEBUG
	NOTICE_MESSAGE("geometry vector added to polygonizer");
#endif

		std::vector<Polygon *>*polys = plgnzr.getPolygons();

#if GEOS_DEBUG
	NOTICE_MESSAGE("output polygons got");
#endif

		// We need a vector of Geometry pointers, not
		// Polygon pointers.
		// STL vector doesn't allow transparent upcast of this
		// nature, so we explicitly convert.
		// (it's just a waste of processor and memory, btw)
                std::vector<Geometry*> *polyvec =
				new std::vector<Geometry *>(polys->size());
		for (i=0; i<polys->size(); i++) (*polyvec)[i] = (*polys)[i];
		delete polys;

		out = geomFactory->createGeometryCollection(polyvec);
		// the above method takes ownership of the passed
		// vector, so we must *not* delete it
	}
	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
		return NULL;
	}
	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}

	return out;
}

Geometry *
GEOSPolygonizer_getCutEdges(const Geometry * const * g, unsigned int ngeoms)
{
	using geos::operation::polygonize::Polygonizer;
	unsigned int i;
	Geometry *out = NULL;

	try{
		// Polygonize
		Polygonizer plgnzr;
		for (i=0; i<ngeoms; i++) plgnzr.add(g[i]);
#if GEOS_DEBUG
	NOTICE_MESSAGE("geometry vector added to polygonizer");
#endif

		std::vector<const LineString *>*lines = plgnzr.getCutEdges();

#if GEOS_DEBUG
	NOTICE_MESSAGE("output polygons got");
#endif

		// We need a vector of Geometry pointers, not
		// Polygon pointers.
		// STL vector doesn't allow transparent upcast of this
		// nature, so we explicitly convert.
		// (it's just a waste of processor and memory, btw)
    std::vector<Geometry*> *linevec =
				new std::vector<Geometry *>(lines->size());
		for (i=0; i<lines->size(); i++) (*linevec)[i] = (*lines)[i]->clone();

		out = geomFactory->createGeometryCollection(linevec);
		// the above method takes ownership of the passed
		// vector, so we must *not* delete it
	}
	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
		return NULL;
	}
	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}

	return out;
}

Geometry *
GEOSLineMerge(const Geometry *g)
{
		using geos::operation::linemerge::LineMerger;
        unsigned int i;
        Geometry *out = NULL;

        try{
                // LineMerge
                LineMerger lmrgr;

                lmrgr.add(g);

                std::vector<LineString *>*lines = lmrgr.getMergedLineStrings();

#if GEOS_DEBUG
        NOTICE_MESSAGE("output lines got");
#endif

                std::vector<Geometry *>*geoms = new std::vector<Geometry *>(lines->size());
                for (i=0; i<lines->size(); i++) (*geoms)[i] = (*lines)[i];
                delete lines;
                out = geomFactory->buildGeometry(geoms);
                //out = geomFactory->createGeometryCollection(geoms);
        }
	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
		return NULL;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}

        return out;
}

int
GEOSGetSRID(const Geometry *g1)
{
	try{
		return g1->getSRID();
	}
	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
		return 0;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return 0;
	}
}

const char *
GEOSversion()
{
	//static string version = GEOS_CAPI_VERSION;
	return GEOS_CAPI_VERSION;
}

const char *
GEOSjtsport()
{
	//string version = jtsport();
	//char *res = strdup(version.c_str());
	//return res;
	return GEOS_JTS_PORT;
}


char 
GEOSHasZ(const Geometry *g)
{
	if ( g->isEmpty() ) return false;
	double az = g->getCoordinate()->z;
	//ERROR_MESSAGE("ZCoord: %g", az);
	return FINITE(az);
}

int
GEOS_getWKBOutputDims()
{
    return WKBOutputDims;
}

int
GEOS_setWKBOutputDims(int newdims)
{
	if ( newdims < 2 || newdims > 3 )
		ERROR_MESSAGE("WKB output dimensions out of range 2..3");
	int olddims = WKBOutputDims;
	WKBOutputDims = newdims;
	return olddims;
}

int
GEOS_getWKBByteOrder()
{
	return WKBByteOrder;
}

int
GEOS_setWKBByteOrder(int byteOrder)
{
	int oldByteOrder = WKBByteOrder;
	WKBByteOrder = byteOrder;
	return oldByteOrder;
}


CoordinateSequence *
GEOSCoordSeq_create(unsigned int size, unsigned int dims)
{
	try {
		return geomFactory->getCoordinateSequenceFactory()->create(size, dims);
	}

	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
		return NULL;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}

int
GEOSCoordSeq_setOrdinate(CoordinateSequence *s, unsigned int idx,
	unsigned int dim, double val)
{
	try {
		s->setOrdinate(static_cast<int>(idx),
			static_cast<int>(dim), val);
		return 1;
	}
	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
		return 0;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return 0;
	}
}

int
GEOSCoordSeq_setX(CoordinateSequence *s, unsigned int idx, double val)
{
	return GEOSCoordSeq_setOrdinate(s, idx, 0, val);
}

int
GEOSCoordSeq_setY(CoordinateSequence *s, unsigned int idx, double val)
{
	return GEOSCoordSeq_setOrdinate(s, idx, 1, val);
}

int
GEOSCoordSeq_setZ(CoordinateSequence *s, unsigned int idx, double val)
{
	return GEOSCoordSeq_setOrdinate(s, idx, 2, val);
}

CoordinateSequence *
GEOSCoordSeq_clone(const CoordinateSequence *s)
{
	try { return s->clone(); }
	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
		return NULL;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}

int
GEOSCoordSeq_getOrdinate(const CoordinateSequence *s, unsigned int idx,
	unsigned int dim, double *val)
{
	try {
		double d = s->getOrdinate(static_cast<int>(idx),
			static_cast<int>(dim));
		*val = d;
		return 1;
	}
	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
		return 0;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return 0;
	}
}

int
GEOSCoordSeq_getX(const CoordinateSequence *s, unsigned int idx, double *val)
{
	return GEOSCoordSeq_getOrdinate(s, idx, 0, val);
}

int
GEOSCoordSeq_getY(const CoordinateSequence *s, unsigned int idx, double *val)
{
	return GEOSCoordSeq_getOrdinate(s, idx, 1, val);
}

int
GEOSCoordSeq_getZ(const CoordinateSequence *s, unsigned int idx, double *val)
{
	return GEOSCoordSeq_getOrdinate(s, idx, 2, val);
}

int
GEOSCoordSeq_getSize(const CoordinateSequence *s, unsigned int *size)
{
	try {
		int sz = s->getSize();
		*size = static_cast<unsigned int>(sz);
		return 1;
	}
	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
		return 0;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return 0;
	}
}

int
GEOSCoordSeq_getDimensions(const CoordinateSequence *s, unsigned int *dims)
{
	try {
		unsigned int dm = s->getDimension();
		*dims = dm;
		return 1;
	}
	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
		return 0;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return 0;
	}
}

void
GEOSCoordSeq_destroy(CoordinateSequence *s)
{
	try{
		delete s;
	}

	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
	}
}

const CoordinateSequence *
GEOSGeom_getCoordSeq(const Geometry *g)
{
        using geos::geom::Point;
	try
	{
		const LineString *ls = dynamic_cast<const LineString *>(g);
		if ( ls )
		{
			return ls->getCoordinatesRO();
		}
		const Point *p = dynamic_cast<const Point *>(g);
		if ( p ) 
		{
			return p->getCoordinatesRO();
		}
		ERROR_MESSAGE("Geometry must be a Point or LineString");
		return NULL;
	}
	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
		return NULL;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}

Geometry *
GEOSGeom_createPoint(CoordinateSequence *cs)
{
	try { return geomFactory->createPoint(cs); }
	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
		return NULL;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}

Geometry *
GEOSGeom_createLinearRing(CoordinateSequence *cs)
{
	try { return geomFactory->createLinearRing(cs); }
	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
		return NULL;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}

Geometry *
GEOSGeom_createLineString(CoordinateSequence *cs)
{
	try { return geomFactory->createLineString(cs); }
	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
		return NULL;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}

Geometry *
GEOSGeom_createPolygon(Geometry *shell, Geometry **holes, 
	unsigned int nholes)
{
        using geos::geom::LinearRing;
	try
	{
		std::vector<Geometry *> *vholes = new std::vector<Geometry *>(holes, holes+nholes);
		LinearRing *nshell = dynamic_cast<LinearRing *>(shell);
		if ( ! nshell )
		{
			ERROR_MESSAGE("Shell is not a LinearRing");
			return NULL;
		}
		return geomFactory->createPolygon(nshell, vholes);
	}
	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
		return NULL;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}

Geometry *
GEOSGeom_clone(const Geometry *g)
{
	try { return g->clone(); }
	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
		return NULL;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}

int
GEOSGeom_getDimensions(const Geometry *g)
{
        using geos::geom::GeometryCollection;
        using geos::geom::Point;
	try {
		const LineString *ls = dynamic_cast<const LineString *>(g);
		if ( ls )
		{
			return ls->getCoordinatesRO()->getDimension();
		}

		const Point *p = dynamic_cast<const Point *>(g);
		if ( p )
		{
			return p->getCoordinatesRO()->getDimension();
		}

		const Polygon *poly = dynamic_cast<const Polygon *>(g);
		if ( poly )
		{
			return GEOSGeom_getDimensions(poly->getExteriorRing());
		}


		const GeometryCollection *coll =
			dynamic_cast<const GeometryCollection *>(g);
		if ( coll )
		{
			if ( coll->isEmpty() ) return 0;
			return GEOSGeom_getDimensions(coll->getGeometryN(0));
		}

		ERROR_MESSAGE("Unknown geometry type");
		return 0;
	}

	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
		return 0;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return 0;
	}
}

Geometry *
GEOSSimplify(const Geometry *g1, double tolerance)
{
	using namespace geos::simplify;

	try
	{
		Geometry::AutoPtr g(DouglasPeuckerSimplifier::simplify(
				g1, tolerance));
		return g.release();
	}
	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
		return NULL;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}

Geometry *
GEOSTopologyPreserveSimplify(const Geometry *g1, double tolerance)
{
	using namespace geos::simplify;

	try
	{
		Geometry::AutoPtr g(TopologyPreservingSimplifier::simplify(
				g1, tolerance));
		return g.release();
	}
	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
		return NULL;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}


/* WKT Reader */
WKTReader *
GEOSWKTReader_create()
{
	using geos::io::WKTReader;
	try
	{
		return new WKTReader(geomFactory);
	}
	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
		return NULL;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}

void
GEOSWKTReader_destroy(WKTReader *reader)
{
	try
	{
		delete reader;
	}
	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
	}
}


Geometry*
GEOSWKTReader_read(WKTReader *reader, const char *wkt)
{
    CLocalizer clocale;
	try
	{
		const std::string wktstring = std::string(wkt);
		Geometry *g = reader->read(wktstring);
		return g;
	}
	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
		return NULL;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}

/* WKT Writer */
WKTWriter *
GEOSWKTWriter_create()
{
	using geos::io::WKTWriter;
	try
	{
		return new WKTWriter();
	}
	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
		return NULL;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}

void
GEOSWKTWriter_destroy(WKTWriter *Writer)
{
	try
	{
		delete Writer;
	}
	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
	}
}


char*
GEOSWKTWriter_write(WKTWriter *writer, const Geometry *geom)
{
    CLocalizer clocale;
	try
	{
		std::string s = writer->write(geom);
		char *result = NULL;
		result = (char*) std::malloc( s.length() + 1);
		std::strcpy(result, s.c_str() );
		return result;
	}
	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
	}
	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
	}
    
    return NULL;
}

/* WKB Reader */
WKBReader *
GEOSWKBReader_create()
{
	using geos::io::WKBReader;
	try
	{
		return new WKBReader(*geomFactory);
	}
	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
	}
	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
	}
    
    return NULL;
}

void
GEOSWKBReader_destroy(WKBReader *reader)
{
	try
	{
		delete reader;
	}
	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
	}
}


Geometry*
GEOSWKBReader_read(WKBReader *reader, const unsigned char *wkb, size_t size)
{
	try
	{
		std::string wkbstring = std::string((const char*)wkb, size); // make it binary !
		std::istringstream s(std::ios_base::binary);
		s.str(wkbstring);

		s.seekg(0, std::ios::beg); // rewind reader pointer
		Geometry *g = reader->read(s);
		return g;
	}
	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
	}
	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
	}
    
    return NULL;
}

Geometry*
GEOSWKBReader_readHEX(WKBReader *reader, const unsigned char *hex, size_t size)
{
	try
	{
		std::string hexstring = std::string((const char*)hex, size); 
		std::istringstream s(std::ios_base::binary);
		s.str(hexstring);

		s.seekg(0, std::ios::beg); // rewind reader pointer
		Geometry *g = reader->readHEX(s);
		return g;
	}
	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
	}
	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
	}
    
    return NULL;
}

/* WKB Writer */
WKBWriter *
GEOSWKBWriter_create()
{
	using geos::io::WKBWriter;
	try
	{
		return new WKBWriter();
	}
	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
	}
	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
	}
    
    return NULL;
}

void
GEOSWKBWriter_destroy(WKBWriter *Writer)
{
	try
	{
		delete Writer;
	}
	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
	}
}


/* The owner owns the result */
unsigned char*
GEOSWKBWriter_write(WKBWriter *writer, const Geometry *geom, size_t *size)
{
	try
	{
		std::ostringstream s(std::ios_base::binary);
		writer->write(*geom, s);
		std::string wkbstring = s.str();
		size_t len = wkbstring.length();

		unsigned char *result = NULL;
		result = (unsigned char*) std::malloc(len);
        std::memcpy(result, wkbstring.c_str(), len);
		*size = len;
		return result;
	}
	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
	}
	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
	}
    return NULL;
}

/* The owner owns the result */
unsigned char*
GEOSWKBWriter_writeHEX(WKBWriter *writer, const Geometry *geom, size_t *size)
{
	try
	{
		std::ostringstream s(std::ios_base::binary);
		writer->writeHEX(*geom, s);
		std::string wkbstring = s.str();
		const size_t len = wkbstring.length();

		unsigned char *result = NULL;
		result = (unsigned char*) std::malloc(len);
        std::memcpy(result, wkbstring.c_str(), len);
		*size = len;
		return result;
	}
	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
	}
    return NULL;
}

int
GEOSWKBWriter_getOutputDimension(const GEOSWKBWriter* writer)
{
	try
	{
		return writer->getOutputDimension();
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
	}
    return 0;
}

void
GEOSWKBWriter_setOutputDimension(GEOSWKBWriter* writer, int newDimension)
{
	try
	{
		return writer->setOutputDimension(newDimension);
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
	}
}

int
GEOSWKBWriter_getByteOrder(const GEOSWKBWriter* writer)
{
	try
	{
		return writer->getByteOrder();
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
	}
    return 0;
}

void
GEOSWKBWriter_setByteOrder(GEOSWKBWriter* writer, int newByteOrder)
{
	try
	{
		return writer->setByteOrder(newByteOrder);
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
	}
}

char
GEOSWKBWriter_getIncludeSRID(const GEOSWKBWriter* writer)
{
	try
	{
		return writer->getIncludeSRID();
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}

void
GEOSWKBWriter_setIncludeSRID(GEOSWKBWriter* writer, const char newIncludeSRID)
{
	try
	{
		writer->setIncludeSRID(newIncludeSRID);
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
	}
}


//-----------------------------------------------------------------
// Prepared Geometry 
//-----------------------------------------------------------------

const geos::geom::prep::PreparedGeometry*
GEOSPrepare(const Geometry *g)
{
    const geos::geom::prep::PreparedGeometry* prep = NULL;
	
    try
	{
		prep = geos::geom::prep::PreparedGeometryFactory::prepare(g);
	}
	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
	}
	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
	}
	
    return prep;
}

void
GEOSPreparedGeom_destroy(geos::geom::prep::PreparedGeometry *a)
{
	try
	{
		delete a;
	}
	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
	}
	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
	}
}

char
GEOSPreparedContains(const geos::geom::prep::PreparedGeometry *pg1, const Geometry *g2)
{
	try 
	{
		bool result;
		result = pg1->contains(g2);
		return result;
	}
	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
		return 2;
	}
	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return 2;
	}
}

char
GEOSPreparedContainsProperly(const geos::geom::prep::PreparedGeometry *pg1, const Geometry *g2)
{
	try 
	{
		bool result;
		result = pg1->containsProperly(g2);
		return result;
	}
	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
		return 2;
	}
	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return 2;
	}
}

char
GEOSPreparedCovers(const geos::geom::prep::PreparedGeometry *pg1, const Geometry *g2)
{
	try 
	{
		bool result;
		result = pg1->covers(g2);
		return result;
	}
	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
		return 2;
	}
	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return 2;
	}
}

char
GEOSPreparedIntersects(const geos::geom::prep::PreparedGeometry *pg1, const Geometry *g2)
{
	try 
	{
		bool result;
		result = pg1->intersects(g2);
		return result;
	}
	catch (const std::exception &e)
	{
		ERROR_MESSAGE("%s", e.what());
		return 2;
	}
	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return 2;
	}
}

} //extern "C"
