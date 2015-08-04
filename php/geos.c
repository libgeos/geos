/***********************************************************************
 * 
 *    GEOS - Geometry Engine Open Source
 *    http://trac.osgeo.org/geos
 *
 *    Copyright (C) 2010 Sandro Santilli <strk@keybit.net>
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License as published by the Free Software Foundation; either
 *    version 2.1 of the License, or (at your option) any later version.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 51 Franklin St, Fifth Floor,
 *    Boston, MA  02110-1301  USA
 *
 ***********************************************************************/

/* PHP stuff */
#include "php.h"
#include "ext/standard/info.h" /* for php_info_... */
#include "Zend/zend_exceptions.h" /* for zend_throw_exception_object */

/* GEOS stuff */
#include "geos_c.h"

/* Own stuff */
#include "php_geos.h"

PHP_MINIT_FUNCTION(geos);
PHP_MSHUTDOWN_FUNCTION(geos);
PHP_RINIT_FUNCTION(geos);
PHP_RSHUTDOWN_FUNCTION(geos);
PHP_MINFO_FUNCTION(geos);
PHP_FUNCTION(GEOSVersion);
PHP_FUNCTION(GEOSPolygonize);
PHP_FUNCTION(GEOSLineMerge);
PHP_FUNCTION(GEOSSharedPaths);
PHP_FUNCTION(GEOSRelateMatch);

#if PHP_VERSION_ID < 50399 
#define zend_function_entry function_entry
#endif

static zend_function_entry geos_functions[] = {
    PHP_FE(GEOSVersion, NULL)
    PHP_FE(GEOSPolygonize, NULL)
    PHP_FE(GEOSLineMerge, NULL)
    PHP_FE(GEOSSharedPaths, NULL)
    PHP_FE(GEOSRelateMatch, NULL)
    {NULL, NULL, NULL}
};

zend_module_entry geos_module_entry = {
    STANDARD_MODULE_HEADER,
    PHP_GEOS_EXTNAME,
    geos_functions,
    PHP_MINIT(geos),              /* module init function */
    PHP_MSHUTDOWN(geos),          /* module shutdown function */
    PHP_RINIT(geos),              /* request init function */
    PHP_RSHUTDOWN(geos),          /* request shutdown function */
    PHP_MINFO(geos),              /* module info function */
    PHP_GEOS_VERSION,
    STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_GEOS
ZEND_GET_MODULE(geos)
#endif

/* -- Utility functions ---------------------- */

static void noticeHandler(const char *fmt, ...)
{
    TSRMLS_FETCH();
    char message[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(message, sizeof(message) - 1, fmt, args);
    va_end(args);

    php_error_docref(NULL TSRMLS_CC, E_NOTICE, "%s", message);
}

static void errorHandler(const char *fmt, ...)
{
    TSRMLS_FETCH();
    char message[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(message, sizeof(message) - 1, fmt, args);
    va_end(args);

    /* TODO: use a GEOSException ? */
    zend_throw_exception_ex(zend_exception_get_default(TSRMLS_C),
        1 TSRMLS_CC, "%s", message); 

}

typedef struct Proxy_t {
    zend_object std;
    void* relay;
} Proxy;

static void 
setRelay(zval* val, void* obj) {
    TSRMLS_FETCH();
    Proxy* proxy = (Proxy*)zend_object_store_get_object(val TSRMLS_CC);
    proxy->relay = obj;
}

static inline void *
getRelay(zval* val, zend_class_entry* ce) {
    TSRMLS_FETCH();
    Proxy *proxy =  (Proxy*)zend_object_store_get_object(val TSRMLS_CC);
    if ( proxy->std.ce != ce ) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR,
            "Relay object is not an %s", ce->name);
    }
    if ( ! proxy->relay ) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR,
            "Relay object for object of type %s is not set", ce->name);
    }
    return proxy->relay;
}

static long getZvalAsLong(zval* val)
{
    long ret;
    zval tmp;

    tmp = *val;
    zval_copy_ctor(&tmp);
    convert_to_long(&tmp);
    ret = Z_LVAL(tmp);
    zval_dtor(&tmp);
    return ret;
}

static long getZvalAsDouble(zval* val)
{
    double ret;
    zval tmp;

    tmp = *val;
    zval_copy_ctor(&tmp);
    convert_to_double(&tmp);
    ret = Z_DVAL(tmp);
    zval_dtor(&tmp);
    return ret;
}

static zend_object_value
Gen_create_obj (zend_class_entry *type,
    zend_objects_free_object_storage_t st, zend_object_handlers* handlers)
{
    TSRMLS_FETCH();
    zend_object_value retval;

    Proxy *obj = (Proxy *)emalloc(sizeof(Proxy));
    memset(obj, 0, sizeof(Proxy));
    obj->std.ce = type;

    ALLOC_HASHTABLE(obj->std.properties);
    zend_hash_init(obj->std.properties, 0, NULL, ZVAL_PTR_DTOR, 0);
#if PHP_VERSION_ID < 50399 
    zend_hash_copy(obj->std.properties, &type->default_properties,
        (copy_ctor_func_t)zval_add_ref, NULL, sizeof(zval *));
#else
    object_properties_init(&(obj->std), type);
#endif

    retval.handle = zend_objects_store_put(obj, NULL, st, NULL TSRMLS_CC);
    retval.handlers = handlers;

    return retval;
}


/* -- class GEOSGeometry -------------------- */

PHP_METHOD(Geometry, __construct);
PHP_METHOD(Geometry, __toString);
PHP_METHOD(Geometry, project);
PHP_METHOD(Geometry, interpolate);
PHP_METHOD(Geometry, buffer);
PHP_METHOD(Geometry, offsetCurve);
PHP_METHOD(Geometry, envelope);
PHP_METHOD(Geometry, intersection);
PHP_METHOD(Geometry, convexHull);
PHP_METHOD(Geometry, difference);
PHP_METHOD(Geometry, symDifference);
PHP_METHOD(Geometry, boundary);
PHP_METHOD(Geometry, union); /* also does union cascaded */
PHP_METHOD(Geometry, pointOnSurface); 
PHP_METHOD(Geometry, centroid); 
PHP_METHOD(Geometry, relate); 
PHP_METHOD(Geometry, relateBoundaryNodeRule); 
PHP_METHOD(Geometry, simplify); /* also does topology-preserving */
PHP_METHOD(Geometry, normalize);
PHP_METHOD(Geometry, extractUniquePoints); 
PHP_METHOD(Geometry, disjoint);
PHP_METHOD(Geometry, touches);
PHP_METHOD(Geometry, intersects);
PHP_METHOD(Geometry, crosses);
PHP_METHOD(Geometry, within);
PHP_METHOD(Geometry, contains);
PHP_METHOD(Geometry, overlaps);
PHP_METHOD(Geometry, covers);
PHP_METHOD(Geometry, coveredBy);
PHP_METHOD(Geometry, equals);
PHP_METHOD(Geometry, equalsExact);
PHP_METHOD(Geometry, isEmpty);
PHP_METHOD(Geometry, checkValidity);
PHP_METHOD(Geometry, isSimple);
PHP_METHOD(Geometry, isRing);
PHP_METHOD(Geometry, hasZ);
PHP_METHOD(Geometry, isClosed);
PHP_METHOD(Geometry, typeName);
PHP_METHOD(Geometry, typeId);
PHP_METHOD(Geometry, getSRID);
PHP_METHOD(Geometry, setSRID);
PHP_METHOD(Geometry, numGeometries);
PHP_METHOD(Geometry, geometryN);
PHP_METHOD(Geometry, numInteriorRings);
PHP_METHOD(Geometry, numPoints);
PHP_METHOD(Geometry, getX);
PHP_METHOD(Geometry, getY);
PHP_METHOD(Geometry, interiorRingN);
PHP_METHOD(Geometry, exteriorRing);
PHP_METHOD(Geometry, numCoordinates);
PHP_METHOD(Geometry, dimension);
PHP_METHOD(Geometry, coordinateDimension);
PHP_METHOD(Geometry, pointN);
PHP_METHOD(Geometry, startPoint);
PHP_METHOD(Geometry, endPoint);
PHP_METHOD(Geometry, area);
PHP_METHOD(Geometry, length);
PHP_METHOD(Geometry, distance);
PHP_METHOD(Geometry, hausdorffDistance);
PHP_METHOD(Geometry, snapTo);
PHP_METHOD(Geometry, node);
PHP_METHOD(Geometry, delaunayTriangulation);
PHP_METHOD(Geometry, voronoiDiagram);
PHP_METHOD(Geometry, clipByRect);

