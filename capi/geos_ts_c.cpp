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
 *    Thread Safety modifications: Chuck Thibert <charles.thibert@ingres.com>
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
#include <geos/operation/union/CascadedPolygonUnion.h>
#include <geos/geom/BinaryOp.h>
#include <geos/version.h> 

// This should go away
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <memory>

// Some extra magic to make type declarations in geos_c.h work - 
// for cross-checking of types in header.
#define GEOSGeometry geos::geom::Geometry
#define GEOSPreparedGeometry geos::geom::prep::PreparedGeometry
#define GEOSCoordSequence geos::geom::CoordinateSequence
#define GEOSWKTReader_t geos::io::WKTReader
#define GEOSWKTWriter_t geos::io::WKTWriter
#define GEOSWKBReader_t geos::io::WKBReader
#define GEOSWKBWriter_t geos::io::WKBWriter

#include "geos_c.h"

/// Define this if you want operations triggering Exceptions to
/// be printed.
/// (will use the NOTIFY channel - only implemented for GEOSUnion so far)
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
using geos::operation::geounion::CascadedPolygonUnion;

typedef std::auto_ptr<Geometry> GeomAutoPtr;

typedef struct GEOSContextHandleInternal
{
    const GeometryFactory *geomFactory;
    GEOSMessageHandler NOTICE_MESSAGE;
    GEOSMessageHandler ERROR_MESSAGE;
    int WKBOutputDims;
    int WKBByteOrder;
    int initialized;
} GEOSContextHandleInternal_t;

//## PROTOTYPES #############################################

extern "C" const char GEOS_DLL *GEOSjtsport();
extern "C" char GEOS_DLL *GEOSasText(Geometry *g1);

extern "C" {

GEOSContextHandle_t
initGEOS_r(GEOSMessageHandler nf, GEOSMessageHandler ef)
{
    GEOSContextHandleInternal_t *handle;
    void *extHandle;

    extHandle = malloc(sizeof(GEOSContextHandleInternal_t));

    if(extHandle != NULL)
    {
        handle = (GEOSContextHandleInternal_t*)extHandle;
        handle->NOTICE_MESSAGE = nf;
        handle->ERROR_MESSAGE = ef;
        handle->geomFactory = GeometryFactory::getDefaultInstance();
        handle->WKBOutputDims = 2;
        handle->WKBByteOrder = getMachineByteOrder();
        handle->initialized = 1;
    }

    return (GEOSContextHandle_t)extHandle;
}

void
finishGEOS_r(GEOSContextHandle_t extHandle)
{
    //Fix up freeing handle w.r.t. malloc above

    if( extHandle == NULL )
    {
        return;
    }
    free(extHandle);
    extHandle = NULL;
}

//-----------------------------------------------------------
// relate()-related functions
//  return 0 = false, 1 = true, 2 = error occured
//-----------------------------------------------------------

char
GEOSDisjoint_r(GEOSContextHandle_t extHandle, const Geometry *g1, const Geometry *g2)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return 2;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return 2;
    }

	try {
		bool result;
		result = g1->disjoint(g2);
		return result;
	}
	catch (const std::exception &e)
	{
		handle->ERROR_MESSAGE("%s", e.what());
		return 2;
	}

	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
		return 2;
	}
}

char
GEOSTouches_r(GEOSContextHandle_t extHandle, const Geometry *g1, const Geometry *g2)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return 2;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return 2;
    }

	try {
		bool result;
		result =  g1->touches(g2);
		return result;
	}

	catch (const std::exception &e)
	{
		handle->ERROR_MESSAGE("%s", e.what());
		return 2;
	}

	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
		return 2;
	}
}

char
GEOSIntersects_r(GEOSContextHandle_t extHandle, const Geometry *g1, const Geometry *g2)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return 2;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return 2;
    }

	try {
		bool result;
		result = g1->intersects(g2);
		return result;
	}

	catch (const std::exception &e)
	{
		handle->ERROR_MESSAGE("%s", e.what());
		return 2;
	}

	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
		return 2;
	}
}

char
GEOSCrosses_r(GEOSContextHandle_t extHandle, const Geometry *g1, const Geometry *g2)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return 2;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return 2;
    }

	try {
		bool result;
		result = g1->crosses(g2);
		return result;
	}

	catch (const std::exception &e)
	{
		handle->ERROR_MESSAGE("%s", e.what());
		return 2;
	}

	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
		return 2;
	}
}

char
GEOSWithin_r(GEOSContextHandle_t extHandle, const Geometry *g1, const Geometry *g2)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return 2;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return 2;
    }

	try {
		bool result;
		result = g1->within(g2);
		return result;
	}

	catch (const std::exception &e)
	{
		handle->ERROR_MESSAGE("%s", e.what());
		return 2;
	}

	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
		return 2;
	}
}

// call g1->contains(g2)
// returns 0 = false
//         1 = true
//         2 = error was trapped
char
GEOSContains_r(GEOSContextHandle_t extHandle, const Geometry *g1, const Geometry *g2)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return 2;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return 2;
    }

	try {
		bool result;
		result = g1->contains(g2);
		return result;
	}

	catch (const std::exception &e)
	{
		handle->ERROR_MESSAGE("%s", e.what());
		return 2;
	}

	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
		return 2;
	}
}

char
GEOSOverlaps_r(GEOSContextHandle_t extHandle, const Geometry *g1, const Geometry *g2)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return 2;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return 2;
    }

	try {
		bool result;
		result = g1->overlaps(g2);
		return result;
	}

	catch (const std::exception &e)
	{
		handle->ERROR_MESSAGE("%s", e.what());
		return 2;
	}

	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
		return 2;
	}
}


//-------------------------------------------------------------------
// low-level relate functions
//------------------------------------------------------------------

char
GEOSRelatePattern_r(GEOSContextHandle_t extHandle, const Geometry *g1, const Geometry *g2, const char *pat)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return 2;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return 2;
    }

	try {
		bool result;
		std::string s = pat;
		result = g1->relate(g2,s);
		return result;
	}

	catch (const std::exception &e)
	{
		handle->ERROR_MESSAGE("%s", e.what());
		return 2;
	}

	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
		return 2;
	}
}

char *
GEOSRelate_r(GEOSContextHandle_t extHandle, const Geometry *g1, const Geometry *g2)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return NULL;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return NULL;
    }

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
		handle->ERROR_MESSAGE("%s", e.what());
	}
	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
	}
    
    return NULL;
}



//-----------------------------------------------------------------
// isValid
//-----------------------------------------------------------------


char
GEOSisValid_r(GEOSContextHandle_t extHandle, const Geometry *g1)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return 2;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return 2;
    }

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
				handle->NOTICE_MESSAGE("%s", errmsg.c_str());
			}
		}
		return result;
	}

	catch (const std::exception &e)
	{
		handle->ERROR_MESSAGE("%s", e.what());
		return 2;
	}

	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
		return 2;
	}

}

