/************************************************************************
 *
 * $Id$
 *
 * C-Wrapper for GEOS library
 *
 * Copyright (C) 2005 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 * Author: Sandro Santilli <strk@refractions.net>
 *
 ***********************************************************************/

#include <stdio.h>

#include <string>
#include <iostream>
#include <sstream>
#include <fstream>

#include <geos.h>
#include <geos/opValid.h>
#include <geos/opPolygonize.h>
#include <geos/opLinemerge.h>

#ifndef GEOS_CAPI_VERSION
#  define GEOS_CAPI_VERSION "3.0.0-CAPI-1.0.0"
#endif

#if defined(_MSC_VER)
#  define GEOS_DLL     __declspec(dllexport)
#else
#  define GEOS_DLL
#endif

using namespace geos;

typedef void (*GEOSMessageHandler)(const char *fmt, ...);

//## PROTOTYPES #############################################

/* Initialize GEOS library */
extern "C" void GEOS_DLL initGEOS(GEOSMessageHandler notice, GEOSMessageHandler err);

/* Release GEOS resources */
extern "C" void GEOS_DLL finishGEOS();

/* Input and Output functions, return NULL on exception. */
extern "C" Geometry GEOS_DLL *GEOSGeomFromWKT(const char *wkt);
extern "C" Geometry GEOS_DLL *GEOSGeomFromWKB_buf(const char *wkb, size_t size);
extern "C" char GEOS_DLL *GEOSGeomToWKT(const Geometry *g);
extern "C" char GEOS_DLL *GEOSGeomToWKB_buf(const Geometry *g, size_t *size);
extern "C" int GEOS_DLL GEOS_setWKBOutputDims(int newdims);

extern "C" void GEOS_DLL GEOSSetSRID(Geometry *g, int SRID);

extern "C" char GEOS_DLL *GEOSRelate(const Geometry *g1, const Geometry *g2);
extern "C" char GEOS_DLL GEOSRelatePattern(const Geometry *g1, const Geometry *g2, const char *pat);
extern "C" char GEOS_DLL GEOSDisjoint(const Geometry *g1, const Geometry *g2);
extern "C" char GEOS_DLL GEOSTouches(const Geometry *g1, const Geometry *g2);
extern "C" char GEOS_DLL GEOSIntersects(const Geometry *g1, const Geometry *g2);
extern "C" char GEOS_DLL GEOSCrosses(const Geometry *g1, const Geometry *g2);
extern "C" char GEOS_DLL GEOSWithin(const Geometry *g1, const Geometry *g2);
extern "C" char GEOS_DLL GEOSContains(const Geometry *g1, const Geometry *g2);
extern "C" char GEOS_DLL GEOSOverlaps(const Geometry *g1, const Geometry *g2);

extern "C" Geometry GEOS_DLL *GEOSpolygonize(Geometry **geoms, unsigned int ngeoms);
extern "C" char GEOS_DLL GEOSisValid(const Geometry *g1);
extern "C" char GEOS_DLL GEOSisEmpty(const Geometry *g1);
extern "C" Geometry GEOS_DLL *GEOSIntersection(Geometry *g1,Geometry *g2);
extern "C" Geometry GEOS_DLL *GEOSBuffer(Geometry *g1,double width,int quadsegs);
extern "C" Geometry GEOS_DLL *GEOSConvexHull(Geometry *g1);
extern "C" Geometry GEOS_DLL *GEOSDifference(Geometry *g1,Geometry *g2);
extern "C" Geometry GEOS_DLL *GEOSBoundary(Geometry *g1);
extern "C" Geometry GEOS_DLL *GEOSSymDifference(Geometry *g1,Geometry *g2);
extern "C" Geometry GEOS_DLL *GEOSUnion(Geometry *g1,Geometry *g2);
extern "C" const Geometry GEOS_DLL *GEOSGetGeometryN(Geometry *g1, int n);
extern "C" const Geometry GEOS_DLL *GEOSGetExteriorRing(Geometry *g1);
extern "C" const Geometry GEOS_DLL *GEOSGetInteriorRingN(Geometry *g1, int n);
extern "C" int GEOS_DLL GEOSGetNumInteriorRings(Geometry *g1);
extern "C" int GEOS_DLL GEOSGetSRID(Geometry *g1);
extern "C" int GEOS_DLL GEOSGetNumGeometries(Geometry *g1);
extern "C" char GEOS_DLL GEOSisSimple(Geometry *g1);
extern "C" char GEOS_DLL GEOSEquals(const Geometry *g1, const Geometry*g2);
extern "C" char GEOS_DLL GEOSisRing(Geometry *g1);
extern "C" Geometry GEOS_DLL *GEOSPointOnSurface(Geometry *g1);
extern "C" Geometry GEOS_DLL *GEOSGetCentroid(Geometry *g);
extern "C" CoordinateSequence GEOS_DLL *GEOSGeom_getCoordSeq(Geometry *g1);

