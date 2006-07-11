%module geos

%include "attribute.i"
%include "exception.i"
%include "std_string.i"
%include "std_vector.i"
%include "std_except.i"
%include "factory.i"

%{ 
#include "geos_c.h"
%}

/* Constants copied from geos_c.h.  Would be nice
   to reuse the originals but we can't without exposing
   the whole c api. */
#define GEOS_VERSION_MAJOR 3
#define GEOS_VERSION_MINOR 0
#define GEOS_VERSION_PATCH 0rc1
#define GEOS_VERSION "3.0.0rc1"
#define GEOS_JTS_PORT "1.7.1"

#define GEOS_CAPI_VERSION_MAJOR 1
#define GEOS_CAPI_VERSION_MINOR 1
#define GEOS_CAPI_VERSION_PATCH 1
#define GEOS_CAPI_FIRST_INTERFACE GEOS_CAPI_VERSION_MAJOR 
#define GEOS_CAPI_LAST_INTERFACE (GEOS_CAPI_VERSION_MAJOR+GEOS_CAPI_VERSION_MINOR)
#define GEOS_CAPI_VERSION "3.0.0rc1-CAPI-1.1.1"

/* Supported geometry types */
enum GEOSGeomTypes { 
    GEOS_POINT,
    GEOS_LINESTRING,
    GEOS_LINEARRING,
    GEOS_POLYGON,
    GEOS_MULTIPOINT,
    GEOS_MULTILINESTRING,
    GEOS_MULTIPOLYGON,
    GEOS_GEOMETRYCOLLECTION
};

%inline %{
enum GEOSByteOrders {
	GEOS_WKB_XDR = 0, /* Big Endian */
	GEOS_WKB_NDR = 1 /* Little Endian */
};
%}


/* Message and Error Handling */
%{

/* This is not thread safe ! */
static const int MESSAGE_SIZE = 1000;
static char message[MESSAGE_SIZE];

void noticeHandler(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vsnprintf(message, sizeof(message) - 1, fmt, args);
    va_end(args);
}

void errorHandler(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vsnprintf(message, sizeof(message) - 1, fmt, args);
    va_end(args);
}
%}


/* First initialize geos */
%init %{
    initGEOS(noticeHandler, errorHandler);
%}


/* Module level methods */
const char *GEOSversion();
void finishGEOS(void);



/* ==============  Language Specific Files ============ */

/* Import language specific SWIG files.  This allows each language
   to define its own renames as well as any special functionality
   such as language specific iterators for collections. Note 
   that %include allows the included files to generate interface
   wrapper code while %import does not.  Thus use %include since
   this is an important feature (for example, Ruby needs it to #undef
   the select macro) */


#ifdef SWIGPYTHON
//	%include ../python/python.i
#endif

#ifdef SWIGRUBY
	%include ../ruby/ruby.i
#endif



// ===  CoordinateSequence ===
%{
typedef struct GeosCoordinateSequence_t *GeosCoordinateSequence;
%}

