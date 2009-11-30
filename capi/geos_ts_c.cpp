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
#include <geos/geom/Envelope.h> 
#include <geos/index/strtree/STRtree.h> 
#include <geos/index/ItemVisitor.h>
#include <geos/io/WKTReader.h>
#include <geos/io/WKBReader.h>
#include <geos/io/WKTWriter.h>
#include <geos/io/WKBWriter.h>
#include <geos/algorithm/distance/DiscreteHausdorffDistance.h>
#include <geos/simplify/DouglasPeuckerSimplifier.h>
#include <geos/simplify/TopologyPreservingSimplifier.h>
#include <geos/operation/valid/IsValidOp.h>
#include <geos/operation/polygonize/Polygonizer.h>
#include <geos/operation/linemerge/LineMerger.h>
#include <geos/operation/overlay/OverlayOp.h>
#include <geos/operation/union/CascadedPolygonUnion.h>
#include <geos/operation/buffer/BufferOp.h>
#include <geos/operation/buffer/BufferParameters.h>
#include <geos/operation/buffer/BufferBuilder.h>
#include <geos/linearref/LengthIndexedLine.h>
#include <geos/geom/BinaryOp.h>
#include <geos/util/IllegalArgumentException.h>
#include <geos/version.h> 
#include <geos/platform.h>  // for FINITE

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

#ifdef _MSC_VER
#pragma warning(disable : 4099)
#endif

// Some extra magic to make type declarations in geos_c.h work - 
// for cross-checking of types in header.
#define GEOSGeometry geos::geom::Geometry
#define GEOSPreparedGeometry geos::geom::prep::PreparedGeometry
#define GEOSCoordSequence geos::geom::CoordinateSequence
#define GEOSSTRtree geos::index::strtree::STRtree
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

#include <geos/export.h>


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

using geos::operation::overlay::OverlayOp;
using geos::operation::overlay::overlayOp;
using geos::operation::geounion::CascadedPolygonUnion;

using geos::algorithm::distance::DiscreteHausdorffDistance;

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

// CAPI_ItemVisitor is used internally by the CAPI STRtree
// wrappers. It's defined here just to keep it out of the
// extern "C" block.
class CAPI_ItemVisitor : public geos::index::ItemVisitor {
    GEOSQueryCallback callback;
    void *userdata;
  public:
    CAPI_ItemVisitor (GEOSQueryCallback cb, void *ud)
        : ItemVisitor(), callback(cb), userdata(ud) {};
    void visitItem (void *item) { callback(item, userdata); };
};


//## PROTOTYPES #############################################

extern "C" const char GEOS_DLL *GEOSjtsport();
extern "C" char GEOS_DLL *GEOSasText(Geometry *g1);