char *
GEOSisValidReason_r(GEOSContextHandle_t extHandle, const Geometry *g1)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return NULL;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return NULL;
    }

    using geos::operation::valid::IsValidOp;
    using geos::operation::valid::TopologyValidationError;
    IsValidOp ivo(g1);
    try {
        char *result = NULL;
		char *validstr = "Valid Geometry";
		const std::size_t validstrlen = std::strlen(validstr) + 1;
        bool isvalid = ivo.isValid();
        if ( ! isvalid )
        {
            TopologyValidationError *err = ivo.getValidationError();
            std::string errmsg = err->getMessage();
            std::string errloc = err->getCoordinate().toString();
            const std::size_t msglen = errmsg.length();
            const std::size_t loclen = errloc.length();
            result = (char*)std::malloc(msglen + loclen + 3);
            sprintf(result, "%s [%s]", errmsg.c_str(), errloc.c_str());
        }
        else {
			result = (char*)std::malloc(validstrlen);
			std::memcpy(result, validstr, validstrlen);
        }
        return result;
    }

    catch (const std::exception &e)
    {
        handle->ERROR_MESSAGE("%s", e.what());
        return NULL;
    }

    catch (...)
    {
        handle->ERROR_MESSAGE("Unknown exception thrown");
        return NULL;
    }

}

//-----------------------------------------------------------------
// general purpose
//-----------------------------------------------------------------

char
GEOSEquals_r(GEOSContextHandle_t extHandle, const Geometry *g1, const Geometry *g2)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return 2;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return 2;
    }

	try {
		bool result;
		result = g1->equals(g2);
		return result;
	}

	catch (const std::exception &e)
	{
		handle->ERROR_MESSAGE("%s", e.what());
		return 2;
	}

	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
		return 2;
	}
}

char
GEOSEqualsExact_r(GEOSContextHandle_t extHandle, const Geometry *g1, const Geometry *g2, double tolerance)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return 2;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return 2;
    }

	try {
		bool result;
		result = g1->equalsExact(g2, tolerance);
		return result;
	}

	catch (const std::exception &e)
	{
		handle->ERROR_MESSAGE("%s", e.what());
		return 2;
	}

	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
		return 2;
	}
}

int
GEOSDistance_r(GEOSContextHandle_t extHandle, const Geometry *g1, const Geometry *g2, double *dist)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return 0;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return 0;
    }

	try {
		*dist = g1->distance(g2);
		return 1;
	}

	catch (const std::exception &e)
	{
		handle->ERROR_MESSAGE("%s", e.what());
		return 0;
	}

	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
		return 0;
	}
}

int
GEOSArea_r(GEOSContextHandle_t extHandle, const Geometry *g, double *area)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return 0;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return 0;
    }

	try {
		*area = g->getArea();
		return 1;
	}

	catch (const std::exception &e)
	{
		handle->ERROR_MESSAGE("%s", e.what());
		return 0;
	}

	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
		return 0;
	}
}

int
GEOSLength_r(GEOSContextHandle_t extHandle, const Geometry *g, double *length)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return 2;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return 0;
    }

	try {
		*length = g->getLength();
		return 1;
	}

	catch (const std::exception &e)
	{
		handle->ERROR_MESSAGE("%s", e.what());
		return 0;
	}

	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
		return 0;
	}
}

Geometry *
GEOSGeomFromWKT_r(GEOSContextHandle_t extHandle, const char *wkt)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return NULL;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return NULL;
    }

    CLocalizer clocale;
	try
	{
		WKTReader r((GeometryFactory*)handle->geomFactory);
		const std::string wktstring = std::string(wkt);
		Geometry *g = r.read(wktstring);
		return g;
	}
	catch (const std::exception &e)
	{
		handle->ERROR_MESSAGE("%s", e.what());
		return NULL;
	}

	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}

char *
GEOSGeomToWKT_r(GEOSContextHandle_t extHandle, const Geometry *g1)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return NULL;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return NULL;
    }

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
		handle->ERROR_MESSAGE("%s", e.what());
		return NULL;
	}

	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}

// Remember to free the result!
unsigned char *
GEOSGeomToWKB_buf_r(GEOSContextHandle_t extHandle, const Geometry *g, size_t *size)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return NULL;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return NULL;
    }

	using geos::io::WKBWriter;
	try
	{
		int byteOrder = (int) handle->WKBByteOrder;
		WKBWriter w(handle->WKBOutputDims, byteOrder);
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
		handle->ERROR_MESSAGE("%s", e.what());
		return NULL;
	}

	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}

Geometry *
GEOSGeomFromWKB_buf_r(GEOSContextHandle_t extHandle, const unsigned char *wkb, size_t size)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return NULL;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return NULL;
    }

	using geos::io::WKBReader;
	try
	{
		std::string wkbstring = std::string((const char*)wkb, size); // make it binary !
		WKBReader r(*(GeometryFactory*)handle->geomFactory);
		std::istringstream s(std::ios_base::binary);
		s.str(wkbstring);

		s.seekg(0, std::ios::beg); // rewind reader pointer
		Geometry *g = r.read(s);
		return g;
	}
	catch (const std::exception &e)
	{
		handle->ERROR_MESSAGE("%s", e.what());
		return NULL;
	}

	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}

/* Read/write wkb hex values.  Returned geometries are
   owned by the caller.*/
unsigned char *
GEOSGeomToHEX_buf_r(GEOSContextHandle_t extHandle, const Geometry *g, size_t *size)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return NULL;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return NULL;
    }

	using geos::io::WKBWriter;
	try
	{
        int byteOrder = (int) handle->WKBByteOrder;
		WKBWriter w(handle->WKBOutputDims, byteOrder);
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
		handle->ERROR_MESSAGE("%s", e.what());
		return NULL;
	}

	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}

Geometry *
GEOSGeomFromHEX_buf_r(GEOSContextHandle_t extHandle, const unsigned char *hex, size_t size)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return NULL;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return NULL;
    }

	using geos::io::WKBReader;
	try
	{
		std::string hexstring = std::string((const char*)hex, size); 
		WKBReader r(*(GeometryFactory*)handle->geomFactory);
		std::istringstream s(std::ios_base::binary);
		s.str(hexstring);

		s.seekg(0, std::ios::beg); // rewind reader pointer
		Geometry *g = r.readHEX(s);
		return g;
	}
	catch (const std::exception &e)
	{
		handle->ERROR_MESSAGE("%s", e.what());
		return NULL;
	}

	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}