extern "C" int GEOS_DLL GEOSDistance(const Geometry *g1, const Geometry *g2,
	double *dist);

extern "C" const char GEOS_DLL *GEOSversion();
extern "C" char GEOS_DLL *GEOSjtsport();
extern "C" char GEOS_DLL *GEOSasText(Geometry *g1);
extern "C" char GEOS_DLL *GEOSGeomType(Geometry *g1);
extern "C" int GEOS_DLL GEOSGeomTypeId(Geometry *g1);

//extern "C" void GEOSdeleteChar(char *a);
extern "C" void GEOS_DLL  GEOSGeom_destroy(Geometry *a);
extern "C" bool GEOS_DLL GEOSHasZ(Geometry *g1);

extern "C" Geometry GEOS_DLL *GEOSPolygonize(Geometry **, unsigned int);
extern "C" Geometry GEOS_DLL *GEOSLineMerge(Geometry *);

extern "C" int GEOS_DLL GEOSGeom_getDimensions(const Geometry *);
extern "C" int GEOS_DLL GEOSGetNumCoordinates(const Geometry *);

/*************************************************************************
 *
 * Coordinate Sequences
 *
 *************************************************************************/

extern "C" CoordinateSequence GEOS_DLL *GEOSCoordSeq_create(unsigned int, unsigned int);
extern "C" int GEOS_DLL GEOSCoordSeq_getSize(CoordinateSequence *, unsigned int *size);
extern "C" int GEOS_DLL GEOSCoordSeq_getDimensions(CoordinateSequence *, unsigned int *dims);
extern "C" int GEOS_DLL GEOSCoordSeq_setX(CoordinateSequence *, unsigned int, double);
extern "C" int GEOS_DLL GEOSCoordSeq_setY(CoordinateSequence *, unsigned int, double);
extern "C" int GEOS_DLL GEOSCoordSeq_setZ(CoordinateSequence *, unsigned int, double);
extern "C" int GEOS_DLL GEOSCoordSeq_setOrdinate(CoordinateSequence *, unsigned int, unsigned int, double);
extern "C" int GEOS_DLL GEOSCoordSeq_getX(CoordinateSequence *, unsigned int, double *);
extern "C" int GEOS_DLL GEOSCoordSeq_getY(CoordinateSequence *, unsigned int, double *);
extern "C" int GEOS_DLL GEOSCoordSeq_getZ(CoordinateSequence *, unsigned int, double *);
extern "C" int GEOS_DLL GEOSCoordSeq_getOrdinate(unsigned int, unsigned int, double *);
extern "C" CoordinateSequence GEOS_DLL *GEOSCoordSeq_clone(CoordinateSequence *);
extern "C" void GEOS_DLL GEOSCoordSeq_destroy(CoordinateSequence *);

/*************************************************************************
 *
 * Geometry constructors
 *
 *************************************************************************/

extern "C" Geometry GEOS_DLL *GEOSGeom_createPoint(CoordinateSequence *);
extern "C" Geometry GEOS_DLL *GEOSGeom_createLinearRing(CoordinateSequence *);
extern "C" Geometry GEOS_DLL *GEOSGeom_createLineString(CoordinateSequence *);
extern "C" Geometry GEOS_DLL *GEOSGeom_createPolygon(Geometry *, Geometry **, unsigned int);
extern "C" Geometry GEOS_DLL *GEOSGeom_createCollection(int type, Geometry **, unsigned int);
extern "C" Geometry GEOS_DLL *GEOSGeom_clone(Geometry *);

//## GLOBALS ################################################

static GeometryFactory *geomFactory = NULL;
static GEOSMessageHandler NOTICE_MESSAGE;
static GEOSMessageHandler ERROR_MESSAGE;
static int WKBOutputDims = 2;