%rename (CoordinateSequence) GeosCoordinateSequence;
class GeosCoordinateSequence
{
public:
    /* Typemap to verify index is in bounds. */
    %typemap(check) size_t idx  {
        /* %typemap(check) size_t idx */
        GEOSCoordSeq coords = (GEOSCoordSeq) self;
        size_t size;
        
        GEOSCoordSeq_getSize(coords, &size);
        if ($1 < 0 || $1 >= size)
            SWIG_exception(SWIG_IndexError, "Index out of bounds");
    }
%extend
{
    GeosCoordinateSequence(size_t size, size_t dims)
    {
        return (GeosCoordinateSequence*) GEOSCoordSeq_create(size, dims);
    }

    GeosCoordinateSequence(const GeosCoordinateSequence& other)
    {
        GEOSCoordSeq coords = (GEOSCoordSeq) other;
        return (GeosCoordinateSequence*) GEOSCoordSeq_clone(coords);
    }

    ~GeosCoordinateSequence()
    {
        GEOSCoordSeq coords = (GEOSCoordSeq) self;
        return GEOSCoordSeq_destroy(coords);
    }

    int setX(size_t idx, double val)
    {
        GEOSCoordSeq coords = (GEOSCoordSeq) self;
        return GEOSCoordSeq_setX(coords, idx, val);
    }

    int setY(size_t idx, double val)
    {
        GEOSCoordSeq coords = (GEOSCoordSeq) self;
        return GEOSCoordSeq_setY(coords, idx, val);
    }

    int setZ(size_t idx, double val)
    {
        GEOSCoordSeq coords = (GEOSCoordSeq) self;
        return GEOSCoordSeq_setZ(coords, idx, val);
    }

    int setOrdinate(size_t idx, size_t dim, double val)
    {
        GEOSCoordSeq coords = (GEOSCoordSeq) self;
        return GEOSCoordSeq_setOrdinate(coords, idx, dim, val);
    }

    double getX(size_t idx)
    {
        double result;
        GEOSCoordSeq coords = (GEOSCoordSeq) self;
        GEOSCoordSeq_getX(coords, idx, &result);
        return result;
    }

    double getY(size_t idx)
    {
        double result;
        GEOSCoordSeq coords = (GEOSCoordSeq) self;
        GEOSCoordSeq_getY(coords, idx, &result);
        return result;
    }
    
    double getZ(size_t idx)
    {
        double result;
        GEOSCoordSeq coords = (GEOSCoordSeq) self;
        GEOSCoordSeq_getZ(coords, idx, &result);
        return result;
    }
    
    double getOrdinate(size_t idx, size_t dim)
    {
        double result;
        GEOSCoordSeq coords = (GEOSCoordSeq) self;
        GEOSCoordSeq_getOrdinate(coords, idx, dim, &result);
        return result;
    }

    int getSize()
    {
        size_t result;
        GEOSCoordSeq coords = (GEOSCoordSeq) self;
        GEOSCoordSeq_getSize(coords, &result);
        return result;
    }

    int getDimensions()
    {
        size_t result;
        GEOSCoordSeq coords = (GEOSCoordSeq) self;
        GEOSCoordSeq_getDimensions(coords, &result);
        return result;
    }
}
};



/* Install exception handler for topology operations. */
/*%exception GeosGeom* Geometry::*
{
    $action
    if (result ==  NULL)
        SWIG_exception(SWIG_RuntimeError, message);
}


*/

/* Install exception handler for the binary and unary predicates. */
 /*   bool relatePattern(const GeosGeometry* other, const char *pat)
    bool disjoint(const GeosGeometry* other)
    bool touches(const GeosGeometry* other)
    bool intersects(const GeosGeometry* other)
    bool crosses(const GeosGeometry* other)
    bool within(const GeosGeometry* other)
    bool contains(const GeosGeometry* other)
    bool overlaps(const GeosGeometry* other)
    bool equals(const GeosGeometry* other)
    bool isEmpty()
    bool isValid()
    bool isSimple()
    bool isRing()
    bool isHasZ()*/



/* ========  Wrapper Classes to Recreate Geom Hierarchy ====== */
%rename(Geometry) GeosGeometry;
%rename(Point) GeosPoint;
%rename(LineString) GeosLineString;
%rename(LinearRing) GeosLinearRing;
%rename(Polygon) GeosPolygon;
%rename(GeometryCollection) GeosGeometryCollection;
%rename(MultiPoint) GeosMultiPoint;
%rename(MultiLineString) GeosMultiLineString;
%rename(MultiLinearRing) GeosMultiLinearRing;
%rename(MultiPolygon) GeosMultiPolygon;

%newobject GeosGeometry::intersection;
%newobject GeosGeometry::buffer;
%newobject GeosGeometry::convexHull;
%newobject GeosGeometry::difference;
%newobject GeosGeometry::symDifference;
%newobject GeosGeometry::boundary;
%newobject GeosGeometry::geomUnion;
%newobject GeosGeometry::pointOnSurface;
%newobject GeosGeometry::getCentroid;
%newobject GeosGeometry::relate;
%newobject GeosGeometry::lineMerge;
%newobject GeosGeometry::simplify;
%newobject GeosGeometry::topologyPreserveSimplify;


%factory(GeosGeometry *,
         GeosPoint, 
		 GeosLinearRing,
         GeosLineString,
		 GeosPolygon, 
		 GeosMultiPoint,
         GeosMultiLineString,
		 GeosMultiPolygon,
         GeosGeometryCollection);

%{
class GeosGeometry;
extern GeosGeometry* createGeometry(GEOSGeom geom);
%}

// ===  Attributes ===
%attribute(GeosGeometry, int, srid, getSRID, setSRID);

