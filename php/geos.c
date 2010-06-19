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
PHP_RINIT_FUNCTION(geos);
PHP_RSHUTDOWN_FUNCTION(geos);
PHP_MINFO_FUNCTION(geos);
PHP_FUNCTION(GEOSVersion);

static function_entry geos_functions[] = {
    PHP_FE(GEOSVersion, NULL)
    {NULL, NULL, NULL}
};

zend_module_entry geos_module_entry = {
    STANDARD_MODULE_HEADER,
    PHP_GEOS_EXTNAME,
    geos_functions,
    PHP_MINIT(geos),              /* module init function */
    NULL,                         /* module shutdown function */
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
    char message[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(message, sizeof(message) - 1, fmt, args);
    va_end(args);

    php_error_docref(NULL TSRMLS_CC, E_NOTICE, "%s", message);
}

static void errorHandler(const char *fmt, ...)
{
    char message[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(message, sizeof(message) - 1, fmt, args);
    va_end(args);

    /* TODO: use a GEOSException ? */
    zend_throw_exception_ex(zend_exception_get_default(TSRMLS_CC),
        1 TSRMLS_CC, "%s", message); 

}

typedef struct Proxy_t {
    zend_object std;
    void* relay;
} Proxy;

static void 
setRelay(zval* val, void* obj) {
    Proxy* proxy = (Proxy*)zend_object_store_get_object(val TSRMLS_CC);
    proxy->relay = obj;
}

static inline void *
getRelay(zval* val, zend_class_entry* ce) {
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

static zend_object_value
Gen_create_obj (zend_class_entry *type TSRMLS_DC,
    zend_objects_free_object_storage_t st, zend_object_handlers* handlers)
{
    zval *tmp;
    zend_object_value retval;

    Proxy *obj = (Proxy *)emalloc(sizeof(Proxy));
    memset(obj, 0, sizeof(Proxy));
    obj->std.ce = type;

    ALLOC_HASHTABLE(obj->std.properties);
    zend_hash_init(obj->std.properties, 0, NULL, ZVAL_PTR_DTOR, 0);
    zend_hash_copy(obj->std.properties, &type->default_properties,
        (copy_ctor_func_t)zval_add_ref, (void *)&tmp, sizeof(zval *));

    retval.handle = zend_objects_store_put(obj, NULL, st, NULL TSRMLS_CC);
    retval.handlers = handlers;

    return retval;
}


/* -- Free functions ------------------------- */

PHP_FUNCTION(GEOSVersion)
{
    char *str;

    str = estrdup(GEOSversion());
    RETURN_STRING(str, 0);
}

/* -- class GEOSGeometry -------------------- */

PHP_METHOD(Geometry, __construct);
PHP_METHOD(Geometry, project);

PHP_METHOD(Geometry, numGeometries);

static function_entry Geometry_methods[] = {
    PHP_ME(Geometry, __construct, NULL, 0)
    PHP_ME(Geometry, project, NULL, 0)
    PHP_ME(Geometry, numGeometries, NULL, 0)
    {NULL, NULL, NULL}
};

static zend_class_entry *Geometry_ce_ptr;

static zend_object_handlers Geometry_object_handlers;

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

PHP_METHOD(Geometry, numGeometries)
{
    GEOSGeometry *geom;
    long int ret;

    geom = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    ret = GEOSGetNumGeometries(geom);
    if ( ret == -1 ) RETURN_NULL(); /* should get an exception first */

    RETURN_LONG(ret);
}

PHP_METHOD(Geometry, project)
{
    GEOSGeometry *this;
    GEOSGeometry *other;
    zval *zobj;
    double ret;

    this = (GEOSGeometry*)getRelay(getThis(), Geometry_ce_ptr);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &zobj)
        == FAILURE) {
        RETURN_NULL();
    }
    other = getRelay(zobj, Geometry_ce_ptr);

    ret = GEOSProject(this, other);
    if ( ret < 0 ) RETURN_NULL(); /* should get an exception first */

    RETURN_DOUBLE(ret);
}

/* -- class GEOSWKTReader -------------------- */

PHP_METHOD(WKTReader, __construct);
PHP_METHOD(WKTReader, read);

static function_entry WKTReader_methods[] = {
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
PHP_METHOD(WKTWriter, setOld3D);

static function_entry WKTWriter_methods[] = {
    PHP_ME(WKTWriter, __construct, NULL, 0)
    PHP_ME(WKTWriter, write, NULL, 0)
    PHP_ME(WKTWriter, setTrim, NULL, 0)
    PHP_ME(WKTWriter, setRoundingPrecision, NULL, 0)
    PHP_ME(WKTWriter, setOutputDimension, NULL, 0)
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
 
    /* return_value is a zval */
    object_init_ex(return_value, Geometry_ce_ptr);
    setRelay(return_value, geom);

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