extern "C" {

char* gstrdup_s(const char* str, const std::size_t size)
{
    char* out = static_cast<char*>(std::malloc(size + 1));
    if (0 != out)
    {
        // as no strlen call necessary, memcpy may be faster than strcpy
        std::memcpy(out, str, size + 1);
    }

    assert(0 != out);
    return out;
}

char* gstrdup(std::string const& str)
{
    return gstrdup_s(str.c_str(), str.size());
}


GEOSContextHandle_t
initGEOS_r(GEOSMessageHandler nf, GEOSMessageHandler ef)
{
    GEOSContextHandleInternal_t *handle = 0;
    void *extHandle = 0;

    extHandle = std::malloc(sizeof(GEOSContextHandleInternal_t));
    if (0 != extHandle)
    {
        handle = static_cast<GEOSContextHandleInternal_t*>(extHandle);
        handle->NOTICE_MESSAGE = nf;
        handle->ERROR_MESSAGE = ef;
        handle->geomFactory = GeometryFactory::getDefaultInstance();
        handle->WKBOutputDims = 2;
        handle->WKBByteOrder = getMachineByteOrder();
        handle->initialized = 1;
    }

    return static_cast<GEOSContextHandle_t>(extHandle);
}

void
finishGEOS_r(GEOSContextHandle_t extHandle)
{
    // Fix up freeing handle w.r.t. malloc above
    std::free(extHandle);
    extHandle = NULL;
}

void 
GEOSFree_r (GEOSContextHandle_t extHandle, void* buffer) 
{ 
    assert(0 != extHandle);

    std::free(buffer); 
} 

//-----------------------------------------------------------
// relate()-related functions
//  return 0 = false, 1 = true, 2 = error occured
//-----------------------------------------------------------

char
GEOSDisjoint_r(GEOSContextHandle_t extHandle, const Geometry *g1, const Geometry *g2)
{
    if ( 0 == extHandle )
    {
        return 2;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( handle->initialized == 0 )
    {
        return 2;
    }

    try
    {
        bool result = g1->disjoint(g2);
        return result;
    }

    // TODO: mloskot is going to replace these double-catch block
    // with a macro to remove redundant code in this and
    // following functions.
    catch (const std::exception &e)
    {
        handle->ERROR_MESSAGE("%s", e.what());
    }
    catch (...)
    {
        handle->ERROR_MESSAGE("Unknown exception thrown");
    }

    return 2;
}

char
GEOSTouches_r(GEOSContextHandle_t extHandle, const Geometry *g1, const Geometry *g2)
{
    if ( 0 == extHandle )
    {
        return 2;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
    {
        return 2;
    }

    try
    {
        bool result = g1->touches(g2);
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
    
    return 2;
}

char
GEOSIntersects_r(GEOSContextHandle_t extHandle, const Geometry *g1, const Geometry *g2)
{
    if ( 0 == extHandle )
    {
        return 2;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
    {
        return 2;
    }

    try
    {
        bool result = g1->intersects(g2);
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
    
    return 2;
}

char
GEOSCrosses_r(GEOSContextHandle_t extHandle, const Geometry *g1, const Geometry *g2)
{
    if ( 0 == extHandle )
    {
        return 2;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
    {
        return 2;
    }

    try
    {
        bool result = g1->crosses(g2);
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
    
    return 2;
}

char
GEOSWithin_r(GEOSContextHandle_t extHandle, const Geometry *g1, const Geometry *g2)
{
    if ( 0 == extHandle )
    {
        return 2;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
    {
        return 2;
    }

    try
    {
        bool result = g1->within(g2);
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
    
    return 2;
}

// call g1->contains(g2)
// returns 0 = false
//         1 = true
//         2 = error was trapped
char
GEOSContains_r(GEOSContextHandle_t extHandle, const Geometry *g1, const Geometry *g2)
{
    if ( 0 == extHandle )
    {
        return 2;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
    {
        return 2;
    }

    try
    {
        bool result = g1->contains(g2);
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
    
    return 2;
}

char
GEOSOverlaps_r(GEOSContextHandle_t extHandle, const Geometry *g1, const Geometry *g2)
{
    if ( 0 == extHandle )
    {
        return 2;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
    {
        return 2;
    }

    try
    {
        bool result = g1->overlaps(g2);
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
    
    return 2;
}


//-------------------------------------------------------------------
// low-level relate functions
//------------------------------------------------------------------

char
GEOSRelatePattern_r(GEOSContextHandle_t extHandle, const Geometry *g1, const Geometry *g2, const char *pat)
{
    if ( 0 == extHandle )
    {
        return 2;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
    {
        return 2;
    }

    try
    {
        std::string s(pat);
        bool result = g1->relate(g2, s);
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
    
    return 2;
}

char *
GEOSRelate_r(GEOSContextHandle_t extHandle, const Geometry *g1, const Geometry *g2)
{
    if ( 0 == extHandle )
    {
        return NULL;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
    {
        return NULL;
    }

    try
    {
        using geos::geom::IntersectionMatrix;

        IntersectionMatrix* im = g1->relate(g2);
        if (0 == im)
        {
            return 0;
        }
       
        char *result = gstrdup(im->toString());

        delete im;
        im = 0;

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
    if ( 0 == extHandle )
    {
        return 2;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
    {
        return 2;
    }

    try
    {
        using geos::operation::valid::IsValidOp;
        using geos::operation::valid::TopologyValidationError;

        IsValidOp ivo(g1);
        bool valid = ivo.isValid();
        if (!valid)
        {
            TopologyValidationError *err = ivo.getValidationError();
            if ( err )
            {
                handle->NOTICE_MESSAGE("%s", err->toString().c_str());
            }
        }
        return valid;
    }
    catch (const std::exception &e)
    {
        handle->ERROR_MESSAGE("%s", e.what());
    }
    catch (...)
    {
        handle->ERROR_MESSAGE("Unknown exception thrown");
    }
    
    return 2;
}

char *
GEOSisValidReason_r(GEOSContextHandle_t extHandle, const Geometry *g1)
{
    if ( 0 == extHandle )
    {
        return NULL;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
    {
        return NULL;
    }

    try
    {
        using geos::operation::valid::IsValidOp;
        using geos::operation::valid::TopologyValidationError;

        char* result = 0;
        char const* const validstr = "Valid Geometry";

        IsValidOp ivo(g1);
        bool isvalid = ivo.isValid();
        if ( ! isvalid )
        {
            TopologyValidationError *err = ivo.getValidationError();
            if (0 != err)
            {
                const std::string errloc = err->getCoordinate().toString();
                std::string errmsg(err->getMessage());
                errmsg += "[" + errloc + "]";
                result = gstrdup(errmsg);
            }
        }
        else
        {
            result = gstrdup(std::string(validstr));
        }

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

    return 0;
}

//-----------------------------------------------------------------
// general purpose
//-----------------------------------------------------------------

char
GEOSEquals_r(GEOSContextHandle_t extHandle, const Geometry *g1, const Geometry *g2)
{
    if ( 0 == extHandle )
    {
        return 2;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
    {
        return 2;
    }

    try
    {
        bool result = g1->equals(g2);
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

    return 2;
}

char
GEOSEqualsExact_r(GEOSContextHandle_t extHandle, const Geometry *g1, const Geometry *g2, double tolerance)
{
    if ( 0 == extHandle )
    {
        return 2;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
    {
        return 2;
    }

    try
    {
        bool result = g1->equalsExact(g2, tolerance);
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
    
    return 2;
}

int
GEOSDistance_r(GEOSContextHandle_t extHandle, const Geometry *g1, const Geometry *g2, double *dist)
{
    assert(0 != dist);

    if ( 0 == extHandle )
    {
        return 0;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
    {
        return 0;
    }

    try
    {
        *dist = g1->distance(g2);
        return 1;
    }
    catch (const std::exception &e)
    {
        handle->ERROR_MESSAGE("%s", e.what());
    }
    catch (...)
    {
        handle->ERROR_MESSAGE("Unknown exception thrown");
    }
    
    return 0;
}

int
GEOSHausdorffDistance_r(GEOSContextHandle_t extHandle, const Geometry *g1, const Geometry *g2, double *dist)
{
    assert(0 != dist);

    if ( 0 == extHandle )
    {
        return 0;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
    {
        return 0;
    }

    try
    {
        *dist = DiscreteHausdorffDistance::distance(*g1, *g2);
        return 1;
    }
    catch (const std::exception &e)
    {
        handle->ERROR_MESSAGE("%s", e.what());
    }
    catch (...)
    {
        handle->ERROR_MESSAGE("Unknown exception thrown");
    }
    
    return 0;
}

int
GEOSHausdorffDistanceDensify_r(GEOSContextHandle_t extHandle, const Geometry *g1, const Geometry *g2, double densifyFrac, double *dist)
{
    assert(0 != dist);

    if ( 0 == extHandle )
    {
        return 0;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
    {
        return 0;
    }

    try
    {
        *dist = DiscreteHausdorffDistance::distance(*g1, *g2, densifyFrac);
        return 1;
    }
    catch (const std::exception &e)
    {
        handle->ERROR_MESSAGE("%s", e.what());
    }
    catch (...)
    {
        handle->ERROR_MESSAGE("Unknown exception thrown");
    }
    
    return 0;
}

int
GEOSArea_r(GEOSContextHandle_t extHandle, const Geometry *g, double *area)
{
    assert(0 != area);

    if ( 0 == extHandle )
    {
        return 0;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
    {
        return 0;
    }

    try
    {
        *area = g->getArea();
        return 1;
    }
    catch (const std::exception &e)
    {
        handle->ERROR_MESSAGE("%s", e.what());
    }
    catch (...)
    {
        handle->ERROR_MESSAGE("Unknown exception thrown");
    }
    
    return 0;
}

int
GEOSLength_r(GEOSContextHandle_t extHandle, const Geometry *g, double *length)
{
    assert(0 != length);

    if ( 0 == extHandle )
    {
        return 2;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
    {
        return 0;
    }

    try
    {
        *length = g->getLength();
        return 1;
    }
    catch (const std::exception &e)
    {
        handle->ERROR_MESSAGE("%s", e.what());
    }
    catch (...)
    {
        handle->ERROR_MESSAGE("Unknown exception thrown");
    }
    
    return 0;
}

Geometry *
GEOSGeomFromWKT_r(GEOSContextHandle_t extHandle, const char *wkt)
{
    if ( 0 == extHandle )
    {
        return NULL;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
    {
        return NULL;
    }

    try
    {
        const std::string wktstring(wkt);
        WKTReader r(static_cast<GeometryFactory const*>(handle->geomFactory));

        Geometry *g = r.read(wktstring);
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

char *
GEOSGeomToWKT_r(GEOSContextHandle_t extHandle, const Geometry *g1)
{
    if ( 0 == extHandle )
    {
        return NULL;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
    {
        return NULL;
    }

    try
    {

        char *result = gstrdup(g1->toString());
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

// Remember to free the result!
unsigned char *
GEOSGeomToWKB_buf_r(GEOSContextHandle_t extHandle, const Geometry *g, size_t *size)
{
    assert(0 != size);

    if ( 0 == extHandle )
    {
        return NULL;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
    {
        return NULL;
    }

    using geos::io::WKBWriter;
    try
    {
        int byteOrder = static_cast<int>(handle->WKBByteOrder);
        WKBWriter w(handle->WKBOutputDims, byteOrder);
        std::ostringstream os(std::ios_base::binary);
        w.write(*g, os);
        std::string wkbstring(os.str());
        const std::size_t len = wkbstring.length();

        unsigned char* result = 0;
        result = static_cast<unsigned char*>(std::malloc(len));
        if (0 != result)
        {
            std::memcpy(result, wkbstring.c_str(), len);
            *size = len;
        }
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

Geometry *
GEOSGeomFromWKB_buf_r(GEOSContextHandle_t extHandle, const unsigned char *wkb, size_t size)
{
    if ( 0 == extHandle )
    {
        return NULL;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
    {
        return NULL;
    }

    using geos::io::WKBReader;
    try
    {
        std::string wkbstring(reinterpret_cast<const char*>(wkb), size); // make it binary !
        WKBReader r(*(static_cast<GeometryFactory const*>(handle->geomFactory)));
        std::istringstream is(std::ios_base::binary);
        is.str(wkbstring);
        is.seekg(0, std::ios::beg); // rewind reader pointer
        Geometry *g = r.read(is);
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

/* Read/write wkb hex values.  Returned geometries are
   owned by the caller.*/
unsigned char *
GEOSGeomToHEX_buf_r(GEOSContextHandle_t extHandle, const Geometry *g, size_t *size)
{
    if ( 0 == extHandle )
    {
        return NULL;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
    {
        return NULL;
    }

    using geos::io::WKBWriter;
    try
    {
        int byteOrder = static_cast<int>(handle->WKBByteOrder);
        WKBWriter w(handle->WKBOutputDims, byteOrder);
        std::ostringstream os(std::ios_base::binary);
        w.writeHEX(*g, os);
        std::string hexstring(os.str());

        char *result = gstrdup(hexstring);
        if (0 != result)
        {
            *size = hexstring.length();
        }

        return reinterpret_cast<unsigned char*>(result);
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

Geometry *
GEOSGeomFromHEX_buf_r(GEOSContextHandle_t extHandle, const unsigned char *hex, size_t size)
{
    if ( 0 == extHandle )
    {
        return NULL;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
    {
        return NULL;
    }

    using geos::io::WKBReader;
    try
    {
        std::string hexstring(reinterpret_cast<const char*>(hex), size);
        WKBReader r(*(static_cast<GeometryFactory const*>(handle->geomFactory)));
        std::istringstream is(std::ios_base::binary);
        is.str(hexstring);
        is.seekg(0, std::ios::beg); // rewind reader pointer

        Geometry *g = r.readHEX(is);
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

char
GEOSisEmpty_r(GEOSContextHandle_t extHandle, const Geometry *g1)
{
    if ( 0 == extHandle )
    {
        return 2;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
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
    }
    catch (...)
    {
        handle->ERROR_MESSAGE("Unknown exception thrown");
    }
    
    return 2;
}

char
GEOSisSimple_r(GEOSContextHandle_t extHandle, const Geometry *g1)
{
    if ( 0 == extHandle )
    {
        return 2;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
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
    if ( 0 == extHandle )
    {
        return 2;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
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
    if ( 0 == extHandle )
    {
        return NULL;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
    {
        return NULL;
    }

    try
    {
        std::string s = g1->getGeometryType();

        char *result = gstrdup(s);
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

// Return postgis geometry type index
int
GEOSGeomTypeId_r(GEOSContextHandle_t extHandle, const Geometry *g1)
{
    if ( 0 == extHandle )
    {
        return -1;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
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
    }
    catch (...)
    {
        handle->ERROR_MESSAGE("Unknown exception thrown");
    }
    
    return -1;
}

//-------------------------------------------------------------------
// GEOS functions that return geometries
//-------------------------------------------------------------------

Geometry *
GEOSEnvelope_r(GEOSContextHandle_t extHandle, const Geometry *g1)
{
    if ( 0 == extHandle )
    {
        return NULL;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
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
    }
    catch (...)
    {
        handle->ERROR_MESSAGE("Unknown exception thrown");
    }
    
    return NULL;
}

Geometry *
GEOSIntersection_r(GEOSContextHandle_t extHandle, const Geometry *g1, const Geometry *g2)
{
    if ( 0 == extHandle )
    {
        return NULL;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
    {
        return NULL;
    }

    try
    {
        GeomAutoPtr g3(BinaryOp(g1, g2, overlayOp(OverlayOp::opINTERSECTION)));
        return g3.release();

        // XXX: old version
        //Geometry *g3 = g1->intersection(g2);
        //return g3;
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

Geometry *
GEOSBuffer_r(GEOSContextHandle_t extHandle, const Geometry *g1, double width, int quadrantsegments)
{
    if ( 0 == extHandle )
    {
        return NULL;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
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
    }
    catch (...)
    {
        handle->ERROR_MESSAGE("Unknown exception thrown");
    }
    
    return NULL;
}

Geometry *
GEOSBufferWithStyle_r(GEOSContextHandle_t extHandle, const Geometry *g1, double width, int quadsegs, int endCapStyle, int joinStyle, double mitreLimit)
{
    using geos::operation::buffer::BufferParameters;
    using geos::operation::buffer::BufferOp;
    using geos::util::IllegalArgumentException;

    if ( 0 == extHandle )
    {
        return NULL;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
    {
        return NULL;
    }

    try
    {
        BufferParameters bp;
        bp.setQuadrantSegments(quadsegs);

        if ( endCapStyle > BufferParameters::CAP_SQUARE )
        {
        	throw IllegalArgumentException("Invalid buffer endCap style");
        }
        bp.setEndCapStyle(
        	static_cast<BufferParameters::EndCapStyle>(endCapStyle)
        );

        if ( joinStyle > BufferParameters::JOIN_BEVEL )
        {
        	throw IllegalArgumentException("Invalid buffer join style");
        }
        bp.setJoinStyle(
        	static_cast<BufferParameters::JoinStyle>(joinStyle)
        );
        bp.setMitreLimit(mitreLimit);
        BufferOp op(g1, bp);
        Geometry *g3 = op.getResultGeometry(width);
        return g3;
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

Geometry *
GEOSSingleSidedBuffer_r(GEOSContextHandle_t extHandle, const Geometry *g1, double width, int quadsegs, int joinStyle, double mitreLimit, int leftSide)
{
    using geos::operation::buffer::BufferParameters;
    using geos::operation::buffer::BufferBuilder;
    using geos::operation::buffer::BufferOp;
    using geos::util::IllegalArgumentException;

    if ( 0 == extHandle )
    {
        return NULL;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
    {
        return NULL;
    }

    try
    {
        BufferParameters bp;
	bp.setEndCapStyle( BufferParameters::CAP_FLAT );
        bp.setQuadrantSegments(quadsegs);

        if ( joinStyle > BufferParameters::JOIN_BEVEL )
        {
        	throw IllegalArgumentException("Invalid buffer join style");
        }
        bp.setJoinStyle(
        	static_cast<BufferParameters::JoinStyle>(joinStyle)
        );
        bp.setMitreLimit(mitreLimit);

	BufferBuilder bufBuilder (bp);
        Geometry *g3 = bufBuilder.bufferLineSingleSided(g1, width, leftSide);
	
        return g3;
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

Geometry *
GEOSConvexHull_r(GEOSContextHandle_t extHandle, const Geometry *g1)
{
    if ( 0 == extHandle )
    {
        return NULL;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
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
    }
    catch (...)
    {
        handle->ERROR_MESSAGE("Unknown exception thrown");
    }
    
    return NULL;
}

Geometry *
GEOSDifference_r(GEOSContextHandle_t extHandle, const Geometry *g1, const Geometry *g2)
{
    if ( 0 == extHandle )
    {
        return NULL;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
    {
        return NULL;
    }

    try
    {
        GeomAutoPtr g3(BinaryOp(g1, g2, overlayOp(OverlayOp::opDIFFERENCE)));
        return g3.release();

        // XXX: old version
        //Geometry *g3 = g1->difference(g2);
        //return g3;
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

Geometry *
GEOSBoundary_r(GEOSContextHandle_t extHandle, const Geometry *g1)
{
    if ( 0 == extHandle )
    {
        return NULL;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
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
    }
    catch (...)
    {
        handle->ERROR_MESSAGE("Unknown exception thrown");
    }
    
    return NULL;
}

Geometry *
GEOSSymDifference_r(GEOSContextHandle_t extHandle, const Geometry *g1, const Geometry *g2)
{
    if ( 0 == extHandle )
    {
        return NULL;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
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
    if ( 0 == extHandle )
    {
        return NULL;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
    {
        return NULL;
    }

    try
    {
        GeomAutoPtr g3 = BinaryOp(g1, g2, overlayOp(OverlayOp::opUNION));
        return g3.release();

        // XXX: old version
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
    }
    catch (...)
    {
        handle->ERROR_MESSAGE("Unknown exception thrown");
    }
    
    return NULL;
}

Geometry *
GEOSUnionCascaded_r(GEOSContextHandle_t extHandle, const Geometry *g1)
{
    if ( 0 == extHandle )
    {
        return NULL;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
    {
        return NULL;
    }

    try
    {
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
    }
    catch (...)
    {
        handle->ERROR_MESSAGE("Unknown exception thrown");
    }
    
    return NULL;
}

Geometry *
GEOSPointOnSurface_r(GEOSContextHandle_t extHandle, const Geometry *g1)
{
    if ( 0 == extHandle )
    {
        return NULL;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
    {
        return NULL;
    }

    try
    {
        Geometry *ret = g1->getInteriorPoint();
        if ( ! ret )
        {
            const GeometryFactory* gf = handle->geomFactory;
            // return an empty collection 
            return gf->createGeometryCollection();
        }
        return ret;
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

//-------------------------------------------------------------------
// memory management functions
//------------------------------------------------------------------

void
GEOSGeom_destroy_r(GEOSContextHandle_t extHandle, Geometry *a)
{
    GEOSContextHandleInternal_t *handle = 0;

    // FIXME: mloskot: Does this try-catch around delete means that 
    // destructors in GEOS may throw? If it does, this is a serious
    // violation of "never throw an exception from a destructor" principle

    try
    {
        delete a;
    }
    catch (const std::exception &e)
    {
        if ( 0 == extHandle )
        {
            return;
        }

        handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
        if ( 0 == handle->initialized )
        {
            return;
        }

        handle->ERROR_MESSAGE("%s", e.what());
    }
    catch (...)
    {
        if ( 0 == extHandle )
        {
            return;
        }

        handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
        if ( 0 == handle->initialized )
        {
            return;
        }

        handle->ERROR_MESSAGE("Unknown exception thrown");
    }
}

void
GEOSSetSRID_r(GEOSContextHandle_t extHandle, Geometry *g, int srid)
{
    assert(0 != g);

    if ( 0 == extHandle )
    {
        return;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
    {
        return;
    }

    g->setSRID(srid);
}


int
GEOSGetNumCoordinates_r(GEOSContextHandle_t extHandle, const Geometry *g)
{
    assert(0 != g);

    if ( 0 == extHandle )
    {
        return -1;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
    {
        return -1;
    }

    try
    {
        return static_cast<int>(g->getNumPoints());
    }
    catch (const std::exception &e)
    {
        handle->ERROR_MESSAGE("%s", e.what());
    }
    catch (...)
    {
        handle->ERROR_MESSAGE("Unknown exception thrown");
    }
    
    return -1;
}

/*
 * Return -1 on exception, 0 otherwise. 
 * Converts Geometry to normal form (or canonical form).
 */
int
GEOSNormalize_r(GEOSContextHandle_t extHandle, Geometry *g)
{
    assert(0 != g);

    if ( 0 == extHandle )
    {
        return -1;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
    {
        return -1;
    }

    try
    {
        g->normalize();
        return 0; // SUCCESS
    }
    catch (const std::exception &e)
    {
        handle->ERROR_MESSAGE("%s", e.what());
    }
    catch (...)
    {
        handle->ERROR_MESSAGE("Unknown exception thrown");
    }
    
    return -1;
}

int
GEOSGetNumInteriorRings_r(GEOSContextHandle_t extHandle, const Geometry *g1)
{
    if ( 0 == extHandle )
    {
        return -1;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
    {
        return -1;
    }

    try
    {
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
    }
    catch (...)
    {
        handle->ERROR_MESSAGE("Unknown exception thrown");
    }
    
    return -1;
}


// returns -1 on error and 1 for non-multi geometries
int
GEOSGetNumGeometries_r(GEOSContextHandle_t extHandle, const Geometry *g1)
{
    if ( 0 == extHandle )
    {
        return -1;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
    {
        return -1;
    }

    try
    {
        return static_cast<int>(g1->getNumGeometries());
    }
    catch (const std::exception &e)
    {
        handle->ERROR_MESSAGE("%s", e.what());
    }
    catch (...)
    {
        handle->ERROR_MESSAGE("Unknown exception thrown");
    }
    
    return -1;
}


/*
 * Call only on GEOMETRYCOLLECTION or MULTI*.
 * Return a pointer to the internal Geometry.
 */
const Geometry *
GEOSGetGeometryN_r(GEOSContextHandle_t extHandle, const Geometry *g1, int n)
{
    if ( 0 == extHandle )
    {
        return NULL;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
    {
        return NULL;
    }

    try
    {
        using geos::geom::GeometryCollection;
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
    }
    catch (...)
    {
        handle->ERROR_MESSAGE("Unknown exception thrown");
    }
    
    return NULL;
}


/*
 * Call only on polygon
 * Return a copy of the internal Geometry.
 */
const Geometry *
GEOSGetExteriorRing_r(GEOSContextHandle_t extHandle, const Geometry *g1)
{
    if ( 0 == extHandle )
    {
        return NULL;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
    {
        return NULL;
    }

    try
    {
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
    }
    catch (...)
    {
        handle->ERROR_MESSAGE("Unknown exception thrown");
    }
    
    return NULL;
}

/*
 * Call only on polygon
 * Return a pointer to internal storage, do not destroy it.
 */
const Geometry *
GEOSGetInteriorRingN_r(GEOSContextHandle_t extHandle, const Geometry *g1, int n)
{
    if ( 0 == extHandle )
    {
        return NULL;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
    {
        return NULL;
    }

    try
    {
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
    }
    catch (...)
    {
        handle->ERROR_MESSAGE("Unknown exception thrown");
    }
    
    return NULL;
}

Geometry *
GEOSGetCentroid_r(GEOSContextHandle_t extHandle, const Geometry *g)
{
    if ( 0 == extHandle )
    {
        return NULL;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
    {
        return NULL;
    }

    try
    {
        Geometry *ret = g->getCentroid();
        if (0 == ret)
        {
            const GeometryFactory *gf = handle->geomFactory;
            return gf->createGeometryCollection();
        }
        return ret;
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

Geometry *
GEOSGeom_createCollection_r(GEOSContextHandle_t extHandle, int type, Geometry **geoms, unsigned int ngeoms)
{
    if ( 0 == extHandle )
    {
        return NULL;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
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
        const GeometryFactory* gf = handle->geomFactory;
        std::vector<Geometry*>* vgeoms = new std::vector<Geometry*>(geoms, geoms + ngeoms);

        Geometry *g = 0;
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
                g = 0;
                
        }
        
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

    return 0;
}

Geometry *
GEOSPolygonize_r(GEOSContextHandle_t extHandle, const Geometry * const * g, unsigned int ngeoms)
{
    if ( 0 == extHandle )
    {
        return 0;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
    {
        return 0;
    }

    Geometry *out = 0;

    try
    {
        // Polygonize
        using geos::operation::polygonize::Polygonizer;
        Polygonizer plgnzr;
        for (std::size_t i = 0; i < ngeoms; ++i)
        {
            plgnzr.add(g[i]);
        }

#if GEOS_DEBUG
        handle->NOTICE_MESSAGE("geometry vector added to polygonizer");
#endif

        std::vector<Polygon*> *polys = plgnzr.getPolygons();
        assert(0 != polys);

#if GEOS_DEBUG
        handle->NOTICE_MESSAGE("output polygons got");
#endif

        // We need a vector of Geometry pointers, not Polygon pointers.
        // STL vector doesn't allow transparent upcast of this
        // nature, so we explicitly convert.
        // (it's just a waste of processor and memory, btw)
        //
        // XXX mloskot: Why not to extent GeometryFactory to accept
        // vector of polygons or extend Polygonizer to return list of Geometry*
        // or add a wrapper which semantic is similar to:
        // std::vector<as_polygon<Geometry*> >
        std::vector<Geometry*> *polyvec = new std::vector<Geometry *>(polys->size());
        
        for (std::size_t i = 0; i < polys->size(); ++i)
        {
            (*polyvec)[i] = (*polys)[i];
        }
        delete polys;
        polys = 0;

        const GeometryFactory *gf = handle->geomFactory;

        // The below takes ownership of the passed vector,
        // so we must *not* delete it
        out = gf->createGeometryCollection(polyvec);
    }
    catch (const std::exception &e)
    {
        handle->ERROR_MESSAGE("%s", e.what());
    }
    catch (...)
    {
        handle->ERROR_MESSAGE("Unknown exception thrown");
    }

    return out;
}

Geometry *
GEOSPolygonizer_getCutEdges_r(GEOSContextHandle_t extHandle, const Geometry * const * g, unsigned int ngeoms)
{
    if ( 0 == extHandle )
    {
        return 0;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
    {
        return 0;
    }

    Geometry *out = 0;

    try
    {
        // Polygonize
        using geos::operation::polygonize::Polygonizer;
        Polygonizer plgnzr;
        for (std::size_t i = 0; i < ngeoms; ++i)
        {
            plgnzr.add(g[i]);
        }

#if GEOS_DEBUG
        handle->NOTICE_MESSAGE("geometry vector added to polygonizer");
#endif

        std::vector<const LineString *>* lines = plgnzr.getCutEdges();
        assert(0 != lines);

#if GEOS_DEBUG
        handle->NOTICE_MESSAGE("output polygons got");
#endif

        // We need a vector of Geometry pointers, not Polygon pointers.
        // STL vector doesn't allow transparent upcast of this
        // nature, so we explicitly convert.
        // (it's just a waste of processor and memory, btw)
        // XXX mloskot: See comment for GEOSPolygonize_r
        std::vector<Geometry*> *linevec = new std::vector<Geometry *>(lines->size());

        for (std::size_t i = 0; i < lines->size(); ++i)
        {
            (*linevec)[i] = (*lines)[i]->clone();
        }
        // FIXME mloskot: Who deallocates vector pointed by lines* ?

        const GeometryFactory *gf = handle->geomFactory;

        // The below takes ownership of the passed vector,
        // so we must *not* delete it
        out = gf->createGeometryCollection(linevec);
    }
    catch (const std::exception &e)
    {
        handle->ERROR_MESSAGE("%s", e.what());
    }
    catch (...)
    {
        handle->ERROR_MESSAGE("Unknown exception thrown");
    }

    return out;
}

Geometry *
GEOSLineMerge_r(GEOSContextHandle_t extHandle, const Geometry *g)
{
    if ( 0 == extHandle )
    {
        return 0;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
    {
        return 0;
    }

    Geometry *out = 0;

    try
    {
        using geos::operation::linemerge::LineMerger;
        LineMerger lmrgr;
        lmrgr.add(g);

        std::vector<LineString *>* lines = lmrgr.getMergedLineStrings();
        assert(0 != lines);

#if GEOS_DEBUG
        handle->NOTICE_MESSAGE("output lines got");
#endif

        std::vector<Geometry *>*geoms = new std::vector<Geometry *>(lines->size());
        for (std::vector<Geometry *>::size_type i = 0; i < lines->size(); ++i)
        {
            (*geoms)[i] = (*lines)[i];
        }
        delete lines;
        lines = 0;

        const GeometryFactory *gf = handle->geomFactory;
        out = gf->buildGeometry(geoms);

        // XXX: old version
        //out = gf->createGeometryCollection(geoms);
    }
    catch (const std::exception &e)
    {
        handle->ERROR_MESSAGE("%s", e.what());
    }
    catch (...)
    {
        handle->ERROR_MESSAGE("Unknown exception thrown");
    }

    return out;
}

int
GEOSGetSRID_r(GEOSContextHandle_t extHandle, const Geometry *g)
{
    assert(0 != g);

    if ( 0 == extHandle )
    {
        return 0;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
    {
        return 0;
    }

    try
    {
        return g->getSRID();
    }
    catch (const std::exception &e)
    {
        handle->ERROR_MESSAGE("%s", e.what());
    }
    catch (...)
    {
        handle->ERROR_MESSAGE("Unknown exception thrown");
    }
    
    return 0;
}

const char* GEOSversion()
{
    return GEOS_CAPI_VERSION;
}

const char* GEOSjtsport()
{
    return GEOS_JTS_PORT;
}

char 
GEOSHasZ_r(GEOSContextHandle_t extHandle, const Geometry *g)
{
    assert(0 != g);

    if ( 0 == extHandle )
    {
        return -1;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
    {
        return -1;
    }

    if (g->isEmpty())
    {
        return false;
    }
    assert(0 != g->getCoordinate());

    double az = g->getCoordinate()->z;
    //handle->ERROR_MESSAGE("ZCoord: %g", az);

    return static_cast<char>(FINITE(az));
}

int
GEOS_getWKBOutputDims_r(GEOSContextHandle_t extHandle)
{
    if ( 0 == extHandle )
    {
        return -1;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
    {
        return -1;
    }

    return handle->WKBOutputDims;
}

int
GEOS_setWKBOutputDims_r(GEOSContextHandle_t extHandle, int newdims)
{
    if ( 0 == extHandle )
    {
        return -1;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
    {
        return -1;
    }

    if ( newdims < 2 || newdims > 3 )
    {
        handle->ERROR_MESSAGE("WKB output dimensions out of range 2..3");
    }

    const int olddims = handle->WKBOutputDims;
    handle->WKBOutputDims = newdims;

    return olddims;
}

int
GEOS_getWKBByteOrder_r(GEOSContextHandle_t extHandle)
{
    if ( 0 == extHandle )
    {
        return -1;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
    {
        return -1;
    }

    return handle->WKBByteOrder;
}

int
GEOS_setWKBByteOrder_r(GEOSContextHandle_t extHandle, int byteOrder)
{
    if ( 0 == extHandle )
    {
        return -1;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
    {
        return -1;
    }

    const int oldByteOrder = handle->WKBByteOrder;
    handle->WKBByteOrder = byteOrder;

    return oldByteOrder;
}


CoordinateSequence *
GEOSCoordSeq_create_r(GEOSContextHandle_t extHandle, unsigned int size, unsigned int dims)
{
    if ( 0 == extHandle )
    {
        return NULL;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
    {
        return NULL;
    }

    try
    {
        const GeometryFactory *gf = handle->geomFactory;
        return gf->getCoordinateSequenceFactory()->create(size, dims);
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
GEOSCoordSeq_setOrdinate_r(GEOSContextHandle_t extHandle, CoordinateSequence *cs,
                           unsigned int idx, unsigned int dim, double val)
{
    assert(0 != cs);
    if ( 0 == extHandle )
    {
        return 0;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
    {
        return 0;
    }

    try
    {
        cs->setOrdinate(static_cast<int>(idx), static_cast<int>(dim), val);
        return 1;
    }
    catch (const std::exception &e)
    {
        handle->ERROR_MESSAGE("%s", e.what());
    }
    catch (...)
    {
        handle->ERROR_MESSAGE("Unknown exception thrown");
    }
    
    return 0;
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
GEOSCoordSeq_clone_r(GEOSContextHandle_t extHandle, const CoordinateSequence *cs)
{
    assert(0 != cs);

    if ( 0 == extHandle )
    {
        return NULL;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
    {
        return NULL;
    }

    try
    {
        return cs->clone();
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
GEOSCoordSeq_getOrdinate_r(GEOSContextHandle_t extHandle, const CoordinateSequence *cs,
                           unsigned int idx, unsigned int dim, double *val)
{
    assert(0 != cs);
    assert(0 != val);

    if ( 0 == extHandle )
    {
        return 0;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
    {
        return 0;
    }

    try
    {
        double d = cs->getOrdinate(static_cast<int>(idx), static_cast<int>(dim));
        *val = d;

        return 1;
    }
    catch (const std::exception &e)
    {
        handle->ERROR_MESSAGE("%s", e.what());
    }
    catch (...)
    {
        handle->ERROR_MESSAGE("Unknown exception thrown");
    }
    
    return 0;
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
GEOSCoordSeq_getSize_r(GEOSContextHandle_t extHandle, const CoordinateSequence *cs, unsigned int *size)
{
    assert(0 != cs);
    assert(0 != size);

    if ( 0 == extHandle )
    {
        return 0;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
    {
        return 0;
    }

    try
    {
        const std::size_t sz = cs->getSize();
        *size = static_cast<unsigned int>(sz);
        return 1;
    }
    catch (const std::exception &e)
    {
        handle->ERROR_MESSAGE("%s", e.what());
    }
    catch (...)
    {
        handle->ERROR_MESSAGE("Unknown exception thrown");
    }
    
    return 0;
}

int
GEOSCoordSeq_getDimensions_r(GEOSContextHandle_t extHandle, const CoordinateSequence *cs, unsigned int *dims)
{
    assert(0 != cs);
    assert(0 != dims);

    if ( 0 == extHandle )
    {
        return 0;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
    {
        return 0;
    }

    try
    {
        const std::size_t dim = cs->getDimension();
        *dims = static_cast<unsigned int>(dim);

        return 1;
    }
    catch (const std::exception &e)
    {
        handle->ERROR_MESSAGE("%s", e.what());
    }

    catch (...)
    {
        handle->ERROR_MESSAGE("Unknown exception thrown");
    }
    
    return 0;
}

void
GEOSCoordSeq_destroy_r(GEOSContextHandle_t extHandle, CoordinateSequence *s)
{
    GEOSContextHandleInternal_t *handle = 0;

    try
    {
        delete s;
    }
    catch (const std::exception &e)
    {
        if ( 0 == extHandle )
        {
            return;
        }

        handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
        if ( 0 == handle->initialized )
        {
            return;
        }

        handle->ERROR_MESSAGE("%s", e.what());
    }
    catch (...)
    {
        if ( 0 == extHandle )
        {
            return;
        }

        handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
        if ( 0 == handle->initialized )
        {
            return;
        }

        handle->ERROR_MESSAGE("Unknown exception thrown");
    }
}

const CoordinateSequence *
GEOSGeom_getCoordSeq_r(GEOSContextHandle_t extHandle, const Geometry *g)
{
    if ( 0 == extHandle )
    {
        return 0;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
    {
        return 0;
    }

    try
    {
        using geos::geom::Point;

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
        return 0;
    }
    catch (const std::exception &e)
    {
        handle->ERROR_MESSAGE("%s", e.what());
    }
    catch (...)
    {
        handle->ERROR_MESSAGE("Unknown exception thrown");
    }
    
    return 0;
}

Geometry *
GEOSGeom_createPoint_r(GEOSContextHandle_t extHandle, CoordinateSequence *cs)
{
    if ( 0 == extHandle )
    {
        return 0;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
    {
        return 0;
    }

    try
    { 
        const GeometryFactory *gf = handle->geomFactory;
        return gf->createPoint(cs);
    }
    catch (const std::exception &e)
    {
        handle->ERROR_MESSAGE("%s", e.what());
    }
    catch (...)
    {
        handle->ERROR_MESSAGE("Unknown exception thrown");
    }

    return 0;
}

Geometry *
GEOSGeom_createLinearRing_r(GEOSContextHandle_t extHandle, CoordinateSequence *cs)
{
    if ( 0 == extHandle )
    {
        return NULL;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
    {
        return NULL;
    }

    try
    { 
        const GeometryFactory *gf = handle->geomFactory;

        return gf->createLinearRing(cs);
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

Geometry *
GEOSGeom_createLineString_r(GEOSContextHandle_t extHandle, CoordinateSequence *cs)
{
    if ( 0 == extHandle )
    {
        return NULL;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
    {
        return NULL;
    }

    try
    { 
        const GeometryFactory *gf = handle->geomFactory;

        return gf->createLineString(cs);
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

Geometry *
GEOSGeom_createPolygon_r(GEOSContextHandle_t extHandle, Geometry *shell, Geometry **holes, unsigned int nholes)
{
    // FIXME: holes must be non-nullptr or may be nullptr?
    //assert(0 != holes);

    if ( 0 == extHandle )
    {
        return NULL;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
    {
        return NULL;
    }

    try
    {
        using geos::geom::LinearRing;
        
        std::vector<Geometry *> *vholes = new std::vector<Geometry *>(holes, holes + nholes);

        LinearRing *nshell = dynamic_cast<LinearRing *>(shell);
        if ( ! nshell )
        {
            handle->ERROR_MESSAGE("Shell is not a LinearRing");
            return NULL;
        }
        const GeometryFactory *gf = handle->geomFactory;

        return gf->createPolygon(nshell, vholes);
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

Geometry *
GEOSGeom_clone_r(GEOSContextHandle_t extHandle, const Geometry *g)
{
    assert(0 != g);

    if ( 0 == extHandle )
    {
        return NULL;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
    {
        return NULL;
    }

    try
    {
        return g->clone();
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
GEOSGeom_getDimensions_r(GEOSContextHandle_t extHandle, const Geometry *g)
{
    if ( 0 == extHandle )
    {
        return 0;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
    {
        return 0;
    }

    try
    {
        using geos::geom::Point;
        using geos::geom::GeometryCollection;

	if ( g->isEmpty() )
	{
		return 0;
	}

        std::size_t dim = 0;
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

        const GeometryCollection *coll = dynamic_cast<const GeometryCollection *>(g);
        if ( coll )
        {
            return GEOSGeom_getDimensions_r(extHandle, coll->getGeometryN(0));
        }

        handle->ERROR_MESSAGE("Unknown geometry type");
        return 0;
    }
    catch (const std::exception &e)
    {
        handle->ERROR_MESSAGE("%s", e.what());
    }
    catch (...)
    {
        handle->ERROR_MESSAGE("Unknown exception thrown");
    }
    
    return 0;
}

Geometry *
GEOSSimplify_r(GEOSContextHandle_t extHandle, const Geometry *g1, double tolerance)
{
    if ( 0 == extHandle )
    {
        return NULL;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
    {
        return NULL;
    }

    try
    {
        using namespace geos::simplify;
        Geometry::AutoPtr g(DouglasPeuckerSimplifier::simplify(g1, tolerance));
        return g.release();
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

Geometry *
GEOSTopologyPreserveSimplify_r(GEOSContextHandle_t extHandle, const Geometry *g1, double tolerance)
{
    if ( 0 == extHandle )
    {
        return NULL;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
    {
        return NULL;
    }

    try
    {
        using namespace geos::simplify;
        Geometry::AutoPtr g(TopologyPreservingSimplifier::simplify(g1, tolerance));
        return g.release();
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


/* WKT Reader */
WKTReader *
GEOSWKTReader_create_r(GEOSContextHandle_t extHandle)
{
    if ( 0 == extHandle )
    {
        return NULL;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
    {
        return NULL;
    }

    try
    {
        using geos::io::WKTReader;
        return new WKTReader((GeometryFactory*)handle->geomFactory);
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
GEOSWKTReader_destroy_r(GEOSContextHandle_t extHandle, WKTReader *reader)
{
    GEOSContextHandleInternal_t *handle = 0;

    try
    {
        delete reader;
    }
    catch (const std::exception &e)
    {
        if ( 0 == extHandle )
        {
            return;
        }

        handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
        if ( 0 == handle->initialized )
        {
            return;
        }

        handle->ERROR_MESSAGE("%s", e.what());
    }
    catch (...)
    {
        if ( 0 == extHandle )
        {
            return;
        }

        handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
        if ( 0 == handle->initialized )
        {
            return;
        }

        handle->ERROR_MESSAGE("Unknown exception thrown");
    }
}


Geometry*
GEOSWKTReader_read_r(GEOSContextHandle_t extHandle, WKTReader *reader, const char *wkt)
{
    assert(0 != reader);

    if ( 0 == extHandle )
    {
        return 0;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
    {
        return 0;
    }

    try
    {
        const std::string wktstring(wkt);
        Geometry *g = reader->read(wktstring);
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
    
    return 0;
}

/* WKT Writer */
WKTWriter *
GEOSWKTWriter_create_r(GEOSContextHandle_t extHandle)
{
    if ( 0 == extHandle )
    {
        return 0;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
    {
        return 0;
    }

    try
    {
        using geos::io::WKTWriter;
        return new WKTWriter();
    }
    catch (const std::exception &e)
    {
        handle->ERROR_MESSAGE("%s", e.what());
    }
    catch (...)
    {
        handle->ERROR_MESSAGE("Unknown exception thrown");
    }
    
    return 0;
}

void
GEOSWKTWriter_destroy_r(GEOSContextHandle_t extHandle, WKTWriter *Writer)
{

    GEOSContextHandleInternal_t *handle = 0;

    try
    {
        delete Writer;
    }
    catch (const std::exception &e)
    {
        if ( 0 == extHandle )
        {
            return;
        }

        handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
        if ( 0 == handle->initialized )
        {
            return;
        }

        handle->ERROR_MESSAGE("%s", e.what());
    }
    catch (...)
    {
        if ( 0 == extHandle )
        {
            return;
        }

        handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
        if ( 0 == handle->initialized )
        {
            return;
        }

        handle->ERROR_MESSAGE("Unknown exception thrown");
    }
}


char*
GEOSWKTWriter_write_r(GEOSContextHandle_t extHandle, WKTWriter *writer, const Geometry *geom)
{
    assert(0 != writer);

    if ( 0 == extHandle )
    {
        return NULL;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
    {
        return NULL;
    }

    try
    {
        std::string sgeom(writer->write(geom));
        char *result = gstrdup(sgeom);
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
    if ( 0 == extHandle )
    {
        return NULL;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
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
    GEOSContextHandleInternal_t *handle = 0;

    try
    {
        delete reader;
    }
    catch (const std::exception &e)
    {
        if ( 0 == extHandle )
        {
            return;
        }

        handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
        if ( 0 == handle->initialized )
        {
            return;
        }

        handle->ERROR_MESSAGE("%s", e.what());
    }
    catch (...)
    {
        if ( 0 == extHandle )
        {
            return;
        }

        handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
        if ( 0 == handle->initialized )
        {
            return;
        }

        handle->ERROR_MESSAGE("Unknown exception thrown");
    }
}


Geometry*
GEOSWKBReader_read_r(GEOSContextHandle_t extHandle, WKBReader *reader, const unsigned char *wkb, size_t size)
{
    assert(0 != reader);
    assert(0 != wkb);

    if ( 0 == extHandle )
    {
        return 0;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
    {
        return 0;
    }

    try
    {
        std::string wkbstring(reinterpret_cast<const char*>(wkb), size); // make it binary !
        std::istringstream is(std::ios_base::binary);
        is.str(wkbstring);
        is.seekg(0, std::ios::beg); // rewind reader pointer
        
        Geometry *g = reader->read(is);
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
    
    return 0;
}

Geometry*
GEOSWKBReader_readHEX_r(GEOSContextHandle_t extHandle, WKBReader *reader, const unsigned char *hex, size_t size)
{
    assert(0 != reader);
    assert(0 != hex);
    
    if ( 0 == extHandle )
    {
        return 0;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
    {
        return 0;
    }

    try
    {
        std::string hexstring(reinterpret_cast<const char*>(hex), size); 
        std::istringstream is(std::ios_base::binary);
        is.str(hexstring);
        is.seekg(0, std::ios::beg); // rewind reader pointer

        Geometry *g = reader->readHEX(is);
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
    
    return 0;
}

/* WKB Writer */
WKBWriter *
GEOSWKBWriter_create_r(GEOSContextHandle_t extHandle)
{
    if ( 0 == extHandle )
    {
        return NULL;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
    {
        return NULL;
    }

    try
    {
        using geos::io::WKBWriter;
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
    GEOSContextHandleInternal_t *handle = 0;

    try
    {
        delete Writer;
    }
    catch (const std::exception &e)
    {
        if ( 0 == extHandle )
        {
            return;
        }

        handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
        if ( 0 == handle->initialized )
        {
            return;
        }

        handle->ERROR_MESSAGE("%s", e.what());
    }
    catch (...)
    {
        if ( 0 == extHandle )
        {
            return;
        }

        handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
        if ( 0 == handle->initialized )
        {
            return;
        }

        handle->ERROR_MESSAGE("Unknown exception thrown");
    }
}


/* The caller owns the result */
unsigned char*
GEOSWKBWriter_write_r(GEOSContextHandle_t extHandle, WKBWriter *writer, const Geometry *geom, size_t *size)
{
    assert(0 != writer);
    assert(0 != geom);
    assert(0 != size);

    if ( 0 == extHandle )
    {
        return NULL;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
    {
        return NULL;
    }

    try
    {
        std::ostringstream os(std::ios_base::binary);
        writer->write(*geom, os);
        std::string wkbstring(os.str());
        const std::size_t len = wkbstring.length();

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

/* The caller owns the result */
unsigned char*
GEOSWKBWriter_writeHEX_r(GEOSContextHandle_t extHandle, WKBWriter *writer, const Geometry *geom, size_t *size)
{
    assert(0 != writer);
    assert(0 != geom);
    assert(0 != size);

    if ( 0 == extHandle )
    {
        return NULL;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
    {
        return NULL;
    }

    try
    {
        std::ostringstream os(std::ios_base::binary);
        writer->writeHEX(*geom, os);
        std::string wkbstring(os.str());
        const std::size_t len = wkbstring.length();

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
    assert(0 != writer);
    
    if ( 0 == extHandle )
    {
        return 0;
    }

    int ret = 0;

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 != handle->initialized )
    {
        try
        {
            ret = writer->getOutputDimension();
        }
        catch (...)
        {
            handle->ERROR_MESSAGE("Unknown exception thrown");
        }
    }

    return ret;
}

void
GEOSWKBWriter_setOutputDimension_r(GEOSContextHandle_t extHandle, GEOSWKBWriter* writer, int newDimension)
{
    assert(0 != writer);

    if ( 0 == extHandle )
    {
        return;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 != handle->initialized )
    {
        try
        {
            writer->setOutputDimension(newDimension);
        }
        catch (...)
        {
            handle->ERROR_MESSAGE("Unknown exception thrown");
        }
    }
}

int
GEOSWKBWriter_getByteOrder_r(GEOSContextHandle_t extHandle, const GEOSWKBWriter* writer)
{
    assert(0 != writer);

    if ( 0 == extHandle )
    {
        return 0;
    }

    int ret = 0;

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 != handle->initialized )
    {
        try
        {
            ret = writer->getByteOrder();
        }

        catch (...)
        {
            handle->ERROR_MESSAGE("Unknown exception thrown");
        }
    }

    return ret;
}

void
GEOSWKBWriter_setByteOrder_r(GEOSContextHandle_t extHandle, GEOSWKBWriter* writer, int newByteOrder)
{
    assert(0 != writer);

    if ( 0 == extHandle )
    {
        return;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 != handle->initialized )
    {
        try
        {
            writer->setByteOrder(newByteOrder);
        }
        catch (...)
        {
            handle->ERROR_MESSAGE("Unknown exception thrown");
        }
    }
}

char
GEOSWKBWriter_getIncludeSRID_r(GEOSContextHandle_t extHandle, const GEOSWKBWriter* writer)
{
    assert(0 != writer);

    if ( 0 == extHandle )
    {
        return -1;
    }

    int ret = -1;

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 != handle->initialized )
    {
        try
        {
            int srid = writer->getIncludeSRID();
            ret = static_cast<char>(srid);
        }
        catch (...)
        {
            handle->ERROR_MESSAGE("Unknown exception thrown");
        }
    }
    
    return static_cast<char>(ret);
}

void
GEOSWKBWriter_setIncludeSRID_r(GEOSContextHandle_t extHandle, GEOSWKBWriter* writer, const char newIncludeSRID)
{
    assert(0 != writer);

    if ( 0 == extHandle )
    {
        return;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 != handle->initialized )
    {
        try
        {
            writer->setIncludeSRID(newIncludeSRID);
        }
        catch (...)
        {
            handle->ERROR_MESSAGE("Unknown exception thrown");
        }
    }
}


//-----------------------------------------------------------------
// Prepared Geometry 
//-----------------------------------------------------------------

const geos::geom::prep::PreparedGeometry*
GEOSPrepare_r(GEOSContextHandle_t extHandle, const Geometry *g)
{
    if ( 0 == extHandle )
    {
        return 0;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
    {
        return 0;
    }

    const geos::geom::prep::PreparedGeometry* prep = 0;

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
    GEOSContextHandleInternal_t *handle = 0;

    try
    {
        delete a;
    }
    catch (const std::exception &e)
    {
        if ( 0 == extHandle )
        {
            return;
        }

        handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
        if ( 0 == handle->initialized )
        {
            return;
        }

        handle->ERROR_MESSAGE("%s", e.what());
    }
    catch (...)
    {
        if ( 0 == extHandle )
        {
            return;
        }

        handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
        if ( 0 == handle->initialized )
        {
            return;
        }

        handle->ERROR_MESSAGE("Unknown exception thrown");
    }
}

char
GEOSPreparedContains_r(GEOSContextHandle_t extHandle,
        const geos::geom::prep::PreparedGeometry *pg, const Geometry *g)
{
    assert(0 != pg);
    assert(0 != g);

    if ( 0 == extHandle )
    {
        return 2;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
    {
        return 2;
    }

    try 
    {
        bool result = pg->contains(g);
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
    
    return 2;
}

char
GEOSPreparedContainsProperly_r(GEOSContextHandle_t extHandle,
        const geos::geom::prep::PreparedGeometry *pg, const Geometry *g)
{
    assert(0 != pg);
    assert(0 != g);

    if ( 0 == extHandle )
    {
        return 2;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
    {
        return 2;
    }

    try 
    {
        bool result = pg->containsProperly(g);
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
    
    return 2;
}

char
GEOSPreparedCovers_r(GEOSContextHandle_t extHandle,
        const geos::geom::prep::PreparedGeometry *pg, const Geometry *g)
{
    assert(0 != pg);
    assert(0 != g);

    if ( 0 == extHandle )
    {
        return 2;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
    {
        return 2;
    }

    try 
    {
        bool result = pg->covers(g);
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
    
    return 2;
}

char
GEOSPreparedIntersects_r(GEOSContextHandle_t extHandle,
        const geos::geom::prep::PreparedGeometry *pg, const Geometry *g)
{
    assert(0 != pg);
    assert(0 != g);

    if ( 0 == extHandle )
    {
        return 2;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
    {
        return 2;
    }

    try 
    {
        bool result = pg->intersects(g);
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
    
    return 2;
}

//-----------------------------------------------------------------
// STRtree
//-----------------------------------------------------------------

geos::index::strtree::STRtree *
GEOSSTRtree_create_r(GEOSContextHandle_t extHandle,
                                  size_t nodeCapacity)
{
    if ( 0 == extHandle )
    {
        return 0;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
    {
        return 0;
    }

    geos::index::strtree::STRtree *tree = 0;

    try
    {
        tree = new geos::index::strtree::STRtree(nodeCapacity);
    }
    catch (const std::exception &e)
    {
        handle->ERROR_MESSAGE("%s", e.what());
    }
    catch (...)
    {
        handle->ERROR_MESSAGE("Unknown exception thrown");
    }
    
    return tree;
}

void
GEOSSTRtree_insert_r(GEOSContextHandle_t extHandle,
                     geos::index::strtree::STRtree *tree,
                     const geos::geom::Geometry *g,
                     void *item)
{
    GEOSContextHandleInternal_t *handle = 0;
    assert(tree != 0);
    assert(g != 0);

    try
    {
        tree->insert(g->getEnvelopeInternal(), item);
    }
    catch (const std::exception &e)
    {
        if ( 0 == extHandle )
        {
            return;
        }

        handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
        if ( 0 == handle->initialized )
        {
            return;
        }

        handle->ERROR_MESSAGE("%s", e.what());
    }
    catch (...)
    {
        if ( 0 == extHandle )
        {
            return;
        }

        handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
        if ( 0 == handle->initialized )
        {
            return;
        }

        handle->ERROR_MESSAGE("Unknown exception thrown");
    }
}

void 
GEOSSTRtree_query_r(GEOSContextHandle_t extHandle,
                    geos::index::strtree::STRtree *tree,
                    const geos::geom::Geometry *g,
                    GEOSQueryCallback callback,
                    void *userdata)
{
    GEOSContextHandleInternal_t *handle = 0;
    assert(tree != 0);
    assert(g != 0);
    assert(callback != 0);

    try
    {
        CAPI_ItemVisitor visitor(callback, userdata);
        tree->query(g->getEnvelopeInternal(), visitor);
    }
    catch (const std::exception &e)
    {
        if ( 0 == extHandle )
        {
            return;
        }

        handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
        if ( 0 == handle->initialized )
        {
            return;
        }

        handle->ERROR_MESSAGE("%s", e.what());
    }
    catch (...)
    {
        if ( 0 == extHandle )
        {
            return;
        }

        handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
        if ( 0 == handle->initialized )
        {
            return;
        }

        handle->ERROR_MESSAGE("Unknown exception thrown");
    }
}

void 
GEOSSTRtree_iterate_r(GEOSContextHandle_t extHandle,
                    geos::index::strtree::STRtree *tree,
                    GEOSQueryCallback callback,
                    void *userdata)
{
    GEOSContextHandleInternal_t *handle = 0;
    assert(tree != 0);
    assert(callback != 0);

    try
    {
        CAPI_ItemVisitor visitor(callback, userdata);
        tree->iterate(visitor);
    }
    catch (const std::exception &e)
    {
        if ( 0 == extHandle )
        {
            return;
        }

        handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
        if ( 0 == handle->initialized )
        {
            return;
        }

        handle->ERROR_MESSAGE("%s", e.what());
    }
    catch (...)
    {
        if ( 0 == extHandle )
        {
            return;
        }

        handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
        if ( 0 == handle->initialized )
        {
            return;
        }

        handle->ERROR_MESSAGE("Unknown exception thrown");
    }
}

char
GEOSSTRtree_remove_r(GEOSContextHandle_t extHandle,
                     geos::index::strtree::STRtree *tree,
                     const geos::geom::Geometry *g,
                     void *item)
{
    assert(0 != tree);
    assert(0 != g);

    if ( 0 == extHandle )
    {
        return 2;
    }

    GEOSContextHandleInternal_t *handle = 0;
    handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    if ( 0 == handle->initialized )
    {
        return 2;
    }

    try 
    {
        bool result = tree->remove(g->getEnvelopeInternal(), item);
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
    
    return 2;
}

void
GEOSSTRtree_destroy_r(GEOSContextHandle_t extHandle,
                      geos::index::strtree::STRtree *tree)
{
    GEOSContextHandleInternal_t *handle = 0;

    try
    {
        delete tree;
    }
    catch (const std::exception &e)
    {
        if ( 0 == extHandle )
        {
            return;
        }

        handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
        if ( 0 == handle->initialized )
        {
            return;
        }

        handle->ERROR_MESSAGE("%s", e.what());
    }
    catch (...)
    {
        if ( 0 == extHandle )
        {
            return;
        }

        handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
        if ( 0 == handle->initialized )
        {
            return;
        }

        handle->ERROR_MESSAGE("Unknown exception thrown");
    }
}

double
GEOSProject_r(GEOSContextHandle_t extHandle,
              const Geometry *g,
              const Geometry *p)
{

    const geos::geom::Point* point = dynamic_cast<const geos::geom::Point*>(p);
    if (!point) {
        if ( 0 == extHandle )
        {
            return -1.0;
        }
        GEOSContextHandleInternal_t *handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
        if ( 0 == handle->initialized )
        {
            return -1.0;
        }

        handle->ERROR_MESSAGE("third argument of GEOSProject_r must be Point*");
        return -1.0;
    }
    const geos::geom::Coordinate* inputPt = p->getCoordinate();
    return geos::linearref::LengthIndexedLine(g).project(*inputPt);
}


Geometry*
GEOSInterpolate_r(GEOSContextHandle_t extHandle, const Geometry *g, double d)
{
    geos::linearref::LengthIndexedLine lil(g);
    geos::geom::Coordinate coord = lil.extractPoint(d);
    GEOSContextHandleInternal_t *handle = reinterpret_cast<GEOSContextHandleInternal_t*>(extHandle);
    const GeometryFactory *gf = handle->geomFactory;
    Geometry* point = gf->createPoint(coord);
    return point;
}


double
GEOSProjectNormalized_r(GEOSContextHandle_t extHandle, const Geometry *g,
                        const Geometry *p)
{

    double length;
    GEOSLength_r(extHandle, g, &length);
    return GEOSProject_r(extHandle, g, p) / length;
}


Geometry*
GEOSInterpolateNormalized_r(GEOSContextHandle_t extHandle, const Geometry *g,
                            double d)
{
    double length;
    GEOSLength_r(extHandle, g, &length);
    return GEOSInterpolate_r(extHandle, g, d * length);
}


} /* extern "C" */