%exception
{
    try
    {
        $action
    }
    catch (const std::exception& e)
    {
        SWIG_exception(SWIG_RuntimeError, e.what());
    }
}


%inline %{

class GeosGeometry
{
public:
    const GEOSGeom geom_;

    GeosGeometry(GEOSGeom geom): geom_(geom)
    {
    }

    virtual ~GeosGeometry()
    {
        GEOSGeom_destroy(this->geom_);
    }

    char *geomType()
    {
        return GEOSGeomType(this->geom_);
    }

    int typeId()
    {
        return GEOSGeomTypeId(this->geom_);
    }
    
    int getSRID()
    {
        return GEOSGetSRID(this->geom_);
    }

    void setSRID(int SRID)
    {
        return GEOSSetSRID(this->geom_, SRID);
    }

    size_t getDimensions()
    {
        return GEOSGeom_getDimensions(this->geom_);
    }


    /* Topology Operations */
   /* GeosGeometry *envelope()
    {
        return new GeosGeometry(GEOSEnvelope(this->geom_);
    }*/

    GeosGeometry *intersection(GeosGeometry *other)
    {
        return createGeometry(GEOSIntersection(this->geom_, other->geom_));
    }

    GeosGeometry *buffer(double width, int quadsegs)
    {
        return createGeometry(GEOSBuffer(this->geom_, width, quadsegs));
    }

    GeosGeometry *convexHull()
    {
        return createGeometry(GEOSConvexHull(this->geom_));
    }

    GeosGeometry *difference(GeosGeometry *other)
    {
        return createGeometry(GEOSDifference(this->geom_, other->geom_));
    }

    GeosGeometry *symDifference(GeosGeometry *other)
    {
        return createGeometry(GEOSSymDifference(this->geom_, other->geom_));
    }

    GeosGeometry *boundary()
    {
        return createGeometry(GEOSBoundary(this->geom_));
    }

    GeosGeometry *geomUnion(GeosGeometry *other)
    {
        return createGeometry(GEOSUnion(this->geom_, other->geom_));
    }
    
    GeosGeometry *pointOnSurface()
    {
        return createGeometry(GEOSPointOnSurface(this->geom_));
    }

    GeosGeometry *getCentroid()
    {
        return createGeometry(GEOSGetCentroid(this->geom_));
    }

    char *relate(GeosGeometry *other)
    {
        return GEOSRelate(this->geom_, other->geom_);
    }

    /* TODO - expose GEOSPolygonize*/
    GeosGeometry *lineMerge()
    {
        return createGeometry(GEOSLineMerge(this->geom_));
    }

    GeosGeometry *simplify(double tolerance)
    {
        return createGeometry(GEOSSimplify(this->geom_, tolerance));
    }

    GeosGeometry *topologyPreserveSimplify(double tolerance)
    {
        return createGeometry(GEOSTopologyPreserveSimplify(this->geom_, tolerance));
    }

    /* Binary predicates - return 2 on exception, 1 on true, 0 on false */
    bool relatePattern(const GeosGeometry* other, const char *pat)
    {
        return checkBoolResult(GEOSRelatePattern(this->geom_, other->geom_, pat));
    }

    bool disjoint(const GeosGeometry* other)
    {
        return checkBoolResult(GEOSDisjoint(this->geom_, other->geom_));
    }

    bool touches(const GeosGeometry* other)
    {
        return checkBoolResult(GEOSTouches(this->geom_, other->geom_));
    }

    bool intersects(const GeosGeometry* other)
    {
        return checkBoolResult(GEOSIntersects(this->geom_, other->geom_));
    }

    bool crosses(const GeosGeometry* other)
    {
        return checkBoolResult(GEOSCrosses(this->geom_, other->geom_));
    }

    bool within(const GeosGeometry* other)
    {
        return checkBoolResult(GEOSWithin(this->geom_, other->geom_));
    }

    bool contains(const GeosGeometry* other)
    {
        return checkBoolResult(GEOSContains(this->geom_, other->geom_));
    }

    bool overlaps(const GeosGeometry* other)
    {
        return checkBoolResult(GEOSOverlaps(this->geom_, other->geom_));
    }

    bool equals(const GeosGeometry* other)
    {
        return checkBoolResult(GEOSEquals(this->geom_, other->geom_));
    }

    /* Unary predicate - return 2 on exception, 1 on true, 0 on false */
    bool isEmpty()
    {
        return checkBoolResult(GEOSisEmpty(this->geom_));
    }

    bool isValid()
    {
        return checkBoolResult(GEOSisValid(this->geom_));
    }

    bool isSimple()
    {
        return checkBoolResult(GEOSisSimple(this->geom_));
    }

    bool isRing()
    {
        return checkBoolResult(GEOSisRing(this->geom_));
    }

    bool isHasZ()
    {
        return checkBoolResult(GEOSHasZ(this->geom_));
    }
protected:
    bool checkBoolResult(char result)
    {
        int intResult = (int) result;

        if (result == 1)
            return true;
        else if (result == 0)
            return false;
        else
            throw std::runtime_error(message);
    }
};

class GeosPoint: public GeosGeometry
{
public:
    GeosPoint(GEOSGeom geom): GeosGeometry(geom)
    {
    }
};

class GeosLineString: public GeosGeometry
{
public:
    GeosLineString(GEOSGeom geom): GeosGeometry(geom)
    {
    }
};

class GeosLinearRing: public GeosGeometry
{
public:
    GeosLinearRing(GEOSGeom geom): GeosGeometry(geom)
    {
    }
};

class GeosPolygon: public GeosGeometry
{
public:
    GeosPolygon(GEOSGeom geom): GeosGeometry(geom)
    {
    }
};

class GeosGeometryCollection: public GeosGeometry
{
public:
    GeosGeometryCollection(GEOSGeom geom): GeosGeometry(geom)
    {
    }
};

class GeosMultiPoint: public GeosGeometryCollection
{
public:
    GeosMultiPoint(GEOSGeom geom): GeosGeometryCollection(geom)
    {
    }
};

class GeosMultiLineString: public GeosGeometryCollection
{
public:
    GeosMultiLineString(GEOSGeom geom): GeosGeometryCollection(geom)
    {
    }
};

class GeosMultiLinearRing: public GeosGeometryCollection
{
public:
    GeosMultiLinearRing(GEOSGeom geom): GeosGeometryCollection(geom)
    {
    }
};

class GeosMultiPolygon: public GeosGeometryCollection
{
public:
    GeosMultiPolygon(GEOSGeom geom): GeosGeometryCollection(geom)
    {
    }
};
%}
/* Turn off exception handler */
%exception;