char
GEOSisEmpty_r(GEOSContextHandle_t extHandle, const Geometry *g1)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return 2;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return 2;
    }

	try
	{
		return g1->isEmpty();
	}
	catch (const std::exception &e)
	{
		handle->ERROR_MESSAGE("%s", e.what());
		return 2;
	}

	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
		return 2;
	}
}

char
GEOSisSimple_r(GEOSContextHandle_t extHandle, const Geometry *g1)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return 2;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return 2;
    }

	try
	{
		return g1->isSimple();
	}
	catch (const std::exception &e)
	{
		handle->ERROR_MESSAGE("%s", e.what());
		return 2;
	}

	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
		return 2;
	}
}

char
GEOSisRing_r(GEOSContextHandle_t extHandle, const Geometry *g)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return 2;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return 2;
    }

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
		handle->ERROR_MESSAGE("%s", e.what());
		return 2;
	}

	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
		return 2;
	}
}



//free the result of this
char *
GEOSGeomType_r(GEOSContextHandle_t extHandle, const Geometry *g1)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return NULL;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return NULL;
    }

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
		handle->ERROR_MESSAGE("%s", e.what());
		return NULL;
	}

	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}

// Return postgis geometry type index
int
GEOSGeomTypeId_r(GEOSContextHandle_t extHandle, const Geometry *g1)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return -1;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return -1;
    }

	try
	{
		return g1->getGeometryTypeId();
	}
	catch (const std::exception &e)
	{
		handle->ERROR_MESSAGE("%s", e.what());
		return -1;
	}

	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
		return -1;
	}
}




//-------------------------------------------------------------------
// GEOS functions that return geometries
//-------------------------------------------------------------------

Geometry *
GEOSEnvelope_r(GEOSContextHandle_t extHandle, const Geometry *g1)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return NULL;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return NULL;
    }

	try
	{
		Geometry *g3 = g1->getEnvelope();
		return g3;
	}
	catch (const std::exception &e)
	{
		handle->ERROR_MESSAGE("%s", e.what());
		return NULL;
	}

	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}

Geometry *
GEOSIntersection_r(GEOSContextHandle_t extHandle, const Geometry *g1, const Geometry *g2)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return NULL;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return NULL;
    }

	try
	{
		GeomAutoPtr g3 = BinaryOp(g1, g2, overlayOp(OverlayOp::opINTERSECTION));
		return g3.release();
		//Geometry *g3 = g1->intersection(g2);
		//return g3;
	}
	catch (const std::exception &e)
	{
		handle->ERROR_MESSAGE("%s", e.what());
		return NULL;
	}

	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}

Geometry *
GEOSBuffer_r(GEOSContextHandle_t extHandle, const Geometry *g1, double width, int quadrantsegments)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return NULL;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return NULL;
    }

	try
	{
		Geometry *g3 = g1->buffer(width, quadrantsegments);
		return g3;
	}
	catch (const std::exception &e)
	{
		handle->ERROR_MESSAGE("%s", e.what());
		return NULL;
	}

	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}

Geometry *
GEOSConvexHull_r(GEOSContextHandle_t extHandle, const Geometry *g1)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return NULL;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return NULL;
    }

	try
	{
		Geometry *g3 = g1->convexHull();
		return g3;
	}
	catch (const std::exception &e)
	{
		handle->ERROR_MESSAGE("%s", e.what());
		return NULL;
	}

	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}

Geometry *
GEOSDifference_r(GEOSContextHandle_t extHandle, const Geometry *g1, const Geometry *g2)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return NULL;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return NULL;
    }

	try
	{
		GeomAutoPtr g3 = BinaryOp(g1, g2, overlayOp(OverlayOp::opDIFFERENCE));
		return g3.release();
		//Geometry *g3 = g1->difference(g2);
		//return g3;
	}
	catch (const std::exception &e)
	{
		handle->ERROR_MESSAGE("%s", e.what());
		return NULL;
	}

	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}

Geometry *
GEOSBoundary_r(GEOSContextHandle_t extHandle, const Geometry *g1)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return NULL;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return NULL;
    }

	try
	{
		Geometry *g3 = g1->getBoundary();
		return g3;
	}
	catch (const std::exception &e)
	{
		handle->ERROR_MESSAGE("%s", e.what());
		return NULL;
	}

	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}

Geometry *
GEOSSymDifference_r(GEOSContextHandle_t extHandle, const Geometry *g1, const Geometry *g2)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return NULL;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return NULL;
    }

	try
	{
		GeomAutoPtr g3 = BinaryOp(g1, g2, overlayOp(OverlayOp::opSYMDIFFERENCE));
		return g3.release();
		//Geometry *g3 = g1->symDifference(g2);
		//return g3;
	}
	catch (const std::exception &e)
	{
		handle->ERROR_MESSAGE("%s", e.what());
		return NULL;
	}

	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}

Geometry *
GEOSUnion_r(GEOSContextHandle_t extHandle, const Geometry *g1, const Geometry *g2)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return NULL;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return NULL;
    }

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
		handle->NOTICE_MESSAGE("%s", s.str().c_str());
#endif // VERBOSE_EXCEPTIONS
		handle->ERROR_MESSAGE("%s", e.what());
		return NULL;
	}

	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}

Geometry *
GEOSUnionCascaded_r(GEOSContextHandle_t extHandle, const Geometry *g1)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return NULL;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return NULL;
    }

	try{
		const geos::geom::MultiPolygon *p = dynamic_cast<const geos::geom::MultiPolygon *>(g1);
		if ( ! p ) 
		{
			handle->ERROR_MESSAGE("Invalid argument (must be a MultiPolygon)");
			return NULL;
		}
        using geos::operation::geounion::CascadedPolygonUnion;
		return CascadedPolygonUnion::Union(p);
	}
	catch (const std::exception &e)
	{
		handle->ERROR_MESSAGE("%s", e.what());
		return NULL;
	}
	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}

Geometry *
GEOSPointOnSurface_r(GEOSContextHandle_t extHandle, const Geometry *g1)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return NULL;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return NULL;
    }

	try
	{
		Geometry *ret = g1->getInteriorPoint();
		if ( ! ret )
                {
                    const GeometryFactory *gf;
                    gf=handle->geomFactory;
					// return an empty collection 
                    return gf->createGeometryCollection();
                }
		return ret;
	}
	catch (const std::exception &e)
	{
		handle->ERROR_MESSAGE("%s", e.what());
		return NULL;
	}

	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}





//-------------------------------------------------------------------
// memory management functions
//------------------------------------------------------------------


void
GEOSGeom_destroy_r(GEOSContextHandle_t extHandle, Geometry *a)
{
    GEOSContextHandleInternal_t *handle;

	try{
		delete a;
	}
	catch (const std::exception &e)
	{
        if( extHandle == NULL )
        {
            return;
        }

        handle = (GEOSContextHandleInternal_t*)extHandle;
        if( handle->initialized == 0 )
        {
            return;
        }

		handle->ERROR_MESSAGE("%s", e.what());
	}

	catch (...)
	{
        if( extHandle == NULL )
        {
            return;
        }

        handle = (GEOSContextHandleInternal_t*)extHandle;
        if( handle->initialized == 0 )
        {
            return;
        }

		handle->ERROR_MESSAGE("Unknown exception thrown");
	}
}

