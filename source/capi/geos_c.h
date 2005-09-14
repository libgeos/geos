/************************************************************************
 *
 * C-Wrapper for JTS library
 *
 * (C) 2005 Sandro Santilli, strk@refractions.net
 *
 ***********************************************************************
 *
 * GENERAL NOTES:
 *
 *	- Remember to call initGEOS() before any use of this library's
 *	  functions. 
 *
 ***********************************************************************/
 
/************************************************************************
 *
 * (Abstract) type definitions
 *
 ***********************************************************************/

typedef void (*GEOSMessageHandler)(const char *fmt, ...);
typedef struct GEOSGeom *GEOSGeom;

/* Supported geometry types */
enum GEOSGeomTypeId {
	GEOS_POINT,
	GEOS_LINESTRING,
	GEOS_LINEARRING,
	GEOS_POLYGON,
	GEOS_MULTIPOINT,
	GEOS_MULTILINESTRING,
	GEOS_MULTIPOLYGON,
	GEOS_GEOMETRYCOLLECTION
};


/************************************************************************
 *
 * Initialization, cleanup
 *
 ***********************************************************************/

extern void initGEOS(GEOSMessageHandler notice_function,
	GEOSMessageHandler error_function);
extern void finishGEOS(void);


/************************************************************************
 *
 * Input and Output functions, return NULL on exception.
 *
 ***********************************************************************/

extern GEOSGeom GEOSGeomFromWKT(const char *wkt);
extern char *GEOSGeomToWKT(const GEOSGeom g);

extern GEOSGeom GEOSGeomFromWKB_buf(const char *wkb, size_t size);
extern char *GEOSGeomToWKB_buf(const GEOSGeom g, size_t *size);


/************************************************************************
 *
 * Memory management
 *
 ***********************************************************************/

extern void GEOSGeom_destroy(GEOSGeom g);


/************************************************************************
 *
 * Topology operations - return NULL on exception.
 *
 ***********************************************************************/

extern GEOSGeom GEOSIntersection(GEOSGeom g1, GEOSGeom g2);
extern GEOSGeom GEOSBuffer(GEOSGeom g1,double width, int quadsegs);
extern GEOSGeom GEOSConvexHull(GEOSGeom g1);
extern GEOSGeom GEOSDifference(GEOSGeom g1,GEOSGeom g2);
extern GEOSGeom GEOSSymDifference(GEOSGeom g1,GEOSGeom g2);
extern GEOSGeom GEOSBoundary(GEOSGeom g1);
extern GEOSGeom GEOSUnion(GEOSGeom g1,GEOSGeom g2);
extern GEOSGeom GEOSPointOnSurface(GEOSGeom g1);
extern GEOSGeom GEOSGetCentroid(GEOSGeom g);
extern char *GEOSRelate(const GEOSGeom g1, const GEOSGeom g2);
extern GEOSGeom GEOSPolygonize(GEOSGeom geoms[], unsigned int ngeoms);

/************************************************************************
 *
 *  Binary predicates - return 2 on exception.
 *
 ***********************************************************************/

extern char GEOSRelatePattern(const GEOSGeom g1, const GEOSGeom g2,
	const char *pat);
extern char GEOSDisjoint(const GEOSGeom g1, const GEOSGeom g2);
extern char GEOSTouches(const GEOSGeom g1, const GEOSGeom g2);
extern char GEOSIntersects(const GEOSGeom g1, const GEOSGeom g2);
extern char GEOSCrosses(const GEOSGeom g1, const GEOSGeom g2);
extern char GEOSWithin(const GEOSGeom g1, const GEOSGeom g2);
extern char GEOSContains(const GEOSGeom g1, const GEOSGeom g2);
extern char GEOSOverlaps(const GEOSGeom g1, const GEOSGeom g2);
extern char GEOSEquals(const GEOSGeom g1, const GEOSGeom g2);


/************************************************************************
 *
 *  Unary predicate - return 2 on exception
 *
 ***********************************************************************/

extern char GEOSisEmpty(const GEOSGeom g1);
extern char GEOSisValid(const GEOSGeom g1);
extern char GEOSisSimple(const GEOSGeom g1);
extern char GEOSisRing(const GEOSGeom g1);
extern char GEOSHasZ(const GEOSGeom g1);


/************************************************************************
 *
 *  Geometry info
 *
 ***********************************************************************/

extern int       GEOSGeomTypeId(const GEOSGeom g1);
extern int       GEOSGetNumCoordinate(const GEOSGeom g1);
extern GEOSGeom  GEOSGetGeometryN(GEOSGeom g1, int n);
extern GEOSGeom  GEOSGetExteriorRing(GEOSGeom g1);
extern GEOSGeom  GEOSGetInteriorRingN(GEOSGeom g1,int n);
extern int       GEOSGetNumInteriorRings(const GEOSGeom g1);
extern int       GEOSGetSRID(const GEOSGeom g1);
extern int       GEOSGetNumGeometries(const GEOSGeom g1);
extern void      GEOSSetSRID(GEOSGeom g, int SRID);