%wrapper %{
GeosGeometry* createGeometry(GEOSGeom geom)
{
    if(geom == NULL)
        throw std::runtime_error(message);

    GEOSGeomTypes geomId = (GEOSGeomTypes)GEOSGeomTypeId(geom);

    switch (geomId)
    {
    case GEOS_POINT:
        return new GeosPoint(geom);
        break;
	case GEOS_LINESTRING:
        return new GeosLineString(geom);
        break;
	case GEOS_LINEARRING:
        return new GeosLinearRing(geom);
        break;
	case GEOS_POLYGON:
        return new GeosPolygon(geom);
        break;
	case GEOS_MULTIPOINT:
        return new GeosMultiPoint(geom);
        break;
	case GEOS_MULTILINESTRING:
        return new GeosMultiLineString(geom);
        break;
	case GEOS_MULTIPOLYGON:
        return new GeosMultiPolygon(geom);
        break;
	case GEOS_GEOMETRYCOLLECTION:
        return new GeosGeometryCollection(geom);
        break;
    }
}
%}


// ==== Geometry Constructors ===========
%newobject createPoint;
%newobject createLineString;
%newobject createLinearRing;
%newobject createPolygon;

%apply SWIGTYPE *DISOWN {GeosCoordinateSequence *s};

%inline %{
GeosPoint *createPoint(GeosCoordinateSequence *s)
{
    GEOSCoordSeq coords = (GEOSCoordSeq) s;
    return (GeosPoint*) GEOSGeom_createPoint(coords);
}

GeosLinearRing *createLinearRing(GeosCoordinateSequence *s)
{
    GEOSCoordSeq coords = (GEOSCoordSeq) s;
    return (GeosLinearRing*) GEOSGeom_createLinearRing(coords);
}

GeosLineString *createLineString(GeosCoordinateSequence *s)
{
    GEOSCoordSeq coords = (GEOSCoordSeq) s;
    return (GeosLineString*) GEOSGeom_createLineString(coords);
}

GeosPolygon *createPolygon(GEOSGeom shell, GEOSGeom *holes, size_t nholes)
{
    return (GeosPolygon*) GEOSGeom_createPolygon(shell, holes, nholes);
}

%}