void
GEOSSetSRID(Geometry *g, int SRID)
{
	g->setSRID(SRID);
}


int
GEOSGetNumCoordinates_r(GEOSContextHandle_t extHandle, const Geometry *g1)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return -1;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return -1;
    }

	try{
		return static_cast<int>(g1->getNumPoints());
	}
	catch (const std::exception &e)
	{
		handle->ERROR_MESSAGE("%s", e.what());
		return -1;
	}

	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
		return -1;
	}
}

/*
 * Return -1 on exception, 0 otherwise. 
 * Converts Geometry to normal form (or canonical form).
 */
int
GEOSNormalize_r(GEOSContextHandle_t extHandle, Geometry *g1)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return -1;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return -1;
    }

	try{
		g1->normalize();
	}
	catch (const std::exception &e)
	{
		handle->ERROR_MESSAGE("%s", e.what());
		return -1;
	}

	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
		return -1;
	}
    return 0;
}

int
GEOSGetNumInteriorRings_r(GEOSContextHandle_t extHandle, const Geometry *g1)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return -1;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return -1;
    }

	try{
		const Polygon *p = dynamic_cast<const Polygon *>(g1);
                if ( ! p )
                {
                        handle->ERROR_MESSAGE("Argument is not a Polygon");
                        return -1;
                }
		return static_cast<int>(p->getNumInteriorRing());
	}
	catch (const std::exception &e)
	{
		handle->ERROR_MESSAGE("%s", e.what());
		return -1;
	}

	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
		return -1;
	}
}


// returns -1 on error and 1 for non-multi geometries
int
GEOSGetNumGeometries_r(GEOSContextHandle_t extHandle, const Geometry *g1)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return -1;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return -1;
    }

	try{
		return static_cast<int>(g1->getNumGeometries());
	}
	catch (const std::exception &e)
	{
		handle->ERROR_MESSAGE("%s", e.what());
		return -1;
	}

	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
		return -1;
	}
}


/*
 * Call only on GEOMETRYCOLLECTION or MULTI*.
 * Return a pointer to the internal Geometry.
 */
const Geometry *
GEOSGetGeometryN_r(GEOSContextHandle_t extHandle, const Geometry *g1, int n)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return NULL;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return NULL;
    }

	using geos::geom::GeometryCollection;
	try{
		const GeometryCollection *gc = dynamic_cast<const GeometryCollection *>(g1);
		if ( ! gc )
		{
			handle->ERROR_MESSAGE("Argument is not a GeometryCollection");
			return NULL;
		}
		return gc->getGeometryN(n);
	}
	catch (const std::exception &e)
	{
		handle->ERROR_MESSAGE("%s", e.what());
		return NULL;
	}

	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}


/*
 * Call only on polygon
 * Return a copy of the internal Geometry.
 */
const Geometry *
GEOSGetExteriorRing_r(GEOSContextHandle_t extHandle, const Geometry *g1)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return NULL;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return NULL;
    }

	try{
		const Polygon *p = dynamic_cast<const Polygon *>(g1);
		if ( ! p ) 
		{
			handle->ERROR_MESSAGE("Invalid argument (must be a Polygon)");
			return NULL;
		}
		return p->getExteriorRing();
	}
	catch (const std::exception &e)
	{
		handle->ERROR_MESSAGE("%s", e.what());
		return NULL;
	}

	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}

/*
 * Call only on polygon
 * Return a pointer to internal storage, do not destroy it.
 */
const Geometry *
GEOSGetInteriorRingN_r(GEOSContextHandle_t extHandle, const Geometry *g1, int n)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return NULL;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return NULL;
    }

	try{
		const Polygon *p = dynamic_cast<const Polygon *>(g1);
		if ( ! p ) 
		{
			handle->ERROR_MESSAGE("Invalid argument (must be a Polygon)");
			return NULL;
		}
		return p->getInteriorRingN(n);
	}
	catch (const std::exception &e)
	{
		handle->ERROR_MESSAGE("%s", e.what());
		return NULL;
	}

	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}



Geometry *
GEOSGetCentroid_r(GEOSContextHandle_t extHandle, const Geometry *g)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return NULL;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return NULL;
    }

	try{
		Geometry *ret = g->getCentroid();
                if ( ! ret )
                {
                    const GeometryFactory *gf;
                    gf=handle->geomFactory;

                    return gf->createGeometryCollection();
                }
		return ret;
	}
	catch (const std::exception &e)
	{
		handle->ERROR_MESSAGE("%s", e.what());
		return NULL;
	}

	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}

Geometry *
GEOSGeom_createCollection_r(GEOSContextHandle_t extHandle, int type, Geometry **geoms, unsigned int ngeoms)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return NULL;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return NULL;
    }

#ifdef GEOS_DEBUG
	char buf[256];
	sprintf(buf, "PostGIS2GEOS_collection: requested type %d, ngeoms: %d",
			type, ngeoms);
	handle->NOTICE_MESSAGE("%s", buf);// TODO: Can handle->NOTICE_MESSAGE format that directly? 
#endif

	try
	{
		Geometry *g;
        const GeometryFactory *gf;
        gf=handle->geomFactory;
		std::vector<Geometry *> *vgeoms = new std::vector<Geometry *>(geoms, geoms+ngeoms);

		switch (type)
		{
			case GEOS_GEOMETRYCOLLECTION:
				g = gf->createGeometryCollection(vgeoms);
				break;
			case GEOS_MULTIPOINT:
				g = gf->createMultiPoint(vgeoms);
				break;
			case GEOS_MULTILINESTRING:
				g = gf->createMultiLineString(vgeoms);
				break;
			case GEOS_MULTIPOLYGON:
				g = gf->createMultiPolygon(vgeoms);
				break;
			default:
				handle->ERROR_MESSAGE("Unsupported type request for PostGIS2GEOS_collection");
				g = NULL;
				
		}
		if (g==NULL) return NULL;
		return g;
	}
	catch (const std::exception &e)
	{
		handle->ERROR_MESSAGE("%s", e.what());
		return NULL;
	}

	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}