static zend_function_entry Geometry_methods[] = {
    PHP_ME(Geometry, __construct, NULL, 0)
    PHP_ME(Geometry, __toString, NULL, 0)
    PHP_ME(Geometry, project, NULL, 0)
    PHP_ME(Geometry, interpolate, NULL, 0)
    PHP_ME(Geometry, buffer, NULL, 0)
    PHP_ME(Geometry, offsetCurve, NULL, 0)
    PHP_ME(Geometry, envelope, NULL, 0)
    PHP_ME(Geometry, intersection, NULL, 0)
    PHP_ME(Geometry, convexHull, NULL, 0)
    PHP_ME(Geometry, difference, NULL, 0)
    PHP_ME(Geometry, symDifference, NULL, 0)
    PHP_ME(Geometry, boundary, NULL, 0)
    PHP_ME(Geometry, union, NULL, 0)
    PHP_ME(Geometry, pointOnSurface, NULL, 0)
    PHP_ME(Geometry, centroid, NULL, 0)
    PHP_ME(Geometry, relate, NULL, 0)
    PHP_ME(Geometry, relateBoundaryNodeRule, NULL, 0)
    PHP_ME(Geometry, simplify, NULL, 0)
    PHP_ME(Geometry, normalize, NULL, 0)
    PHP_ME(Geometry, extractUniquePoints, NULL, 0)
    PHP_ME(Geometry, disjoint, NULL, 0)
    PHP_ME(Geometry, touches, NULL, 0)
    PHP_ME(Geometry, intersects, NULL, 0)
    PHP_ME(Geometry, crosses, NULL, 0)
    PHP_ME(Geometry, within, NULL, 0)
    PHP_ME(Geometry, contains, NULL, 0)
    PHP_ME(Geometry, overlaps, NULL, 0)
    PHP_ME(Geometry, covers, NULL, 0)
    PHP_ME(Geometry, coveredBy, NULL, 0)
    PHP_ME(Geometry, equals, NULL, 0)
    PHP_ME(Geometry, equalsExact, NULL, 0)
    PHP_ME(Geometry, isEmpty, NULL, 0)
    PHP_ME(Geometry, checkValidity, NULL, 0)
    PHP_ME(Geometry, isSimple, NULL, 0)
    PHP_ME(Geometry, isRing, NULL, 0)
    PHP_ME(Geometry, hasZ, NULL, 0)
    PHP_ME(Geometry, isClosed, NULL, 0)
    PHP_ME(Geometry, typeName, NULL, 0)
    PHP_ME(Geometry, typeId, NULL, 0)
    PHP_ME(Geometry, getSRID, NULL, 0)
    PHP_ME(Geometry, setSRID, NULL, 0)
    PHP_ME(Geometry, numGeometries, NULL, 0)
    PHP_ME(Geometry, geometryN, NULL, 0)
    PHP_ME(Geometry, numInteriorRings, NULL, 0)
    PHP_ME(Geometry, numPoints, NULL, 0)
    PHP_ME(Geometry, getX, NULL, 0)
    PHP_ME(Geometry, getY, NULL, 0)
    PHP_ME(Geometry, interiorRingN, NULL, 0)
    PHP_ME(Geometry, exteriorRing, NULL, 0)
    PHP_ME(Geometry, numCoordinates, NULL, 0)
    PHP_ME(Geometry, dimension, NULL, 0)
    PHP_ME(Geometry, coordinateDimension, NULL, 0)
    PHP_ME(Geometry, pointN, NULL, 0)
    PHP_ME(Geometry, startPoint, NULL, 0)
    PHP_ME(Geometry, endPoint, NULL, 0)
    PHP_ME(Geometry, area, NULL, 0)
    PHP_ME(Geometry, length, NULL, 0)
    PHP_ME(Geometry, distance, NULL, 0)
    PHP_ME(Geometry, hausdorffDistance, NULL, 0)
    PHP_ME(Geometry, snapTo, NULL, 0)
    PHP_ME(Geometry, node, NULL, 0)
    PHP_ME(Geometry, delaunayTriangulation, NULL, 0)
    PHP_ME(Geometry, voronoiDiagram, NULL, 0)
    PHP_ME(Geometry, clipByRect, NULL, 0)
    {NULL, NULL, NULL}
};

static zend_class_entry *Geometry_ce_ptr;

static zend_object_handlers Geometry_object_handlers;

/* Geometry serializer */

static GEOSWKBWriter* Geometry_serializer = 0;

static GEOSWKBWriter* getGeometrySerializer()
{
    if ( ! Geometry_serializer ) {
        Geometry_serializer = GEOSWKBWriter_create();
        GEOSWKBWriter_setIncludeSRID(Geometry_serializer, 1);
        GEOSWKBWriter_setOutputDimension(Geometry_serializer, 3);
    }
    return Geometry_serializer;
}

static void delGeometrySerializer()
{
    if ( Geometry_serializer ) {
        GEOSWKBWriter_destroy(Geometry_serializer);
    }
}

/* Geometry deserializer */

static GEOSWKBReader* Geometry_deserializer = 0;

static GEOSWKBReader* getGeometryDeserializer()
{
    if ( ! Geometry_deserializer ) {
        Geometry_deserializer = GEOSWKBReader_create();
    }
    return Geometry_deserializer;
}

static void delGeometryDeserializer()
{
    if ( Geometry_deserializer ) {
        GEOSWKBReader_destroy(Geometry_deserializer);
    }
}

/* Serializer function for GEOSGeometry */

static int
Geometry_serialize(zval *object, unsigned char **buffer, zend_uint *buf_len,
        zend_serialize_data *data TSRMLS_DC)
{
    GEOSWKBWriter *serializer;
    GEOSGeometry *geom;
    char* ret;
    size_t retsize;


    serializer = getGeometrySerializer();
    geom = (GEOSGeometry*)getRelay(object, Geometry_ce_ptr);

    /* NOTE: we might be fine using binary here */
    ret = (char*)GEOSWKBWriter_writeHEX(serializer, geom, &retsize);
    /* we'll probably get an exception if ret is null */
    if ( ! ret ) return FAILURE;

    *buffer = (unsigned char*)estrndup(ret, retsize);
    GEOSFree(ret);

    *buf_len = retsize;

    return SUCCESS; 
}

static int
Geometry_deserialize(zval **object, zend_class_entry *ce, const unsigned char *buf,
        zend_uint buf_len, zend_unserialize_data *data TSRMLS_DC)
{
    GEOSWKBReader* deserializer;
    GEOSGeometry* geom;

    deserializer = getGeometryDeserializer();
    geom = GEOSWKBReader_readHEX(deserializer, buf, buf_len);

    /* TODO: check zend_class_entry being what we expect! */
    if ( ce != Geometry_ce_ptr ) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR,
                "Geometry_deserialize called with unexpected zend_class_entry");
        return FAILURE;
    }
    object_init_ex(*object, ce);
    setRelay(*object, geom);

    return SUCCESS;
}

/*
 * Push components of the given geometry
 * to the given array zval.
 * Components geometries are cloned.
 * NOTE: collection components are not descended into
 */
static void
dumpGeometry(GEOSGeometry* g, zval* array)
{
    TSRMLS_FETCH();
    int ngeoms, i;

    /*
    MAKE_STD_ZVAL(array);
    array_init(array);
    */

    ngeoms = GEOSGetNumGeometries(g);
    for (i=0; i<ngeoms; ++i)
    {
        zval *tmp;
        GEOSGeometry* cc;
        const GEOSGeometry* c = GEOSGetGeometryN(g, i);
        if ( ! c ) continue; /* should get an exception */
        /* we _need_ to clone as this one is owned by 'g' */
        cc = GEOSGeom_clone(c);
        if ( ! cc ) continue; /* should get an exception */

        MAKE_STD_ZVAL(tmp);
        object_init_ex(tmp, Geometry_ce_ptr);
        setRelay(tmp, cc);
        add_next_index_zval(array, tmp); 
    }
}


static void
Geometry_dtor (void *object TSRMLS_DC)
{
    Proxy *obj = (Proxy *)object;
    GEOSGeom_destroy((GEOSGeometry*)obj->relay);

    zend_hash_destroy(obj->std.properties);
    FREE_HASHTABLE(obj->std.properties);

    efree(obj);
}

static zend_object_value
Geometry_create_obj (zend_class_entry *type TSRMLS_DC)
{
    return Gen_create_obj(type, Geometry_dtor, &Geometry_object_handlers);
}


PHP_METHOD(Geometry, __construct)
{
    php_error_docref(NULL TSRMLS_CC, E_ERROR,
            "GEOSGeometry can't be constructed using new, check WKTReader");

}

PHP_METHOD(Geometry, __toString)
{
    GEOSGeometry *geom;
    GEOSWKTWriter *writer;
    char *wkt;
    char *ret;

    geom = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);
    writer = GEOSWKTWriter_create();
    /* NOTE: if we get an exception before reaching
     *       GEOSWKTWriter_destory below we'll be leaking memory.
     *       One fix could be storing the object in a refcounted
     *       zval.
     */
    GEOSWKTWriter_setTrim(writer, 1);

    wkt = GEOSWKTWriter_write(writer, geom);
    /* we'll probably get an exception if wkt is null */
    if ( ! wkt ) RETURN_NULL();

    GEOSWKTWriter_destroy(writer);
    

    ret = estrdup(wkt);
    GEOSFree(wkt);

    RETURN_STRING(ret, 0);
}

PHP_METHOD(Geometry, project)
{
    GEOSGeometry *this;
    GEOSGeometry *other;
    zval *zobj;
    zend_bool normalized = 0;
    double ret;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o|b", &zobj,
            &normalized) == FAILURE) {
        RETURN_NULL();
    }
    other = getRelay(zobj, Geometry_ce_ptr);

    if ( normalized ) {
        ret = GEOSProjectNormalized(this, other);
    } else {
        ret = GEOSProject(this, other);
    }
    if ( ret < 0 ) RETURN_NULL(); /* should get an exception first */

    RETURN_DOUBLE(ret);
}

PHP_METHOD(Geometry, interpolate)
{
    GEOSGeometry *this;
    double dist;
    GEOSGeometry *ret;
    zend_bool normalized = 0;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "d|b",
            &dist, &normalized) == FAILURE) {
        RETURN_NULL();
    }

    if ( normalized ) {
        ret = GEOSInterpolateNormalized(this, dist);
    } else {
        ret = GEOSInterpolate(this, dist);
    }
    if ( ! ret ) RETURN_NULL(); /* should get an exception first */

    /* return_value is a zval */
    object_init_ex(return_value, Geometry_ce_ptr);
    setRelay(return_value, ret);
}

/**
 * GEOSGeometry::buffer(dist, [<styleArray>])
 *
 * styleArray keys supported:
 *  'quad_segs'
 *       Type: int 
 *       Number of segments used to approximate
 *       a quarter circle (defaults to 8).
 *  'endcap'
 *       Type: long
 *       Endcap style (defaults to GEOSBUF_CAP_ROUND)
 *  'join'
 *       Type: long
 *       Join style (defaults to GEOSBUF_JOIN_ROUND)
 *  'mitre_limit'
 *       Type: double
 *       mitre ratio limit (only affects joins with GEOSBUF_JOIN_MITRE style)
 *       'miter_limit' is also accepted as a synonym for 'mitre_limit'.
 *  'single_sided'
 *       Type: bool
 *       If true buffer lines only on one side, so that the input line
 *       will be a portion of the boundary of the returned polygon.
 *       Only applies to lineal input. Defaults to false.
 */