void
initGEOS (GEOSMessageHandler nf, GEOSMessageHandler ef)
{
	if (geomFactory == NULL)
	{
		// NOTE: SRID will have to be changed after geometry creation
		geomFactory = new GeometryFactory( new PrecisionModel(), -1);
	}
	NOTICE_MESSAGE = nf;
	ERROR_MESSAGE = ef;
}

void
finishGEOS ()
{
	delete geomFactory;
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
	catch (GEOSException *ge)
	{
		ERROR_MESSAGE((char *)ge->toString().c_str());
		delete ge;
		return 2;
	}
	catch (std::exception &e)
	{
		ERROR_MESSAGE(e.what());
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
	catch (GEOSException *ge)
	{
		ERROR_MESSAGE((char *)ge->toString().c_str());
		delete ge;
		return 2;
	}

	catch (std::exception &e)
	{
		ERROR_MESSAGE(e.what());
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
	catch (GEOSException *ge)
	{
		ERROR_MESSAGE((char *)ge->toString().c_str());
		delete ge;
		return 2;
	}

	catch (std::exception &e)
	{
		ERROR_MESSAGE(e.what());
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
	catch (GEOSException *ge)
	{
		ERROR_MESSAGE((char *)ge->toString().c_str());
		delete ge;
		return 2;
	}

	catch (std::exception &e)
	{
		ERROR_MESSAGE(e.what());
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
	catch (GEOSException *ge)
	{
		ERROR_MESSAGE((char *)ge->toString().c_str());
		delete ge;
		return 2;
	}

	catch (std::exception &e)
	{
		ERROR_MESSAGE(e.what());
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
	catch (GEOSException *ge)
	{
		ERROR_MESSAGE((char *)ge->toString().c_str());
		delete ge;
		return 2;
	}

	catch (std::exception &e)
	{
		ERROR_MESSAGE(e.what());
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
	catch (GEOSException *ge)
	{
		ERROR_MESSAGE((char *)ge->toString().c_str());
		delete ge;
		return 2;
	}

	catch (std::exception &e)
	{
		ERROR_MESSAGE(e.what());
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
		string s = pat;
		result = g1->relate(g2,pat);
		return result;
	}
	catch (GEOSException *ge)
	{
		ERROR_MESSAGE((char *)ge->toString().c_str());
		delete ge;
		return 2;
	}

	catch (std::exception &e)
	{
		ERROR_MESSAGE(e.what());
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

	try {

		IntersectionMatrix *im = g1->relate(g2);

		string s;
		char *result;
		if (im == NULL)
				return NULL;

		s= im->toString();
		result = (char*) malloc( s.length() + 1);
		strcpy(result, s.c_str() );
		delete im;

		return result;
	}
	catch (GEOSException *ge)
	{
		ERROR_MESSAGE((char *)ge->toString().c_str());
		delete ge;
		return NULL;
	}

	catch (std::exception &e)
	{
		ERROR_MESSAGE(e.what());
		return NULL;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}



//-----------------------------------------------------------------
// isValid
//-----------------------------------------------------------------


char
GEOSisValid(const Geometry *g1)
{
#if GEOS_FIRST_INTERFACE <= 3 && GEOS_LAST_INTERFACE >= 3
	IsValidOp ivo(g1);
#endif
	bool result;
	try {
#if GEOS_FIRST_INTERFACE <= 3 && GEOS_LAST_INTERFACE >= 3
		result = ivo.isValid();
		if ( result == 0 )
		{
			TopologyValidationError *err = ivo.getValidationError();
			if ( err ) {
				string errmsg = err->getMessage();
				NOTICE_MESSAGE((char *)errmsg.c_str());
			}
		}
#else // GEOS_INTERFACE 3 not supported
		result = g1->isValid();
#endif
		return result;
	}
	catch (GEOSException *ge)
	{
		ERROR_MESSAGE((char *)ge->toString().c_str());
		delete ge;
		return 2;
	}

	catch (std::exception &e)
	{
		ERROR_MESSAGE(e.what());
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
	catch (GEOSException *ge)
	{
		ERROR_MESSAGE((char *)ge->toString().c_str());
		delete ge;
		return 2;
	}

	catch (std::exception &e)
	{
		ERROR_MESSAGE(e.what());
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
	catch (GEOSException *ge)
	{
		ERROR_MESSAGE((char *)ge->toString().c_str());
		delete ge;
		return 0;
	}

	catch (std::exception &e)
	{
		ERROR_MESSAGE(e.what());
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
	try
	{
		WKTReader r(geomFactory);
		const string wktstring = string(wkt);
		Geometry *g = r.read(wktstring);
		return g;
	}
	catch (GEOSException *ge)
	{
		ERROR_MESSAGE((char *)ge->toString().c_str());
		delete ge;
		return NULL;
	}

	catch (std::exception &e)
	{
		ERROR_MESSAGE(e.what());
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
	try
	{
		string s = g1->toString();


		char *result;
		result = (char*) malloc( s.length() + 1);
		strcpy(result, s.c_str() );
		return result;
	}
	catch (GEOSException *ge)
	{
		ERROR_MESSAGE((char *)ge->toString().c_str());
		delete ge;
		return NULL;
	}

	catch (std::exception &e)
	{
		ERROR_MESSAGE(e.what());
		return NULL;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}

// Remember to free the result!
char *
GEOSGeomToWKB_buf(const Geometry *g, size_t *size)
{
	try
	{
		WKBWriter w(WKBOutputDims);
		ostringstream s(ios_base::binary);
		w.write(*g, s);
		string wkbstring = s.str();
		size_t len = wkbstring.length();

		char *result;
		result = (char*) malloc(len);
		memcpy(result, wkbstring.c_str(), len);
		*size = len;
		return result;
	}
	catch (GEOSException *ge)
	{
		ERROR_MESSAGE((char *)ge->toString().c_str());
		delete ge;
		return NULL;
	}

	catch (std::exception &e)
	{
		ERROR_MESSAGE(e.what());
		return NULL;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}

Geometry *
GEOSGeomFromWKB_buf(const char *wkb, size_t size)
{
	try
	{
		string wkbstring = string(wkb, size); // make it binary !
		WKBReader r(*geomFactory);
		istringstream s(ios_base::binary);
		s.str(wkbstring);

		s.seekg(0, ios::beg); // rewind reader pointer
		Geometry *g = r.read(s);
		return g;
	}
	catch (GEOSException *ge)
	{
		ERROR_MESSAGE((char *)ge->toString().c_str());
		delete ge;
		return NULL;
	}

	catch (std::exception &e)
	{
		ERROR_MESSAGE(e.what());
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
	catch (GEOSException *ge)
	{
		ERROR_MESSAGE((char *)ge->toString().c_str());
		delete ge;
		return 2;
	}

	catch (std::exception &e)
	{
		ERROR_MESSAGE(e.what());
		return 2;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return 2;
	}
}

char
GEOSisSimple(Geometry *g1)
{
	try
	{
		return g1->isSimple();
	}
	catch (GEOSException *ge)
	{
		ERROR_MESSAGE((char *)ge->toString().c_str());
		delete ge;
		return 2;
	}

	catch (std::exception &e)
	{
		ERROR_MESSAGE(e.what());
		return 2;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return 2;
	}
}

char
GEOSisRing(Geometry *g)
{
	try
	{
		LineString *ls = dynamic_cast<LineString *>(g);
		if ( ls ) {
			return (ls->isRing());
		} else {
			return 0;
		}
	}
	catch (GEOSException *ge)
	{
		ERROR_MESSAGE((char *)ge->toString().c_str());
		delete ge;
		return 2;
	}

	catch (std::exception &e)
	{
		ERROR_MESSAGE(e.what());
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
GEOSGeomType(Geometry *g1)
{
	try
	{
		string s = g1->getGeometryType();


		char *result;
		result = (char*) malloc( s.length() + 1);
		strcpy(result, s.c_str() );
		return result;
	}
	catch (GEOSException *ge)
	{
		ERROR_MESSAGE((char *)ge->toString().c_str());
		delete ge;
		return NULL;
	}

	catch (std::exception &e)
	{
		ERROR_MESSAGE(e.what());
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
GEOSGeomTypeId(Geometry *g1)
{
	try
	{
		return g1->getGeometryTypeId();
	}
	catch (GEOSException *ge)
	{
		ERROR_MESSAGE((char *)ge->toString().c_str());
		delete ge;
		return -1;
	}

	catch (std::exception &e)
	{
		ERROR_MESSAGE(e.what());
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
GEOSIntersection(Geometry *g1, Geometry *g2)
{
	try
	{
		Geometry *g3 = g1->intersection(g2);
		return g3;
	}
	catch (GEOSException *ge)
	{
		ERROR_MESSAGE((char *)ge->toString().c_str());
		delete ge;
		return NULL;
	}

	catch (std::exception &e)
	{
		ERROR_MESSAGE(e.what());
		return NULL;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}

Geometry *
GEOSBuffer(Geometry *g1, double width, int quadrantsegments)
{
	try
	{
		Geometry *g3 = g1->buffer(width, quadrantsegments);
		return g3;
	}
	catch (GEOSException *ge)
	{
		ERROR_MESSAGE((char *)ge->toString().c_str());
		delete ge;
		return NULL;
	}

	catch (std::exception &e)
	{
		ERROR_MESSAGE(e.what());
		return NULL;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}

Geometry *
GEOSConvexHull(Geometry *g1)
{
	try
	{
		Geometry *g3 = g1->convexHull();
		return g3;
	}
	catch (GEOSException *ge)
	{
		ERROR_MESSAGE((char *)ge->toString().c_str());
		delete ge;
		return NULL;
	}

	catch (std::exception &e)
	{
		ERROR_MESSAGE(e.what());
		return NULL;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}

Geometry *
GEOSDifference(Geometry *g1, Geometry *g2)
{
	try
	{
		Geometry *g3 = g1->difference(g2);
		return g3;
	}
	catch (GEOSException *ge)
	{
		ERROR_MESSAGE((char *)ge->toString().c_str());
		delete ge;
		return NULL;
	}

	catch (std::exception &e)
	{
		ERROR_MESSAGE(e.what());
		return NULL;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}

Geometry *
GEOSBoundary(Geometry *g1)
{
	try
	{
		Geometry *g3 = g1->getBoundary();
		return g3;
	}
	catch (GEOSException *ge)
	{
		ERROR_MESSAGE((char *)ge->toString().c_str());
		delete ge;
		return NULL;
	}

	catch (std::exception &e)
	{
		ERROR_MESSAGE(e.what());
		return NULL;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}

Geometry *
GEOSSymDifference(Geometry *g1, Geometry *g2)
{
	try
	{
		Geometry *g3 = g1->symDifference(g2);
		return g3;
	}
	catch (GEOSException *ge)
	{
		ERROR_MESSAGE((char *)ge->toString().c_str());
		delete ge;
		return NULL;
	}

	catch (std::exception &e)
	{
		ERROR_MESSAGE(e.what());
		return NULL;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}

Geometry *
GEOSUnion(Geometry *g1, Geometry *g2)
{
	try
	{
		Geometry *g3 = g1->Union(g2);
		return g3;
	}
	catch (GEOSException *ge)
	{
		ERROR_MESSAGE((char *)ge->toString().c_str());
		delete ge;
		return NULL;
	}
	catch (std::exception &e)
	{
		ERROR_MESSAGE(e.what());
		return NULL;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}


Geometry *
GEOSPointOnSurface(Geometry *g1)
{
	try
	{
		Geometry *g3 = g1->getInteriorPoint();
		return g3;
	}
	catch (GEOSException *ge)
	{
		ERROR_MESSAGE((char *)ge->toString().c_str());
		delete ge;
		return NULL;
	}

	catch (std::exception &e)
	{
		ERROR_MESSAGE(e.what());
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
	catch (GEOSException *ge)
	{
		ERROR_MESSAGE((char *)ge->toString().c_str());
		delete ge;
	}

	catch (std::exception &e)
	{
		ERROR_MESSAGE(e.what());
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
	catch (GEOSException *ge) // ???
	{
		ERROR_MESSAGE((char *)ge->toString().c_str());
		delete ge;
	}

	catch (std::exception &e)
	{
		ERROR_MESSAGE(e.what());
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
	catch (GEOSException *ge)
	{
		ERROR_MESSAGE((char *)ge->toString().c_str());
		delete ge;
		return -1;
	}

	catch (std::exception &e)
	{
		ERROR_MESSAGE(e.what());
		return -1;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return -1;
	}
}

int
GEOSGetNumInteriorRings(Geometry *g1)
{
	try{
		Polygon *p = (Polygon *) g1;
		return p->getNumInteriorRing();
	}
	catch (GEOSException *ge)
	{
		ERROR_MESSAGE((char *)ge->toString().c_str());
		delete ge;
		return -1;
	}

	catch (std::exception &e)
	{
		ERROR_MESSAGE(e.what());
		return -1;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return -1;
	}
}


//only call on GCs (or multi*)
int
GEOSGetNumGeometries(Geometry *g1)
{
	try{
		GeometryCollection *gc = (GeometryCollection *) g1;
		return gc->getNumGeometries();
	}
	catch (GEOSException *ge)
	{
		ERROR_MESSAGE((char *)ge->toString().c_str());
		delete ge;
		return -1;
	}

	catch (std::exception &e)
	{
		ERROR_MESSAGE(e.what());
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
GEOSGetGeometryN(Geometry *g1, int n)
{
	try{
		const GeometryCollection *gc = dynamic_cast<GeometryCollection *>(g1);
		if ( ! gc )
		{
			ERROR_MESSAGE("Argument is not a GeometryCollection");
			return NULL;
		}
		return gc->getGeometryN(n);
	}
	catch (GEOSException *ge)
	{
		ERROR_MESSAGE((char *)ge->toString().c_str());
		delete ge;
		return NULL;
	}

	catch (std::exception &e)
	{
		ERROR_MESSAGE(e.what());
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
GEOSGetExteriorRing(Geometry *g1)
{
	try{
		Polygon *p = dynamic_cast<Polygon *>(g1);
		if ( ! p ) 
		{
			ERROR_MESSAGE("Invalid argument (must be a Polygon)");
			return NULL;
		}
		return p->getExteriorRing();
	}
	catch (GEOSException *ge)
	{
		ERROR_MESSAGE((char *)ge->toString().c_str());
		delete ge;
		return NULL;
	}

	catch (std::exception &e)
	{
		ERROR_MESSAGE(e.what());
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
GEOSGetInteriorRingN(Geometry *g1, int n)
{
	try{
		Polygon *p = dynamic_cast<Polygon *>(g1);
		if ( ! p ) 
		{
			ERROR_MESSAGE("Invalid argument (must be a Polygon)");
			return NULL;
		}
		return p->getInteriorRingN(n);
	}
	catch (GEOSException *ge)
	{
		ERROR_MESSAGE((char *)ge->toString().c_str());
		delete ge;
		return NULL;
	}

	catch (std::exception &e)
	{
		ERROR_MESSAGE(e.what());
		return NULL;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}

Geometry *
GEOSGetCentroid(Geometry *g)
{
	try{
		Geometry *ret = g->getCentroid();
		if ( ! ret ) return geomFactory->createGeometryCollection();
		return ret;
	}
	catch (GEOSException *ge)
	{
		ERROR_MESSAGE((char *)ge->toString().c_str());
		delete ge;
		return NULL;
	}

	catch (std::exception &e)
	{
		ERROR_MESSAGE(e.what());
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
#ifdef DEBUG
	char buf[256];
	sprintf(buf, "PostGIS2GEOS_collection: requested type %d, ngeoms: %d",
			type, ngeoms);
	ERROR_MESSAGE(buf);
#endif

	try
	{
		Geometry *g;
		vector<Geometry *> *vgeoms=new vector<Geometry *>(geoms, geoms+ngeoms);

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
	catch (GEOSException *ge)
	{
		ERROR_MESSAGE((char *)ge->toString().c_str());
		delete ge;
		return NULL;
	}

	catch (std::exception &e)
	{
		ERROR_MESSAGE(e.what());
		return NULL;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}

Geometry *
GEOSPolygonize(Geometry **g, unsigned int ngeoms)
{
	unsigned int i;
	Geometry *out = NULL;

	// construct vector
	vector<Geometry *> *geoms = new vector<Geometry *>(ngeoms);
	for (i=0; i<ngeoms; i++) (*geoms)[i] = g[i];

#if DEBUG
	ERROR_MESSAGE("geometry vector constructed");
#endif

	try{
		// Polygonize
		Polygonizer plgnzr;
		plgnzr.add(geoms);
#if DEBUG
	ERROR_MESSAGE("geometry vector added to polygonizer");
#endif

		vector<Polygon *>*polys = plgnzr.getPolygons();

#if DEBUG
	ERROR_MESSAGE("output polygons got");
#endif

		delete geoms;

#if DEBUG
	ERROR_MESSAGE("geometry vector deleted");
#endif

		geoms = new vector<Geometry *>(polys->size());
		for (i=0; i<polys->size(); i++) (*geoms)[i] = (*polys)[i];
		delete polys;
		out = geomFactory->createGeometryCollection(geoms);
	}
	catch (GEOSException *ge)
	{
		ERROR_MESSAGE((char *)ge->toString().c_str());
		delete ge;
		return NULL;
	}

	catch (std::exception &e)
	{
		ERROR_MESSAGE(e.what());
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
GEOSLineMerge(Geometry *g)
{
        unsigned int i;
        Geometry *out = NULL;

        try{
                // LineMerge
                LineMerger lmrgr;

                lmrgr.add(g);

                vector<LineString *>*lines = lmrgr.getMergedLineStrings();

#if DEBUG
        ERROR_MESSAGE("output lines got");
#endif

                vector<Geometry *>*geoms = new vector<Geometry *>(lines->size());
                for (i=0; i<lines->size(); i++) (*geoms)[i] = (*lines)[i];
                delete lines;
                out = geomFactory->buildGeometry(geoms);
                //out = geomFactory->createGeometryCollection(geoms);
        }
        catch (GEOSException *ge)
        {
                ERROR_MESSAGE((char *)ge->toString().c_str());
                delete ge;
                return NULL;
        }

	catch (std::exception &e)
	{
		ERROR_MESSAGE(e.what());
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
GEOSGetSRID(Geometry *g1)
{
	try{
		return g1->getSRID();
	}
	catch (GEOSException *ge)
	{
		ERROR_MESSAGE((char *)ge->toString().c_str());
		delete ge;
		return 0;
	}

	catch (std::exception &e)
	{
		ERROR_MESSAGE(e.what());
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

char *
GEOSjtsport()
{
#if GEOS_LAST_INTERFACE < 2
	/*
	 * GEOS upgrade needs postgis re-build, so this static
	 * assignment is not going to be a problem
	 */
	char *res = strdup("1.3");
#else
	string version = jtsport();
	char *res = strdup(version.c_str());
#endif
	return res;
}


bool 
GEOSHasZ(Geometry *g)
{
	//char msg[256];
	if ( g->isEmpty() ) return false;
	double az = g->getCoordinate()->z;
	//sprintf(msg, "ZCoord: %g", az);
	//ERROR_MESSAGE(msg);
	return (finite(az) && az != DoubleNotANumber);
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

CoordinateSequence *
GEOSCoordSeq_create(unsigned int size, unsigned int dims)
{
	try {
		return geomFactory->getCoordinateSequenceFactory()->create(size, static_cast<int>(dims));
	}

	catch (GEOSException *ge)
	{
		ERROR_MESSAGE((char *)ge->toString().c_str());
		delete ge;
		return NULL;
	}
	catch (std::exception &e)
	{
		ERROR_MESSAGE(e.what());
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
	catch (GEOSException *ge)
	{
		ERROR_MESSAGE((char *)ge->toString().c_str());
		delete ge;
		return 0;
	}
	catch (std::exception &e)
	{
		ERROR_MESSAGE(e.what());
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
GEOSCoordSeq_clone(CoordinateSequence *s)
{
	try { return s->clone(); }
	catch (GEOSException *ge)
	{
		ERROR_MESSAGE((char *)ge->toString().c_str());
		delete ge;
		return NULL;
	}
	catch (std::exception &e)
	{
		ERROR_MESSAGE(e.what());
		return NULL;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}

int
GEOSCoordSeq_getOrdinate(CoordinateSequence *s, unsigned int idx,
	unsigned int dim, double *val)
{
	try {
		double d = s->getOrdinate(static_cast<int>(idx),
			static_cast<int>(dim));
		*val = d;
		return 1;
	}
	catch (GEOSException *ge)
	{
		ERROR_MESSAGE((char *)ge->toString().c_str());
		delete ge;
		return 0;
	}
	catch (std::exception &e)
	{
		ERROR_MESSAGE(e.what());
		return 0;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return 0;
	}
}

int
GEOSCoordSeq_getX(CoordinateSequence *s, unsigned int idx, double *val)
{
	return GEOSCoordSeq_getOrdinate(s, idx, 0, val);
}

int
GEOSCoordSeq_getY(CoordinateSequence *s, unsigned int idx, double *val)
{
	return GEOSCoordSeq_getOrdinate(s, idx, 1, val);
}

int
GEOSCoordSeq_getZ(CoordinateSequence *s, unsigned int idx, double *val)
{
	return GEOSCoordSeq_getOrdinate(s, idx, 2, val);
}

int
GEOSCoordSeq_getSize(CoordinateSequence *s, unsigned int *size)
{
	try {
		int sz = s->getSize();
		*size = static_cast<unsigned int>(sz);
		return 1;
	}
	catch (GEOSException *ge)
	{
		ERROR_MESSAGE((char *)ge->toString().c_str());
		delete ge;
		return 0;
	}
	catch (std::exception &e)
	{
		ERROR_MESSAGE(e.what());
		return 0;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return 0;
	}
}

int
GEOSCoordSeq_getDimensions(CoordinateSequence *s, unsigned int *dims)
{
	try {
		unsigned int dm = s->getDimension();
		*dims = dm;
		return 1;
	}
	catch (GEOSException *ge)
	{
		ERROR_MESSAGE((char *)ge->toString().c_str());
		delete ge;
		return 0;
	}
	catch (std::exception &e)
	{
		ERROR_MESSAGE(e.what());
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
	catch (GEOSException *ge)
	{
		ERROR_MESSAGE((char *)ge->toString().c_str());
		delete ge;
	}

	catch (std::exception &e)
	{
		ERROR_MESSAGE(e.what());
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
	}
}

CoordinateSequence *
GEOSGeom_getCoordSeq(Geometry *g)
{
	try
	{
		LineString *ls = dynamic_cast<LineString *>(g);
		if ( ls )
		{
	return const_cast<CoordinateSequence *>(ls->getCoordinatesRO());
		}
		Point *p = dynamic_cast<Point *>(g);
		if ( p ) 
		{
	return const_cast<CoordinateSequence *>(p->getCoordinatesRO());
		}
		ERROR_MESSAGE("Geometry must be a Point or LineString");
		return NULL;
	}
	catch (GEOSException *ge)
	{
		ERROR_MESSAGE((char *)ge->toString().c_str());
		delete ge;
		return NULL;
	}
	catch (std::exception &e)
	{
		ERROR_MESSAGE(e.what());
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
	catch (GEOSException *ge)
	{
		ERROR_MESSAGE((char *)ge->toString().c_str());
		delete ge;
		return NULL;
	}
	catch (std::exception &e)
	{
		ERROR_MESSAGE(e.what());
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
	catch (GEOSException *ge)
	{
		ERROR_MESSAGE((char *)ge->toString().c_str());
		delete ge;
		return NULL;
	}
	catch (std::exception &e)
	{
		ERROR_MESSAGE(e.what());
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
	catch (GEOSException *ge)
	{
		ERROR_MESSAGE((char *)ge->toString().c_str());
		delete ge;
		return NULL;
	}
	catch (std::exception &e)
	{
		ERROR_MESSAGE(e.what());
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
	try
	{
		vector<Geometry *> *vholes=new vector<Geometry *>(holes, holes+nholes);
		LinearRing *nshell = dynamic_cast<LinearRing *>(shell);
		if ( ! nshell )
		{
			ERROR_MESSAGE("Shell is not a LinearRing");
			return NULL;
		}
		return geomFactory->createPolygon(nshell, vholes);
	}
	catch (GEOSException *ge)
	{
		ERROR_MESSAGE((char *)ge->toString().c_str());
		delete ge;
		return NULL;
	}
	catch (std::exception &e)
	{
		ERROR_MESSAGE(e.what());
		return NULL;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}

Geometry *
GEOSGeom_clone(Geometry *g)
{
	try { return g->clone(); }
	catch (GEOSException *ge)
	{
		ERROR_MESSAGE((char *)ge->toString().c_str());
		delete ge;
		return NULL;
	}
	catch (std::exception &e)
	{
		ERROR_MESSAGE(e.what());
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
			return GEOSGeom_getDimensions(coll->getGeometryN(0));
		}

		ERROR_MESSAGE("Unknown geometry type");
		return 0;
	}

	catch (GEOSException *ge)
	{
		ERROR_MESSAGE((char *)ge->toString().c_str());
		delete ge;
		return 0;
	}
	catch (std::exception &e)
	{
		ERROR_MESSAGE(e.what());
		return 0;
	}

	catch (...)
	{
		ERROR_MESSAGE("Unknown exception thrown");
		return 0;
	}
}