Geometry *
GEOSPolygonize_r(GEOSContextHandle_t extHandle, const Geometry * const * g, unsigned int ngeoms)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return NULL;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return NULL;
    }

	using geos::operation::polygonize::Polygonizer;
	unsigned int i;
	Geometry *out = NULL;

	try{
		// Polygonize
		Polygonizer plgnzr;
		for (i=0; i<ngeoms; i++) plgnzr.add(g[i]);
#if GEOS_DEBUG
	handle->NOTICE_MESSAGE("geometry vector added to polygonizer");
#endif

		std::vector<Polygon *>*polys = plgnzr.getPolygons();

#if GEOS_DEBUG
	handle->NOTICE_MESSAGE("output polygons got");
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

        const GeometryFactory *gf;
        gf=handle->geomFactory;

		out = gf->createGeometryCollection(polyvec);
		// the above method takes ownership of the passed
		// vector, so we must *not* delete it
	}
	catch (const std::exception &e)
	{
		handle->ERROR_MESSAGE("%s", e.what());
		return NULL;
	}
	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}

	return out;
}

Geometry *
GEOSPolygonizer_getCutEdges_r(GEOSContextHandle_t extHandle, const Geometry * const * g, unsigned int ngeoms)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return NULL;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return NULL;
    }

	using geos::operation::polygonize::Polygonizer;
	unsigned int i;
	Geometry *out = NULL;

	try{
		// Polygonize
		Polygonizer plgnzr;
		for (i=0; i<ngeoms; i++) plgnzr.add(g[i]);
#if GEOS_DEBUG
	handle->NOTICE_MESSAGE("geometry vector added to polygonizer");
#endif

		std::vector<const LineString *>*lines = plgnzr.getCutEdges();

#if GEOS_DEBUG
	handle->NOTICE_MESSAGE("output polygons got");
#endif

		// We need a vector of Geometry pointers, not
		// Polygon pointers.
		// STL vector doesn't allow transparent upcast of this
		// nature, so we explicitly convert.
		// (it's just a waste of processor and memory, btw)
    std::vector<Geometry*> *linevec =
				new std::vector<Geometry *>(lines->size());
		for (i=0; i<lines->size(); i++) (*linevec)[i] = (*lines)[i]->clone();

        const GeometryFactory *gf;
        gf=handle->geomFactory;

		out = gf->createGeometryCollection(linevec);
		// the above method takes ownership of the passed
		// vector, so we must *not* delete it
	}
	catch (const std::exception &e)
	{
		handle->ERROR_MESSAGE("%s", e.what());
		return NULL;
	}
	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}

	return out;
}

Geometry *
GEOSLineMerge_r(GEOSContextHandle_t extHandle, const Geometry *g)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return NULL;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return NULL;
    }

	using geos::operation::linemerge::LineMerger;
        unsigned int i;
        Geometry *out = NULL;

        try{
                // LineMerge
                LineMerger lmrgr;

                lmrgr.add(g);

                std::vector<LineString *>*lines = lmrgr.getMergedLineStrings();

#if GEOS_DEBUG
        handle->NOTICE_MESSAGE("output lines got");
#endif

                std::vector<Geometry *>*geoms = 
                           new std::vector<Geometry *>(lines->size());
                for (i=0; i<lines->size(); i++) (*geoms)[i] = (*lines)[i];
                delete lines;
                const GeometryFactory *gf;
                gf=handle->geomFactory;

                out = gf->buildGeometry(geoms);
                //out = gf->createGeometryCollection(geoms);
        }
	catch (const std::exception &e)
	{
		handle->ERROR_MESSAGE("%s", e.what());
		return NULL;
	}

	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}

        return out;
}

int
GEOSGetSRID_r(GEOSContextHandle_t extHandle, const Geometry *g1)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return 0;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return 0;
    }

	try{
		return g1->getSRID();
	}
	catch (const std::exception &e)
	{
		handle->ERROR_MESSAGE("%s", e.what());
		return 0;
	}

	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
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
GEOSHasZ_r(GEOSContextHandle_t extHandle, const Geometry *g)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return -1;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return -1;
    }

	if ( g->isEmpty() ) return false;
	double az = g->getCoordinate()->z;
	//handle->ERROR_MESSAGE("ZCoord: %g", az);
	return static_cast<char>(FINITE(az));
}

int
GEOS_getWKBOutputDims_r(GEOSContextHandle_t extHandle)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return -1;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return -1;
    }

    return handle->WKBOutputDims;
}

int
GEOS_setWKBOutputDims_r(GEOSContextHandle_t extHandle, int newdims)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return -1;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return -1;
    }

	if ( newdims < 2 || newdims > 3 )
		handle->ERROR_MESSAGE("WKB output dimensions out of range 2..3");
	int olddims = handle->WKBOutputDims;
	handle->WKBOutputDims = newdims;
	return olddims;
}

int
GEOS_getWKBByteOrder_r(GEOSContextHandle_t extHandle)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return -1;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return -1;
    }

	return handle->WKBByteOrder;
}

int
GEOS_setWKBByteOrder_r(GEOSContextHandle_t extHandle, int byteOrder)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return -1;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return -1;
    }

	int oldByteOrder = handle->WKBByteOrder;
	handle->WKBByteOrder = byteOrder;
	return oldByteOrder;
}


CoordinateSequence *
GEOSCoordSeq_create_r(GEOSContextHandle_t extHandle, unsigned int size, unsigned int dims)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return NULL;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return NULL;
    }

	try {
        const GeometryFactory *gf;
        gf=handle->geomFactory;

		return gf->getCoordinateSequenceFactory()->create(size, dims);
	}

	catch (const std::exception &e)
	{
		handle->ERROR_MESSAGE("%s", e.what());
		return NULL;
	}

	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}

int
GEOSCoordSeq_setOrdinate_r(GEOSContextHandle_t extHandle, CoordinateSequence *s, unsigned int idx, unsigned int dim, double val)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return 0;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return 0;
    }

	try {
		s->setOrdinate(static_cast<int>(idx),
			static_cast<int>(dim), val);
		return 1;
	}
	catch (const std::exception &e)
	{
		handle->ERROR_MESSAGE("%s", e.what());
		return 0;
	}

	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
		return 0;
	}
}

int
GEOSCoordSeq_setX_r(GEOSContextHandle_t extHandle, CoordinateSequence *s, unsigned int idx, double val)
{
	return GEOSCoordSeq_setOrdinate_r(extHandle, s, idx, 0, val);
}

int
GEOSCoordSeq_setY_r(GEOSContextHandle_t extHandle, CoordinateSequence *s, unsigned int idx, double val)
{
	return GEOSCoordSeq_setOrdinate_r(extHandle, s, idx, 1, val);
}

int
GEOSCoordSeq_setZ_r(GEOSContextHandle_t extHandle, CoordinateSequence *s, unsigned int idx, double val)
{
	return GEOSCoordSeq_setOrdinate_r(extHandle, s, idx, 2, val);
}

CoordinateSequence *
GEOSCoordSeq_clone_r(GEOSContextHandle_t extHandle, const CoordinateSequence *s)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return NULL;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return NULL;
    }

	try { return s->clone(); }
	catch (const std::exception &e)
	{
		handle->ERROR_MESSAGE("%s", e.what());
		return NULL;
	}

	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}