PHP_METHOD(Geometry, buffer)
{
    GEOSGeometry *this;
    double dist;
    GEOSGeometry *ret;
    GEOSBufferParams *params;
    static const double default_mitreLimit = 5.0;
    static const int default_endCapStyle = GEOSBUF_CAP_ROUND;
    static const int default_joinStyle = GEOSBUF_JOIN_ROUND;
    static const int default_quadSegs = 8;
    long int quadSegs = default_quadSegs;
    long int endCapStyle = default_endCapStyle;
    long int joinStyle = default_joinStyle;
    double mitreLimit = default_mitreLimit;
    long singleSided = 0;
    zval *style_val = NULL;
    zval **data;
    HashTable *style;
    char *key;
    ulong index;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "d|a",
            &dist, &style_val) == FAILURE) {
        RETURN_NULL();
    }

    params = GEOSBufferParams_create();

    if ( style_val )
    {
        style = HASH_OF(style_val);
        while(zend_hash_get_current_key(style, &key, &index, 0)
              == HASH_KEY_IS_STRING)
        {
            if(!strcmp(key, "quad_segs"))
            {
                zend_hash_get_current_data(style, (void**)&data);
                quadSegs = getZvalAsLong(*data);
                GEOSBufferParams_setQuadrantSegments(params, quadSegs);
            }
            else if(!strcmp(key, "endcap"))
            {
                zend_hash_get_current_data(style, (void**)&data);
                endCapStyle = getZvalAsLong(*data);
                GEOSBufferParams_setEndCapStyle(params, endCapStyle);
            }
            else if(!strcmp(key, "join"))
            {
                zend_hash_get_current_data(style, (void**)&data);
                joinStyle = getZvalAsLong(*data);
                GEOSBufferParams_setJoinStyle(params, joinStyle);
            }
            else if(!strcmp(key, "mitre_limit"))
            {
                zend_hash_get_current_data(style, (void**)&data);
                mitreLimit = getZvalAsDouble(*data);
                GEOSBufferParams_setMitreLimit(params, mitreLimit);
            }
            else if(!strcmp(key, "single_sided"))
            {
                zend_hash_get_current_data(style, (void**)&data);
                singleSided = getZvalAsLong(*data);
                GEOSBufferParams_setSingleSided(params, singleSided);
            }

            zend_hash_move_forward(style);
        }
    }

    ret = GEOSBufferWithParams(this, params, dist);
    GEOSBufferParams_destroy(params);
    if ( ! ret ) RETURN_NULL(); /* should get an exception first */

    /* return_value is a zval */
    object_init_ex(return_value, Geometry_ce_ptr);
    setRelay(return_value, ret);
}

/**
 * GEOSGeometry::offsetCurve(dist, [<styleArray>])
 *
 * styleArray keys supported:
 *  'quad_segs'
 *       Type: int 
 *       Number of segments used to approximate
 *       a quarter circle (defaults to 8).
 *  'join'
 *       Type: long
 *       Join style (defaults to GEOSBUF_JOIN_ROUND)
 *  'mitre_limit'
 *       Type: double
 *       mitre ratio limit (only affects joins with GEOSBUF_JOIN_MITRE style)
 *       'miter_limit' is also accepted as a synonym for 'mitre_limit'.
 */
PHP_METHOD(Geometry, offsetCurve)
{
    GEOSGeometry *this;
    double dist;
    GEOSGeometry *ret;
    static const double default_mitreLimit = 5.0;
    static const int default_joinStyle = GEOSBUF_JOIN_ROUND;
    static const int default_quadSegs = 8;
    long int quadSegs = default_quadSegs;
    long int joinStyle = default_joinStyle;
    double mitreLimit = default_mitreLimit;
    zval *style_val = NULL;
    zval **data;
    HashTable *style;
    char *key;
    ulong index;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "d|a",
            &dist, &style_val) == FAILURE) {
        RETURN_NULL();
    }

    if ( style_val )
    {
        style = HASH_OF(style_val);
        while(zend_hash_get_current_key(style, &key, &index, 0)
              == HASH_KEY_IS_STRING)
        {
            if(!strcmp(key, "quad_segs"))
            {
                zend_hash_get_current_data(style, (void**)&data);
                quadSegs = getZvalAsLong(*data);
            }
            else if(!strcmp(key, "join"))
            {
                zend_hash_get_current_data(style, (void**)&data);
                joinStyle = getZvalAsLong(*data);
            }
            else if(!strcmp(key, "mitre_limit"))
            {
                zend_hash_get_current_data(style, (void**)&data);
                mitreLimit = getZvalAsDouble(*data);
            }

            zend_hash_move_forward(style);
        }
    }

    ret = GEOSOffsetCurve(this, dist, quadSegs, joinStyle, mitreLimit);
    if ( ! ret ) RETURN_NULL(); /* should get an exception first */

    /* return_value is a zval */
    object_init_ex(return_value, Geometry_ce_ptr);
    setRelay(return_value, ret);
}

PHP_METHOD(Geometry, envelope)
{
    GEOSGeometry *this;
    GEOSGeometry *ret;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    ret = GEOSEnvelope(this);
    if ( ! ret ) RETURN_NULL(); /* should get an exception first */

    /* return_value is a zval */
    object_init_ex(return_value, Geometry_ce_ptr);
    setRelay(return_value, ret);
}

PHP_METHOD(Geometry, intersection)
{
    GEOSGeometry *this;
    GEOSGeometry *other;
    GEOSGeometry *ret;
    zval *zobj;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &zobj)
            == FAILURE) {
        RETURN_NULL();
    }
    other = getRelay(zobj, Geometry_ce_ptr);

    ret = GEOSIntersection(this, other);
    if ( ! ret ) RETURN_NULL(); /* should get an exception first */

    /* return_value is a zval */
    object_init_ex(return_value, Geometry_ce_ptr);
    setRelay(return_value, ret);
}

/**
 * GEOSGeometry GEOSGeometry::clipByRect(xmin,ymin,xmax,ymax)
 */
PHP_METHOD(Geometry, clipByRect)
{
    GEOSGeometry *this;
    GEOSGeometry *ret;
    double xmin,ymin,xmax,ymax;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "dddd",
            &xmin, &ymin, &xmax, &ymax) == FAILURE) {
        RETURN_NULL();
    }

    ret = GEOSClipByRect(this, xmin, ymin, xmax, ymax);
    if ( ! ret ) RETURN_NULL(); /* should get an exception first */

    /* return_value is a zval */
    object_init_ex(return_value, Geometry_ce_ptr);
    setRelay(return_value, ret);
}

PHP_METHOD(Geometry, convexHull)
{
    GEOSGeometry *this;
    GEOSGeometry *ret;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    ret = GEOSConvexHull(this);
    if ( ret == NULL ) RETURN_NULL(); /* should get an exception first */

    /* return_value is a zval */
    object_init_ex(return_value, Geometry_ce_ptr);
    setRelay(return_value, ret);
}

PHP_METHOD(Geometry, difference)
{
    GEOSGeometry *this;
    GEOSGeometry *other;
    GEOSGeometry *ret;
    zval *zobj;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &zobj)
            == FAILURE) {
        RETURN_NULL();
    }
    other = getRelay(zobj, Geometry_ce_ptr);

    ret = GEOSDifference(this, other);
    if ( ! ret ) RETURN_NULL(); /* should get an exception first */

    /* return_value is a zval */
    object_init_ex(return_value, Geometry_ce_ptr);
    setRelay(return_value, ret);
}

PHP_METHOD(Geometry, symDifference)
{
    GEOSGeometry *this;
    GEOSGeometry *other;
    GEOSGeometry *ret;
    zval *zobj;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &zobj)
            == FAILURE) {
        RETURN_NULL();
    }
    other = getRelay(zobj, Geometry_ce_ptr);

    ret = GEOSSymDifference(this, other);
    if ( ! ret ) RETURN_NULL(); /* should get an exception first */

    /* return_value is a zval */
    object_init_ex(return_value, Geometry_ce_ptr);
    setRelay(return_value, ret);
}

PHP_METHOD(Geometry, boundary)
{
    GEOSGeometry *this;
    GEOSGeometry *ret;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    ret = GEOSBoundary(this);
    if ( ret == NULL ) RETURN_NULL(); /* should get an exception first */

    /* return_value is a zval */
    object_init_ex(return_value, Geometry_ce_ptr);
    setRelay(return_value, ret);
}

/**
 * GEOSGeometry::union(otherGeom)
 * GEOSGeometry::union()
 */
PHP_METHOD(Geometry, union)
{
    GEOSGeometry *this;
    GEOSGeometry *other;
    GEOSGeometry *ret;
    zval *zobj = NULL;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|o", &zobj)
            == FAILURE) {
        RETURN_NULL();
    }

    if ( zobj ) {
        other = getRelay(zobj, Geometry_ce_ptr);
        ret = GEOSUnion(this, other);
    } else {
        ret = GEOSUnaryUnion(this);
    }

    if ( ! ret ) RETURN_NULL(); /* should get an exception first */

    /* return_value is a zval */
    object_init_ex(return_value, Geometry_ce_ptr);
    setRelay(return_value, ret);
}

/**
 * GEOSGeometry::pointOnSurface()
 */
PHP_METHOD(Geometry, pointOnSurface)
{
    GEOSGeometry *this;
    GEOSGeometry *ret;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    ret = GEOSPointOnSurface(this);
    if ( ret == NULL ) RETURN_NULL(); /* should get an exception first */

    /* return_value is a zval */
    object_init_ex(return_value, Geometry_ce_ptr);
    setRelay(return_value, ret);
}

/**
 * GEOSGeometry::centroid()
 */
PHP_METHOD(Geometry, centroid)
{
    GEOSGeometry *this;
    GEOSGeometry *ret;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    ret = GEOSGetCentroid(this);
    if ( ret == NULL ) RETURN_NULL(); /* should get an exception first */

    /* return_value is a zval */
    object_init_ex(return_value, Geometry_ce_ptr);
    setRelay(return_value, ret);
}

/**
 * GEOSGeometry::relate(otherGeom)
 * GEOSGeometry::relate(otherGeom, pattern)
 */
PHP_METHOD(Geometry, relate)
{
    GEOSGeometry *this;
    GEOSGeometry *other;
    zval *zobj;
    char* pat = NULL;
    int patlen;
    int retInt;
    zend_bool retBool;
    char* retStr;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o|s",
        &zobj, &pat, &patlen) == FAILURE)
    {
        RETURN_NULL();
    }

    other = getRelay(zobj, Geometry_ce_ptr);

    if ( ! pat ) {
        /* we'll compute it */
        pat = GEOSRelate(this, other);
        if ( ! pat ) RETURN_NULL(); /* should get an exception first */
        retStr = estrdup(pat);
        GEOSFree(pat);
        RETURN_STRING(retStr, 0);
    } else {
        retInt = GEOSRelatePattern(this, other, pat);
        if ( retInt == 2 ) RETURN_NULL(); /* should get an exception first */
        retBool = retInt;
        RETURN_BOOL(retBool);
    }

}

/**
 * GEOSGeometry::relateBoundaryNodeRule(otherGeom, rule)
 */