/*
 * Second argument is an array of GEOSGeom objects.
 * The caller remains owner of the array, but pointed-to
 * objects become ownership of the returned GEOSGeom.
 
extern GEOSGeom GEOS_DLL GEOSGeom_createPolygon(GEOSGeom shell,
	GEOSGeom *holes, size_t nholes);
extern GEOSGeom GEOS_DLL GEOSGeom_createCollection(int type,
	GEOSGeom *geoms, size_t ngeoms);

extern GEOSGeom GEOS_DLL GEOSGeom_clone(const GEOSGeom g);
*/
%clear GeosCoordinateSequence *s;

// === Input/Output ===
%newobject geomFromWKT;
%newobject geomFromWKB;
%newobject geomFromHEX;


/* This typemap allows the scripting language to pass in buffers
   to the geometry write methods. */
%typemap(in) (const unsigned char* wkb, size_t size) (int alloc = 0)
{
    /* %typemap(in) (const unsigned char* wkb, size_t size) (int alloc = 0) */
    if (SWIG_AsCharPtrAndSize($input, (char**)&$1, &$2, &alloc) != SWIG_OK)
        SWIG_exception(SWIG_RuntimeError, "Expecting a string");
    /* Don't want to include last null character! */
    $2--;
}

/* These three type maps are for geomToWKB and geomToHEX.  We need
to ignore the size input argument, then create a new string in the
scripting language of the correct size, and then free the 
provided string. */

/* set the size parameter to a temporary variable. */
%typemap(in, numinputs=1) (const GeosGeometry* g, size_t *size) (size_t temp = 0)
{
    // %typemap(in, numinputs=1) (const GEOSGeom* g, size_t *size) (size_t temp) 
  int res1 = SWIG_ConvertPtr(argv[0], (void**) &$1, SWIGTYPE_p_GeosGeometry, 0 |  0 );
  if (!SWIG_IsOK(res1)) {
    SWIG_exception_fail(SWIG_ArgError(res1), "in method '" "geomToWKB" "', argument " "1"" of type '" "GEOSGeom const *""'"); 
  }
  $2 = &temp;
}

/* Create a new target string of the correct size. */
%typemap(argout) (const GeosGeometry* g, size_t *size)
{
    /* %typemap(argout) (const GEOSGeom* g, size_t *size) */
    $result = SWIG_FromCharPtrAndSize(&result, $2);
}

/* Free the c-string returned  by the function. */
%typemap(freearg) (const GeosGeometry* g, size_t *size)
{
    /* %typemap(freearg) (const GEOSGeom* g, size_t *size) */
    std::free(result);
}

/* This typemap create a string object in the target object from
   a c-string and length.*/
%typemap(argout) (const GeosGeometry* g, size_t *size)
{
    /* %typemap(argout) (const GEOSGeom* g, size_t *size) */
    $result = SWIG_FromCharPtrAndSize((const char*) result, *$2);
}


%inline %{
GeosGeometry* geomFromWKT(const char *wkt)
{
    return createGeometry(GEOSGeomFromWKT(wkt));
}

char *geomToWKT(const GeosGeometry* g)
{
    return GEOSGeomToWKT(g->geom_);
}

int getWKBOutputDims()
{
    return GEOS_getWKBOutputDims();
}

int setWKBOutputDims(int newDims)
{
    return GEOS_setWKBOutputDims(newDims);
}

int getWKBByteOrder()
{
    return GEOS_getWKBByteOrder();
}

int setWKBByteOrder(int byteOrder)
{
    return GEOS_setWKBByteOrder((GEOSByteOrders)byteOrder);
}

GeosGeometry* geomFromWKB(const unsigned char *wkb, size_t size)
{
    return createGeometry(GEOSGeomFromWKB_buf(wkb, size));
}

unsigned char *geomToWKB(const GeosGeometry* g, size_t *size)
{
    return GEOSGeomToWKB_buf(g->geom_, size);
}

/* use wkb parameter instead of hex so we can reuse the typemap above. */
GeosGeometry* geomFromHEX(const unsigned char *wkb, size_t size)
{
    return createGeometry(GEOSGeomFromHEX_buf(wkb, size));
}

unsigned char *geomToHEX(const GeosGeometry* g, size_t *size)
{
    return GEOSGeomToHEX_buf(g->geom_, size);
}
%}