int
GEOSCoordSeq_getOrdinate_r(GEOSContextHandle_t extHandle, const CoordinateSequence *s, unsigned int idx, unsigned int dim, double *val)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return 0;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return 0;
    }

	try {
		double d = s->getOrdinate(static_cast<int>(idx),
			static_cast<int>(dim));
		*val = d;
		return 1;
	}
	catch (const std::exception &e)
	{
		handle->ERROR_MESSAGE("%s", e.what());
		return 0;
	}

	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
		return 0;
	}
}

int
GEOSCoordSeq_getX_r(GEOSContextHandle_t extHandle, const CoordinateSequence *s, unsigned int idx, double *val)
{
	return GEOSCoordSeq_getOrdinate_r(extHandle, s, idx, 0, val);
}

int
GEOSCoordSeq_getY_r(GEOSContextHandle_t extHandle, const CoordinateSequence *s, unsigned int idx, double *val)
{
	return GEOSCoordSeq_getOrdinate_r(extHandle, s, idx, 1, val);
}

int
GEOSCoordSeq_getZ_r(GEOSContextHandle_t extHandle, const CoordinateSequence *s, unsigned int idx, double *val)
{
	return GEOSCoordSeq_getOrdinate_r(extHandle, s, idx, 2, val);
}

int
GEOSCoordSeq_getSize_r(GEOSContextHandle_t extHandle, const CoordinateSequence *s, unsigned int *size)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return 0;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return 0;
    }

	try {
        std::size_t sz = s->getSize();
		*size = static_cast<unsigned int>(sz);
		return 1;
	}
	catch (const std::exception &e)
	{
		handle->ERROR_MESSAGE("%s", e.what());
		return 0;
	}

	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
		return 0;
	}
}

int
GEOSCoordSeq_getDimensions_r(GEOSContextHandle_t extHandle, const CoordinateSequence *s, unsigned int *dims)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return 0;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return 0;
    }

	try {
        std::size_t dim = s->getDimension();
		*dims = static_cast<unsigned int>(dim);
		return 1;
	}
	catch (const std::exception &e)
	{
		handle->ERROR_MESSAGE("%s", e.what());
		return 0;
	}

	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
		return 0;
	}
}

void
GEOSCoordSeq_destroy_r(GEOSContextHandle_t extHandle, CoordinateSequence *s)
{
    GEOSContextHandleInternal_t *handle;

	try{
		delete s;
	}

	catch (const std::exception &e)
	{
        if( extHandle == NULL )
        {
            return;
        }

        handle = (GEOSContextHandleInternal_t*)extHandle;
        if( handle->initialized == 0 )
        {
            return;
        }

		handle->ERROR_MESSAGE("%s", e.what());
	}

	catch (...)
	{
        if( extHandle == NULL )
        {
            return;
        }

        handle = (GEOSContextHandleInternal_t*)extHandle;
        if( handle->initialized == 0 )
        {
            return;
        }

		handle->ERROR_MESSAGE("Unknown exception thrown");
	}
}

const CoordinateSequence *
GEOSGeom_getCoordSeq_r(GEOSContextHandle_t extHandle, const Geometry *g)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return NULL;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return NULL;
    }

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
		handle->ERROR_MESSAGE("Geometry must be a Point or LineString");
		return NULL;
	}
	catch (const std::exception &e)
	{
		handle->ERROR_MESSAGE("%s", e.what());
		return NULL;
	}

	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}

Geometry *
GEOSGeom_createPoint_r(GEOSContextHandle_t extHandle, CoordinateSequence *cs)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return NULL;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return NULL;
    }

	try { 
        const GeometryFactory *gf;
        gf=handle->geomFactory;
        return gf->createPoint(cs);
    }
	catch (const std::exception &e)
	{
		handle->ERROR_MESSAGE("%s", e.what());
		return NULL;
	}

	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}

Geometry *
GEOSGeom_createLinearRing_r(GEOSContextHandle_t extHandle, CoordinateSequence *cs)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return NULL;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return NULL;
    }

	try { 
        const GeometryFactory *gf;
        gf=handle->geomFactory;

        return gf->createLinearRing(cs);
    }
	catch (const std::exception &e)
	{
		handle->ERROR_MESSAGE("%s", e.what());
		return NULL;
	}

	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}

Geometry *
GEOSGeom_createLineString_r(GEOSContextHandle_t extHandle, CoordinateSequence *cs)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return NULL;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return NULL;
    }

	try { 
        const GeometryFactory *gf;
        gf=handle->geomFactory;

        return gf->createLineString(cs);
    }
	catch (const std::exception &e)
	{
		handle->ERROR_MESSAGE("%s", e.what());
		return NULL;
	}

	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}

Geometry *
GEOSGeom_createPolygon_r(GEOSContextHandle_t extHandle, Geometry *shell, Geometry **holes, unsigned int nholes)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return NULL;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return NULL;
    }

    using geos::geom::LinearRing;
	try
	{
        std::vector<Geometry *> *vholes = 
                   new std::vector<Geometry *>(holes, holes+nholes);
		LinearRing *nshell = dynamic_cast<LinearRing *>(shell);
		if ( ! nshell )
		{
			handle->ERROR_MESSAGE("Shell is not a LinearRing");
			return NULL;
		}
        const GeometryFactory *gf;
        gf=handle->geomFactory;

		return gf->createPolygon(nshell, vholes);
	}
	catch (const std::exception &e)
	{
		handle->ERROR_MESSAGE("%s", e.what());
		return NULL;
	}

	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}

Geometry *
GEOSGeom_clone_r(GEOSContextHandle_t extHandle, const Geometry *g)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return NULL;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return NULL;
    }

	try { return g->clone(); }
	catch (const std::exception &e)
	{
		handle->ERROR_MESSAGE("%s", e.what());
		return NULL;
	}

	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}

int
GEOSGeom_getDimensions_r(GEOSContextHandle_t extHandle, const Geometry *g)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return 0;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return 0;
    }

    using geos::geom::GeometryCollection;
    using geos::geom::Point;

    std::size_t dim = 0;
	try {
		const LineString *ls = dynamic_cast<const LineString *>(g);
		if ( ls )
		{
            dim = ls->getCoordinatesRO()->getDimension();
			return static_cast<int>(dim);
		}

		const Point *p = dynamic_cast<const Point *>(g);
		if ( p )
		{
            dim = p->getCoordinatesRO()->getDimension();
			return static_cast<int>(dim);
		}

		const Polygon *poly = dynamic_cast<const Polygon *>(g);
		if ( poly )
		{
			return GEOSGeom_getDimensions_r(extHandle, poly->getExteriorRing());
		}

		const GeometryCollection *coll =
			dynamic_cast<const GeometryCollection *>(g);
		if ( coll )
		{
			if ( coll->isEmpty() ) return 0;
			return GEOSGeom_getDimensions_r(extHandle, coll->getGeometryN(0));
		}

		handle->ERROR_MESSAGE("Unknown geometry type");
		return 0;
	}

	catch (const std::exception &e)
	{
		handle->ERROR_MESSAGE("%s", e.what());
		return 0;
	}

	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
		return 0;
	}
}