PHP_METHOD(Geometry, relateBoundaryNodeRule)
{
    GEOSGeometry *this;
    GEOSGeometry *other;
    zval *zobj;
    char* pat;
    long int bnr = GEOSRELATE_BNR_OGC;
    char* retStr;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ol",
        &zobj, &bnr) == FAILURE)
    {
        RETURN_NULL();
    }

    other = getRelay(zobj, Geometry_ce_ptr);

    /* we'll compute it */
    pat = GEOSRelateBoundaryNodeRule(this, other, bnr);
    if ( ! pat ) RETURN_NULL(); /* should get an exception first */
    retStr = estrdup(pat);
    GEOSFree(pat);
    RETURN_STRING(retStr, 0);
}

/**
 * GEOSGeometry GEOSGeometry::simplify(tolerance)
 * GEOSGeometry GEOSGeometry::simplify(tolerance, preserveTopology)
 */
PHP_METHOD(Geometry, simplify)
{
    GEOSGeometry *this;
    double tolerance;
    zend_bool preserveTopology = 0;
    GEOSGeometry *ret;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "d|b",
            &tolerance, &preserveTopology) == FAILURE) {
        RETURN_NULL();
    }

    if ( preserveTopology ) {
        ret = GEOSTopologyPreserveSimplify(this, tolerance);
    } else {
        ret = GEOSSimplify(this, tolerance);
    }

    if ( ! ret ) RETURN_NULL(); /* should get an exception first */

    /* return_value is a zval */
    object_init_ex(return_value, Geometry_ce_ptr);
    setRelay(return_value, ret);
}

/**
 * GEOSGeometry GEOSGeometry::normalize()
 */
PHP_METHOD(Geometry, normalize)
{
    GEOSGeometry *this;
    GEOSGeometry *ret;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    ret = GEOSGeom_clone(this);

    if ( ! ret ) RETURN_NULL();

    GEOSNormalize(ret); /* exception should be gotten automatically */

    /* return_value is a zval */
    object_init_ex(return_value, Geometry_ce_ptr);
    setRelay(return_value, ret);
}

/**
 * GEOSGeometry GEOSGeometry::extractUniquePoints()
 */
PHP_METHOD(Geometry, extractUniquePoints)
{
    GEOSGeometry *this;
    GEOSGeometry *ret;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    ret = GEOSGeom_extractUniquePoints(this);
    if ( ret == NULL ) RETURN_NULL(); /* should get an exception first */

    /* return_value is a zval */
    object_init_ex(return_value, Geometry_ce_ptr);
    setRelay(return_value, ret);
}

/**
 * bool GEOSGeometry::disjoint(GEOSGeometry)
 */
PHP_METHOD(Geometry, disjoint)
{
    GEOSGeometry *this;
    GEOSGeometry *other;
    int ret;
    zend_bool retBool;
    zval *zobj;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &zobj)
            == FAILURE) {
        RETURN_NULL();
    }
    other = getRelay(zobj, Geometry_ce_ptr);

    ret = GEOSDisjoint(this, other);
    if ( ret == 2 ) RETURN_NULL(); /* should get an exception first */

    /* return_value is a zval */
    retBool = ret;
    RETURN_BOOL(retBool);
}

/**
 * bool GEOSGeometry::touches(GEOSGeometry)
 */
PHP_METHOD(Geometry, touches)
{
    GEOSGeometry *this;
    GEOSGeometry *other;
    int ret;
    zend_bool retBool;
    zval *zobj;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &zobj)
            == FAILURE) {
        RETURN_NULL();
    }
    other = getRelay(zobj, Geometry_ce_ptr);

    ret = GEOSTouches(this, other);
    if ( ret == 2 ) RETURN_NULL(); /* should get an exception first */

    /* return_value is a zval */
    retBool = ret;
    RETURN_BOOL(retBool);
}

/**
 * bool GEOSGeometry::intersects(GEOSGeometry)
 */
PHP_METHOD(Geometry, intersects)
{
    GEOSGeometry *this;
    GEOSGeometry *other;
    int ret;
    zend_bool retBool;
    zval *zobj;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &zobj)
            == FAILURE) {
        RETURN_NULL();
    }
    other = getRelay(zobj, Geometry_ce_ptr);

    ret = GEOSIntersects(this, other);
    if ( ret == 2 ) RETURN_NULL(); /* should get an exception first */

    /* return_value is a zval */
    retBool = ret;
    RETURN_BOOL(retBool);
}

/**
 * bool GEOSGeometry::crosses(GEOSGeometry)
 */
PHP_METHOD(Geometry, crosses)
{
    GEOSGeometry *this;
    GEOSGeometry *other;
    int ret;
    zend_bool retBool;
    zval *zobj;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &zobj)
            == FAILURE) {
        RETURN_NULL();
    }
    other = getRelay(zobj, Geometry_ce_ptr);

    ret = GEOSCrosses(this, other);
    if ( ret == 2 ) RETURN_NULL(); /* should get an exception first */

    /* return_value is a zval */
    retBool = ret;
    RETURN_BOOL(retBool);
}

/**
 * bool GEOSGeometry::within(GEOSGeometry)
 */
PHP_METHOD(Geometry, within)
{
    GEOSGeometry *this;
    GEOSGeometry *other;
    int ret;
    zend_bool retBool;
    zval *zobj;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &zobj)
            == FAILURE) {
        RETURN_NULL();
    }
    other = getRelay(zobj, Geometry_ce_ptr);

    ret = GEOSWithin(this, other);
    if ( ret == 2 ) RETURN_NULL(); /* should get an exception first */

    /* return_value is a zval */
    retBool = ret;
    RETURN_BOOL(retBool);
}

/**
 * bool GEOSGeometry::contains(GEOSGeometry)
 */
PHP_METHOD(Geometry, contains)
{
    GEOSGeometry *this;
    GEOSGeometry *other;
    int ret;
    zend_bool retBool;
    zval *zobj;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &zobj)
            == FAILURE) {
        RETURN_NULL();
    }
    other = getRelay(zobj, Geometry_ce_ptr);

    ret = GEOSContains(this, other);
    if ( ret == 2 ) RETURN_NULL(); /* should get an exception first */

    /* return_value is a zval */
    retBool = ret;
    RETURN_BOOL(retBool);
}

/**
 * bool GEOSGeometry::overlaps(GEOSGeometry)
 */
PHP_METHOD(Geometry, overlaps)
{
    GEOSGeometry *this;
    GEOSGeometry *other;
    int ret;
    zend_bool retBool;
    zval *zobj;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &zobj)
            == FAILURE) {
        RETURN_NULL();
    }
    other = getRelay(zobj, Geometry_ce_ptr);

    ret = GEOSOverlaps(this, other);
    if ( ret == 2 ) RETURN_NULL(); /* should get an exception first */

    /* return_value is a zval */
    retBool = ret;
    RETURN_BOOL(retBool);
}

/**
 * bool GEOSGeometry::covers(GEOSGeometry)
 */
PHP_METHOD(Geometry, covers)
{
    GEOSGeometry *this;
    GEOSGeometry *other;
    int ret;
    zend_bool retBool;
    zval *zobj;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &zobj)
            == FAILURE) {
        RETURN_NULL();
    }
    other = getRelay(zobj, Geometry_ce_ptr);

    ret = GEOSCovers(this, other);
    if ( ret == 2 ) RETURN_NULL(); /* should get an exception first */

    /* return_value is a zval */
    retBool = ret;
    RETURN_BOOL(retBool);
}

/**
 * bool GEOSGeometry::coveredBy(GEOSGeometry)
 */
PHP_METHOD(Geometry, coveredBy)
{
    GEOSGeometry *this;
    GEOSGeometry *other;
    int ret;
    zend_bool retBool;
    zval *zobj;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &zobj)
            == FAILURE) {
        RETURN_NULL();
    }
    other = getRelay(zobj, Geometry_ce_ptr);

    ret = GEOSCoveredBy(this, other);
    if ( ret == 2 ) RETURN_NULL(); /* should get an exception first */

    /* return_value is a zval */
    retBool = ret;
    RETURN_BOOL(retBool);
}

/**
 * bool GEOSGeometry::equals(GEOSGeometry)
 */
PHP_METHOD(Geometry, equals)
{
    GEOSGeometry *this;
    GEOSGeometry *other;
    int ret;
    zend_bool retBool;
    zval *zobj;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o",
        &zobj) == FAILURE) {
        RETURN_NULL();
    }
    other = getRelay(zobj, Geometry_ce_ptr);

    ret = GEOSEquals(this, other);
    if ( ret == 2 ) RETURN_NULL(); /* should get an exception first */

    /* return_value is a zval */
    retBool = ret;
    RETURN_BOOL(retBool);
}

/**
 * bool GEOSGeometry::equalsExact(GEOSGeometry)
 * bool GEOSGeometry::equalsExact(GEOSGeometry, double tolerance)
 */
PHP_METHOD(Geometry, equalsExact)
{
    GEOSGeometry *this;
    GEOSGeometry *other;
    int ret;
    double tolerance = 0;
    zend_bool retBool;
    zval *zobj;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o|d",
        &zobj, &tolerance) == FAILURE) {
        RETURN_NULL();
    }
    other = getRelay(zobj, Geometry_ce_ptr);

    ret = GEOSEqualsExact(this, other, tolerance);
    if ( ret == 2 ) RETURN_NULL(); /* should get an exception first */

    /* return_value is a zval */
    retBool = ret;
    RETURN_BOOL(retBool);
}

/**
 * bool GEOSGeometry::isEmpty()
 */
PHP_METHOD(Geometry, isEmpty)
{
    GEOSGeometry *this;
    int ret;
    zend_bool retBool;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    ret = GEOSisEmpty(this);
    if ( ret == 2 ) RETURN_NULL(); /* should get an exception first */

    /* return_value is a zval */
    retBool = ret;
    RETURN_BOOL(retBool);
}

/**
 * array GEOSGeometry::checkValidity()
 */
PHP_METHOD(Geometry, checkValidity)
{
    GEOSGeometry *this;
    GEOSGeometry *location = NULL;
    int ret;
    char *reason = NULL;
    zend_bool retBool;
    char *reasonVal = NULL;
    zval *locationVal = NULL;
    long int flags = 0;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|l",
        &flags) == FAILURE) {
        RETURN_NULL();
    }

    ret = GEOSisValidDetail(this, flags, &reason, &location);
    if ( ret == 2 ) RETURN_NULL(); /* should get an exception first */

    if ( reason ) {
        reasonVal = estrdup(reason);
        GEOSFree(reason);
    }

    if ( location ) {
        MAKE_STD_ZVAL(locationVal);
        object_init_ex(locationVal, Geometry_ce_ptr);
        setRelay(locationVal, location);
    }

    retBool = ret;

    /* return value is an array */
    array_init(return_value);
    add_assoc_bool(return_value, "valid", retBool); 
    if ( reasonVal ) add_assoc_string(return_value, "reason", reasonVal, 0); 
    if ( locationVal ) add_assoc_zval(return_value, "location", locationVal); 

}