Geometry *
GEOSSimplify_r(GEOSContextHandle_t extHandle, const Geometry *g1, double tolerance)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return NULL;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return NULL;
    }

	using namespace geos::simplify;

	try
	{
		Geometry::AutoPtr g(DouglasPeuckerSimplifier::simplify(
				g1, tolerance));
		return g.release();
	}
	catch (const std::exception &e)
	{
		handle->ERROR_MESSAGE("%s", e.what());
		return NULL;
	}

	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}

Geometry *
GEOSTopologyPreserveSimplify_r(GEOSContextHandle_t extHandle, const Geometry *g1, double tolerance)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return NULL;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return NULL;
    }

	using namespace geos::simplify;

	try
	{
		Geometry::AutoPtr g(TopologyPreservingSimplifier::simplify(
				g1, tolerance));
		return g.release();
	}
	catch (const std::exception &e)
	{
		handle->ERROR_MESSAGE("%s", e.what());
		return NULL;
	}

	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}


/* WKT Reader */
WKTReader *
GEOSWKTReader_create_r(GEOSContextHandle_t extHandle)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return NULL;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return NULL;
    }

	using geos::io::WKTReader;
	try
	{
		return new WKTReader((GeometryFactory*)handle->geomFactory);
	}
	catch (const std::exception &e)
	{
		handle->ERROR_MESSAGE("%s", e.what());
		return NULL;
	}

	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}

void
GEOSWKTReader_destroy_r(GEOSContextHandle_t extHandle, WKTReader *reader)
{
    GEOSContextHandleInternal_t *handle;

	try
	{
		delete reader;
	}
	catch (const std::exception &e)
	{
        if( extHandle == NULL )
        {
            return;
        }

        handle = (GEOSContextHandleInternal_t*)extHandle;
        if( handle->initialized == 0 )
        {
            return;
        }

		handle->ERROR_MESSAGE("%s", e.what());
	}

	catch (...)
	{
        if( extHandle == NULL )
        {
            return;
        }

        handle = (GEOSContextHandleInternal_t*)extHandle;
        if( handle->initialized == 0 )
        {
            return;
        }

		handle->ERROR_MESSAGE("Unknown exception thrown");
	}
}


Geometry*
GEOSWKTReader_read_r(GEOSContextHandle_t extHandle, WKTReader *reader, const char *wkt)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return NULL;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return NULL;
    }

    CLocalizer clocale;
	try
	{
		const std::string wktstring = std::string(wkt);
		Geometry *g = reader->read(wktstring);
		return g;
	}
	catch (const std::exception &e)
	{
		handle->ERROR_MESSAGE("%s", e.what());
		return NULL;
	}

	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}

/* WKT Writer */
WKTWriter *
GEOSWKTWriter_create_r(GEOSContextHandle_t extHandle)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return NULL;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return NULL;
    }

	using geos::io::WKTWriter;
	try
	{
		return new WKTWriter();
	}
	catch (const std::exception &e)
	{
		handle->ERROR_MESSAGE("%s", e.what());
		return NULL;
	}

	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
		return NULL;
	}
}

void
GEOSWKTWriter_destroy_r(GEOSContextHandle_t extHandle, WKTWriter *Writer)
{
    GEOSContextHandleInternal_t *handle;

	try
	{
		delete Writer;
	}
	catch (const std::exception &e)
	{
        if( extHandle == NULL )
        {
            return;
        }

        handle = (GEOSContextHandleInternal_t*)extHandle;
        if( handle->initialized == 0 )
        {
            return;
        }

		handle->ERROR_MESSAGE("%s", e.what());
	}

	catch (...)
	{
        if( extHandle == NULL )
        {
            return;
        }

        handle = (GEOSContextHandleInternal_t*)extHandle;
        if( handle->initialized == 0 )
        {
            return;
        }

		handle->ERROR_MESSAGE("Unknown exception thrown");
	}
}


char*
GEOSWKTWriter_write_r(GEOSContextHandle_t extHandle, WKTWriter *writer, const Geometry *geom)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return NULL;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return NULL;
    }

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
		handle->ERROR_MESSAGE("%s", e.what());
	}
	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
	}
    
    return NULL;
}

/* WKB Reader */
WKBReader *
GEOSWKBReader_create_r(GEOSContextHandle_t extHandle)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return NULL;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return NULL;
    }

	using geos::io::WKBReader;
	try
	{
		return new WKBReader(*(GeometryFactory*)handle->geomFactory);
	}
	catch (const std::exception &e)
	{
		handle->ERROR_MESSAGE("%s", e.what());
	}
	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
	}
    
    return NULL;
}

void
GEOSWKBReader_destroy_r(GEOSContextHandle_t extHandle, WKBReader *reader)
{
    GEOSContextHandleInternal_t *handle;

	try
	{
		delete reader;
	}
	catch (const std::exception &e)
	{
        if( extHandle == NULL )
        {
            return;
        }

        handle = (GEOSContextHandleInternal_t*)extHandle;
        if( handle->initialized == 0 )
        {
            return;
        }

		handle->ERROR_MESSAGE("%s", e.what());
	}

	catch (...)
	{
        if( extHandle == NULL )
        {
            return;
        }

        handle = (GEOSContextHandleInternal_t*)extHandle;
        if( handle->initialized == 0 )
        {
            return;
        }

		handle->ERROR_MESSAGE("Unknown exception thrown");
	}
}


Geometry*
GEOSWKBReader_read_r(GEOSContextHandle_t extHandle, WKBReader *reader, const unsigned char *wkb, size_t size)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return NULL;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return NULL;
    }

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
		handle->ERROR_MESSAGE("%s", e.what());
	}
	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
	}
    
    return NULL;
}

Geometry*
GEOSWKBReader_readHEX_r(GEOSContextHandle_t extHandle, WKBReader *reader, const unsigned char *hex, size_t size)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return NULL;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return NULL;
    }

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
		handle->ERROR_MESSAGE("%s", e.what());
	}
	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
	}
    
    return NULL;
}

/* WKB Writer */
WKBWriter *
GEOSWKBWriter_create_r(GEOSContextHandle_t extHandle)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return NULL;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return NULL;
    }

	using geos::io::WKBWriter;
	try
	{
		return new WKBWriter();
	}
	catch (const std::exception &e)
	{
		handle->ERROR_MESSAGE("%s", e.what());
	}
	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
	}
    
    return NULL;
}