/**
 * bool GEOSGeometry::isSimple()
 */
PHP_METHOD(Geometry, isSimple)
{
    GEOSGeometry *this;
    int ret;
    zend_bool retBool;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    ret = GEOSisSimple(this);
    if ( ret == 2 ) RETURN_NULL(); /* should get an exception first */

    /* return_value is a zval */
    retBool = ret;
    RETURN_BOOL(retBool);
}

/**
 * bool GEOSGeometry::isRing()
 */
PHP_METHOD(Geometry, isRing)
{
    GEOSGeometry *this;
    int ret;
    zend_bool retBool;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    ret = GEOSisRing(this);
    if ( ret == 2 ) RETURN_NULL(); /* should get an exception first */

    /* return_value is a zval */
    retBool = ret;
    RETURN_BOOL(retBool);
}

/**
 * bool GEOSGeometry::hasZ()
 */
PHP_METHOD(Geometry, hasZ)
{
    GEOSGeometry *this;
    int ret;
    zend_bool retBool;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    ret = GEOSHasZ(this);
    if ( ret == 2 ) RETURN_NULL(); /* should get an exception first */

    /* return_value is a zval */
    retBool = ret;
    RETURN_BOOL(retBool);
}

/**
 * bool GEOSGeometry::isClosed()
 */
PHP_METHOD(Geometry, isClosed)
{
    GEOSGeometry *this;
    int ret;
    zend_bool retBool;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    ret = GEOSisClosed(this);
    if ( ret == 2 ) RETURN_NULL(); /* should get an exception first */

    /* return_value is a zval */
    retBool = ret;
    RETURN_BOOL(retBool);
}

/**
 * string GEOSGeometry::typeName()
 */
PHP_METHOD(Geometry, typeName)
{
    GEOSGeometry *this;
    char *typ;
    char *typVal;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    /* TODO: define constant strings instead... */

    typ = GEOSGeomType(this);
    if ( ! typ ) RETURN_NULL(); /* should get an exception first */

    typVal = estrdup(typ);
    GEOSFree(typ);

    RETURN_STRING(typVal, 0);
}

/**
 * long GEOSGeometry::typeId()
 */
PHP_METHOD(Geometry, typeId)
{
    GEOSGeometry *this;
    long typ;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    /* TODO: define constant strings instead... */

    typ = GEOSGeomTypeId(this);
    if ( typ == -1 ) RETURN_NULL(); /* should get an exception first */

    RETURN_LONG(typ);
}

/**
 * long GEOSGeometry::getSRID()
 */
PHP_METHOD(Geometry, getSRID)
{
    GEOSGeometry *geom;
    long int ret;

    geom = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    ret = GEOSGetSRID(geom);

    RETURN_LONG(ret);
}

/**
 * void GEOSGeometry::setSRID(long)
 */
PHP_METHOD(Geometry, setSRID)
{
    GEOSGeometry *geom;
    long int srid;

    geom = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l",
        &srid) == FAILURE) {
        RETURN_NULL();
    }

    GEOSSetSRID(geom, srid);
}

/**
 * long GEOSGeometry::numGeometries()
 */
PHP_METHOD(Geometry, numGeometries)
{
    GEOSGeometry *geom;
    long int ret;

    geom = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    ret = GEOSGetNumGeometries(geom);
    if ( ret == -1 ) RETURN_NULL(); /* should get an exception first */

    RETURN_LONG(ret);
}

/**
 * GEOSGeometry GEOSGeometry::geometryN()
 */
PHP_METHOD(Geometry, geometryN)
{
    GEOSGeometry *geom;
    const GEOSGeometry *c;
    GEOSGeometry *cc;
    long int num;

    geom = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l",
        &num) == FAILURE) {
        RETURN_NULL();
    }

    if ( num >= GEOSGetNumGeometries(geom) ) RETURN_NULL(); 
    c = GEOSGetGeometryN(geom, num);
    if ( ! c ) RETURN_NULL(); /* should get an exception first */
    cc = GEOSGeom_clone(c);
    if ( ! cc ) RETURN_NULL(); /* should get an exception first */

    object_init_ex(return_value, Geometry_ce_ptr);
    setRelay(return_value, cc);
}

/**
 * long GEOSGeometry::numInteriorRings()
 */
PHP_METHOD(Geometry, numInteriorRings)
{
    GEOSGeometry *geom;
    long int ret;

    geom = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    ret = GEOSGetNumInteriorRings(geom);
    if ( ret == -1 ) RETURN_NULL(); /* should get an exception first */

    RETURN_LONG(ret);
}

/**
 * long GEOSGeometry::numPoints()
 */
PHP_METHOD(Geometry, numPoints)
{
    GEOSGeometry *geom;
    long int ret;

    geom = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    ret = GEOSGeomGetNumPoints(geom);
    if ( ret == -1 ) RETURN_NULL(); /* should get an exception first */

    RETURN_LONG(ret);
}

/**
 * double GEOSGeometry::getX()
 */
PHP_METHOD(Geometry, getX)
{
    GEOSGeometry *geom;
    int ret;
    double x;

    geom = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    ret = GEOSGeomGetX(geom, &x);
    if ( ret == -1 ) RETURN_NULL(); /* should get an exception first */

    RETURN_DOUBLE(x);
}

/**
 * double GEOSGeometry::getY()
 */
PHP_METHOD(Geometry, getY)
{
    GEOSGeometry *geom;
    int ret;
    double y;

    geom = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    ret = GEOSGeomGetY(geom, &y);
    if ( ret == -1 ) RETURN_NULL(); /* should get an exception first */

    RETURN_DOUBLE(y);
}

/**
 * GEOSGeometry GEOSGeometry::interiorRingN()
 */
PHP_METHOD(Geometry, interiorRingN)
{
    GEOSGeometry *geom;
    const GEOSGeometry *c;
    GEOSGeometry *cc;
    long int num;

    geom = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l",
        &num) == FAILURE) {
        RETURN_NULL();
    }

    if ( num >= GEOSGetNumInteriorRings(geom) ) RETURN_NULL(); 
    c = GEOSGetInteriorRingN(geom, num);
    if ( ! c ) RETURN_NULL(); /* should get an exception first */
    cc = GEOSGeom_clone(c);
    if ( ! cc ) RETURN_NULL(); /* should get an exception first */

    object_init_ex(return_value, Geometry_ce_ptr);
    setRelay(return_value, cc);
}

/**
 * GEOSGeometry GEOSGeometry::exteriorRing()
 */
PHP_METHOD(Geometry, exteriorRing)
{
    GEOSGeometry *geom;
    const GEOSGeometry *c;
    GEOSGeometry *cc;

    geom = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    c = GEOSGetExteriorRing(geom);
    if ( ! c ) RETURN_NULL(); /* should get an exception first */
    cc = GEOSGeom_clone(c);
    if ( ! cc ) RETURN_NULL(); /* should get an exception first */

    object_init_ex(return_value, Geometry_ce_ptr);
    setRelay(return_value, cc);
}

/**
 * long GEOSGeometry::numCoordinates()
 */
PHP_METHOD(Geometry, numCoordinates)
{
    GEOSGeometry *geom;
    long int ret;

    geom = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    ret = GEOSGetNumCoordinates(geom);
    if ( ret == -1 ) RETURN_NULL(); /* should get an exception first */

    RETURN_LONG(ret);
}

/**
 * long GEOSGeometry::dimension()
 * 0:puntual 1:lineal 2:areal
 */
PHP_METHOD(Geometry, dimension)
{
    GEOSGeometry *geom;
    long int ret;

    geom = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    ret = GEOSGeom_getDimensions(geom);
    if ( ret == -1 ) RETURN_NULL(); /* should get an exception first */

    RETURN_LONG(ret);
}

/**
 * long GEOSGeometry::coordinateDimension()
 */
PHP_METHOD(Geometry, coordinateDimension)
{
    GEOSGeometry *geom;
    long int ret;

    geom = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    ret = GEOSGeom_getCoordinateDimension(geom);
    if ( ret == -1 ) RETURN_NULL(); /* should get an exception first */

    RETURN_LONG(ret);
}

/**
 * GEOSGeometry GEOSGeometry::pointN()
 */
PHP_METHOD(Geometry, pointN)
{
    GEOSGeometry *geom;
    GEOSGeometry *c;
    long int num;

    geom = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l",
        &num) == FAILURE) {
        RETURN_NULL();
    }

    if ( num >= GEOSGeomGetNumPoints(geom) ) RETURN_NULL(); 
    c = GEOSGeomGetPointN(geom, num);
    if ( ! c ) RETURN_NULL(); /* should get an exception first */

    object_init_ex(return_value, Geometry_ce_ptr);
    setRelay(return_value, c);
}

/**
 * GEOSGeometry GEOSGeometry::startPoint()
 */
PHP_METHOD(Geometry, startPoint)
{
    GEOSGeometry *geom;
    GEOSGeometry *c;

    geom = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    c = GEOSGeomGetStartPoint(geom);
    if ( ! c ) RETURN_NULL(); /* should get an exception first */

    object_init_ex(return_value, Geometry_ce_ptr);
    setRelay(return_value, c);
}

/**
 * GEOSGeometry GEOSGeometry::endPoint()
 */
PHP_METHOD(Geometry, endPoint)
{
    GEOSGeometry *geom;
    GEOSGeometry *c;

    geom = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    c = GEOSGeomGetEndPoint(geom);
    if ( ! c ) RETURN_NULL(); /* should get an exception first */

    object_init_ex(return_value, Geometry_ce_ptr);
    setRelay(return_value, c);
}

/**
 * double GEOSGeometry::area()
 */
PHP_METHOD(Geometry, area)
{
    GEOSGeometry *geom;
    double area;
    int ret;

    geom = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    ret = GEOSArea(geom, &area);
    if ( ! ret ) RETURN_NULL(); /* should get an exception first */

    RETURN_DOUBLE(area);
}

/**
 * double GEOSGeometry::length()
 */
PHP_METHOD(Geometry, length)
{
    GEOSGeometry *geom;
    double length;
    int ret;

    geom = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    ret = GEOSLength(geom, &length);
    if ( ! ret ) RETURN_NULL(); /* should get an exception first */

    RETURN_DOUBLE(length);
}