void
GEOSWKBWriter_destroy_r(GEOSContextHandle_t extHandle, WKBWriter *Writer)
{
    GEOSContextHandleInternal_t *handle;

	try
	{
		delete Writer;
	}
	catch (const std::exception &e)
	{
        if( extHandle == NULL )
        {
            return;
        }

        handle = (GEOSContextHandleInternal_t*)extHandle;
        if( handle->initialized == 0 )
        {
            return;
        }

		handle->ERROR_MESSAGE("%s", e.what());
	}

	catch (...)
	{
        if( extHandle == NULL )
        {
            return;
        }

        handle = (GEOSContextHandleInternal_t*)extHandle;
        if( handle->initialized == 0 )
        {
            return;
        }

		handle->ERROR_MESSAGE("Unknown exception thrown");
	}
}


/* The owner owns the result */
unsigned char*
GEOSWKBWriter_write_r(GEOSContextHandle_t extHandle, WKBWriter *writer, const Geometry *geom, size_t *size)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return NULL;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return NULL;
    }

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
		handle->ERROR_MESSAGE("%s", e.what());
	}
	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
	}
    return NULL;
}

/* The owner owns the result */
unsigned char*
GEOSWKBWriter_writeHEX_r(GEOSContextHandle_t extHandle, WKBWriter *writer, const Geometry *geom, size_t *size)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return NULL;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return NULL;
    }

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
		handle->ERROR_MESSAGE("%s", e.what());
	}

	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
	}
    return NULL;
}

int
GEOSWKBWriter_getOutputDimension_r(GEOSContextHandle_t extHandle, const GEOSWKBWriter* writer)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return 0;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return 0;
    }

	try
	{
		return writer->getOutputDimension();
	}

	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
	}
    return 0;
}

void
GEOSWKBWriter_setOutputDimension_r(GEOSContextHandle_t extHandle, GEOSWKBWriter* writer, int newDimension)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return;
    }

	try
	{
		return writer->setOutputDimension(newDimension);
	}

	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
	}
}

int
GEOSWKBWriter_getByteOrder_r(GEOSContextHandle_t extHandle, const GEOSWKBWriter* writer)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return 0;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return 0;
    }

	try
	{
		return writer->getByteOrder();
	}

	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
	}
    return 0;
}

void
GEOSWKBWriter_setByteOrder_r(GEOSContextHandle_t extHandle, GEOSWKBWriter* writer, int newByteOrder)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return;
    }

	try
	{
		return writer->setByteOrder(newByteOrder);
	}

	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
	}
}

char
GEOSWKBWriter_getIncludeSRID_r(GEOSContextHandle_t extHandle, const GEOSWKBWriter* writer)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return -1;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return -1;
    }

	try
	{
		int srid = writer->getIncludeSRID();
        return static_cast<char>(srid);
	}

	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
		return -1;
	}
}

void
GEOSWKBWriter_setIncludeSRID_r(GEOSContextHandle_t extHandle, GEOSWKBWriter* writer, const char newIncludeSRID)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return;
    }

	try
	{
		writer->setIncludeSRID(newIncludeSRID);
	}

	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
	}
}


//-----------------------------------------------------------------
// Prepared Geometry 
//-----------------------------------------------------------------

const geos::geom::prep::PreparedGeometry*
GEOSPrepare_r(GEOSContextHandle_t extHandle, const Geometry *g)
{
    const geos::geom::prep::PreparedGeometry* prep = NULL;
	
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return NULL;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return NULL;
    }

    try
	{
		prep = geos::geom::prep::PreparedGeometryFactory::prepare(g);
	}
	catch (const std::exception &e)
	{
		handle->ERROR_MESSAGE("%s", e.what());
	}
	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
	}
	
    return prep;
}

void
GEOSPreparedGeom_destroy_r(GEOSContextHandle_t extHandle, const geos::geom::prep::PreparedGeometry *a)
{
    GEOSContextHandleInternal_t *handle;

	try
	{
		delete a;
	}
	catch (const std::exception &e)
	{
        if( extHandle == NULL )
        {
            return;
        }

        handle = (GEOSContextHandleInternal_t*)extHandle;
        if( handle->initialized == 0 )
        {
            return;
        }

		handle->ERROR_MESSAGE("%s", e.what());
	}
	catch (...)
	{
        if( extHandle == NULL )
        {
            return;
        }

        handle = (GEOSContextHandleInternal_t*)extHandle;
        if( handle->initialized == 0 )
        {
            return;
        }

		handle->ERROR_MESSAGE("Unknown exception thrown");
	}
}

char
GEOSPreparedContains_r(GEOSContextHandle_t extHandle, const geos::geom::prep::PreparedGeometry *pg1, const Geometry *g2)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return 2;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return 2;
    }

	try 
	{
		bool result;
		result = pg1->contains(g2);
		return result;
	}
	catch (const std::exception &e)
	{
		handle->ERROR_MESSAGE("%s", e.what());
		return 2;
	}
	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
		return 2;
	}
}

char
GEOSPreparedContainsProperly_r(GEOSContextHandle_t extHandle, const geos::geom::prep::PreparedGeometry *pg1, const Geometry *g2)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return 2;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return 2;
    }

	try 
	{
		bool result;
		result = pg1->containsProperly(g2);
		return result;
	}
	catch (const std::exception &e)
	{
		handle->ERROR_MESSAGE("%s", e.what());
		return 2;
	}
	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
		return 2;
	}
}

char
GEOSPreparedCovers_r(GEOSContextHandle_t extHandle, const geos::geom::prep::PreparedGeometry *pg1, const Geometry *g2)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return 2;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return 2;
    }

	try 
	{
		bool result;
		result = pg1->covers(g2);
		return result;
	}
	catch (const std::exception &e)
	{
		handle->ERROR_MESSAGE("%s", e.what());
		return 2;
	}
	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
		return 2;
	}
}

char
GEOSPreparedIntersects_r(GEOSContextHandle_t extHandle, const geos::geom::prep::PreparedGeometry *pg1, const Geometry *g2)
{
    GEOSContextHandleInternal_t *handle;

    if( extHandle == NULL )
    {
        return 2;
    }

    handle = (GEOSContextHandleInternal_t*)extHandle;
    if( handle->initialized == 0 )
    {
        return 2;
    }

	try 
	{
		bool result;
		result = pg1->intersects(g2);
		return result;
	}
	catch (const std::exception &e)
	{
		handle->ERROR_MESSAGE("%s", e.what());
		return 2;
	}
	catch (...)
	{
		handle->ERROR_MESSAGE("Unknown exception thrown");
		return 2;
	}
}

} /* extern "C" */