/**
 * double GEOSGeometry::distance(GEOSGeometry)
 */
PHP_METHOD(Geometry, distance)
{
    GEOSGeometry *this;
    GEOSGeometry *other;
    zval *zobj;
    double dist;
    int ret;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o",
        &zobj) == FAILURE)
    {
        RETURN_NULL();
    }

    other = getRelay(zobj, Geometry_ce_ptr);

    ret = GEOSDistance(this, other, &dist);
    if ( ! ret ) RETURN_NULL(); /* should get an exception first */

    RETURN_DOUBLE(dist);
}

/**
 * double GEOSGeometry::hausdorffDistance(GEOSGeometry)
 */
PHP_METHOD(Geometry, hausdorffDistance)
{
    GEOSGeometry *this;
    GEOSGeometry *other;
    zval *zobj;
    double dist;
    int ret;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o",
        &zobj) == FAILURE)
    {
        RETURN_NULL();
    }

    other = getRelay(zobj, Geometry_ce_ptr);

    ret = GEOSHausdorffDistance(this, other, &dist);
    if ( ! ret ) RETURN_NULL(); /* should get an exception first */

    RETURN_DOUBLE(dist);
}

PHP_METHOD(Geometry, snapTo)
{
    GEOSGeometry *this;
    GEOSGeometry *other;
    GEOSGeometry *ret;
    double tolerance;
    zval *zobj;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "od", &zobj,
            &tolerance) == FAILURE) {
        RETURN_NULL();
    }
    other = getRelay(zobj, Geometry_ce_ptr);

    ret = GEOSSnap(this, other, tolerance);
    if ( ! ret ) RETURN_NULL(); /* should get an exception first */

    /* return_value is a zval */
    object_init_ex(return_value, Geometry_ce_ptr);
    setRelay(return_value, ret);
}

PHP_METHOD(Geometry, node)
{
    GEOSGeometry *this;
    GEOSGeometry *ret;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    ret = GEOSNode(this);
    if ( ! ret ) RETURN_NULL(); /* should get an exception first */

    /* return_value is a zval */
    object_init_ex(return_value, Geometry_ce_ptr);
    setRelay(return_value, ret);
}



/* -- class GEOSWKTReader -------------------- */

PHP_METHOD(WKTReader, __construct);
PHP_METHOD(WKTReader, read);

static zend_function_entry WKTReader_methods[] = {
    PHP_ME(WKTReader, __construct, NULL, 0)
    PHP_ME(WKTReader, read, NULL, 0)
    {NULL, NULL, NULL}
};

static zend_class_entry *WKTReader_ce_ptr;

static zend_object_handlers WKTReader_object_handlers;

static void
WKTReader_dtor (void *object TSRMLS_DC)
{
    Proxy *obj = (Proxy *)object;
    GEOSWKTReader_destroy((GEOSWKTReader*)obj->relay);

    zend_hash_destroy(obj->std.properties);
    FREE_HASHTABLE(obj->std.properties);

    efree(obj);
}

static zend_object_value
WKTReader_create_obj (zend_class_entry *type TSRMLS_DC)
{
    return Gen_create_obj(type, WKTReader_dtor, &WKTReader_object_handlers);
}


PHP_METHOD(WKTReader, __construct)
{
    GEOSWKTReader* obj;
    zval *object = getThis();

    obj = GEOSWKTReader_create();
    if ( ! obj ) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR,
                "GEOSWKTReader_create() failed (didn't initGEOS?)");
    }

    setRelay(object, obj);
}

PHP_METHOD(WKTReader, read)
{
    GEOSWKTReader *reader;
    GEOSGeometry *geom;
    char* wkt;
    int wktlen;

    reader = (GEOSWKTReader*)getRelay(getThis(), WKTReader_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s",
        &wkt, &wktlen) == FAILURE)
    {
        RETURN_NULL();
    }

    geom = GEOSWKTReader_read(reader, wkt);
    /* we'll probably get an exception if geom is null */
    if ( ! geom ) RETURN_NULL();
 
    /* return_value is a zval */
    object_init_ex(return_value, Geometry_ce_ptr);
    setRelay(return_value, geom);

}

/* -- class GEOSWKTWriter -------------------- */

PHP_METHOD(WKTWriter, __construct);
PHP_METHOD(WKTWriter, write);
PHP_METHOD(WKTWriter, setTrim);
PHP_METHOD(WKTWriter, setRoundingPrecision);
PHP_METHOD(WKTWriter, setOutputDimension);
PHP_METHOD(WKTWriter, getOutputDimension);
PHP_METHOD(WKTWriter, setOld3D);

static zend_function_entry WKTWriter_methods[] = {
    PHP_ME(WKTWriter, __construct, NULL, 0)
    PHP_ME(WKTWriter, write, NULL, 0)
    PHP_ME(WKTWriter, setTrim, NULL, 0)
    PHP_ME(WKTWriter, setRoundingPrecision, NULL, 0)
    PHP_ME(WKTWriter, setOutputDimension, NULL, 0)
    PHP_ME(WKTWriter, getOutputDimension, NULL, 0)
    PHP_ME(WKTWriter, setOld3D, NULL, 0)
    {NULL, NULL, NULL}
};

static zend_class_entry *WKTWriter_ce_ptr;

static zend_object_handlers WKTWriter_object_handlers;

static void
WKTWriter_dtor (void *object TSRMLS_DC)
{
    Proxy *obj = (Proxy *)object;
    GEOSWKTWriter_destroy((GEOSWKTWriter*)obj->relay);

    zend_hash_destroy(obj->std.properties);
    FREE_HASHTABLE(obj->std.properties);

    efree(obj);
}

static zend_object_value
WKTWriter_create_obj (zend_class_entry *type TSRMLS_DC)
{
    return Gen_create_obj(type, WKTWriter_dtor, &WKTWriter_object_handlers);
}

PHP_METHOD(WKTWriter, __construct)
{
    GEOSWKTWriter* obj;
    zval *object = getThis();

    obj = GEOSWKTWriter_create();
    if ( ! obj ) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR,
                "GEOSWKTWriter_create() failed (didn't initGEOS?)");
    }

    setRelay(object, obj);
}

PHP_METHOD(WKTWriter, write)
{
    GEOSWKTWriter *writer;
    zval *zobj;
    GEOSGeometry *geom;
    char* wkt;
    char* retstr;

    writer = (GEOSWKTWriter*)getRelay(getThis(), WKTWriter_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &zobj)
        == FAILURE)
    {
        RETURN_NULL();
    }

    geom = getRelay(zobj, Geometry_ce_ptr);

    wkt = GEOSWKTWriter_write(writer, geom);
    /* we'll probably get an exception if wkt is null */
    if ( ! wkt ) RETURN_NULL();

    retstr = estrdup(wkt);
    GEOSFree(wkt);

    RETURN_STRING(retstr, 0);
}

PHP_METHOD(WKTWriter, setTrim)
{
    GEOSWKTWriter *writer;
    zend_bool trimval;
    char trim;

    writer = (GEOSWKTWriter*)getRelay(getThis(), WKTWriter_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "b", &trimval)
        == FAILURE)
    {
        RETURN_NULL();
    }

    trim = trimval;
    GEOSWKTWriter_setTrim(writer, trim);
}

PHP_METHOD(WKTWriter, setRoundingPrecision)
{
    GEOSWKTWriter *writer;
    long int prec;

    writer = (GEOSWKTWriter*)getRelay(getThis(), WKTWriter_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &prec)
        == FAILURE)
    {
        RETURN_NULL();
    }

    GEOSWKTWriter_setRoundingPrecision(writer, prec);
}

/**
 * void GEOSWKTWriter::setOutputDimension()
 */
PHP_METHOD(WKTWriter, setOutputDimension)
{
    GEOSWKTWriter *writer;
    long int dim;

    writer = (GEOSWKTWriter*)getRelay(getThis(), WKTWriter_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &dim)
        == FAILURE)
    {
        RETURN_NULL();
    }

    GEOSWKTWriter_setOutputDimension(writer, dim);
}

/**
 * long GEOSWKTWriter::getOutputDimension()
 */
PHP_METHOD(WKTWriter, getOutputDimension)
{
    GEOSWKTWriter *writer;
    long int ret;

    writer = (GEOSWKTWriter*)getRelay(getThis(), WKTWriter_ce_ptr);

    ret = GEOSWKTWriter_getOutputDimension(writer);

    RETURN_LONG(ret);
}

PHP_METHOD(WKTWriter, setOld3D)
{
    GEOSWKTWriter *writer;
    zend_bool bval;
    int val;

    writer = (GEOSWKTWriter*)getRelay(getThis(), WKTWriter_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "b", &bval)
        == FAILURE)
    {
        RETURN_NULL();
    }

    val = bval;
    GEOSWKTWriter_setOld3D(writer, val);
}

/* -- class GEOSWKBWriter -------------------- */

PHP_METHOD(WKBWriter, __construct);
PHP_METHOD(WKBWriter, getOutputDimension);
PHP_METHOD(WKBWriter, setOutputDimension);
PHP_METHOD(WKBWriter, getByteOrder);
PHP_METHOD(WKBWriter, setByteOrder);
PHP_METHOD(WKBWriter, setIncludeSRID);
PHP_METHOD(WKBWriter, getIncludeSRID);
PHP_METHOD(WKBWriter, write);
PHP_METHOD(WKBWriter, writeHEX);

static zend_function_entry WKBWriter_methods[] = {
    PHP_ME(WKBWriter, __construct, NULL, 0)
    PHP_ME(WKBWriter, getOutputDimension, NULL, 0)
    PHP_ME(WKBWriter, setOutputDimension, NULL, 0)
    PHP_ME(WKBWriter, getByteOrder, NULL, 0)
    PHP_ME(WKBWriter, setByteOrder, NULL, 0)
    PHP_ME(WKBWriter, getIncludeSRID, NULL, 0)
    PHP_ME(WKBWriter, setIncludeSRID, NULL, 0)
    PHP_ME(WKBWriter, write, NULL, 0)
    PHP_ME(WKBWriter, writeHEX, NULL, 0)
    {NULL, NULL, NULL}
};

static zend_class_entry *WKBWriter_ce_ptr;

static zend_object_handlers WKBWriter_object_handlers;

static void
WKBWriter_dtor (void *object TSRMLS_DC)
{
    Proxy *obj = (Proxy *)object;
    GEOSWKBWriter_destroy((GEOSWKBWriter*)obj->relay);

    zend_hash_destroy(obj->std.properties);
    FREE_HASHTABLE(obj->std.properties);

    efree(obj);
}

static zend_object_value
WKBWriter_create_obj (zend_class_entry *type TSRMLS_DC)
{
    return Gen_create_obj(type, WKBWriter_dtor, &WKBWriter_object_handlers);
}

/**
 * GEOSWKBWriter w = new GEOSWKBWriter()
 */
PHP_METHOD(WKBWriter, __construct)
{
    GEOSWKBWriter* obj;
    zval *object = getThis();

    obj = GEOSWKBWriter_create();
    if ( ! obj ) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR,
                "GEOSWKBWriter_create() failed (didn't initGEOS?)");
    }

    setRelay(object, obj);
}

/**
 * long GEOSWKBWriter::getOutputDimension();
 */
PHP_METHOD(WKBWriter, getOutputDimension)
{
    GEOSWKBWriter *writer;
    long int ret;

    writer = (GEOSWKBWriter*)getRelay(getThis(), WKBWriter_ce_ptr);

    ret = GEOSWKBWriter_getOutputDimension(writer);

    RETURN_LONG(ret);
}

/**
 * void GEOSWKBWriter::setOutputDimension(dims);
 */
PHP_METHOD(WKBWriter, setOutputDimension)
{
    GEOSWKBWriter *writer;
    long int dim;

    writer = (GEOSWKBWriter*)getRelay(getThis(), WKBWriter_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &dim)
        == FAILURE)
    {
        RETURN_NULL();
    }

    GEOSWKBWriter_setOutputDimension(writer, dim);

}

/**
 * string GEOSWKBWriter::write(GEOSGeometry)
 */
PHP_METHOD(WKBWriter, write)
{
    GEOSWKBWriter *writer;
    zval *zobj;
    GEOSGeometry *geom;
    char *ret;
    size_t retsize;
    char* retstr;

    writer = (GEOSWKBWriter*)getRelay(getThis(), WKBWriter_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &zobj)
        == FAILURE)
    {
        RETURN_NULL();
    }

    geom = getRelay(zobj, Geometry_ce_ptr);

    ret = (char*)GEOSWKBWriter_write(writer, geom, &retsize);
    /* we'll probably get an exception if ret is null */
    if ( ! ret ) RETURN_NULL();

    retstr = estrndup(ret, retsize);
    GEOSFree(ret);

    RETURN_STRINGL(retstr, retsize, 0);
}

/**
 * string GEOSWKBWriter::writeHEX(GEOSGeometry)
 */
PHP_METHOD(WKBWriter, writeHEX)
{
    GEOSWKBWriter *writer;
    zval *zobj;
    GEOSGeometry *geom;
    char *ret;
    size_t retsize; /* useless... */
    char* retstr;

    writer = (GEOSWKBWriter*)getRelay(getThis(), WKBWriter_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &zobj)
        == FAILURE)
    {
        RETURN_NULL();
    }

    geom = getRelay(zobj, Geometry_ce_ptr);

    ret = (char*)GEOSWKBWriter_writeHEX(writer, geom, &retsize);
    /* we'll probably get an exception if ret is null */
    if ( ! ret ) RETURN_NULL();

    retstr = estrndup(ret, retsize);
    GEOSFree(ret);

    RETURN_STRING(retstr, 0);
}

/**
 * long GEOSWKBWriter::getByteOrder();
 */
PHP_METHOD(WKBWriter, getByteOrder)
{
    GEOSWKBWriter *writer;
    long int ret;

    writer = (GEOSWKBWriter*)getRelay(getThis(), WKBWriter_ce_ptr);

    ret = GEOSWKBWriter_getByteOrder(writer);

    RETURN_LONG(ret);
}

/**
 * void GEOSWKBWriter::setByteOrder(dims);
 */
PHP_METHOD(WKBWriter, setByteOrder)
{
    GEOSWKBWriter *writer;
    long int dim;

    writer = (GEOSWKBWriter*)getRelay(getThis(), WKBWriter_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &dim)
        == FAILURE)
    {
        RETURN_NULL();
    }

    GEOSWKBWriter_setByteOrder(writer, dim);

}

/**
 * bool GEOSWKBWriter::getIncludeSRID();
 */
PHP_METHOD(WKBWriter, getIncludeSRID)
{
    GEOSWKBWriter *writer;
    int ret;
    zend_bool retBool;

    writer = (GEOSWKBWriter*)getRelay(getThis(), WKBWriter_ce_ptr);

    ret = GEOSWKBWriter_getIncludeSRID(writer);
    retBool = ret;

    RETURN_BOOL(retBool);
}

/**
 * void GEOSWKBWriter::setIncludeSRID(bool);
 */
PHP_METHOD(WKBWriter, setIncludeSRID)
{
    GEOSWKBWriter *writer;
    int inc;
    zend_bool incVal;

    writer = (GEOSWKBWriter*)getRelay(getThis(), WKBWriter_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "b", &incVal)
        == FAILURE)
    {
        RETURN_NULL();
    }

    inc = incVal;
    GEOSWKBWriter_setIncludeSRID(writer, inc);
}

/* -- class GEOSWKBReader -------------------- */

PHP_METHOD(WKBReader, __construct);
PHP_METHOD(WKBReader, read);
PHP_METHOD(WKBReader, readHEX);

static zend_function_entry WKBReader_methods[] = {
    PHP_ME(WKBReader, __construct, NULL, 0)
    PHP_ME(WKBReader, read, NULL, 0)
    PHP_ME(WKBReader, readHEX, NULL, 0)
    {NULL, NULL, NULL}
};

static zend_class_entry *WKBReader_ce_ptr;

static zend_object_handlers WKBReader_object_handlers;

static void
WKBReader_dtor (void *object TSRMLS_DC)
{
    Proxy *obj = (Proxy *)object;
    GEOSWKBReader_destroy((GEOSWKBReader*)obj->relay);

    zend_hash_destroy(obj->std.properties);
    FREE_HASHTABLE(obj->std.properties);

    efree(obj);
}

static zend_object_value
WKBReader_create_obj (zend_class_entry *type TSRMLS_DC)
{
    return Gen_create_obj(type, WKBReader_dtor, &WKBReader_object_handlers);
}


PHP_METHOD(WKBReader, __construct)
{
    GEOSWKBReader* obj;
    zval *object = getThis();

    obj = GEOSWKBReader_create();
    if ( ! obj ) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR,
                "GEOSWKBReader_create() failed (didn't initGEOS?)");
    }

    setRelay(object, obj);
}

PHP_METHOD(WKBReader, read)
{
    GEOSWKBReader *reader;
    GEOSGeometry *geom;
    unsigned char* wkb;
    int wkblen;

    reader = (GEOSWKBReader*)getRelay(getThis(), WKBReader_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s",
        &wkb, &wkblen) == FAILURE)
    {
        RETURN_NULL();
    }

    geom = GEOSWKBReader_read(reader, wkb, wkblen);
    /* we'll probably get an exception if geom is null */
    if ( ! geom ) RETURN_NULL();

    /* return_value is a zval */
    object_init_ex(return_value, Geometry_ce_ptr);
    setRelay(return_value, geom);

}

PHP_METHOD(WKBReader, readHEX)
{
    GEOSWKBReader *reader;
    GEOSGeometry *geom;
    unsigned char* wkb;
    int wkblen;

    reader = (GEOSWKBReader*)getRelay(getThis(), WKBReader_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s",
        &wkb, &wkblen) == FAILURE)
    {
        RETURN_NULL();
    }

    geom = GEOSWKBReader_readHEX(reader, wkb, wkblen);
    /* we'll probably get an exception if geom is null */
    if ( ! geom ) RETURN_NULL();
 
    /* return_value is a zval */
    object_init_ex(return_value, Geometry_ce_ptr);
    setRelay(return_value, geom);

}


/* -- Free functions ------------------------- */

/**
 * string GEOSVersion()
 */
PHP_FUNCTION(GEOSVersion)
{
    char *str;

    str = estrdup(GEOSversion());
    RETURN_STRING(str, 0);
}

/**
 * array GEOSPolygonize(GEOSGeometry $geom)
 *
 * The returned array contains the following elements:
 *
 *  - 'rings'
 *      Type: array of GEOSGeometry 
 *      Rings that can be formed by the costituent
 *      linework of geometry.
 *  - 'cut_edges' (optional)
 *      Type: array of GEOSGeometry 
 *      Edges which are connected at both ends but
 *      which do not form part of polygon.
 *  - 'dangles'
 *      Type: array of GEOSGeometry 
 *      Edges which have one or both ends which are
 *      not incident on another edge endpoint
 *  - 'invalid_rings' 
 *      Type: array of GEOSGeometry
 *      Edges which form rings which are invalid
 *      (e.g. the component lines contain a self-intersection)
 *
 */
PHP_FUNCTION(GEOSPolygonize)
{
    GEOSGeometry *this;
    GEOSGeometry *rings;
    GEOSGeometry *cut_edges;
    GEOSGeometry *dangles;
    GEOSGeometry *invalid_rings;
    zval *array_elem;
    zval *zobj;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &zobj)
        == FAILURE)
    {
        RETURN_NULL();
    }
    this = getRelay(zobj, Geometry_ce_ptr);

    rings = GEOSPolygonize_full(this, &cut_edges, &dangles, &invalid_rings);
    if ( ! rings ) RETURN_NULL(); /* should get an exception first */

    /* return value should be an array */
    array_init(return_value);

    MAKE_STD_ZVAL(array_elem);
    array_init(array_elem);
    dumpGeometry(rings, array_elem);
    GEOSGeom_destroy(rings);
    add_assoc_zval(return_value, "rings", array_elem); 

    MAKE_STD_ZVAL(array_elem);
    array_init(array_elem);
    dumpGeometry(cut_edges, array_elem);
    GEOSGeom_destroy(cut_edges);
    add_assoc_zval(return_value, "cut_edges", array_elem);

    MAKE_STD_ZVAL(array_elem);
    array_init(array_elem);
    dumpGeometry(dangles, array_elem);
    GEOSGeom_destroy(dangles);
    add_assoc_zval(return_value, "dangles", array_elem);

    MAKE_STD_ZVAL(array_elem);
    array_init(array_elem);
    dumpGeometry(invalid_rings, array_elem);
    GEOSGeom_destroy(invalid_rings);
    add_assoc_zval(return_value, "invalid_rings", array_elem);

}

/**
 * array GEOSLineMerge(GEOSGeometry $geom)
 */
PHP_FUNCTION(GEOSLineMerge)
{
    GEOSGeometry *geom_in;
    GEOSGeometry *geom_out;
    zval *zobj;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &zobj)
        == FAILURE)
    {
        RETURN_NULL();
    }
    geom_in = getRelay(zobj, Geometry_ce_ptr);

    geom_out = GEOSLineMerge(geom_in);
    if ( ! geom_out ) RETURN_NULL(); /* should get an exception first */

    /* return value should be an array */
    array_init(return_value);
    dumpGeometry(geom_out, return_value);
    GEOSGeom_destroy(geom_out);
}

/**
 * GEOSGeometry GEOSSharedPaths(GEOSGeometry $geom1, GEOSGeometry *geom2)
 */
PHP_FUNCTION(GEOSSharedPaths)
{
    GEOSGeometry *geom_in_1;
    GEOSGeometry *geom_in_2;
    GEOSGeometry *geom_out;
    zval *zobj1, *zobj2;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "oo", &zobj1, &zobj2)
        == FAILURE)
    {
        RETURN_NULL();
    }
    geom_in_1 = getRelay(zobj1, Geometry_ce_ptr);
    geom_in_2 = getRelay(zobj2, Geometry_ce_ptr);

    geom_out = GEOSSharedPaths(geom_in_1, geom_in_2);
    if ( ! geom_out ) RETURN_NULL(); /* should get an exception first */

    /* return_value is a zval */
    object_init_ex(return_value, Geometry_ce_ptr);
    setRelay(return_value, geom_out);
}

/**
 * GEOSGeometry::delaunayTriangulation([<tolerance>], [<onlyEdges>])
 *
 *  'tolerance'
 *       Type: double
 *       snapping tolerance to use for improved robustness
 *  'onlyEdges'
 *       Type: boolean
 *       if true will return a MULTILINESTRING, otherwise (the default)
 *       it will return a GEOMETRYCOLLECTION containing triangular POLYGONs.
 */
PHP_METHOD(Geometry, delaunayTriangulation)
{
    GEOSGeometry *this;
    GEOSGeometry *ret;
    double tolerance = 0.0;
    zend_bool edgeonly = 0;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|db",
            &tolerance, &edgeonly) == FAILURE) {
        RETURN_NULL();
    }

    ret = GEOSDelaunayTriangulation(this, tolerance, edgeonly ? 1 : 0);
    if ( ! ret ) RETURN_NULL(); /* should get an exception first */

    /* return_value is a zval */
    object_init_ex(return_value, Geometry_ce_ptr);
    setRelay(return_value, ret);
}

/**
 * GEOSGeometry::voronoiDiagram([<tolerance>], [<onlyEdges>], [<extent>])
 *
 *  'tolerance'
 *       Type: double
 *       snapping tolerance to use for improved robustness
 *  'onlyEdges'
 *       Type: boolean
 *       if true will return a MULTILINESTRING, otherwise (the default)
 *       it will return a GEOMETRYCOLLECTION containing POLYGONs.
 *  'extent'
 *       Type: geometry
 *       Clip returned diagram by the extent of the given geometry
 */
PHP_METHOD(Geometry, voronoiDiagram)
{
    GEOSGeometry *this;
    GEOSGeometry *ret;
    zval *zobj = 0;
    GEOSGeometry *env = 0;
    double tolerance = 0.0;
    zend_bool edgeonly = 0;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|dbo",
            &tolerance, &edgeonly, &zobj) == FAILURE) {
        RETURN_NULL();
    }

    if ( zobj ) env = getRelay(zobj, Geometry_ce_ptr);
    ret = GEOSVoronoiDiagram(this, env, tolerance, edgeonly ? 1 : 0);
    if ( ! ret ) RETURN_NULL(); /* should get an exception first */

    /* return_value is a zval */
    object_init_ex(return_value, Geometry_ce_ptr);
    setRelay(return_value, ret);
}

/**
 * bool GEOSRelateMatch(string matrix, string pattern)
 */
PHP_FUNCTION(GEOSRelateMatch)
{
    char* mat = NULL;
    int matlen;
    char* pat = NULL;
    int patlen;
    int ret;
    zend_bool retBool;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss",
        &mat, &matlen, &pat, &patlen) == FAILURE)
    {
        RETURN_NULL();
    }

    ret = GEOSRelatePatternMatch(mat, pat);
    if ( ret == 2 ) RETURN_NULL(); /* should get an exception first */

    /* return_value is a zval */
    retBool = ret;
    RETURN_BOOL(retBool);
}

/* ------ Initialization / Deinitialization / Meta ------------------ */

/* per-module initialization */
PHP_MINIT_FUNCTION(geos)
{
    zend_class_entry ce;

    /* WKTReader */
    INIT_CLASS_ENTRY(ce, "GEOSWKTReader", WKTReader_methods);
    WKTReader_ce_ptr = zend_register_internal_class(&ce TSRMLS_CC);
    WKTReader_ce_ptr->create_object = WKTReader_create_obj;
    memcpy(&WKTReader_object_handlers,
        zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    WKTReader_object_handlers.clone_obj = NULL;

    /* WKTWriter */
    INIT_CLASS_ENTRY(ce, "GEOSWKTWriter", WKTWriter_methods);
    WKTWriter_ce_ptr = zend_register_internal_class(&ce TSRMLS_CC);
    WKTWriter_ce_ptr->create_object = WKTWriter_create_obj;
    memcpy(&WKTWriter_object_handlers,
        zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    WKTWriter_object_handlers.clone_obj = NULL;

    /* Geometry */
    INIT_CLASS_ENTRY(ce, "GEOSGeometry", Geometry_methods);
    Geometry_ce_ptr = zend_register_internal_class(&ce TSRMLS_CC);
    Geometry_ce_ptr->create_object = Geometry_create_obj;
    memcpy(&Geometry_object_handlers,
        zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    Geometry_object_handlers.clone_obj = NULL;
    /* Geometry serialization */
    Geometry_ce_ptr->serialize = Geometry_serialize;
    Geometry_ce_ptr->unserialize = Geometry_deserialize;

    /* WKBWriter */
    INIT_CLASS_ENTRY(ce, "GEOSWKBWriter", WKBWriter_methods);
    WKBWriter_ce_ptr = zend_register_internal_class(&ce TSRMLS_CC);
    WKBWriter_ce_ptr->create_object = WKBWriter_create_obj;
    memcpy(&WKBWriter_object_handlers,
        zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    WKBWriter_object_handlers.clone_obj = NULL;

    /* WKBReader */
    INIT_CLASS_ENTRY(ce, "GEOSWKBReader", WKBReader_methods);
    WKBReader_ce_ptr = zend_register_internal_class(&ce TSRMLS_CC);
    WKBReader_ce_ptr->create_object = WKBReader_create_obj;
    memcpy(&WKBReader_object_handlers,
        zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    WKBReader_object_handlers.clone_obj = NULL;


    /* Constants */
    REGISTER_LONG_CONSTANT("GEOSBUF_CAP_ROUND",  GEOSBUF_CAP_ROUND,
        CONST_CS|CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("GEOSBUF_CAP_FLAT",   GEOSBUF_CAP_FLAT,
        CONST_CS|CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("GEOSBUF_CAP_SQUARE", GEOSBUF_CAP_SQUARE,
        CONST_CS|CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("GEOSBUF_JOIN_ROUND", GEOSBUF_JOIN_ROUND,
        CONST_CS|CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("GEOSBUF_JOIN_MITRE", GEOSBUF_JOIN_MITRE,
        CONST_CS|CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("GEOSBUF_JOIN_BEVEL", GEOSBUF_JOIN_BEVEL,
        CONST_CS|CONST_PERSISTENT);

    REGISTER_LONG_CONSTANT("GEOS_POINT", GEOS_POINT,
        CONST_CS|CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("GEOS_LINESTRING", GEOS_LINESTRING,
        CONST_CS|CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("GEOS_LINEARRING", GEOS_LINEARRING,
        CONST_CS|CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("GEOS_POLYGON", GEOS_POLYGON,
        CONST_CS|CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("GEOS_MULTIPOINT", GEOS_MULTIPOINT,
        CONST_CS|CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("GEOS_MULTILINESTRING", GEOS_MULTILINESTRING,
        CONST_CS|CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("GEOS_MULTIPOLYGON", GEOS_MULTIPOLYGON,
        CONST_CS|CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("GEOS_GEOMETRYCOLLECTION", GEOS_GEOMETRYCOLLECTION,
        CONST_CS|CONST_PERSISTENT);

    REGISTER_LONG_CONSTANT("GEOSVALID_ALLOW_SELFTOUCHING_RING_FORMING_HOLE",
        GEOSVALID_ALLOW_SELFTOUCHING_RING_FORMING_HOLE,
        CONST_CS|CONST_PERSISTENT);

    REGISTER_LONG_CONSTANT("GEOSRELATE_BNR_MOD2", GEOSRELATE_BNR_MOD2,
        CONST_CS|CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("GEOSRELATE_BNR_OGC", GEOSRELATE_BNR_OGC,
        CONST_CS|CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("GEOSRELATE_BNR_ENDPOINT", GEOSRELATE_BNR_ENDPOINT,
        CONST_CS|CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("GEOSRELATE_BNR_MULTIVALENT_ENDPOINT",
        GEOSRELATE_BNR_MULTIVALENT_ENDPOINT,
        CONST_CS|CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("GEOSRELATE_BNR_MONOVALENT_ENDPOINT",
        GEOSRELATE_BNR_MONOVALENT_ENDPOINT,
        CONST_CS|CONST_PERSISTENT);

    return SUCCESS;
}

/* per-module shutdown */
PHP_MSHUTDOWN_FUNCTION(geos)
{
    delGeometrySerializer();
    delGeometryDeserializer();
    return SUCCESS;
}

/* per-request initialization */
PHP_RINIT_FUNCTION(geos)
{
    initGEOS(noticeHandler, errorHandler);
    return SUCCESS;
}

/* pre-request destruction */
PHP_RSHUTDOWN_FUNCTION(geos)
{
    finishGEOS();
    return SUCCESS;
}

/* module info */
PHP_MINFO_FUNCTION(geos)
{
    php_info_print_table_start();
    php_info_print_table_row(2,
        "GEOS - Geometry Engine Open Source", "enabled");
    php_info_print_table_row(2,
        "Version", PHP_GEOS_VERSION);
    php_info_print_table_end();
}

