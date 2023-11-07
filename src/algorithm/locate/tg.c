// https://github.com/tidwall/tg
//
// Copyright 2023 Joshua J Baker. All rights reserved.
// Use of this source code is governed by a license 
// that can be found in the LICENSE file.

#include <math.h>
#include <float.h>
#include <stdarg.h>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tg.h"

/******************************************************************************

Implementation Notes:
- Right-hand-rule is not required for polygons.
- The "properties" GeoJSON field is not required for Features.
- All polygons with at least 32 points are indexed automatically (TG_NATURAL).

Developer notes:
- This tg.c source file includes the entire tg library. (amalgamation)
- All dependencies are embedded between the BEGIN/END tags.
- Do not edit a dependency directly in this file. Instead edit the file in
the deps directory and then run deps/embed.sh to replace out its code in
this file.

*******************************************************************************/

enum base {
    BASE_POINT = 1, // tg_point
    BASE_LINE  = 2, // tg_line
    BASE_RING  = 3, // tg_ring
    BASE_POLY  = 4, // tg_poly
    BASE_GEOM  = 5, // tg_geom
};

enum flags {
    HAS_Z          = 1<<0,  // Geometry has additional Z coordinates
    HAS_M          = 1<<1,  // Geometry has additional M coordinates
    IS_ERROR       = 1<<2,  // Geometry is a parse error. Falls back to POINT
    IS_EMPTY       = 1<<3,  // Same as a GeoJSON null object (empty coords)
    IS_FEATURE     = 1<<4,  // GeoJSON. Geometry is Feature
    IS_FEATURE_COL = 1<<5,  // GeoJSON. GeometryGollection is FeatureCollection
    HAS_NULL_PROPS = 1<<6,  // GeoJSON. 'Feature' with 'properties'=null
    IS_UNLOCATED   = 1<<7,  // GeoJSON. 'Feature' with 'geometry'=null
};

// Optionally use non-atomic reference counting when TG_NOATOMICS is defined.
#ifdef TG_NOATOMICS
typedef int rc_t;
static int rc_sub(rc_t *rc) {
    int fetch = *rc;
    (*rc)--;
    return fetch;
}
static int rc_add(rc_t *rc) {
    int fetch = *rc;
    (*rc)++;
    return fetch;
}
#else
#include <stdatomic.h>
typedef atomic_int rc_t;
static int rc_sub(rc_t *rc) {
    return atomic_fetch_sub(rc, 1);
}
static int rc_add(rc_t *rc) {
    return atomic_fetch_add(rc, 1);
}
#endif

struct head { 
    rc_t rc;
    enum base base:8;
    enum tg_geom_type type:8;
    enum flags flags:8;
};

/// A ring is series of tg_segment which creates a shape that does not
/// self-intersect and is fully closed, where the start and end points are the
/// exact same value.
///
/// **Creating**
///
/// To create a new ring use the tg_ring_new() function.
///
/// ```
/// struct tg_ring *ring = tg_ring_new(points, npoints);
/// ```
///
/// **Upcasting**
///
/// A tg_ring can always be safely upcasted to a tg_poly or tg_geom; allowing
/// it to use any tg_poly_&ast;() or tg_geom_&ast;() function.
///
/// ```
/// struct tg_poly *poly = (struct tg_poly*)ring; // Cast to a tg_poly
/// struct tg_geom *geom = (struct tg_geom*)ring; // Cast to a tg_geom
/// ```
/// @see RingFuncs
/// @see PolyFuncs
struct tg_ring {
    struct head head;
    bool closed;
    bool clockwise;
    bool convex;
    double area;
    int npoints;
    int nsegs;
    struct tg_rect rect;
    struct index *index;
    struct ystripes *ystripes;
    struct tg_point points[]; 
};

/// A line is a series of tg_segment that make up a linestring.
///
/// **Creating**
///
/// To create a new line use the tg_line_new() function.
///
/// ```
/// struct tg_line *line = tg_line_new(points, npoints);
/// ```
///
/// **Upcasting**
///
/// A tg_line can always be safely upcasted to a tg_geom; allowing
/// it to use any tg_geom_&ast;() function.
///
/// ```
/// struct tg_geom *geom = (struct tg_geom*)line; // Cast to a tg_geom
/// ```
///
/// @see LineFuncs
struct tg_line { int _; };

/// A polygon consists of one exterior ring and zero or more holes.
/// 
/// **Creating**
///
/// To create a new polygon use the tg_poly_new() function.
///
/// ```
/// struct tg_poly *poly = tg_poly_new(exterior, holes, nholes);
/// ```
///
/// **Upcasting**
///
/// A tg_poly can always be safely upcasted to a tg_geom; allowing
/// it to use any tg_geom_&ast;() function.
///
/// ```
/// struct tg_geom *geom = (struct tg_geom*)poly; // Cast to a tg_geom
/// ```
///
/// @see PolyFuncs
struct tg_poly {
    struct head head;
    struct tg_ring *exterior;
    int nholes;
    struct tg_ring **holes;
};

struct multi {
    struct tg_geom **geoms;
    int ngeoms;
    struct tg_rect rect; // unioned rect child geometries
};

/// A geometry is the common generic type that can represent a Point,
/// LineString, Polygon, MultiPoint, MultiLineString, MultiPolygon, or 
/// GeometryCollection. 
/// 
/// For geometries that are derived from GeoJSON, they may have addtional 
/// attributes such as being a Feature or a FeatureCollection; or include
/// extra json fields.
///
/// **Creating**
///
/// To create a new geometry use one of the @ref GeometryConstructors or
/// @ref GeometryParsing functions.
///
/// ```
/// struct tg_geom *geom = tg_geom_new_point(point);
/// struct tg_geom *geom = tg_geom_new_polygon(poly);
/// struct tg_geom *geom = tg_parse_geojson(geojson);
/// ```
///
/// **Upcasting**
///
/// Other types, specifically tg_line, tg_ring, and tg_poly, can be safely
/// upcasted to a tg_geom; allowing them to use any tg_geom_&ast;()
/// function.
///
/// ```
/// struct tg_geom *geom1 = (struct tg_geom*)line; // Cast to a LineString
/// struct tg_geom *geom2 = (struct tg_geom*)ring; // Cast to a Polygon
/// struct tg_geom *geom3 = (struct tg_geom*)poly; // Cast to a Polygon
/// ```
///
/// @see GeometryConstructors
/// @see GeometryAccessors
/// @see GeometryPredicates
/// @see GeometryParsing
/// @see GeometryWriting
struct tg_geom {
    struct head head;
    union {
        struct tg_point point;
        struct tg_line *line;
        struct tg_poly *poly;
        struct multi *multi;
    };
    union {
        struct {  // TG_POINT
            double z;
            double m;
        };
        struct {  // !TG_POINT
            double *coords; // extra dimensinal coordinates
            int ncoords;
        };
    };
    union {
        char *xjson; // extra json fields, such as "id", "properties", etc.
        char *error; // an error message, when flag IS_ERROR
    };
};

struct boxed_point {
    struct head head;
    struct tg_point point;
};

#define todo(msg) { \
    fprintf(stderr, "todo: %s, line: %d\n", (msg), __LINE__); \
    exit(1); \
}

// private methods
bool tg_ring_empty(const struct tg_ring *ring);
bool tg_line_empty(const struct tg_line *line);
bool tg_poly_empty(const struct tg_poly *poly);
void tg_rect_search(struct tg_rect rect, struct tg_rect target, 
    bool(*iter)(struct tg_segment seg, int index, void *udata),
    void *udata);
void tg_ring_search(const struct tg_ring *ring, struct tg_rect rect, 
    bool(*iter)(struct tg_segment seg, int index, void *udata), 
    void *udata);
void tg_line_search(const struct tg_line *ring, struct tg_rect rect, 
    bool(*iter)(struct tg_segment seg, int index, void *udata), 
    void *udata);
void tg_geom_foreach(const struct tg_geom *geom, 
    bool(*iter)(const struct tg_geom *geom, void *udata), void *udata);
double tg_ring_polsby_popper_score(const struct tg_ring *ring);
double tg_line_polsby_popper_score(const struct tg_line *line);
int tg_rect_num_points(struct tg_rect rect);
struct tg_point tg_rect_point_at(struct tg_rect rect, int index);
int tg_rect_num_segments(struct tg_rect rect);
struct tg_segment tg_rect_segment_at(struct tg_rect rect, int index);
int tg_geom_de9im_dims(const struct tg_geom *geom);
bool tg_point_covers_point(struct tg_point a, struct tg_point b);
bool tg_point_covers_rect(struct tg_point a, struct tg_rect b);
bool tg_point_covers_line(struct tg_point a, const struct tg_line *b);
bool tg_point_covers_poly(struct tg_point a, const struct tg_poly *b);
bool tg_geom_covers_point(const struct tg_geom *a, struct tg_point b);
bool tg_geom_covers_xy(const struct tg_geom *a, double x, double y);
bool tg_segment_covers_segment(struct tg_segment a, struct tg_segment b);
bool tg_segment_covers_point(struct tg_segment a, struct tg_point b);
bool tg_segment_covers_rect(struct tg_segment a, struct tg_rect b);
bool tg_rect_covers_point(struct tg_rect a, struct tg_point b);
bool tg_rect_covers_xy(struct tg_rect a, double x, double y);
bool tg_rect_covers_rect(struct tg_rect a, struct tg_rect b);
bool tg_rect_covers_line(struct tg_rect a, const struct tg_line *b);
bool tg_rect_covers_poly(struct tg_rect a, const struct tg_poly *b);
bool tg_line_covers_point(const struct tg_line *a, struct tg_point b);
bool tg_line_covers_rect(const struct tg_line *a, struct tg_rect b);
bool tg_line_covers_line(const struct tg_line *a, const struct tg_line *b);
bool tg_line_covers_poly(const struct tg_line *a, const struct tg_poly *b);
bool tg_line_intersects_point(const struct tg_line *a, struct tg_point b);
bool tg_line_intersects_rect(const struct tg_line *a, struct tg_rect b);
bool tg_line_intersects_line(const struct tg_line *a, const struct tg_line *b);
bool tg_line_intersects_poly(const struct tg_line *a, const struct tg_poly *b);
bool tg_point_intersects_point(struct tg_point a, struct tg_point b);
bool tg_point_intersects_rect(struct tg_point a, struct tg_rect b);
bool tg_point_intersects_line(struct tg_point a, const struct tg_line *b);
bool tg_point_intersects_poly(struct tg_point a, const struct tg_poly *b);
bool tg_rect_intersects_rect(struct tg_rect a, struct tg_rect b);
bool tg_rect_intersects_line(struct tg_rect a, const struct tg_line *b);
bool tg_rect_intersects_poly(struct tg_rect a, const struct tg_poly *b);
bool tg_segment_intersects_segment(struct tg_segment a, struct tg_segment b);
bool tg_poly_covers_xy(const struct tg_poly *a, double x, double y);
bool tg_poly_touches_line(const struct tg_poly *a, const struct tg_line *b);
bool tg_poly_coveredby_poly(const struct tg_poly *a, const struct tg_poly *b);
bool tg_poly_covers_point(const struct tg_poly *a, struct tg_point b);
bool tg_poly_covers_rect(const struct tg_poly *a, struct tg_rect b);
bool tg_poly_covers_line(const struct tg_poly *a, const struct tg_line *b);
bool tg_poly_covers_poly(const struct tg_poly *a, const struct tg_poly *b);
bool tg_poly_intersects_point(const struct tg_poly *a, struct tg_point b);
bool tg_poly_intersects_rect(const struct tg_poly *a, struct tg_rect b);
bool tg_poly_intersects_line(const struct tg_poly *a, const struct tg_line *b);
bool tg_poly_intersects_poly(const struct tg_poly *a, const struct tg_poly *b);
bool tg_geom_intersects_point(const struct tg_geom *a, struct tg_point b);

static_assert(sizeof(int) == 4 || sizeof(int) == 8,  "invalid int size");

// Function attribute for noinline.
#if defined(__GNUC__)
#define __attr_noinline __attribute__((noinline))
#elif defined(_MSC_VER)
#define __attr_noinline __declspec(noinline)
#else
#define __attr_noinline
#endif

// Fast floating-point min and max for gcc and clang on arm64 and x64.
#if defined(__GNUC__) && defined(__aarch64__)
// arm64 already uses branchless operations for fmin and fmax
static inline double fmin0(double x, double y) {
    return fmin(x, y);
}
static inline double fmax0(double x, double y) {
    return fmax(x, y);
}
static inline float fminf0(float x, float y) {
    return fminf(x, y);
}
static inline float fmaxf0(float x, float y) {
    return fmaxf(x, y);
}
#elif defined(__GNUC__) && defined(__x86_64__)
// gcc/amd64 sometimes uses branching with fmin/fmax. 
// This code use single a asm op instead.
// https://gcc.gnu.org/bugzilla//show_bug.cgi?id=94497
#pragma GCC diagnostic push
#ifdef __clang__
#pragma clang diagnostic ignored "-Wlanguage-extension-token"
#endif
static inline double fmin0(double x, double y) {
    asm("minsd %1, %0" : "+x" (x) : "x" (y));
    return x;
}
static inline double fmax0(double x, double y) {
    asm("maxsd %1, %0" : "+x" (x) : "x" (y));
    return x;
}
static inline float fminf0(float x, float y) {
    asm("minss %1, %0" : "+x" (x) : "x" (y));
    return x;
}
static inline float fmaxf0(float x, float y) {
    asm("maxss %1, %0" : "+x" (x) : "x" (y));
    return x;
}
#pragma GCC diagnostic pop
#else
// for everything else, let the compiler figure it out.
static inline double fmin0(double x, double y) {
    return x < y ? x : y;
}
static inline double fmax0(double x, double y) {
    return x > y ? x : y;
}
static inline float fminf0(double x, float y) {
    return x < y ? x : y;
}
static inline float fmaxf0(float x, float y) {
    return x > y ? x : y;
}
#endif

static inline double fclamp0(double f, double min, double max) {
    return fmin0(fmax0(f, min), max);
}

static bool feq(double x, double y) {
    return !((x < y) | (x > y));
}

static bool eq_zero(double x) {
    return feq(x, 0);
}

static bool collinear(
    double x1, double y1, // point 1
    double x2, double y2, // point 2
    double x3, double y3  // point 3
) { 
    bool x1x2 = feq(x1, x2);
    bool x1x3 = feq(x1, x3);
    bool x2x3 = feq(x2, x3);
    bool y1y2 = feq(y1, y2);
    bool y1y3 = feq(y1, y3);
    bool y2y3 = feq(y2, y3);
    if (x1x2) {
        return x1x3;
    }
    if (y1y2) {
        return y1y3;
    }
    if ((x1x2 & y1y2) | (x1x3 & y1y3) | (x2x3 & y2y3)) {
        return true;
    }
    double cx1 = x3 - x1;
    double cy1 = y3 - y1;
    double cx2 = x2 - x1;
    double cy2 = y2 - y1;
    double s1 = cx1 * cy2;
    double s2 = cy1 * cx2;
    // Check if precision was lost.
    double s3 = (s1 / cy2) - cx1;
    double s4 = (s2 / cx2) - cy1;
    if (s3 < 0) {
        s1 = nexttoward(s1, -INFINITY);
    } else if (s3 > 0) {
        s1 = nexttoward(s1, +INFINITY);
    }
    if (s4 < 0) {
        s2 = nexttoward(s2, -INFINITY);
    } else if (s4 > 0) {
        s2 = nexttoward(s2, +INFINITY);
    }
    return eq_zero(s1 - s2);
}

static double length(double x1, double y1, double x2, double y2) {
    return sqrt((x1-x2) * (x1-x2) + (y1-y2) * (y1-y2));
}
    
#ifndef ludo
#define ludo
#define ludo1(i,f) f; i++;
#define ludo2(i,f) ludo1(i,f); ludo1(i,f);
#define ludo4(i,f) ludo2(i,f); ludo2(i,f);
#define ludo8(i,f) ludo4(i,f); ludo4(i,f);
#define ludo16(i,f) ludo8(i,f); ludo8(i,f);
#define ludo32(i,f) ludo16(i,f); ludo16(i,f);
#define ludo64(i,f) ludo32(i,f); ludo32(i,f);
#define for1(i,n,f) while(i+1<=(n)) { ludo1(i,f); }
#define for2(i,n,f) while(i+2<=(n)) { ludo2(i,f); } for1(i,n,f);
#define for4(i,n,f) while(i+4<=(n)) { ludo4(i,f); } for1(i,n,f);
#define for8(i,n,f) while(i+8<=(n)) { ludo8(i,f); } for1(i,n,f);
#define for16(i,n,f) while(i+16<=(n)) { ludo16(i,f); } for1(i,n,f);
#define for32(i,n,f) while(i+32<=(n)) { ludo32(i,f); } for1(i,n,f);
#define for64(i,n,f) while(i+64<=(n)) { ludo64(i,f); } for1(i,n,f);
#endif

static size_t grow_cap(size_t cap, size_t init_cap) {
    if (cap == 0) {
        return init_cap;
    }
    if (cap < 1000) {
        return cap * 2;
    }
    return cap * 1.25;
}

#define print_segment(s) { \
    printf("(%f %f,%f %f)", (s).a.x, (s).a.y, (s).b.x, (s).b.y); \
}

#define print_geom(g) { \
    size_t sz = tg_geom_wkt(g, 0, 0); \
    char *wkt = tg_malloc(sz+1); \
    assert(wkt); \
    tg_geom_wkt(g, wkt, sz+1); \
    printf("%s\n", wkt); \
    free(wkt); \
}
/////////////////////
// global behaviors
/////////////////////

// Global variables shared by all TG functions.

static void *(*_malloc)(size_t) = NULL;
static void *(*_realloc)(void*, size_t) = NULL;
static void (*_free)(void*) = NULL;
static enum tg_index default_index = TG_NATURAL;
static int index_spread = 16;

/// Allow for configuring a custom allocator.
///
/// This overrides the built-in malloc, realloc, and free functions for all
/// TG operations.
/// @warning This function, if needed, should be called **only once** at
/// program start up and prior to calling any other tg_*() function.
/// @see GlobalFuncs
void tg_env_set_allocator(
    void *(*malloc)(size_t), 
    void *(*realloc)(void*, size_t),
    void (*free)(void*)) 
{
    _malloc = malloc;
    _realloc = realloc;
    _free = free;
}

void *tg_malloc(size_t nbytes) {
    return (_malloc?_malloc:malloc)(nbytes);
}

void *tg_realloc(void *ptr, size_t nbytes) {
    return (_realloc?_realloc:realloc)(ptr, nbytes);
}

void tg_free(void *ptr) {
    (_free?_free:free)(ptr);
}

/// Set the geometry indexing default.
/// 
/// This is a global override to the indexing for all yet-to-be created 
/// geometries.
/// @warning This function, if needed, should be called **only once** at
/// program start up and prior to calling any other tg_*() function.
/// @see [tg_index](.#tg_index)
/// @see tg_env_set_index()
/// @see GlobalFuncs
void tg_env_set_index(enum tg_index ix) {
    switch (ix) {
    case TG_NONE: 
    case TG_NATURAL: 
    case TG_YSTRIPES:
        // only change for none, natural, and ystripes
        default_index = ix;
        break;
    default:
        // no change
        break;
    }
}

/// Get the current geometry indexing default.
/// @see [tg_index](.#tg_index)
/// @see tg_env_set_index()
/// @see GlobalFuncs
enum tg_index tg_env_get_default_index(void) {
    return default_index;
}

/// Set the default index spread.
/// 
/// The "spread" is how many rectangles are grouped together on an indexed
/// level before propagating up to a higher level.
/// 
/// Default is 16.
///
/// This is a global override to the indexing spread for all yet-to-be created 
/// geometries.
/// @warning This function, if needed, should be called **only once** at
/// program start up and prior to calling any other tg_*() function.
/// @see [tg_index](.#tg_index)
/// @see tg_env_set_index()
/// @see <a href="POLYGON_INDEXING.md">About TG indexing</a>
/// @see GlobalFuncs
void tg_env_set_index_spread(int spread) {
    // only allow spreads between 2 and 1024
    if (spread >= 2 && spread <= 4096) {
        index_spread = spread;
    }
}
int tg_env_get_index_spread(void) {
    return index_spread;
}

enum tg_index tg_index_with_spread(enum tg_index ix, int spread) {
    // Only 16 bits of the tg_index is used.
    // first 4 bits is the index. The next 12 is the spread.
    if (spread != 0) {
        spread = spread < 2 ? 2 : spread > 4096 ? 4096 : spread;
        spread--; // ensure range 1-4095 (but will actually be 2-4096)
    }
    return (ix & 0xF) | (spread << 4);
}

enum tg_index tg_index_extract_spread(enum tg_index ix, int *spread) {
    int ixspread = ((unsigned int)(ix >> 4)) & 4095;
    if (ixspread > 0) {
        ixspread++;
    }
    if (ixspread == 0) {
        ixspread = index_spread;
    }
    if (spread) {
        *spread = ixspread;
    }
    return ix & 0xF;
}

////////////////////
// point
////////////////////

static bool pteq(struct tg_point a, struct tg_point b) {
    return feq(a.x, b.x) && feq(a.y, b.y);
}

/// Returns the minimum bounding rectangle of a point. 
/// @see PointFuncs
struct tg_rect tg_point_rect(struct tg_point point) {
    return (struct tg_rect){ .min = point, .max = point };
}

/// Tests whether a point fully contains another point.
/// @see PointFuncs
bool tg_point_covers_point(struct tg_point point, struct tg_point other) {
    return pteq(point, other);
}

bool tg_point_contains_point(struct tg_point point, struct tg_point other) {
    return pteq(point, other);
}

/// Tests whether a point intersects another point.
/// @see PointFuncs
bool tg_point_intersects_point(struct tg_point a, struct tg_point b) {
    return pteq(a, b);
}

bool tg_point_touches_point(struct tg_point a, struct tg_point b) {
    // Points do not have boundaries
    (void)a; (void)b;
    return false;
}

/// Tests whether a point fully contains a rectangle.
/// @see PointFuncs
bool tg_point_covers_rect(struct tg_point point, struct tg_rect rect) {
    return pteq(rect.min, point) && pteq(rect.max, point);
}

/// Tests whether a point fully intersects a rectangle.
/// @see PointFuncs
bool tg_point_intersects_rect(struct tg_point point, struct tg_rect rect) { 
    return tg_rect_covers_point(rect, point);
}

/// Tests whether a point fully contains a line.
/// @see PointFuncs
bool tg_point_covers_line(struct tg_point point, const struct tg_line *line) {
    return !tg_line_empty(line) && 
           tg_point_covers_rect(point, tg_line_rect(line));
}

bool tg_point_contains_line(struct tg_point point, const struct tg_line *line) {
    return !tg_line_empty(line) && 
           tg_point_covers_rect(point, tg_line_rect(line));
}

/// Tests whether a point intersects a line.
/// @see PointFuncs
bool tg_point_intersects_line(struct tg_point point, 
    const struct tg_line *line)
{
    return tg_line_intersects_point(line, point);
}

bool tg_point_touches_line(struct tg_point point, const struct tg_line *line) {
    int nsegs = tg_line_num_segments(line);
    if (nsegs == 0) {
        return false;
    }
    struct tg_segment s0 = tg_line_segment_at(line, 0);
    struct tg_segment sN = tg_line_segment_at(line, nsegs-1);
    return pteq(point, s0.a) || pteq(point, sN.b);
}

/// Tests whether a point fully contains a polygon.
/// @see PointFuncs
bool tg_point_covers_poly(struct tg_point point, const struct tg_poly *poly) {
    return !tg_poly_empty(poly) && 
           tg_point_covers_rect(point, tg_poly_rect(poly));
}

bool tg_point_contains_poly(struct tg_point point, const struct tg_poly *poly) {
    // not possible
    (void)point; (void)poly;
    return false;
}

/// Tests whether a point intersects a polygon.
/// @see PointFuncs
bool tg_point_intersects_poly(struct tg_point point,
    const struct tg_poly *poly)
{
    return tg_poly_intersects_point(poly, point);
}

bool tg_point_touches_poly(struct tg_point point, const struct tg_poly *poly) {
    // Return true if the point touches the boundary of the exterior ring or
    // the boundary of the interior holes. 
    const struct tg_ring *ring = tg_poly_exterior(poly);
    if (tg_line_covers_point((struct tg_line*)ring, point)) {
        return true;
    }
    int nholes = tg_poly_num_holes(poly);
    for (int i = 0; i < nholes; i++) {
        const struct tg_ring *ring = tg_poly_hole_at(poly, i);
        if (tg_line_covers_point((struct tg_line*)ring, point)) {
           return true;
        }
    }
    return false;
}

////////////////////
// segment
////////////////////

static bool point_on_segment(struct tg_point p, struct tg_segment s) {
    if (!tg_rect_covers_point(tg_segment_rect(s), p)) {
        return false;
    }
    return collinear(s.a.x, s.a.y, s.b.x, s.b.y, p.x, p.y);
}

enum tg_raycast_result {
    TG_OUT,  // point is above, below, or to the right of the segment
    TG_IN,   // point is to the left (and inside the vertical bounds)
    TG_ON,   // point is on the segment
};

static enum tg_raycast_result raycast(struct tg_segment seg, 
    struct tg_point p)
{
    struct tg_rect r = tg_segment_rect(seg);
    if (p.y < r.min.y || p.y > r.max.y) {
        return TG_OUT;
    }
    if (p.x < r.min.x) {
        if (p.y != r.min.y && p.y != r.max.y) {
            return TG_IN;
        }
    } else if (p.x > r.max.x) {
        if (r.min.y != r.max.y && r.min.x != r.max.x) {
            return TG_OUT;
        }
    }
    struct tg_point a = seg.a;
    struct tg_point b = seg.b;
    if (b.y < a.y) {
        struct tg_point t = a;
        a = b;
        b = t;
    }
    if (pteq(p, a) || pteq(p, b)) {
        return TG_ON;
    }
    if (a.y == b.y) {
        if (a.x == b.x) {
            return TG_OUT;
        }
        if (p.y == b.y) {
            if (!(p.x < r.min.x || p.x > r.max.x)) {
                return TG_ON;
            }
        }
    }
    if (a.x == b.x && p.x == b.x) {
        if (p.y >= a.y && p.y <= b.y) {
            return TG_ON;
        }
    }
    if (collinear(a.x, a.y, b.x, b.y, p.x, p.y)) {
        if (p.x < r.min.x) {
            if (r.min.y == r.max.y) {
                return TG_OUT;
            }
        } else if (p.x > r.max.x) {
            return TG_OUT;
        }
        return TG_ON;
    }
    if (p.y == a.y || p.y == b.y) {
        p.y = nexttoward(p.y, INFINITY);
    }
    if (p.y < a.y || p.y > b.y) {
        return TG_OUT;
    }
    if (a.x > b.x) {
        if (p.x >= a.x) {
            return TG_OUT;
        }
        if (p.x <= b.x) {
            return TG_IN;
        }
    } else {
        if (p.x >= b.x) {
            return TG_OUT;
        }
        if (p.x <= a.x) {
            return TG_IN;
        }
    }
    if ((p.y-a.y)/(p.x-a.x) >= (b.y-a.y)/(b.x-a.x)) {
        return TG_IN;
    }
    return TG_OUT;
}

/// Performs the raycast operation of a point on segment
enum tg_raycast_result tg_raycast(struct tg_segment seg, struct tg_point p) {
    return raycast(seg, p);
}

struct tg_point tg_point_move(struct tg_point point, 
    double delta_x, double delta_y)
{
    return (struct tg_point){ .x = point.x + delta_x, .y = point.y + delta_y };
}

struct tg_segment tg_segment_move(struct tg_segment seg,
    double delta_x, double delta_y)
{
    return (struct tg_segment){
        .a = tg_point_move(seg.a, delta_x, delta_y),
        .b = tg_point_move(seg.b, delta_x, delta_y),
    };
}

/// Tests whether a segment fully contains a point.
/// @see SegmentFuncs
bool tg_segment_covers_point(struct tg_segment seg, struct tg_point p) {
    return point_on_segment(p, seg);
}

/// Tests whether a segment fully contains another segment.
/// @see SegmentFuncs
bool tg_segment_covers_segment(struct tg_segment a, struct tg_segment b) {
    return tg_segment_covers_point(a, b.a) && 
           tg_segment_covers_point(a, b.b);
}

static void segment_fill_rect(const struct tg_segment *seg, 
    struct tg_rect *rect)
{
    rect->min.x = fmin0(seg->a.x, seg->b.x);
    rect->min.y = fmin0(seg->a.y, seg->b.y);
    rect->max.x = fmax0(seg->a.x, seg->b.x);
    rect->max.y = fmax0(seg->a.y, seg->b.y);
}

/// Returns the minimum bounding rectangle of a segment. 
/// @see SegmentFuncs
struct tg_rect tg_segment_rect(struct tg_segment seg) {
    struct tg_rect rect;
    segment_fill_rect(&seg, &rect);
    return rect;
}

static bool rect_intersects_rect(struct tg_rect *a, struct tg_rect *b) {
    return !(b->min.x > a->max.x || b->max.x < a->min.x ||
             b->min.y > a->max.y || b->max.y < a->min.y);
}

/// Tests whether a rectangle intersects another rectangle.
bool tg_rect_intersects_rect(struct tg_rect a, struct tg_rect b) {
    return rect_intersects_rect(&a, &b);
}

/// Tests whether a rectangle full contains another rectangle.
bool tg_rect_covers_rect(struct tg_rect a, struct tg_rect b) {
    return !(b.min.x < a.min.x || b.max.x > a.max.x ||
             b.min.y < a.min.y || b.max.y > a.max.y);
}

/// Returns the number of points. Always 5 for rects.
int tg_rect_num_points(struct tg_rect r) {
    (void)r;
    return 5;
}

/// Returns the number of segments. Always 4 for rects.
int tg_rect_num_segments(struct tg_rect r) {
    (void)r;
    return 4;
}

/// Returns the point at index.
struct tg_point tg_rect_point_at(struct tg_rect r, int index) {
    switch (index) {
    case 0:
        return (struct tg_point){ r.min.x, r.min.y };
    case 1:
        return (struct tg_point){ r.max.x, r.min.y };
    case 2:
        return (struct tg_point){ r.max.x, r.max.y };
    case 3:
        return (struct tg_point){ r.min.x, r.max.y };
    case 4:
        return (struct tg_point){ r.min.x, r.min.y };
    default:
        return (struct tg_point){ 0 };
    }
}

/// Returns the segment at index.
struct tg_segment tg_rect_segment_at(struct tg_rect r, int index) {
    switch (index) {
    case 0:
        return (struct tg_segment){ { r.min.x, r.min.y}, { r.max.x, r.min.y} };
    case 1:
        return (struct tg_segment){ { r.max.x, r.min.y}, { r.max.x, r.max.y} };
    case 2:
        return (struct tg_segment){ { r.max.x, r.max.y}, { r.min.x, r.max.y} };
    case 3:
        return (struct tg_segment){ { r.min.x, r.max.y}, { r.min.x, r.min.y} };
    default:
        return (struct tg_segment){ 0 };
    }
}

/// Tests whether a segment intersects another segment.
/// @see SegmentFuncs
bool tg_segment_intersects_segment(struct tg_segment seg_a, 
    struct tg_segment seg_b)
{
    struct tg_point a = seg_a.a;
    struct tg_point b = seg_a.b;
    struct tg_point c = seg_b.a;
    struct tg_point d = seg_b.b;
    if (!tg_rect_intersects_rect(tg_segment_rect(seg_a), 
        tg_segment_rect(seg_b)))
    {
        return false;
    }
    
    if (pteq(seg_a.a, seg_b.a) || pteq(seg_a.a, seg_b.b) ||
        pteq(seg_a.b, seg_b.a) || pteq(seg_a.b, seg_b.b))
    {
        return true;
    }

    double cmpx = c.x-a.x;
    double cmpy = c.y-a.y;
    double rx = b.x-a.x;
    double ry = b.y-a.y;
    double cmpxr = cmpx*ry - cmpy*rx;
    if (eq_zero(cmpxr)) {
        // collinear, and so intersect if they have any overlap
        if (!(((c.x-a.x <= 0) != (c.x-b.x <= 0))) ||
              ((c.y-a.y <= 0) != (c.y-b.y <= 0)))
        {
            return (tg_segment_covers_point(seg_a, seg_b.a) || 
                    tg_segment_covers_point(seg_a, seg_b.b));
        }
        return true;
    }
    double sx = d.x-c.x;
    double sy = d.y-c.y;
    double rxs = rx*sy - ry*sx;
    if (eq_zero(rxs)) {
        // Segments are parallel.
        return false; 
    }
    double cmpxs = cmpx*sy - cmpy*sx;
    double rxsr = 1 / rxs;
    double t = cmpxs * rxsr;
    double u = cmpxr * rxsr;
    if (!((t >= 0) && (t <= 1) && (u >= 0) && (u <= 1))) {
        return false;
    }
    return true;
}

/// Tests whether a segment fully contains a rectangle.
/// @see SegmentFuncs
bool tg_segment_covers_rect(struct tg_segment seg, struct tg_rect rect) {
    return tg_segment_covers_point(seg, rect.min) && 
           tg_segment_covers_point(seg, rect.max);
}

//////////////////
// ystripes
//////////////////

struct ystripe {
    int count;
    int *indexes;
};

struct buf {
    uint8_t *data;
    size_t len, cap;
};

static bool buf_ensure(struct buf *buf, size_t len) {
    if (buf->cap-buf->len >= len) return true;
    size_t cap = buf->cap;
    do {
        cap = grow_cap(cap, 16);
    } while (cap-buf->len < len);
    uint8_t *data = tg_realloc(buf->data, cap+1);
    if (!data) return false;
    buf->data = data;
    buf->cap = cap;
    buf->data[buf->cap] = '\0';
    return true;
}

// buf_append_byte append byte to buffer. 
// The buf->len should be greater than before, otherwise out of memory.
static bool buf_append_byte(struct buf *buf, uint8_t b) {
    if (!buf_ensure(buf, 1)) return false;
    buf->data[buf->len++] = b;
    return true;
}

static bool buf_append_bytes(struct buf *buf, uint8_t *bytes, size_t nbytes) {
    if (!buf_ensure(buf, nbytes)) return false;
    memcpy(buf->data+buf->len, bytes, nbytes);
    buf->len += nbytes;
    return true;
}

static bool buf_trunc(struct buf *buf) {
    if (buf->cap-buf->len > 8) {
        uint8_t *data = tg_realloc(buf->data, buf->len);
        if (!data) return false;
        buf->data = data;
        buf->cap = buf->len;
    }
    return true;
}

//////////////////
// index
//////////////////

#if !defined(TG_IXFLOAT64)

// 32-bit floats

struct ixpoint {
    float x;
    float y;
};
struct ixrect {
    struct ixpoint min;
    struct ixpoint max;
};

static inline void ixrect_expand(struct ixrect *rect, struct ixrect *other) {
    rect->min.x = fminf0(rect->min.x, other->min.x);
    rect->min.y = fminf0(rect->min.y, other->min.y);
    rect->max.x = fmaxf0(rect->max.x, other->max.x);
    rect->max.y = fmaxf0(rect->max.y, other->max.y);
}

// fdown/fup returns a value that is a little larger or smaller.
// Works similarly to nextafter, though it's faster and less accurate.
//    fdown(d) -> nextafter(d, -INFINITY);
//    fup(d)   -> nextafter(d, INFINITY); 
// fnext0 returns the next value after d. 
// Param dir can be 0 (towards -INFINITY) or 1 (towards INFINITY). 
static double fnext0(double d, int dir) {
    static double vals[2] = {
        1.0 - 1.0/8388608.0, /* towards zero */
        1.0 + 1.0/8388608.0  /* away from zero */
    };
    return d * vals[((d<0)+dir)&1];
}

static double fdown(double d) {
    return fnext0(d, 0);
}

static double fup(double d) {
    return fnext0(d, 1);
}

static void tg_rect_to_ixrect(struct tg_rect *rect, 
    struct ixrect *ixrect)
{
    ixrect->min.x = fdown(rect->min.x);
    ixrect->min.y = fdown(rect->min.y);
    ixrect->max.x = fup(rect->max.x);
    ixrect->max.y = fup(rect->max.y);
}

#else

// 64-bit floats

struct ixpoint {
    double x;
    double y;
};

struct ixrect {
    struct ixpoint min;
    struct ixpoint max;
};

static inline void ixrect_expand(struct ixrect *rect, struct ixrect *other) {
    rect->min.x = fmin0(rect->min.x, other->min.x);
    rect->min.y = fmin0(rect->min.y, other->min.y);
    rect->max.x = fmax0(rect->max.x, other->max.x);
    rect->max.y = fmax0(rect->max.y, other->max.y);
}
static inline void tg_rect_to_ixrect(struct tg_rect *rect, 
    struct ixrect *ixrect)
{
    ixrect->min.x = rect->min.x;
    ixrect->min.y = rect->min.y;
    ixrect->max.x = rect->max.x;
    ixrect->max.y = rect->max.y;
}

#endif

struct level {
    int nrects;
    struct ixrect *rects;
};

struct index {
    size_t memsz;             // memory size of index
    int spread;               // index spread (num items in each "node")
    int nlevels;              // number of levels
    struct level levels[];    // all levels starting with root
};

static inline void tg_point_to_ixpoint(struct tg_point *point, 
    struct ixpoint *ixpoint)
{
    ixpoint->x = point->x;
    ixpoint->y = point->y;
}


static inline void ixrect_to_tg_rect(struct ixrect *ixrect,
    struct tg_rect *rect)
{
    rect->min.x = ixrect->min.x;
    rect->min.y = ixrect->min.y;
    rect->max.x = ixrect->max.x;
    rect->max.y = ixrect->max.y;
}

static inline bool ixrect_intersects_ixrect(struct ixrect *a, 
    struct ixrect *b)
{
    if (a->min.y > b->max.y || a->max.y < b->min.y) return false;
    if (a->min.x > b->max.x || a->max.x < b->min.x) return false;
    return true;
}

static int calc_num_keys(int spread, int level, int count) {
    return (int)ceil((double)count / pow((double)spread, (double)level));
}

static int calc_num_levels(int spread, int count) {
    int level = 1;
    for (; calc_num_keys(spread, level, count) > 1; level++);
    return level;
}

static size_t aligned_size(size_t size) {
    return size&7 ? size+8-(size&7) : size;
}

size_t tg_aligned_size(size_t size) {
    return aligned_size(size);
}

// calc_index_size returns the space needed to hold all the data for an index.
static size_t calc_index_size(int ixspread, int nsegs, int *nlevelsout) {
    int nlevels = calc_num_levels(ixspread, nsegs);
    int inlevels = nlevels-1;
    size_t size = sizeof(struct index);
    size += inlevels*sizeof(struct level);
    for (int i = 0; i < inlevels; i++) {
        int nkeys = calc_num_keys(ixspread, inlevels-i, nsegs);
        size += nkeys*sizeof(struct ixrect);
    }
    *nlevelsout = nlevels;
    size = aligned_size(size);
    return size;
}

struct ystripes {
    size_t memsz;
    int nstripes;
    struct ystripe stripes[];
};

static bool process_ystripes(struct tg_ring *ring) {
    double score = tg_ring_polsby_popper_score(ring);
    int nstripes = ring->nsegs * score;
    nstripes = fmax0(nstripes, 32);
    double height = ring->rect.max.y - ring->rect.min.y;

    // ycounts is used to log the number of segments in each stripe.
    int *ycounts = tg_malloc(nstripes*sizeof(int));
    if (!ycounts) {
        return false;
    }
    memset(ycounts, 0, nstripes*sizeof(int));

    // nmap is the the total number of mapped segments. This will be greater
    // than the total number of segments because a segment can exist in
    // multiple stripes.
    int nmap = 0;

    // Run through each segment and determine which stripes it belongs to and
    // increment the nmap counter.
    for (int i = 0; i < ring->nsegs; i++) {
        double ymin = fmin0(ring->points[i].y, ring->points[i+1].y);
        double ymax = fmax0(ring->points[i].y, ring->points[i+1].y);
        int min = (ymin - ring->rect.min.y) / height * (double)nstripes;
        int max = (ymax - ring->rect.min.y) / height * (double)nstripes;
        min = fmax0(min, 0);
        max = fmin0(max, nstripes-1);
        for (int j = min; j <= max; j++) {
            ycounts[j]++;
            nmap++;
        }
    }

    size_t tsize = sizeof(struct ystripes);
    tsize += nstripes*sizeof(struct ystripe);
    size_t mark = tsize;
    tsize += nmap*sizeof(int);
    struct ystripes *ystripes = tg_malloc(tsize);
    if (!ystripes) {
        tg_free(ycounts);
        return false;
    }
    ystripes->memsz = tsize;
    ystripes->nstripes = nstripes;

    size_t pos = mark;
    for (int i = 0; i < nstripes; i++) {
        ystripes->stripes[i].count = 0;
        ystripes->stripes[i].indexes = (void*)&((char*)ystripes)[pos]; 
        pos += ycounts[i]*sizeof(int);
    }
    tg_free(ycounts);

    for (int i = 0; i < ring->nsegs; i++) {
        double ymin = fmin0(ring->points[i].y, ring->points[i+1].y);
        double ymax = fmax0(ring->points[i].y, ring->points[i+1].y);
        int min = (ymin - ring->rect.min.y) / height * (double)nstripes;
        int max = (ymax - ring->rect.min.y) / height * (double)nstripes;
        min = fmax0(min, 0);
        max = fmin0(max, nstripes-1);
        for (int j = min; j <= max; j++) {
            struct ystripe *stripe = &ystripes->stripes[j];
            stripe->indexes[stripe->count++] = i;
        }
    }
    ring->ystripes = ystripes;
    return true;
}

static struct tg_segment ring_segment_at(const struct tg_ring *ring, int index);

//////////////////
// rect
//////////////////

/// Tests whether a rectangle fully contains a point using xy coordinates.
/// @see RectFuncs
bool tg_rect_covers_xy(struct tg_rect rect, double x, double y) {
    return !(x < rect.min.x || y < rect.min.y || 
             x > rect.max.x || y > rect.max.y);
}

/// Tests whether a rectangle fully contains a point.
/// @see RectFuncs
bool tg_rect_covers_point(struct tg_rect rect, struct tg_point point) {
    return tg_rect_covers_xy(rect, point.x, point.y);
}

/// Tests whether a rectangle and a point intersect.
/// @see RectFuncs
bool tg_rect_intersects_point(struct tg_rect rect, struct tg_point point) {
    return tg_rect_covers_point(rect, point);
}

/// Returns the center point of a rectangle
/// @see RectFuncs
struct tg_point tg_rect_center(struct tg_rect rect) {
    return (struct tg_point){ 
        .x = (rect.max.x + rect.min.x) / 2,
        .y = (rect.max.y + rect.min.y) / 2,
    };
}

static void rect_inflate(struct tg_rect *rect, struct tg_rect *other) {
    rect->min.x = fmin0(rect->min.x, other->min.x);
    rect->min.y = fmin0(rect->min.y, other->min.y);
    rect->max.x = fmax0(rect->max.x, other->max.x);
    rect->max.y = fmax0(rect->max.y, other->max.y);
}

static void rect_inflate_point(struct tg_rect *rect, struct tg_point *point) {
    rect->min.x = fmin0(rect->min.x, point->x);
    rect->min.y = fmin0(rect->min.y, point->y);
    rect->max.x = fmax0(rect->max.x, point->x);
    rect->max.y = fmax0(rect->max.y, point->y);
}

/// Expands a rectangle to include an additional rectangle.
/// @param rect Input rectangle
/// @param other Input rectangle
/// @return Expanded rectangle
struct tg_rect tg_rect_expand(struct tg_rect rect, struct tg_rect other) {
    rect_inflate(&rect, &other);
    return rect;
}

/// Expands a rectangle to include an additional point.
/// @param rect Input rectangle
/// @param point Input Point
/// @return Expanded rectangle
struct tg_rect tg_rect_expand_point(struct tg_rect rect, struct tg_point point)
{
    rect_inflate_point(&rect, &point);
    return rect;
}

// rect_to_ring fills the ring with rect data.
static void rect_to_ring(struct tg_rect rect, struct tg_ring *ring) {
    memset(ring, 0, sizeof(struct tg_ring));
    ring->head.base = BASE_RING;
    ring->head.type = TG_POLYGON;
    ring->rect = rect;
    ring->closed = true;
    ring->convex = true;
    ring->npoints = 5;
    ring->nsegs = 4;
    for (int i = 0; i < 5; i++) {
        ring->points[i] = tg_rect_point_at(rect, i);
    }
}

static void segment_to_ring(struct tg_segment seg, struct tg_ring *ring) {
    memset(ring, 0, sizeof(struct tg_ring));
    ring->rect = tg_segment_rect(seg);
    ring->closed = false;
    ring->convex = true;
    ring->npoints = 2;
    ring->nsegs = 1;
    ring->points[0] = seg.a;
    ring->points[1] = seg.b;
}

void tg_rect_search(const struct tg_rect rect, struct tg_rect target, 
    bool(*iter)(struct tg_segment seg, int index, void *udata),
    void *udata)
{
    int nsegs = tg_rect_num_segments(rect);
    for (int i = 0; i < nsegs; i++) {
        struct tg_segment seg = tg_rect_segment_at(rect, i);
        if (tg_rect_intersects_rect(tg_segment_rect(seg), target)) {
            if (!iter(seg, i, udata)) return;
        }
    }
}

/// Tests whether a rectangle fully contains line.
bool tg_rect_covers_line(struct tg_rect rect, const struct tg_line *line) {
    return !tg_line_empty(line) && 
        tg_rect_covers_rect(rect, tg_line_rect(line));
}

////////////////////
// ring
////////////////////

static void fill_in_upper_index_levels(struct index *index) {
    int ixspread = index->spread;
    for (int lvl = 1; lvl < index->nlevels; lvl++) {
        struct level *level = &index->levels[index->nlevels-1-lvl+0];
        struct level *plevel = &index->levels[index->nlevels-1-lvl+1];
        for (int i = 0; i < level->nrects; i++) {
            int s = i*ixspread;
            int e = s+ixspread;
            if (e > plevel->nrects) e = plevel->nrects;
            struct ixrect rect = plevel->rects[s];
            for (int j = s+1; j < e; j++) {
                ixrect_expand(&rect, &plevel->rects[j]);
            }
            level->rects[i] = rect;
        }
    }
}

// stack_ring returns a ring on the stack that has enough points to be capable
// of storing a rectangle. This is here to allow for tg_ring style operations
// on tg_rect without an allocation or vla.
#define stack_ring() \
(struct tg_ring*)(&(char[sizeof(struct tg_ring)+sizeof(struct tg_point)*5]){0})

static struct tg_rect process_points(const struct tg_point *points,
    int npoints, struct tg_point *ring_points, struct index *index,
    bool *convex, bool *clockwise, double *area)
{
    struct tg_rect rect = { 0 };
    if (npoints < 2) {
        if (points) {
            memcpy(ring_points, points, sizeof(struct tg_point) * npoints);
        }
        *convex = false;
        *clockwise = false;
        *area = 0;
        return rect;
    }
    
    struct tg_point a, b, c;
    bool concave = false;
    int dir = 0;
    double cwc = 0;
    int ixspread = index ? index->spread : 0;

    // Fill the initial rectangle with the first point.
    rect.min = points[0];
    rect.max = points[0];

    // 
    struct tg_rect spreadrect;
    spreadrect.min = points[0];
    spreadrect.max = points[0];

    // gather some point positions for concave and clockwise detection
    #define step_gather_abc_nowrap() { \
        a = points[i]; \
        b = points[i+1]; \
        c = points[i+2]; \
        ring_points[i] = a; \
    }
    
    #define step_gather_abc_wrap() { \
        a = points[i]; \
        if (i < npoints-2) { \
            b = points[i+1]; \
            c = points[i+2]; \
        } else if (i == npoints-1) { \
            b = points[0]; \
            c = points[1]; \
        } else { \
            b = points[i+1]; \
            c = points[0]; \
        } \
        ring_points[i] = a; \
    }
    
    #define step_inflate_rect_no_index() { \
        rect_inflate_point(&rect, &a); \
    }

    #define inflate_mbr_and_copy_next_index_rect() { \
        rect_inflate(&rect, &spreadrect); \
        /* convert the index rect to tg_rect for storing in tmesa level */ \
        tg_rect_to_ixrect(&spreadrect, \
            &index->levels[index->nlevels-1].rects[r]); \
        r++; \
    }

    // process the rectangle inflation
    #define step_inflate_rect_with_index() { \
        rect_inflate_point(&spreadrect, &a); \
        j++; \
        if (j == ixspread) { \
            rect_inflate_point(&spreadrect, &b); \
            inflate_mbr_and_copy_next_index_rect(); \
            spreadrect.min = b; \
            spreadrect.max = b; \
            j = 0; \
        } \
    }

    #define step_calc_clockwise() { \
        cwc += (b.x - a.x) * (b.y + a.y); \
    }

    #define step_calc_concave(label) { \
        double z_cross_product = (b.x-a.x)*(c.y-b.y) - (b.y-a.y)*(c.x-b.x); \
        if (dir == 0) { \
            dir = z_cross_product < 0 ? -1 : 1; \
        } else if (z_cross_product < 0) { \
            if (dir == 1) { \
                concave = true; \
                i++; \
                goto label; \
            } \
        } else if (z_cross_product > 0) { \
            if (dir == -1) { \
                concave = true; \
                i++; \
                goto label; \
            } \
        } \
    }

    // The following code is a large loop that scans over every point and 
    // performs various processing. 
    //
    // There are two major branch groups, Indexed and Non-indexed.
    // - Indexed is when there is a provided tablemesa type index structure,
    //   and each segment spread (usually eight segments at a time) are
    //   naturally inflated into a single rectangle that is stored contiguously 
    //   in a series of rectangles.
    // - Non-indexed is when there is no provided index. The entire MBR is
    //   inflated to included every point.
    //
    // For each group, there are the two outer loops. Non-wrapped and Wrapped.
    // - Non-wrapped takes the next points A, B, C from points[i+0],
    //   points[i+1], and points[i+2], without checking the points array
    //   boundary. This is safe because of the "npoints-3" condition.
    //   This is be quick because all internal operations are loop-unrolled.
    // - Wrapped is the remaining loop steps, up to three.
    //
    // Finally, there is a Convex and Concave section.
    // - Convex is the initial state. Each step will do some calcuations until
    //   it is determined if the ring is concave. Once it's known to be concave
    //   the code jumps to the Concave section.
    // - Concave section is just like the Convex section, but there is no
    //   calculation needed because it's known that the ring is concave.

    int i = 0;
    int j = 0;
    int r = 0;
    
    // LOOP START
    // Convex section
    if (!index) {
        // Non-index branch group
        // Non-wrapped outer loop
        for (;i < npoints-3; i++) {
            step_gather_abc_nowrap();
            step_inflate_rect_no_index();
            step_calc_clockwise();
            step_calc_concave(lconcave0);
        }
    lconcave0:
        // Wrapped outer loop
        for (;i < npoints; i++) {
            step_gather_abc_wrap();
            step_inflate_rect_no_index();
            step_calc_clockwise();
            step_calc_concave(lconcave);
        }
    } else {
        // Index branch group
        // Non-wrapped outer loop
        for (;i < npoints-3; i++) {
            step_gather_abc_nowrap();
            step_inflate_rect_with_index();
            step_calc_clockwise();
            step_calc_concave(lconcave1);
        }
    lconcave1:
        // Wrapped outer loop
        for (;i < npoints; i++) {
            step_gather_abc_wrap();
            step_inflate_rect_with_index();
            step_calc_clockwise();
            step_calc_concave(lconcave);
        }
    }
lconcave:
    // Concave section
    if (!index) {
        // Non-index branch group
        // Non-wrapped outer loop
        for (;i < npoints-3; i++) {
            step_gather_abc_nowrap();
            step_inflate_rect_no_index();
            step_calc_clockwise();
        }
        // Wrapped outer loop
        for (;i < npoints; i++) {
            step_gather_abc_wrap();
            step_inflate_rect_no_index();
            step_calc_clockwise();
        }
    } else {
        // Index branch group
        // Non-wrapped outer loop
        for (;i < npoints-3; i++) {
            step_gather_abc_nowrap();
            step_inflate_rect_with_index();
            step_calc_clockwise();
        }
        // Wrapped outer loop
        for (;i < npoints; i++) {
            step_gather_abc_wrap();
            step_inflate_rect_with_index();
            step_calc_clockwise();
        }
    }
    // LOOP END

    if (index) {
        // Fill in the final indexing rectangles
        if (r != index->levels[index->nlevels-1].nrects) {
            // There's one last index rectangle remaining.
            inflate_mbr_and_copy_next_index_rect();
        }
        fill_in_upper_index_levels(index);
    }

    *area = fabs(cwc / 2.0);
    *convex = !concave;
    *clockwise = cwc > 0;

    return rect;
}

static int num_segments(const struct tg_point *points, int npoints, bool closed) 
{
    if (closed) {
        if (npoints < 3) return 0;
        if (pteq(points[npoints-1], points[0])) return npoints - 1;
        return npoints;
    }
    if (npoints < 2) return 0;
    return npoints - 1;
}

static size_t calc_series_size(int npoints) {
    // Make room for an extra point to ensure a perfect ring close so that
    // that ring->points[ring->nsegs] never overflows.
    // This also allows for ring calculations to be safely run on lines.
    npoints++;
    size_t size = offsetof(struct tg_ring, points);
    size += sizeof(struct tg_point)*(npoints < 5 ? 5 : npoints);
    size = aligned_size(size);
    return size;
}

static void fill_index_struct(struct index *index, int nlevels, int nsegs, 
    int ixspread, size_t size)
{
    // Allocate the memory needed to store the entire index plus the size 
    // of the allocation in a pre header.
    index->nlevels = nlevels-1;
    index->memsz = size;
    index->spread = ixspread;

    // fill in the index structure fields
    size_t np = sizeof(struct index);
    np += index->nlevels*sizeof(struct level);
    for (int i = 0; i < index->nlevels; i++) {
        int nkeys = calc_num_keys(ixspread, index->nlevels-i, nsegs);
        index->levels[i].nrects = nkeys;
        index->levels[i].rects = (void*)&(((char*)index)[np]);
        np += nkeys*sizeof(struct ixrect);
    }
}

static struct tg_ring *series_new(const struct tg_point *points, int npoints, 
    bool closed, enum tg_index ix) 
{
    npoints = npoints <= 0 ? 0 : npoints;
    int nsegs = num_segments(points, npoints, closed);    
    size_t size = calc_series_size(npoints);

    int ixspread;
    ix = tg_index_extract_spread(ix, &ixspread);
    bool ystripes = false;
    int ixminpoints = ixspread*2;
    size_t ixsize = 0;
    int nlevels = 0;
    if (npoints >= ixminpoints) {
        bool indexed = false;
        if (ix == TG_DEFAULT) {
            ix = tg_env_get_default_index();
        }
        switch (ix) {
        case TG_NATURAL:
        case TG_YSTRIPES:
            indexed = true;
            break;
        default:
            indexed = false;
        }
        if ((ix&TG_NONE) == TG_NONE) {
            // no base index
        } else {
            // use TG_NATURAL
            indexed = true;
        }
        if (closed && ix == TG_YSTRIPES) {
            // Process ystripes for closed series only. e.g. rings, not lines.
            ystripes = true;
        }
        if (indexed) {
            ixsize = calc_index_size(ixspread, nsegs, &nlevels);
        }
    }

    // Allocate the entire ring structure and index in a single allocation.
    struct tg_ring *ring = tg_malloc(size+ixsize);
    if (!ring) return NULL;
    memset(ring, 0, sizeof(struct tg_ring));
    ring->closed = closed;
    ring->npoints = npoints;
    ring->nsegs = nsegs;
    if (ixsize) {
        // size is always 64-bit aligned.
        // assert((size & 7) == 0);
        ring->index = (struct index *)(((char*)ring)+size);
        fill_index_struct(ring->index, nlevels, nsegs, ixspread, ixsize);
    }

    ring->rect = process_points(points, npoints, ring->points, ring->index,
        &ring->convex, &ring->clockwise, &ring->area);
    
    // Fill extra point to ensure perfect close.
    ring->points[npoints] = ring->points[0];

    if (closed) {
        ring->head.base = BASE_RING;
        ring->head.type = TG_POLYGON;
    } else {
        ring->head.base = BASE_LINE;
        ring->head.type = TG_LINESTRING;
    }
    if (ystripes) {
        // Process ystripes for closed series only. e.g. rings, not lines.
        if (!process_ystripes(ring)) {
            tg_ring_free(ring);
            return NULL;
        }
    }
    return ring;
}

static struct tg_ring *series_move(const struct tg_ring *ring, bool closed, 
    double delta_x, double delta_y)
{
    if (!ring) return NULL;
    struct tg_point *points = tg_malloc(ring->npoints*sizeof(struct tg_point));
    if (!points) return NULL;
    for (int i = 0; i < ring->npoints; i++) {
        points[i] = tg_point_move(ring->points[i], delta_x, delta_y);
    }
    enum tg_index ix = 0;
    if (ring->ystripes) {
        ix = TG_YSTRIPES;
    } else if (ring->index) {
        ix = TG_NATURAL;
    } else {
        ix = TG_NONE;
    }
    struct tg_ring *final = series_new(points, ring->npoints, closed, ix);
    tg_free(points);
    return final;
}


/// Creates a ring from a series of points.
/// @param points Array of points
/// @param npoints Number of points in array
/// @return A newly allocated ring
/// @return NULL if out of memory
/// @note A tg_ring can be safely upcasted to a tg_geom. `(struct tg_geom*)ring`
/// @note A tg_ring can be safely upcasted to a tg_poly. `(struct tg_poly*)ring`
/// @note All rings with 32 or more points are automatically indexed.
/// @see tg_ring_new_ix()
/// @see RingFuncs
struct tg_ring *tg_ring_new(const struct tg_point *points, int npoints) {
    return tg_ring_new_ix(points, npoints, TG_DEFAULT);
}

/// Creates a ring from a series of points using provided index option.
/// @param points Array of points
/// @param npoints Number of points in array
/// @param ix Indexing option, e.g. TG_NONE, TG_NATURAL, TG_YSTRIPES
/// @return A newly allocated ring
/// @return NULL if out of memory
/// @note A tg_ring can be safely upcasted to a tg_geom. `(struct tg_geom*)ring`
/// @note A tg_ring can be safely upcasted to a tg_poly. `(struct tg_poly*)ring`
/// @see tg_ring_new()
/// @see [tg_index](.#tg_index)
/// @see RingFuncs
struct tg_ring *tg_ring_new_ix(const struct tg_point *points, int npoints,
    enum tg_index ix)
{
    return series_new(points, npoints, true, ix);
}

/// Releases the memory associated with a ring.
/// @param ring Input ring
/// @see RingFuncs
void tg_ring_free(struct tg_ring *ring) {
    if (!ring) return;
    if (rc_sub(&ring->head.rc) > 0) return;
    if (ring->ystripes) tg_free(ring->ystripes);
    tg_free(ring);
}

static size_t ring_alloc_size(const struct tg_ring *ring) {
    if (ring->index) {
        // The index shares same allocation as the ring.
        return (size_t)ring->index + ring->index->memsz - (size_t)ring;
    } else {
        // There is no index. Calculate the size.
        return calc_series_size(ring->npoints);
    }
}

/// Clones a ring
/// @param ring Input ring, caller retains ownership.
/// @return A duplicate of the provided ring. 
/// @note The caller is responsible for freeing with tg_ring_free().
/// @note This method of cloning uses implicit sharing through an atomic 
/// reference counter.
/// @see RingFuncs
struct tg_ring *tg_ring_clone(const struct tg_ring *ring) {
    if (!ring) return NULL;
    struct tg_ring *ring_mut = (struct tg_ring*)ring;
    rc_add(&ring_mut->head.rc);
    return ring_mut;
}

/// Returns the allocation size of the ring. 
/// @param ring Input ring
/// @return Size of ring in bytes
/// @see RingFuncs
size_t tg_ring_memsize(const struct tg_ring *ring) {
    if (!ring) return 0;
    size_t size = ring_alloc_size(ring);
    if (ring->ystripes) {
        size += ring->ystripes->memsz;
    }
    return size;
}

/// Returns the number of points.
/// @param ring Input ring
/// @return Number of points
/// @see tg_ring_point_at()
/// @see RingFuncs
int tg_ring_num_points(const struct tg_ring *ring) {
    if (!ring) return 0;
    return ring->npoints;
}

/// Returns the minimum bounding rectangle of a rect.
/// @param ring Input ring
/// @returns Minimum bounding retangle
/// @see RingFuncs
struct tg_rect tg_ring_rect(const struct tg_ring *ring) {
    if (!ring) return (struct tg_rect){ 0 };
    return ring->rect;
}

/// Returns the point at index.
/// @param ring Input ring
/// @param index Index of point
/// @return The point at index
/// @note This function performs bounds checking. Use tg_ring_points() for
/// direct access to the points.
/// @see tg_ring_num_points()
/// @see RingFuncs
struct tg_point tg_ring_point_at(const struct tg_ring *ring, int index) {
    if (!ring || index < 0 || index >= ring->npoints) {
        return (struct tg_point){ 0 };
    }
    return ring->points[index];
}

/// Returns the number of segments.
/// @param ring Input ring
/// @return Number of segments
/// @see tg_ring_segment_at()
/// @see RingFuncs
int tg_ring_num_segments(const struct tg_ring *ring) {
    if (!ring) return 0;
    return ring->nsegs;
}

/// Returns the segment at index without bounds checking
static struct tg_segment ring_segment_at(const struct tg_ring *ring, int i) {
    // The process_points operation ensures that there always one point more
    // than the number of segments.
    return (struct tg_segment) { ring->points[i], ring->points[i+1] };
}

static struct tg_segment line_segment_at(const struct tg_line *line, int i) {
    return ring_segment_at((struct tg_ring*)line, i);
}

/// Returns the segment at index.
/// @param ring Input ring
/// @param index Index of segment
/// @return The segment at index
/// @see tg_ring_num_segments()
/// @see RingFuncs
struct tg_segment tg_ring_segment_at(const struct tg_ring *ring, int index) {
    if (!ring || index < 0 || index >= ring->nsegs) {
        return (struct tg_segment){ 0 };
    }
    return ring_segment_at(ring, index);
}

static bool segment_rect_intersects_rect(const struct tg_segment *seg, 
    struct tg_rect *rect) 
{
    struct tg_rect rect2;
    segment_fill_rect(seg, &rect2);
    return rect_intersects_rect(rect, &rect2);
}

static bool index_search(const struct tg_ring *ring, struct tg_rect *rect, 
    int lvl, int start,
    bool(*iter)(struct tg_segment seg, int index, void *udata),
    void *udata)
{
    const struct index *ix = ring->index;
    int ixspread = ix->spread;
    if (lvl == ix->nlevels) {
        int nsegs = ring->nsegs;
        int i = start;
        int e = i+ixspread;
        if (e > nsegs) e = nsegs;
        for (; i < e; i++) {
            int j = i;
            struct tg_segment *seg = (struct tg_segment *)(&ring->points[j]);
            if (segment_rect_intersects_rect(seg, rect)) {
                if (!iter(*seg, j, udata)) {
                    return false;
                }
            }
        }
    } else {
        struct ixrect ixrect;
        tg_rect_to_ixrect(rect, &ixrect);
        const struct level *level = &ix->levels[lvl];
        int i = start;
        int e = i+ixspread;
        if (e > level->nrects) e = level->nrects;
        for (; i < e; i++) {
            if (ixrect_intersects_ixrect(&level->rects[i], &ixrect)) {
                if (!index_search(ring, rect, lvl+1, i*ixspread, iter, 
                    udata)) 
                {
                    return false;
                }
            }
        }
    }
    return true;
}

// ring_search searches for ring segments that intersect the provided rect.
void tg_ring_search(const struct tg_ring *ring, struct tg_rect rect, 
    bool (*iter)(struct tg_segment seg, int index, void *udata),
    void *udata)
{
    if (!ring || !iter) {
        return;
    }
    if (ring->index) {
        index_search(ring, &rect, 0, 0, iter, udata);
    } else {
        for (int i = 0; i < ring->nsegs; i++) {
            struct tg_segment *seg = (struct tg_segment *)(&ring->points[i]);
            if (segment_rect_intersects_rect(seg, &rect)) {
                if (!iter(*seg, i, udata)) return;
            }
        }
    }
}

struct ring_ring_iter_ctx {
    void *udata;
    struct tg_segment seg;
    int index;
    bool swapped;
    bool stop;
    bool (*iter)(struct tg_segment aseg, int aidx, struct tg_segment bseg, 
        int bidx, void *udata);
};

static bool ring_ring_iter(struct tg_segment seg, int index, void *udata) {
    struct ring_ring_iter_ctx *ctx = udata;
    if (tg_segment_intersects_segment(seg, ctx->seg)) {
        bool ok = ctx->swapped ? 
            ctx->iter(ctx->seg, ctx->index, seg, index, ctx->udata):
            ctx->iter(seg, index, ctx->seg, ctx->index, ctx->udata);
        if (!ok) {
            ctx->stop = true;
            return false;
        }
    }
    return true;
}

static bool ring_ring_ix(const struct tg_ring *a, int alvl, int aidx, 
    int aspread, const struct tg_ring *b, int blvl, int bidx, int bspread,
    bool (*iter)(struct tg_segment aseg, int aidx, struct tg_segment bseg, 
        int bidx, void *udata),
    void *udata)
{
    int aleaf = alvl == a->index->nlevels;
    int bleaf = blvl == b->index->nlevels;
    int anrects = aleaf ? a->nsegs : a->index->levels[alvl].nrects;
    int bnrects = bleaf ? b->nsegs : b->index->levels[blvl].nrects;
    int as = aidx;
    int ae = as + aspread;
    if (ae > anrects) ae = anrects;
    int bs = bidx;
    int be = bs + bspread;
    if (be > bnrects) be = bnrects;
    if (aleaf && bleaf) {
        // both are leaves
        for (int i = as; i < ae; i++) {
            struct tg_segment seg_a = ring_segment_at(a, i);
            for (int j = bs; j < be; j++) {
                struct tg_segment seg_b = ring_segment_at(b, j);
                if (tg_segment_intersects_segment(seg_a, seg_b)) {
                    if (!iter(seg_a, i, seg_b, j, udata)) {
                        return false;
                    }
                }
            }
        }
    } else if (aleaf) {
        // A is a leaf and B is a branch
        struct tg_rect arect, brect;
        for (int i = as; i < ae; i++) {
            struct tg_segment seg = ring_segment_at(a, i);
            segment_fill_rect(&seg, &arect);
            for (int j = bs; j < be; j++) {
                ixrect_to_tg_rect(&b->index->levels[blvl].rects[j], &brect);
                if (rect_intersects_rect(&arect, &brect)) {
                    if (!ring_ring_ix(a, alvl, i, 1,
                        b, blvl+1, j*bspread, bspread, iter, udata))
                    {
                        return false;
                    }
                }
            }
        }
    } else if (bleaf) {
        // B is a leaf and A is a branch
        struct tg_rect arect, brect;
        for (int i = as; i < ae; i++) {
            ixrect_to_tg_rect(&a->index->levels[alvl].rects[i], &arect);
            for (int j = bs; j < be; j++) {
                struct tg_segment seg = ring_segment_at(b, j);
                segment_fill_rect(&seg, &brect);
                if (rect_intersects_rect(&arect, &brect)) {
                    if (!ring_ring_ix(a, alvl+1, i*aspread, aspread,
                        b, blvl, j, 1, iter, udata))
                    {
                        return false;
                    }
                }
            }
        }
    } else {
        // both are branches
        for (int i = as; i < ae; i++) {
            for (int j = bs; j < be; j++) {
                struct ixrect *arect = &a->index->levels[alvl].rects[i];
                struct ixrect *brect = &b->index->levels[blvl].rects[j];
                if (ixrect_intersects_ixrect(arect, brect)) {
                    if (!ring_ring_ix(a, alvl+1, i*a->index->spread, aspread,
                        b, blvl+1, j*b->index->spread, bspread, iter, udata))
                    {
                        return false;
                    }
                } 
            }
        }
    }
    return true;
}

/// Iterates over all segments in ring A that intersect with segments in ring B.
/// @note This efficently uses the indexes of each geometry, if available.
/// @see RingFuncs
void tg_ring_ring_search(const struct tg_ring *a, const struct tg_ring *b, 
    bool (*iter)(struct tg_segment aseg, int aidx, struct tg_segment bseg, 
        int bidx, void *udata),
    void *udata)
{
    if (tg_ring_empty(a) || tg_ring_empty(b) || !iter || 
        !tg_rect_intersects_rect(tg_ring_rect(a), tg_ring_rect(b)))
    {
        return;
    }
    if (a->index && b->index) {
        // both indexes are available
        ring_ring_ix(a, 0, 0, a->index->spread, 
            b, 0, 0, b->index->spread, iter, udata);
    } else if (a->index || b->index) {
        // only one index is available
        const struct tg_ring *c = b->index ? b : a;
        const struct tg_ring *d = b->index ? a : b;
        struct ring_ring_iter_ctx ctx = { 
            .iter = iter, 
            .swapped = c == b,
            .udata = udata,
        };
        for (int i = 0; i < d->nsegs; i++) {
            struct tg_segment seg = ring_segment_at(d, i);
            struct tg_rect rect;
            segment_fill_rect(&seg, &rect);
            ctx.seg = seg;
            ctx.index = i;
            tg_ring_search(c, rect, ring_ring_iter, &ctx);
            if (ctx.stop) {
                return;
            }
        }
    } else {
        // no indexes are available
        for (int i = 0; i < a->nsegs; i++) {
            struct tg_segment seg_a = ring_segment_at(a, i);
            for (int j = 0; j < b->nsegs; j++) {
                struct tg_segment seg_b = ring_segment_at(b, j);
                if (tg_segment_intersects_segment(seg_a, seg_b)) {
                    if (!iter(seg_a, i, seg_b, j, udata)) {
                        return;
                    }
                }
            }
        }
    }
}

/// Iterates over all segments in line A that intersect with segments in line B.
/// @note This efficently uses the indexes of each geometry, if available.
/// @see LineFuncs
void tg_line_line_search(const struct tg_line *a, const struct tg_line *b, 
    bool (*iter)(struct tg_segment aseg, int aidx, struct tg_segment bseg, 
        int bidx, void *udata),
    void *udata)
{
    tg_ring_ring_search((struct tg_ring*)a, (struct tg_ring*)b, iter, udata);
}

/// Iterates over all segments in ring A that intersect with segments in line B.
/// @note This efficently uses the indexes of each geometry, if available.
/// @see RingFuncs
void tg_ring_line_search(const struct tg_ring *a, const struct tg_line *b, 
    bool (*iter)(struct tg_segment aseg, int aidx, struct tg_segment bseg, 
        int bidx, void *udata),
    void *udata)
{
    tg_ring_ring_search(a, (struct tg_ring*)b, iter, udata);
}


__attr_noinline
static void pip_eval_seg_slow(const struct tg_ring *ring, int i, 
    struct tg_point point, bool allow_on_edge, bool *in, int *idx)
{
    struct tg_segment seg = { ring->points[i], ring->points[i+1] };    
    switch (raycast(seg, point)) {
    case TG_OUT:
        break;
    case TG_IN:
        if (*idx == -1) {
            *in = !*in;
        }
        break;
    case TG_ON:
        *in = allow_on_edge;
        *idx = i;
        break;
    }
}

static inline void pip_eval_seg(const struct tg_ring *ring, int i, 
    struct tg_point point, bool allow_on_edge, bool *in, int *idx)
{
    // Performs fail-fast raycast boundary tests first.
    double ymin = fmin0(ring->points[i].y, ring->points[i+1].y);
    double ymax = fmax0(ring->points[i].y, ring->points[i+1].y);
    if (point.y < ymin || point.y > ymax) {
        return;
    }
    double xmin = fmin0(ring->points[i].x, ring->points[i+1].x);
    double xmax = fmax0(ring->points[i].x, ring->points[i+1].x);
    if (point.x < xmin) {
        if (point.y != ymin && point.y != ymax) {
            if (*idx != -1) return;
            *in = !*in;
            return;
        }    
    } else if (point.x > xmax) {
        if (ymin != ymax && xmin != xmax) {
            return;
        }
    }
    pip_eval_seg_slow(ring, i, point, allow_on_edge, in, idx);
}

struct ring_result {
    bool hit; // contains/intersects
    int idx;  // edge index
};

static struct ring_result ystripes_pip(const struct tg_ring *ring, 
    struct tg_point point, bool allow_on_edge)
{
    bool in = false;
    int idx = -1;
    struct ystripes *ystripes = ring->ystripes;
    double height = ring->rect.max.y-ring->rect.min.y;
    int y = (point.y - ring->rect.min.y) / height * (double)ystripes->nstripes;
    y = fclamp0(y, 0, ystripes->nstripes-1);
    struct ystripe *ystripe = &ystripes->stripes[y];
    for (int i = 0; i < ystripe->count; i++) {
        int j = ystripe->indexes[i];
        pip_eval_seg(ring, j, point, allow_on_edge, &in, &idx); 
    }
    return (struct ring_result){ .hit = in, .idx = idx};
}

static struct ring_result standard_pip(const struct tg_ring *ring, 
    struct tg_point point, bool allow_on_edge)
{
    bool in = false;
    int idx = -1;
    int i = 0;
    while (i < ring->nsegs) {
        for16(i, ring->nsegs, {
            double ymin = fmin0(ring->points[i].y, ring->points[i+1].y);
            double ymax = fmax0(ring->points[i].y, ring->points[i+1].y);
            if (!(point.y < ymin || point.y > ymax)) {
                goto do_pip;
            }
        });
        break;
    do_pip:
        pip_eval_seg_slow(ring, i, point, allow_on_edge, &in, &idx);
        i++;
    }
    return (struct ring_result){ .hit = in, .idx = idx};
}

static bool branch_maybe_in(struct ixpoint point, struct ixrect rect) {
    if (point.y < rect.min.y || point.y > rect.max.y) {
        return false;
    }
    if (point.x > rect.max.x) {
        if (rect.min.y != rect.max.y && rect.min.x != rect.max.x) {
            return false;
        }
    }
    return true;
}

static void index_pip_counter(const struct tg_ring *ring, struct tg_point point,
    bool allow_on_edge, int lvl, int start, bool *in, int *idx)
{
    struct index *ix = ring->index;
    int ixspread = ix->spread;
    if (lvl == ix->nlevels) {
        // leaf segments
        int i = start;
        int e = i+ixspread;
        if (e > ring->nsegs) e = ring->nsegs;
        for16(i, e, {
            pip_eval_seg(ring, i, point, allow_on_edge, in, idx);
        });
    } else {
        struct ixpoint ixpoint;
        tg_point_to_ixpoint(&point, &ixpoint);
        // branch rectangles
        const struct level *level = &ix->levels[lvl];
        int i = start;
        int e = i+ixspread;
        if (e > level->nrects) e = level->nrects;
        for16(i, e, {
            if (branch_maybe_in(ixpoint, level->rects[i])) {
                index_pip_counter(ring, point, allow_on_edge, lvl+1, 
                    i*ixspread, in, idx);
            }
        });
    }
}

static struct ring_result index_pip(const struct tg_ring *ring, 
    struct tg_point point, bool allow_on_edge)
{
    bool in = false;
    int idx = -1;
    index_pip_counter(ring, point, allow_on_edge, 0, 0, &in, &idx);
    return (struct ring_result){ .hit = in, .idx = idx};
}

struct ring_result tg_ring_contains_point(const struct tg_ring *ring, 
    struct tg_point point, bool allow_on_edge)
{
    if (!tg_rect_covers_point(ring->rect, point)) {
        return (struct ring_result){ .hit = false, .idx = -1 };
    }
    if (ring->ystripes) {
        return ystripes_pip(ring, point, allow_on_edge);
    }
    if (ring->index) {
        return index_pip(ring, point, allow_on_edge);
    }
    return standard_pip(ring, point, allow_on_edge);
}

/// Returns true if ring is convex. 
/// @param ring Input ring
/// @return True if ring is convex.
/// @return False if ring is concave.
/// @see RingFuncs
bool tg_ring_convex(const struct tg_ring *ring) {
    if (!ring) return false;
    return ring->convex;
}

/// Returns true if winding order is clockwise. 
/// @param ring Input ring
/// @return True if clockwise
/// @return False if counter-clockwise
/// @see RingFuncs
bool tg_ring_clockwise(const struct tg_ring *ring) {
    if (!ring) return false;
    return ring->clockwise;
}

struct contsegiterctx {
    struct tg_segment seg;
    bool intersects;
};

static bool contsegiter1(struct tg_segment seg2, int index, void *udata) {
    (void)index;
    struct contsegiterctx *ctx = udata;
    if (tg_segment_intersects_segment(ctx->seg, seg2)) {
        if (tg_raycast(seg2, ctx->seg.a) != TG_ON && 
            tg_raycast(seg2, ctx->seg.b) != TG_ON)
        {
            ctx->intersects = true;
            return false;
        }
    }
    return true;
}

static bool contsegiter4(struct tg_segment seg2, int index, void *udata) {
    (void)index;
    struct contsegiterctx *ctx = udata;
    if (tg_segment_intersects_segment(ctx->seg, seg2)) {
        if (tg_raycast(seg2, ctx->seg.a) != TG_ON) {
            ctx->intersects = true;
            return false;
        }
    }
    return true;
}

static bool contsegiter2(struct tg_segment seg2, int index, void *udata) {
    (void)index;
    struct contsegiterctx *ctx = udata;
    if (tg_segment_intersects_segment(ctx->seg, seg2)) {
        if (tg_raycast(seg2, ctx->seg.b) != TG_ON) {
            ctx->intersects = true;
            return false;
        }
    }
    return true;
}
static bool contsegiter5(struct tg_segment seg2, int index, void *udata) {
    (void)index;
    struct contsegiterctx *ctx = udata;
    if (tg_segment_intersects_segment(ctx->seg, seg2)) {
        if (tg_raycast(ctx->seg, seg2.a) != TG_ON && 
            tg_raycast(ctx->seg, seg2.b) != TG_ON)
        {
            ctx->intersects = true;
            return false;
        }
    }
    return true;
}

static bool contsegiter3(struct tg_segment seg2, int index, void *udata) {
    (void)index;
    struct contsegiterctx *ctx = udata;
    if (tg_segment_intersects_segment(ctx->seg, seg2)) {
            ctx->intersects = true;
            return false;
    }
    return true;
}

bool tg_ring_contains_segment(const struct tg_ring *ring, 
    struct tg_segment seg, bool allow_on_edge)
{
    if (!tg_rect_covers_rect(ring->rect, tg_segment_rect(seg))) {
        return false;
    }
    // Test that segment points are contained in the ring.
    struct ring_result res_a = tg_ring_contains_point(ring, seg.a, 
        allow_on_edge);
    if (!res_a.hit) {
        // seg A is not inside ring
        return false;
    }
    if (pteq(seg.b, seg.a)) {
        return true;
    }
    struct ring_result res_b = tg_ring_contains_point(ring, seg.b,
        allow_on_edge);
    if (!res_b.hit) {
        // seg B is not inside ring
        return false;
    }
    if (ring->convex) {
        // ring is convex so the segment must be contained
        return true;
    }

    // The ring is concave so it's possible that the segment crosses over the
    // edge of the ring.
    if (allow_on_edge) {
        // do some logic around seg points that are on the edge of the ring.
        if (res_a.idx != -1) {
           // seg A is on a ring segment
            if (res_b.idx != -1) {
                // seg B is on a ring segment
                if (res_b.idx == res_a.idx) {
                    // seg A and B share the same ring segment, so it must be
                    // on the inside.
                    return true;
                }

                // seg A and seg B are on different segments.
                // determine if the space that the seg passes over is inside or
                // outside of the ring. To do so we create a ring from the two
                // ring segments and check if that ring winding order matches
                // the winding order of the ring.
                // -- create a ring

                struct tg_segment r_seg_a = ring_segment_at(ring, res_a.idx);
                struct tg_segment r_seg_b = ring_segment_at(ring, res_b.idx);
                if (pteq(r_seg_a.a, seg.a) || pteq(r_seg_a.b, seg.a) ||
                    pteq(r_seg_b.a, seg.a) || pteq(r_seg_b.b, seg.a) ||
                    pteq(r_seg_a.a, seg.b) || pteq(r_seg_a.b, seg.b) ||
                    pteq(r_seg_b.a, seg.b) || pteq(r_seg_b.b, seg.b))
                {
                    return true;
                }

                // fix the order of the
                if (res_b.idx < res_a.idx) {
                    struct tg_segment tmp = r_seg_a;
                    r_seg_a = r_seg_b;
                    r_seg_b = tmp;
                }

                struct tg_point pts[] = {
                    r_seg_a.a, r_seg_a.b, r_seg_b.a, r_seg_b.b, r_seg_a.a
                };
                // -- calc winding order
                double cwc = 0.0;
                for (int i = 0; i < 4; i++) {
                    struct tg_point a = pts[i];
                    struct tg_point b = pts[i+1];
                    cwc += (b.x - a.x) * (b.y + a.y);
                }
                bool clockwise = cwc > 0;
                if (clockwise != ring->clockwise) {
                    // -- on the outside
                    return false;
                }
                // the passover space is on the inside of the ring.
                // check if seg intersects any ring segments where A and B are
                // not on.
                struct contsegiterctx ctx = {
                    .intersects = false,
                    .seg = seg,
                };
                tg_ring_search(ring, tg_segment_rect(seg), contsegiter1, &ctx);
                return !ctx.intersects;
            }
            // case (4)
            // seg A is on a ring segment, but seg B is not.
            // check if seg intersects any ring segments where A is not on.
            struct contsegiterctx ctx = {
                .intersects = false,
                .seg = seg,
            };
            tg_ring_search(ring, tg_segment_rect(seg), contsegiter4, &ctx);
            return !ctx.intersects;
        } else if (res_b.idx != -1) {
            // case (2)
            // seg B is on a ring segment, but seg A is not.
            // check if seg intersects any ring segments where B is not on.
            struct contsegiterctx ctx = {
                .intersects = false,
                .seg = seg,
            };
            tg_ring_search(ring, tg_segment_rect(seg), contsegiter2, &ctx);
            return !ctx.intersects;
        }
        // case (5) (15)
        struct contsegiterctx ctx = {
            .intersects = false,
            .seg = seg,
        };
        tg_ring_search(ring, tg_segment_rect(seg), contsegiter5, &ctx);
        return !ctx.intersects;
    }
    // allow_on_edge is false. (not allow on edge)
    struct contsegiterctx ctx = {
        .intersects = false,
        .seg = seg,
    };
    tg_ring_search(ring, tg_segment_rect(seg), contsegiter3, &ctx);
    return !ctx.intersects;
}

struct intersegiterctx {
    struct tg_segment seg;
    int count;
    bool allow_on_edge;
    bool seg_a_on;
    bool seg_b_on;
    // bool yes;
};

static bool intersegiter(struct tg_segment seg, int index, void *udata) {
    (void)index;
    struct intersegiterctx *ctx = udata;

    if (!tg_segment_intersects_segment(ctx->seg, seg)) {
        return true;
    }
    if (ctx->allow_on_edge) {
        ctx->count++;
        return ctx->count < 2;
    }

    struct tg_point a = ctx->seg.a;
    struct tg_point b = ctx->seg.b;
    struct tg_point c = seg.a;
    struct tg_point d = seg.b;

    // bool acol = collinear(c.x, c.y, d.x, d.y, a.x, a.y);
    // bool bcol = collinear(c.x, c.y, d.x, d.y, b.x, b.y);
    bool ccol = collinear(a.x, a.y, b.x, b.y, c.x, c.y);
    bool dcol = collinear(a.x, a.y, b.x, b.y, d.x, d.y);

    if (ccol && dcol) {
        // lines are parallel.
        ctx->count = 0;
    } else if (!ccol || !dcol) {
        if (!ctx->seg_a_on) {
            if (pteq(a, c) || pteq(a, d)) {
                ctx->seg_a_on = true;
                return true;
            }
        }
        if (!ctx->seg_b_on) {
            if (pteq(b, c) || pteq(b, d)) {
                ctx->seg_b_on = true;
                return true;
            }
        }
        ctx->count++;
    }
    return ctx->count < 2;
}

bool tg_ring_intersects_segment(const struct tg_ring *ring, 
    struct tg_segment seg, bool allow_on_edge)
{
    if (!tg_rect_intersects_rect(tg_segment_rect(seg), ring->rect)) {
        return false;
    }
    // Quick check that either point is inside of the ring
    if (tg_ring_contains_point(ring, seg.a, allow_on_edge).hit ||
        tg_ring_contains_point(ring, seg.b, allow_on_edge).hit)
    {
        return true;
    }

    // Neither point A or B is inside of the ring. It's possible that both
    // are on the outside and are passing over segments. If the segment passes
    // over at least two ring segments then it's intersecting.
    struct intersegiterctx ctx = { 
        .seg = seg,
        .allow_on_edge = allow_on_edge,
    };
    tg_ring_search(ring, tg_segment_rect(seg), intersegiter, &ctx);
    return ctx.count >= 2;
}

// tg_ring_empty returns true when the ring is NULL, or does not form a closed
// ring, ie. it cannot be used as a valid spatial geometry.
bool tg_ring_empty(const struct tg_ring *ring) {
    if (!ring) return true;
    return (ring->closed && ring->npoints < 3) || ring->npoints < 2;
}

bool tg_ring_contains_ring(const struct tg_ring *a, const struct tg_ring *b,
    bool allow_on_edge)
{
    if (tg_ring_empty(a) || tg_ring_empty(b)) {
        return false;
    }
    // test if the inner rect does not contain the outer rect
    if (!tg_rect_covers_rect(a->rect, b->rect)) {
        // not fully contained so it's not possible for the outer ring to
        // contain the inner ring
        return false;
    }
    if (a->convex) {
        // outer ring is convex so test that all inner points are inside of
        // the outer ring
        for (int i = 0; i < b->npoints; i++) {
            if (!tg_ring_contains_point(a, b->points[i], allow_on_edge).hit) {
                // point is on the outside the outer ring
                return false;
            }
        }
    } else {
        // outer ring is concave so let's make sure that all inner segments are
        // fully contained inside of the outer ring.
        for (int i = 0; i < b->nsegs; i++) {
            struct tg_segment seg = ring_segment_at(b, i);
            if (!tg_ring_contains_segment(a, seg, allow_on_edge)) {
                return false;
            }
        }
    }
    return true;
}

struct tg_rect tg_rect_move(struct tg_rect rect, 
    double delta_x, double delta_y)
{
    rect.min = tg_point_move(rect.min, delta_x, delta_y);
    rect.max = tg_point_move(rect.max, delta_x, delta_y);
    return rect;
}

static double rect_area(struct tg_rect rect) {
    return (rect.max.x - rect.min.x) * (rect.max.y - rect.min.y);
}

bool tg_ring_intersects_ring(const struct tg_ring *ring,
    const struct tg_ring *other, bool allow_on_edge)
{
    if (tg_ring_empty(ring) || tg_ring_empty(other)) {
        return false;
    }
    // check outer and innter rects intersection first
    if (!tg_rect_intersects_rect(ring->rect, other->rect)) {
        return false;
    }
    double a1 = rect_area(tg_ring_rect(ring));
    double a2 = rect_area(tg_ring_rect(other));
    if (a2 > a1) {
        // swap the rings so that the inner ring is smaller than the outer ring
        const struct tg_ring *tmp = ring;
        ring = other;
        other = tmp;
    }
    for (int i = 0; i < other->nsegs; i++) {
        if (tg_ring_intersects_segment(ring, ring_segment_at(other, i), 
            allow_on_edge)) 
        {
            return true;
        }
    }
    return false;
}

bool tg_ring_contains_line(const struct tg_ring *a, const struct tg_line *b, 
    bool allow_on_edge, bool respect_boundaries)
{
    // Almost the same logic as tg_ring_contains_line except for boundaries
    // detection for the input line. 
    if (tg_ring_empty(a) || tg_line_empty(b)) {
        return false;
    }
    // test if the inner rect does not contain the outer rect
    if (!tg_rect_covers_rect(a->rect, tg_line_rect(b))) {
        // not fully contained so it's not possible for the outer ring to
        // contain the inner ring
        return false;
    }

    // if (a->convex && !respect_boundaries) {
    //     // outer ring is convex so test that all inner points are inside of
    //     // the outer ring
    //     int npoints = tg_line_num_points(b);
    //     const struct tg_point *points = tg_line_points(b);
    //     for (int i = 0; i < npoints; i++) {
    //         if (!tg_ring_contains_point(a, points[i], allow_on_edge).hit) {
    //             // point is on the outside the outer ring
    //             return false;
    //         }
    //     }
    // } else 
    
    if (!allow_on_edge && respect_boundaries) {
        // outer ring is concave so let's make sure that all inner segments are
        // fully contained inside of the outer ring.
        int nsegs = tg_line_num_segments(b);
        for (int i = 0; i < nsegs; i++) {
            struct tg_segment seg = line_segment_at(b, i);
            if (!tg_ring_contains_segment(a, seg, true)) {
                return false;
            }
            if (!tg_ring_intersects_segment(a, seg, false)) {
                return false;
            }
        }
    } else {
        // outer ring is concave so let's make sure that all inner segments are
        // fully contained inside of the outer ring.
        int nsegs = tg_line_num_segments(b);
        for (int i = 0; i < nsegs; i++) {
            struct tg_segment seg = line_segment_at(b, i);
            if (!tg_ring_contains_segment(a, seg, allow_on_edge)) {
                return false;
            }
        }
    }
    return true;

}

/// Tests whether a ring intersects a line.
/// @see RingFuncs
bool tg_ring_intersects_line(const struct tg_ring *ring, 
    const struct tg_line *line, bool allow_on_edge)
{
    if (tg_ring_empty(ring) || tg_line_empty(line)) {
        return false;
    }
    // check outer and innter rects intersection first
    if (!tg_rect_intersects_rect(tg_ring_rect(ring), tg_line_rect(line))) {
        return false;
    }
    // check if any points are inside ring
    // TODO: use line index if available.
    int nsegs = tg_line_num_segments(line);
    for (int i = 0; i < nsegs; i++) {
        if (tg_ring_intersects_segment(ring, tg_line_segment_at(line, i), 
            allow_on_edge))
        {
            return true;
        }
    }
    return false;
}

/// Tests whether a rectangle intersects a line.
/// @see RectFuncs
bool tg_rect_intersects_line(struct tg_rect rect, const struct tg_line *line) {
    struct tg_ring *ring = stack_ring();
    rect_to_ring(rect, ring);
    return tg_ring_intersects_line(ring, line, true);
}

/// Tests whether a rectangle intersects a polygon.
/// @see RectFuncs
bool tg_rect_intersects_poly(struct tg_rect rect, const struct tg_poly *poly) {
    return tg_poly_intersects_rect(poly, rect);
}

/// Tests whether a rectangle fully contains a polygon.
/// @see RectFuncs
bool tg_rect_covers_poly(struct tg_rect rect, const struct tg_poly *poly) {
    return !tg_poly_empty(poly) && 
        tg_rect_covers_rect(rect, tg_poly_rect(poly));
}

struct tg_ring *tg_ring_move(const struct tg_ring *ring,
    double delta_x, double delta_y)
{
    return series_move(ring, true, delta_x, delta_y);
}

/// Returns the underlying point array of a ring.
/// @param ring Input ring
/// @return Array or points
/// @see tg_ring_num_points()
/// @see RingFuncs
const struct tg_point *tg_ring_points(const struct tg_ring *ring) {
    if (!ring) return NULL;
    return ring->points;
}

////////////////////
// line
////////////////////

/// Creates a line from a series of points.
/// @param points Array of points
/// @param npoints Number of points in array
/// @return A newly allocated line
/// @return NULL if out of memory
/// @note A tg_line can be safely upcasted to a tg_geom. `(struct tg_geom*)line`
/// @note All lines with 32 or more points are automatically indexed.
/// @see LineFuncs
struct tg_line *tg_line_new(const struct tg_point *points, int npoints) {
    return tg_line_new_ix(points, npoints, TG_DEFAULT);
}

/// Creates a line from a series of points using provided index option.
/// @param points Array of points
/// @param npoints Number of points in array
/// @param ix Indexing option, e.g. TG_NONE, TG_NATURAL, TG_YSTRIPES
/// @return A newly allocated line
/// @return NULL if out of memory
/// @note A tg_line can be safely upcasted to a tg_geom. `(struct tg_geom*)poly`
/// @see [tg_index](.#tg_index)
/// @see LineFuncs
struct tg_line *tg_line_new_ix(const struct tg_point *points, int npoints,
    enum tg_index ix)
{
    return (struct tg_line*)series_new(points, npoints, false, ix);
}

/// Releases the memory associated with a line.
/// @param line Input line
/// @see LineFuncs
void tg_line_free(struct tg_line *line) {
    struct tg_ring *ring = (struct tg_ring *)line;
    tg_ring_free(ring);
}

/// Returns the minimum bounding rectangle of a line.
/// @see LineFuncs
struct tg_rect tg_line_rect(const struct tg_line *line) {
    struct tg_ring *ring = (struct tg_ring *)line;
    return tg_ring_rect(ring);
}

/// Returns the number of points.
/// @param line Input line
/// @return Number of points
/// @see tg_line_point_at()
/// @see LineFuncs
int tg_line_num_points(const struct tg_line *line) {
    struct tg_ring *ring = (struct tg_ring *)line;
    return tg_ring_num_points(ring);
}

/// Returns the point at index.
/// @param line Input line
/// @param index Index of point
/// @return The point at index
/// @note This function performs bounds checking. Use tg_line_points() for
/// direct access to the points.
/// @see tg_line_num_points()
/// @see LineFuncs
struct tg_point tg_line_point_at(const struct tg_line *line, int index) {
    struct tg_ring *ring = (struct tg_ring *)line;
    return tg_ring_point_at(ring, index);
}

/// Returns the number of segments.
/// @param line Input line
/// @return Number of segments
/// @see tg_line_segment_at()
/// @see LineFuncs
int tg_line_num_segments(const struct tg_line *line) {
    struct tg_ring *ring = (struct tg_ring *)line;
    return tg_ring_num_segments(ring);
}

/// Returns the segment at index.
/// @param line Input line
/// @param index Index of segment
/// @return The segment at index
/// @see tg_line_num_segments()
/// @see LineFuncs
struct tg_segment tg_line_segment_at(const struct tg_line *line, int index) {
    struct tg_ring *ring = (struct tg_ring *)line;
    return tg_ring_segment_at(ring, index);
}

// tg_line_empty returns true when the line is NULL or has less than two
// points, ie. it cannot be used as a valid spatial geometry.
bool tg_line_empty(const struct tg_line *line) {
    struct tg_ring *ring = (struct tg_ring *)line;
    return tg_ring_empty(ring);
}

void tg_line_search(const struct tg_line *line, struct tg_rect rect, 
    bool(*iter)(struct tg_segment seg, int index, void *udata),
    void *udata)
{
    struct tg_ring *ring = (struct tg_ring *)line;
    tg_ring_search(ring, rect, iter, udata);
}

struct line_covers_point_iter_ctx {
    bool covers;
    struct tg_point point;
};

static bool line_covers_point_iter(struct tg_segment seg, 
    int index, void *udata)
{
    (void)index;
    struct line_covers_point_iter_ctx *ctx = udata;
    if (tg_segment_covers_point(seg, ctx->point)) {
        ctx->covers = true;
        return false;
    }
    return true;
}

/// Tests whether a line fully contains a point
/// @see LineFuncs
bool tg_line_covers_point(const struct tg_line *line, struct tg_point point) {
    struct line_covers_point_iter_ctx ctx = { 
        .point = point,
    };
    tg_line_search(line, (struct tg_rect){ point, point }, 
        line_covers_point_iter, &ctx);
    return ctx.covers;
}

bool tg_line_contains_point(const struct tg_line *line, struct tg_point point) {
    if (!tg_line_covers_point(line, point)) {
        return false;
    }
    int nsegs = tg_line_num_segments(line);
    if (pteq(point, tg_line_segment_at(line, 0).a) || 
        pteq(point, tg_line_segment_at(line, nsegs-1).b))
    {
        return false;
    }
    return true;
}

/// Tests whether a line intersects a point
/// @see LineFuncs
bool tg_line_intersects_point(const struct tg_line *line, 
    struct tg_point point)
{
    return tg_line_covers_point(line, point);
}

bool tg_line_touches_point(const struct tg_line *line, struct tg_point point) {
    return tg_point_touches_line(point, line);
}

/// Tests whether a line fully contains a rectangle
/// @see LineFuncs
bool tg_line_covers_rect(const struct tg_line *line, struct tg_rect rect) {
    // Convert rect into a poly
    struct tg_ring *exterior = stack_ring();
    rect_to_ring(rect, exterior);
    struct tg_poly poly = { .exterior = exterior };
    return tg_line_covers_poly(line, &poly);
}

/// Tests whether a line intersects a rectangle
/// @see LineFuncs
bool tg_line_intersects_rect(const struct tg_line *line, struct tg_rect rect) {
    return tg_rect_intersects_line(rect, line);
}

/// Tests whether a line contains another line
/// @see LineFuncs
bool tg_line_covers_line(const struct tg_line *a, const struct tg_line *b) {
    if (tg_line_empty(a) || tg_line_empty(b)) return false;

    if (!tg_rect_covers_rect(tg_line_rect(a), tg_line_rect(b))) {
        return false;
    }
    // locate the first "other" segment that contains the first "line" segment.
    int ansegs = tg_line_num_segments(a);
    int j = -1;
    for (int k = 0; k < ansegs; k++) {
        if (tg_segment_covers_segment(tg_line_segment_at(a, k),
            tg_line_segment_at(b, 0)))
        {
            j = k;
            break;
        }
    }
    if (j == -1) {
        return false;
    }
    int bnsegs = tg_line_num_segments(b);
    for (int i = 1; i < bnsegs && j < ansegs; i++) {
        struct tg_segment aseg = tg_line_segment_at(a, j);
        struct tg_segment bseg = tg_line_segment_at(b, i);
        if (tg_segment_covers_segment(aseg, bseg)) {
            continue;
        }
        if (pteq(bseg.a, aseg.a)) {
            // reverse it
            if (j == 0) {
                return false;
            }
            j--;
            i--;
        } else if (pteq(bseg.a, aseg.b)) {
            // forward it
            j++;
            i--;
        }
    }
    return true;
}

bool tg_line_contains_line(const struct tg_line *line, 
    const struct tg_line *other)
{
    return tg_line_covers_line(line, other);
}

enum segment_intersects_kind {
    SI_INTERSECTS, 
    SI_TOUCHES,
};

struct segment_intersects_iter_ctx {
    bool yes;
    int ansegs;
    int bnsegs;
    enum segment_intersects_kind kind;
};

static bool segment_touches0(struct tg_segment seg,
    struct tg_point a, struct tg_point b)
{
    if (!tg_segment_covers_point(seg, a)) {
        return false;
    }
    if (!collinear(seg.a.x, seg.a.y, seg.b.x, seg.b.y, b.x, b.y)) {
        return true;
    }
    if (pteq(seg.a, a)) {
        return !tg_segment_covers_point((struct tg_segment){ a, b }, seg.b);
    }
    if (pteq(seg.b, a)) {
        return !tg_segment_covers_point((struct tg_segment){ a, b }, seg.a);
    }
    return false;
}

static bool any_touching(struct tg_segment a, int aidx, int ansegs,
    struct tg_segment b, int bidx, int bnsegs)
{
    return (aidx == 0 && segment_touches0(b, a.a, a.b)) ||
           (aidx == ansegs-1 && segment_touches0(b, a.b, a.a)) ||
           (bidx == 0 && segment_touches0(a, b.a, b.b)) ||
           (bidx == bnsegs-1 && segment_touches0(a, b.b, b.a));
}

static bool segment_intersects_iter(struct tg_segment a, int aidx, 
    struct tg_segment b, int bidx, void *udata)
{
    struct segment_intersects_iter_ctx *ctx = udata;
    switch (ctx->kind) {
    case SI_INTERSECTS:
        ctx->yes = true;
        break;
    case SI_TOUCHES:
        if (any_touching(a, aidx, ctx->ansegs, b, bidx, ctx->bnsegs)) {
            ctx->yes = true;
            return true;
        }
        ctx->yes = false;
        break;
    }
    return false;
}

static bool line_intersects_line(const struct tg_line *a, 
    const struct tg_line *b, enum segment_intersects_kind kind)
{
    struct segment_intersects_iter_ctx ctx = { 
        .kind = kind,
        .ansegs = tg_line_num_segments(a),
        .bnsegs = tg_line_num_segments(b),
     };
    tg_line_line_search(a, b, segment_intersects_iter, &ctx);
    return ctx.yes;
}

/// Tests whether a line intersects another line
/// @see LineFuncs
bool tg_line_intersects_line(const struct tg_line *a, const struct tg_line *b) {
    return line_intersects_line(a, b, SI_INTERSECTS);
}

bool tg_line_touches_line(const struct tg_line *a,const struct tg_line *b) {    
    return line_intersects_line(a, b, SI_TOUCHES);
}

/// Tests whether a line fully contains a polygon
/// @see LineFuncs
bool tg_line_covers_poly(const struct tg_line *line,
    const struct tg_poly *poly)
{
    if (tg_line_empty(line) || tg_poly_empty(poly)) return false;
    struct tg_rect rect = tg_poly_rect(poly);
    if (rect.min.x != rect.max.x && rect.min.y != rect.max.y) return false;
    
    // polygon can fit in a straight (vertial or horizontal) line
    struct tg_segment seg = { rect.min, rect.max };
    struct tg_ring *other = stack_ring();
    segment_to_ring(seg, other);
    rect_to_ring(rect, other);
    return tg_line_covers_line(line, (struct tg_line*)(other));
}

bool tg_line_contains_poly(const struct tg_line *line,
    const struct tg_poly *poly)
{
    // not possible
    (void)line; (void)poly;
    return false;
}

/// Tests whether a line intersects a polygon
/// @see LineFuncs
bool tg_line_intersects_poly(const struct tg_line *line,
    const struct tg_poly *poly)
{
    return tg_poly_intersects_line(poly, line);
}

bool tg_line_touches_poly(const struct tg_line *a, const struct tg_poly *b) {
    return tg_poly_touches_line(b, a);
}

struct tg_line *tg_line_move(const struct tg_line *line,
    double delta_x, double delta_y)
{
    const struct tg_ring *ring = (const struct tg_ring *)line;
    return (struct tg_line*)series_move(ring, false, delta_x, delta_y);
}

/// Returns true if winding order is clockwise. 
/// @param line Input line
/// @return True if clockwise
/// @return False if counter-clockwise
/// @see LineFuncs
bool tg_line_clockwise(const struct tg_line *line) {
    const struct tg_ring *ring = (const struct tg_ring *)line;
    return tg_ring_clockwise(ring);
}

/// Clones a line
/// @param line Input line, caller retains ownership.
/// @return A duplicate of the provided line. 
/// @note The caller is responsible for freeing with tg_line_free().
/// @note This method of cloning uses implicit sharing through an atomic 
/// reference counter.
/// @see LineFuncs
struct tg_line *tg_line_clone(const struct tg_line *line) {
    const struct tg_ring *ring = (const struct tg_ring *)line;
    return (struct tg_line*)tg_ring_clone(ring);
}

/// Returns the underlying point array of a line.
/// @param line Input line
/// @return Array or points
/// @see tg_line_num_points()
/// @see LineFuncs
const struct tg_point *tg_line_points(const struct tg_line *line) {
    const struct tg_ring *ring = (const struct tg_ring *)line;
    return tg_ring_points(ring);
}

/// Returns the allocation size of the line. 
/// @param line Input line
/// @return Size of line in bytes
/// @see LineFuncs
size_t tg_line_memsize(const struct tg_line *line) {
    const struct tg_ring *ring = (const struct tg_ring *)line;
    return tg_ring_memsize(ring);
}

/// Returns the indexing spread for a line.
/// 
/// The "spread" is the number of segments or rectangles that are grouped 
/// together to produce a unioned rectangle that is stored at a higher level.
/// 
/// For a tree based structure, this would be the number of items per node.
///
/// @param line Input line
/// @return The spread, default is 16
/// @return Zero if line has no indexing
/// @see tg_line_index_num_levels()
/// @see tg_line_index_level_num_rects()
/// @see tg_line_index_level_rect()
/// @see LineFuncs
int tg_line_index_spread(const struct tg_line *line) {
    const struct tg_ring *ring = (const struct tg_ring *)line;
    return tg_ring_index_spread(ring);
}

/// Returns the number of levels.
/// @param line Input line
/// @return The number of levels
/// @return Zero if line has no indexing
/// @see tg_line_index_spread()
/// @see tg_line_index_level_num_rects()
/// @see tg_line_index_level_rect()
/// @see LineFuncs
int tg_line_index_num_levels(const struct tg_line *line) {
    const struct tg_ring *ring = (const struct tg_ring *)line;
    return tg_ring_index_num_levels(ring);
}

/// Returns the number of rectangles at level.
/// @param line Input line
/// @param levelidx The index of level
/// @return The number of index levels
/// @return Zero if line has no indexing or levelidx is out of bounds.
/// @see tg_line_index_spread()
/// @see tg_line_index_num_levels()
/// @see tg_line_index_level_rect()
/// @see LineFuncs
int tg_line_index_level_num_rects(const struct tg_line *line, int levelidx) {
    const struct tg_ring *ring = (const struct tg_ring *)line;
    return tg_ring_index_level_num_rects(ring, levelidx);
}

/// Returns a specific level rectangle.
/// @param line Input line
/// @param levelidx The index of level
/// @param rectidx The index of rectangle
/// @return The rectangle
/// @return Empty rectangle if line has no indexing, or levelidx or rectidx
/// is out of bounds.
/// @see tg_line_index_spread()
/// @see tg_line_index_num_levels()
/// @see tg_line_index_level_num_rects()
/// @see LineFuncs
struct tg_rect tg_line_index_level_rect(const struct tg_line *line, 
    int levelidx, int rectidx)
{
    const struct tg_ring *ring = (const struct tg_ring *)line;
    return tg_ring_index_level_rect(ring, levelidx, rectidx);
}

////////////////////
// poly
////////////////////

/// Creates a polygon.
/// @param exterior Exterior ring
/// @param holes Array of interior rings that are holes
/// @param nholes Number of holes in array
/// @return A newly allocated polygon
/// @return NULL if out of memory
/// @return NULL if exterior or any holes are NULL
/// @note A tg_poly can be safely upcasted to a tg_geom. `(struct tg_geom*)poly`
/// @see PolyFuncs
struct tg_poly *tg_poly_new(const struct tg_ring *exterior, 
    const struct tg_ring *const holes[], int nholes) 
{
    if (!exterior) {
        return NULL;
    }
    if (nholes == 0) {
        // When the user creates a new poly without holes then we can avoid
        // the extra allocations by upcasting the base tg_ring to a tg_poly.
        return (struct tg_poly *)tg_ring_clone(exterior);
    }
    struct tg_poly *poly = tg_malloc(sizeof(struct tg_poly));
    if (!poly) {
        goto fail;
    }
    memset(poly, 0, sizeof(struct tg_poly));
    poly->head.base = BASE_POLY;
    poly->head.type = TG_POLYGON;
    poly->exterior = tg_ring_clone(exterior);
    if (nholes > 0) {
        poly->holes = tg_malloc(sizeof(struct tg_ring*)*nholes);
        if (!poly->holes) {
            goto fail;
        }
        poly->nholes = nholes;
        memset(poly->holes, 0, sizeof(struct tg_ring*)*poly->nholes);
        for (int i = 0; i < poly->nholes; i++) {
            poly->holes[i] = tg_ring_clone(holes[i]);
        }
    }
    return poly;
fail:
    tg_poly_free(poly);
    return NULL;
}

/// Releases the memory associated with a polygon.
/// @param poly Input polygon
/// @see PolyFuncs
void tg_poly_free(struct tg_poly *poly) {
    if (!poly) return;
    if (poly->head.base == BASE_RING) {
        tg_ring_free((struct tg_ring*)poly);
        return;
    }
    if (rc_sub(&poly->head.rc) > 0) return;
    if (poly->exterior) tg_ring_free(poly->exterior);
    if (poly->holes) {
        for (int i = 0; i < poly->nholes; i++) {
            if (poly->holes[i]) tg_ring_free(poly->holes[i]);
        }
        tg_free(poly->holes);
    }
    tg_free(poly);
}

/// Clones a polygon.
/// @param poly Input polygon, caller retains ownership.
/// @return A duplicate of the provided polygon. 
/// @note The caller is responsible for freeing with tg_poly_free().
/// @note This method of cloning uses implicit sharing through an atomic 
/// reference counter.
/// @see PolyFuncs
struct tg_poly *tg_poly_clone(const struct tg_poly *poly) {
    if (!poly) return NULL;
    struct tg_poly *poly_mut = (struct tg_poly*)poly;
    rc_add(&poly_mut->head.rc);
    return poly_mut;
}


/// Returns the exterior ring.
/// @param poly Input polygon
/// @return Exterior ring
/// @note The polygon maintains ownership of the exterior ring.
/// @see PolyFuncs
const struct tg_ring *tg_poly_exterior(const struct tg_poly *poly) {
    if (!poly) return NULL;
    if (poly->head.base == BASE_RING) {
        return (struct tg_ring*)poly;
    }
    return poly->exterior;
}

/// Returns the number of interior holes.
/// @param poly Input polygon
/// @return Number of holes
/// @see tg_poly_hole_at()
/// @see PolyFuncs
int tg_poly_num_holes(const struct tg_poly *poly) {
    if (!poly || poly->head.base == BASE_RING) return 0;
    return poly->nholes;
}

/// Returns an interior hole.
/// @param poly Input polygon
/// @param index Index of hole
/// @return Ring hole
/// @see tg_poly_num_holes()
/// @see PolyFuncs
const struct tg_ring *tg_poly_hole_at(const struct tg_poly *poly, int index) {
    if (!poly || poly->head.base == BASE_RING) return NULL;
    if (index < 0 || index >= poly->nholes) return NULL;
    return poly->holes[index];
}

/// Returns true if winding order is clockwise.
/// @param poly Input polygon
/// @return True if clockwise
/// @return False if counter-clockwise
/// @see PolyFuncs
bool tg_poly_clockwise(const struct tg_poly *poly) {
    return tg_ring_clockwise(tg_poly_exterior(poly));
}

/// Returns true if polygon is empty.
/// @see PolyFuncs
bool tg_poly_empty(const struct tg_poly *poly) {
    return tg_ring_empty(tg_poly_exterior(poly));
}

/// Returns the minimum bounding rectangle of a polygon.
/// @see PolyFuncs
struct tg_rect tg_poly_rect(const struct tg_poly *poly) {
    return tg_ring_rect(tg_poly_exterior(poly));
}

static bool poly_contains_point(const struct tg_poly *poly, 
    struct tg_point point, bool allow_on_edge)
{
    if (poly && poly->head.base == BASE_RING) {
        // downcast fast path
        return tg_ring_contains_point((struct tg_ring*)poly, point, 
            allow_on_edge).hit;
    }
    // standard path
    if (tg_poly_empty(poly)) {
        return false;
    }
    if (!tg_ring_contains_point(poly->exterior, point, allow_on_edge).hit) {
        return false;
    }
    bool covers = true;
    for (int i = 0; i < poly->nholes; i++) {
        if (tg_ring_contains_point(poly->holes[i], point, !allow_on_edge).hit) {
            covers = false;
            break;
        }
    }
    return covers;
}

/// Tests whether a polygon fully contains a point.
/// @see PolyFuncs
bool tg_poly_covers_point(const struct tg_poly *poly, struct tg_point point) {
    return poly_contains_point(poly, point, true);
}

bool tg_poly_contains_point(const struct tg_poly *poly, struct tg_point point) {
    return poly_contains_point(poly, point, false);
}

/// Tests whether a polygon fully contains a point using xy coordinates.
/// @see PolyFuncs
bool tg_poly_covers_xy(const struct tg_poly *poly, double x, double y) {
    return tg_poly_covers_point(poly, (struct tg_point){ .x = x, .y = y });
}

/// Tests whether a polygon intersects a point.
/// @see PolyFuncs
bool tg_poly_intersects_point(const struct tg_poly *poly, struct tg_point point)
{
    return tg_poly_covers_point(poly, point);
}

bool tg_poly_touches_point(const struct tg_poly *poly, struct tg_point point) {
    return tg_point_touches_poly(point, poly);
}


/// Tests whether a polygon fully contains a rectangle.
/// @see PolyFuncs
bool tg_poly_covers_rect(const struct tg_poly *poly, struct tg_rect rect) {
    // Convert rect into a poly
    struct tg_ring *other = stack_ring();
    rect_to_ring(rect, other);
    return tg_poly_covers_poly(poly, (struct tg_poly*)other);
}

/// Tests whether a polygon intersects a rectangle.
/// @see PolyFuncs
bool tg_poly_intersects_rect(const struct tg_poly *poly, struct tg_rect rect) {
    // convert rect into a poly
    struct tg_ring *other = stack_ring();
    rect_to_ring(rect, other);
    return tg_poly_intersects_poly(poly, (struct tg_poly*)other);
}

/// Tests whether a polygon covers (fully contains) a line.
/// @see PolyFuncs
bool tg_poly_covers_line(const struct tg_poly *a, const struct tg_line *b) {
    if (tg_poly_empty(a) || tg_line_empty(b)) {
        return false;
    }
    if (!tg_ring_contains_line(tg_poly_exterior(a), b, true, false)) {
        return false;
    }
    int nholes = tg_poly_num_holes(a);
    for (int i = 0; i < nholes; i++) {
        if (tg_ring_intersects_line(tg_poly_hole_at(a, i), b, false)) {
            return false;
        }
    }
    return true;
}

bool tg_poly_contains_line(const struct tg_poly *a, const struct tg_line *b) {
    if (tg_poly_empty(a) || tg_line_empty(b)) {
        return false;
    }
    if (!tg_ring_contains_line(tg_poly_exterior(a), b, false, true)) {
        return false;
    }
    int nholes = tg_poly_num_holes(a);
    for (int i = 0; i < nholes; i++) {
        if (tg_ring_intersects_line(tg_poly_hole_at(a, i), b, false)) {
            return false;
        }
    }
    return true;
}

/// Tests whether a polygon intersects a line.
/// @see PolyFuncs
bool tg_poly_intersects_line(const struct tg_poly *poly,
    const struct tg_line *line)
{
    if (poly && poly->head.base == BASE_RING) {
        // downcast fast path
        return tg_ring_intersects_line((struct tg_ring*)poly, line, true);
    }
    // standard path
    if (tg_poly_empty(poly) || tg_line_empty(line)) {
        return false;
    }
    if (!tg_ring_intersects_line(poly->exterior, line, true)) {
        return false;
    }
    for (int i = 0; i < poly->nholes; i++) {
        if (tg_ring_contains_line(poly->holes[i], line, false, false)) {
            return false;
        }
    }
    return true;
}

/// Tests whether a polygon fully contains another polygon.
/// @see PolyFuncs
bool tg_poly_covers_poly(const struct tg_poly *a, const struct tg_poly *b) {
    if (a && a->head.base == BASE_RING && 
        b && b->head.base == BASE_RING)
    {
        // downcast fast path
        return tg_ring_contains_ring((struct tg_ring*)a, (struct tg_ring*)b,
            true);
    }
    // standard path
    if (tg_poly_empty(a) || tg_poly_empty(b)) {
        return false;
    }

    const struct tg_ring *a_exterior = tg_poly_exterior(a);
    const struct tg_ring *b_exterior = tg_poly_exterior(b);
    int a_nholes = tg_poly_num_holes(a);
    int b_nholes = tg_poly_num_holes(b);
    struct tg_ring **a_holes = NULL;
    if (a->head.base == BASE_POLY) {
        a_holes = a->holes;
    }
    struct tg_ring **b_holes = NULL;
    if (b->head.base == BASE_POLY) {
        b_holes = b->holes;
    }
    
    // 1) other exterior must be fully contained inside of the poly exterior.
    if (!tg_ring_contains_ring(a_exterior, b_exterior, true)) {
        return false;
    }
    // 2) ring cannot intersect poly holes
    bool covers = true;
    for (int i = 0; i < a_nholes; i++) {
        if (tg_ring_intersects_ring(a_holes[i], b_exterior, false)) {
            covers = false;
            // 3) unless the poly hole is contain inside of a other hole
            for (int j = 0; j < b_nholes; j++) {
                if (tg_ring_contains_ring(b_holes[j], a_holes[i], 
                    true))
                {
                    covers = true;
                    break;
                }
            }
            if (!covers) {
                break;
            }
        }
    }
    return covers;
}

bool tg_poly_contains_poly(const struct tg_poly *a, const struct tg_poly *b) {
    return tg_poly_covers_poly(a, b);
}

/// Tests whether a polygon intesects a polygon.
/// @see PolyFuncs
bool tg_poly_intersects_poly(const struct tg_poly *poly, 
    const struct tg_poly *other)
{
    if (poly && poly->head.base == BASE_RING && 
        other && other->head.base == BASE_RING)
    {
        // downcast fast path
        return tg_ring_intersects_ring((struct tg_ring*)poly,
            (struct tg_ring*)other, true);
    }
    // standard path
    if (tg_poly_empty(poly) || tg_poly_empty(other)) return false;

    const struct tg_ring *poly_exterior = tg_poly_exterior(poly);
    const struct tg_ring *other_exterior = tg_poly_exterior(other);
    int poly_nholes = tg_poly_num_holes(poly);
    int other_nholes = tg_poly_num_holes(other);
    struct tg_ring **poly_holes = NULL;
    if (poly->head.base == BASE_POLY) poly_holes = poly->holes;
    struct tg_ring **other_holes = NULL;
    if (other->head.base == BASE_POLY) other_holes = other->holes;

    if (!tg_ring_intersects_ring(other_exterior, poly_exterior, true)) {
        return false;
    }
    for (int i = 0; i < poly_nholes; i++) {
        if (tg_ring_contains_ring(poly_holes[i], other_exterior, false)) {
            return false;
        }
    }
    for (int i = 0; i < other_nholes; i++) {
        if (tg_ring_contains_ring(other_holes[i], poly_exterior, false)) {
            return false;
        }
    }
    return true;
}

bool tg_poly_touches_line(const struct tg_poly *a, const struct tg_line *b) {
    if (!tg_rect_intersects_rect(tg_poly_rect(a), tg_line_rect(b))) {
        return false;
    }

    // Check if the line is inside any of the polygon holes
    int npoints = tg_line_num_points(b);
    int nholes = tg_poly_num_holes(a);
    for (int i = 0; i < nholes; i++) {
        const struct tg_ring *hole = tg_poly_hole_at(a, i);
        if (tg_ring_contains_line(hole, b, true, false)) {
            // Yes, now check if any of the points touch the hole boundary.
            for (int j = 0; j < npoints; j++) {
                struct tg_point point = tg_line_point_at(b, j);
                if (tg_line_covers_point((struct tg_line*)hole, point)) {
                    return true;
                }
            }
            return false;
        }
    }

    // Check if at least one line point touches the polygon exterior.
    const struct tg_ring *ring = tg_poly_exterior(a);
    bool touches = false;
    for (int i = 0; i < npoints; i++) {
        struct tg_point point = tg_line_point_at(b, i);
        // Cast the exterior ring to a polygon to avoid holes.
        if (tg_poly_touches_point((struct tg_poly*)ring, point)) {
            touches = true;
            break;
        }
    }
    if (!touches) {
        return false;
    }
    int nsegs = tg_line_num_segments(b);
    for (int i = 0; i < nsegs; i++) {
        struct tg_segment seg = tg_line_segment_at(b, i);
        if (tg_ring_intersects_segment(ring, seg, false)) {
            return false;
        }
    }
    return true;
}

bool tg_poly_touches_poly(const struct tg_poly *a, const struct tg_poly *b) {
    if (!tg_rect_intersects_rect(tg_poly_rect(a), tg_poly_rect(b))) {
        return false;
    }
    
    const struct tg_ring *aext = tg_poly_exterior(a);
    const struct tg_ring *bext = tg_poly_exterior(b);

    // Check if one polygon is fully inside a hole of the other and touching
    // the hole boundary.
    for (int ii = 0; ii < 2; ii++) {
        const struct tg_poly *poly = ii == 0 ? a : b; 
        const struct tg_ring *ring = ii == 0 ? bext : aext; 
        int nholes = tg_poly_num_holes(poly);
        for (int i = 0; i < nholes; i++) {
            const struct tg_ring *hole = tg_poly_hole_at(poly, i);
            if (tg_ring_contains_ring(hole, ring, true)) {
                // Yes, now check if any exterior points are on the other 
                // hole boundary.
                int npoints = tg_ring_num_points(ring);
                for (int j = 0; j < npoints; j++) {
                    struct tg_point point = tg_ring_point_at(ring, j);
                    if (tg_line_covers_point((struct tg_line*)hole, point)) {
                        // Touching
                        return true;
                    }
                }
                // Not touching and full enclosed in a hole.
                return false;
            }
        }
    }
    
    // Now we can work with the exterior rings only.
    // Check if one polygon is touching the other
    int ansegs = tg_ring_num_segments(aext);
    int bnsegs = tg_ring_num_segments(bext);
    int atouches = 0;
    int btouches = 0;
    for (int ii = 0; ii < 2; ii++) {
        const struct tg_ring *a = ii == 0 ? aext : bext; 
        const struct tg_ring *b = ii == 0 ? bext : aext; 
        int nsegs = ii == 0 ? bnsegs : ansegs;
        int touches = 0;
        for (int i = 0; i < nsegs; i++) {
            struct tg_segment seg = tg_ring_segment_at(b, i);
            bool isects0 = tg_ring_intersects_segment(a, seg, true);
            bool isects1 = tg_ring_intersects_segment(a, seg, false);
            if (isects0 && !isects1) {
                touches++;
            } else if (isects0 || isects1) {
                return false;
            }
        }
        if (ii == 0) {
            btouches = touches;
        } else {
            atouches = touches;
        }
    }
    if (atouches > 0 || btouches > 0) {
        return !(atouches == ansegs && btouches == bnsegs);
    }
    return false;
}

struct tg_poly *tg_poly_move(const struct tg_poly *poly, double delta_x, 
    double delta_y)
{
    if (!poly) return NULL;
    if (poly->head.base == BASE_RING) {
        return (struct tg_poly*)tg_ring_move((struct tg_ring*)poly, delta_x, 
            delta_y);
    }

    struct tg_poly *final = NULL;
    struct tg_ring *exterior = NULL;
    struct tg_ring **holes = NULL;

    if (poly->exterior) {
        exterior = tg_ring_move(poly->exterior, delta_x, delta_y);
        if (!exterior) goto done;
    }
    if (poly->nholes > 0) {
        holes = tg_malloc(sizeof(struct tg_ring*)*poly->nholes);
        if (!holes) goto done;
        memset(holes, 0, sizeof(struct tg_ring*)*poly->nholes);
        for (int i = 0; i < poly->nholes; i++) {
            holes[i] = tg_ring_move(poly->holes[i], delta_x, delta_y);
            if (!holes[i]) goto done;
        }
    }
    final = tg_poly_new(exterior, (const struct tg_ring**)holes, poly->nholes);
done:
    if (exterior) tg_ring_free(exterior);
    if (holes) {
        for (int i = 0; i < poly->nholes; i++) {
            if (holes[i]) tg_ring_free(holes[i]);
        }
        tg_free(holes);
    }
    return final;
}

/// Returns the allocation size of the polygon. 
/// @param poly Input polygon
/// @return Size of polygon in bytes
/// @see PolyFuncs
size_t tg_poly_memsize(const struct tg_poly *poly) {
    if (!poly) return 0;
    if (poly->head.base == BASE_RING) {
        return tg_ring_memsize((struct tg_ring*)poly);
    }
    size_t size = sizeof(struct tg_poly);
    if (poly->exterior) {
        size += tg_ring_memsize(poly->exterior);
    }
    size += poly->nholes*sizeof(struct tg_ring);
    for (int i = 0; i < poly->nholes; i++) {
        size += tg_ring_memsize(poly->holes[i]);
    }
    return size;
}

////////////////////
// geom
////////////////////

static struct tg_geom *geom_new(enum tg_geom_type type) {
    struct tg_geom *geom = tg_malloc(sizeof(struct tg_geom));
    if (!geom) return NULL;
    memset(geom, 0, sizeof(struct tg_geom));
    geom->head.base = BASE_GEOM;
    geom->head.type = type;
    return geom;
}

static struct tg_geom *geom_new_empty(enum tg_geom_type type) {
    struct tg_geom *geom = geom_new(type);
    if (!geom) return NULL;
    geom->head.flags = IS_EMPTY;
    return geom;
}

/// Creates a Point geometry.
/// @param point Input point
/// @return A newly allocated geometry.
/// @return NULL if system is out of memory. 
/// @note The caller is responsible for freeing with tg_geom_free().
/// @see GeometryConstructors
struct tg_geom *tg_geom_new_point(struct tg_point point) {
    struct boxed_point *geom = tg_malloc(sizeof(struct boxed_point));
    if (!geom) return NULL;
    memset(geom, 0, sizeof(struct boxed_point));
    geom->head.base = BASE_POINT;
    geom->head.type = TG_POINT;
    geom->point = point;
    return (struct tg_geom*)geom;
}

static void boxed_point_free(struct boxed_point *point) {
    if (rc_sub(&point->head.rc) > 0) return;
    tg_free(point);
}

/// Creates a Point geometry that includes a Z coordinate.
/// @param point Input point
/// @param z The Z coordinate
/// @return A newly allocated geometry, or NULL if system is out of 
/// memory. The caller is responsible for freeing with tg_geom_free().
/// @see GeometryConstructorsEx
struct tg_geom *tg_geom_new_point_z(struct tg_point point, double z) {
    struct tg_geom *geom = geom_new(TG_POINT);
    if (!geom) return NULL;
    geom->head.flags = HAS_Z;
    geom->point = point;
    geom->z = z;
    return geom;
}

/// Creates a Point geometry that includes an M coordinate.
/// @param point Input point
/// @param m The M coordinate
/// @return A newly allocated geometry, or NULL if system is out of 
/// memory. The caller is responsible for freeing with tg_geom_free().
/// @see GeometryConstructorsEx
struct tg_geom *tg_geom_new_point_m(struct tg_point point, double m) {
    struct tg_geom *geom = geom_new(TG_POINT);
    if (!geom) return NULL;
    geom->head.flags = HAS_M;
    geom->point = point;
    geom->m = m;
    return geom;
}

/// Creates a Point geometry that includes a Z and M coordinates.
/// @param point Input point
/// @param z The Z coordinate
/// @param m The M coordinate
/// @return A newly allocated geometry, or NULL if system is out of 
/// memory. The caller is responsible for freeing with tg_geom_free().
/// @see GeometryConstructorsEx
struct tg_geom *tg_geom_new_point_zm(struct tg_point point, double z, double m)
{
    struct tg_geom *geom = geom_new(TG_POINT);
    if (!geom) return NULL;
    geom->head.flags = HAS_Z | HAS_M;
    geom->point = point;
    geom->z = z;
    geom->m = m;
    return geom;
}

/// Creates an empty Point geometry.
/// @return A newly allocated geometry, or NULL if system is out of 
/// memory. The caller is responsible for freeing with tg_geom_free().
/// @see GeometryConstructorsEx
struct tg_geom *tg_geom_new_point_empty(void) {
    return geom_new_empty(TG_POINT);
}

/// Creates a LineString geometry.
/// @param line Input line, caller retains ownership.
/// @return A newly allocated geometry.
/// @return NULL if system is out of memory. 
/// @note The caller is responsible for freeing with tg_geom_free().
/// @see GeometryConstructors
struct tg_geom *tg_geom_new_linestring(const struct tg_line *line) {
    return (struct tg_geom*)tg_line_clone(line);
}

/// Creates an empty LineString geometry.
/// @return A newly allocated geometry.
/// @return NULL if system is out of memory. 
/// @note The caller is responsible for freeing with tg_geom_free().
/// @see GeometryConstructorsEx
struct tg_geom *tg_geom_new_linestring_empty(void) {
    return geom_new_empty(TG_LINESTRING);
}

/// Creates a Polygon geometry.
/// @param poly Input polygon, caller retains ownership.
/// @return A newly allocated geometry.
/// @return NULL if system is out of memory. 
/// @note The caller is responsible for freeing with tg_geom_free().
/// @see GeometryConstructors
struct tg_geom *tg_geom_new_polygon(const struct tg_poly *poly) {
    return (struct tg_geom*)tg_poly_clone(poly);
}

/// Creates an empty Polygon geometry.
/// @return A newly allocated geometry.
/// @return NULL if system is out of memory. 
/// @note The caller is responsible for freeing with tg_geom_free().
/// @see GeometryConstructorsEx
struct tg_geom *tg_geom_new_polygon_empty(void) {
    return geom_new_empty(TG_POLYGON);
}

static struct tg_geom *geom_new_multi(enum tg_geom_type type, int ngeoms) {
    ngeoms = ngeoms < 0 ? 0 : ngeoms;
    struct tg_geom *geom = geom_new(type);
    if (!geom) return NULL;
    geom->multi = tg_malloc(sizeof(struct multi));
    if (!geom->multi) {
        tg_free(geom);
        return NULL;
    }
    memset(geom->multi, 0, sizeof(struct multi));
    geom->multi->geoms = tg_malloc(ngeoms*sizeof(struct tg_geom*));
    if (!geom->multi->geoms) {
        tg_free(geom->multi);
        tg_free(geom);
        return NULL;
    }
    memset(geom->multi->geoms, 0, ngeoms*sizeof(struct tg_geom*));
    geom->multi->ngeoms = ngeoms;
    return geom;
}

static void multi_geom_inflate_rect(struct tg_geom *geom) {
    if (geom->multi->ngeoms == 0) {
        geom->multi->rect = (struct tg_rect){ 0 };
        return;
    }
    geom->multi->rect = tg_geom_rect(geom->multi->geoms[0]);
    for (int i = 1; i < geom->multi->ngeoms; i++) {
        geom->multi->rect = tg_rect_expand(geom->multi->rect, 
            tg_geom_rect(geom->multi->geoms[i]));
    }
}

/// Creates a MultiPoint geometry.
/// @param points An array of points, caller retains ownership.
/// @param npoints The number of points in array
/// @return A newly allocated geometry.
/// @return NULL if system is out of memory. 
/// @note The caller is responsible for freeing with tg_geom_free().
/// @see GeometryConstructors
struct tg_geom *tg_geom_new_multipoint(const struct tg_point *points,
    int npoints)
{
    struct tg_geom *geom = geom_new_multi(TG_MULTIPOINT, npoints);
    if (!geom) return NULL;
    for (int i = 0; i < geom->multi->ngeoms; i++) {
        geom->multi->geoms[i] = tg_geom_new_point(points[i]);
        if (!geom->multi->geoms[i]) {
            for (int j = 0; j < i; j++) {
                tg_geom_free(geom->multi->geoms[j]);
            }
            tg_free(geom->multi->geoms);
            tg_free(geom->multi);
            tg_free(geom);
            return NULL;
        }
    }
    multi_geom_inflate_rect(geom);
    return geom;
}

/// Creates an empty MultiPoint geometry.
/// @return A newly allocated geometry.
/// @return NULL if system is out of memory. 
/// @note The caller is responsible for freeing with tg_geom_free().
/// @see GeometryConstructorsEx
struct tg_geom *tg_geom_new_multipoint_empty(void) {
    return geom_new_empty(TG_MULTIPOINT);
}

/// Creates a MultiLineString geometry.
/// @param lines An array of lines, caller retains ownership.
/// @param nlines The number of lines in array
/// @return A newly allocated geometry.
/// @return NULL if system is out of memory. 
/// @note The caller is responsible for freeing with tg_geom_free().
/// @see GeometryConstructors
struct tg_geom *tg_geom_new_multilinestring(const struct tg_line *const lines[],
    int nlines)
{
    struct tg_geom *geom = geom_new_multi(TG_MULTILINESTRING, nlines);
    if (!geom) return NULL;
    for (int i = 0; i < geom->multi->ngeoms; i++) {
        geom->multi->geoms[i] = (struct tg_geom*)tg_line_clone(lines[i]);
    }
    multi_geom_inflate_rect(geom);
    return geom;
}

/// Creates an empty MultiLineString geometry.
/// @return A newly allocated geometry.
/// @return NULL if system is out of memory. 
/// @note The caller is responsible for freeing with tg_geom_free().
/// @see GeometryConstructorsEx
struct tg_geom *tg_geom_new_multilinestring_empty(void) {
    return geom_new_empty(TG_MULTILINESTRING);
}

/// Creates a MultiPolygon geometry.
/// @param polys An array of polygons, caller retains ownership.
/// @param npolys The number of polygons in array
/// @return A newly allocated geometry.
/// @return NULL if system is out of memory. 
/// @note The caller is responsible for freeing with tg_geom_free().
/// @see GeometryConstructors
struct tg_geom *tg_geom_new_multipolygon(const struct tg_poly *const polys[], 
    int npolys)
{
    struct tg_geom *geom = geom_new_multi(TG_MULTIPOLYGON, npolys);
    if (!geom) return NULL;
    for (int i = 0; i < geom->multi->ngeoms; i++) {
        geom->multi->geoms[i] = (struct tg_geom*)tg_poly_clone(polys[i]);
    }
    multi_geom_inflate_rect(geom);
    return geom;
}

/// Creates an empty MultiPolygon geometry.
/// @return A newly allocated geometry.
/// @return NULL if system is out of memory. 
/// @note The caller is responsible for freeing with tg_geom_free().
/// @see GeometryConstructorsEx
struct tg_geom *tg_geom_new_multipolygon_empty(void) {
    return geom_new_empty(TG_MULTIPOLYGON);
}

/// Creates a GeometryCollection geometry.
/// @param geoms An array of geometries, caller retains ownership.
/// @param ngeoms The number of geometries in array
/// @return A newly allocated geometry.
/// @return NULL if system is out of memory. 
/// @note The caller is responsible for freeing with tg_geom_free().
/// @see GeometryConstructors
struct tg_geom *tg_geom_new_geometrycollection(
    const struct tg_geom *const geoms[], int ngeoms)
{
    struct tg_geom *geom = geom_new_multi(TG_GEOMETRYCOLLECTION, ngeoms);
    if (!geom) return NULL;
    for (int i = 0; i < geom->multi->ngeoms; i++) {
        geom->multi->geoms[i] = tg_geom_clone(geoms[i]);
    }
    multi_geom_inflate_rect(geom);
    return geom;
}

/// Creates an empty GeometryCollection geometry.
/// @return A newly allocated geometry.
/// @return NULL if system is out of memory. 
/// @note The caller is responsible for freeing with tg_geom_free().
/// @see GeometryConstructorsEx
struct tg_geom *tg_geom_new_geometrycollection_empty(void) {
    return geom_new_empty(TG_GEOMETRYCOLLECTION);
}

static struct tg_geom *fill_extra_coords(struct tg_geom *geom,
    const double *coords, int ncoords, enum flags flags)
{
    ncoords = ncoords < 0 ? 0 : ncoords;
    // if (!geom) return NULL; // already checked
    geom->head.flags = flags;
    geom->ncoords = ncoords;
    if (ncoords == 0) {
        geom->coords = NULL;
    } else {
        geom->coords = tg_malloc(ncoords*sizeof(double));
        if (!geom->coords) {
            tg_geom_free(geom);
            return NULL;
        }
        memcpy(geom->coords, coords, ncoords*sizeof(double));
    }
    return geom;
}

/// Creates a LineString geometry that includes Z coordinates.
/// @param line Input line, caller retains ownership.
/// @param coords Array of doubles representing each Z coordinate, caller
/// retains ownership.
/// @param ncoords Number of doubles in array.
/// @return A newly allocated geometry.
/// @return NULL if system is out of memory. 
/// @note The caller is responsible for freeing with tg_geom_free().
/// @see GeometryConstructorsEx
struct tg_geom *tg_geom_new_linestring_z(const struct tg_line *line, 
    const double *coords, int ncoords)
{
    struct tg_geom *geom = geom_new(TG_LINESTRING);
    if (!geom) return NULL;
    geom->line = tg_line_clone(line);
    return fill_extra_coords(geom, coords, ncoords, HAS_Z);
}

/// Creates a LineString geometry that includes M coordinates.
/// @param line Input line, caller retains ownership.
/// @param coords Array of doubles representing each M coordinate, caller
/// retains ownership.
/// @param ncoords Number of doubles in array.
/// @return A newly allocated geometry.
/// @return NULL if system is out of memory. 
/// @note The caller is responsible for freeing with tg_geom_free().
/// @see GeometryConstructorsEx
struct tg_geom *tg_geom_new_linestring_m(const struct tg_line *line, 
    const double *coords, int ncoords)
{
    struct tg_geom *geom = geom_new(TG_LINESTRING);
    if (!geom) return NULL;
    geom->line = tg_line_clone(line);
    return fill_extra_coords(geom, coords, ncoords, HAS_M);
}

/// Creates a LineString geometry that includes ZM coordinates.
/// @param line Input line, caller retains ownership.
/// @param coords Array of doubles representing each Z and M coordinate, 
/// interleaved. Caller retains ownership.
/// @param ncoords Number of doubles in array.
/// @return A newly allocated geometry.
/// @return NULL if system is out of memory. 
/// @note The caller is responsible for freeing with tg_geom_free().
/// @see GeometryConstructorsEx
struct tg_geom *tg_geom_new_linestring_zm(const struct tg_line *line, 
    const double *coords, int ncoords)
{
    struct tg_geom *geom = geom_new(TG_LINESTRING);
    if (!geom) return NULL;
    geom->line = tg_line_clone(line);
    return fill_extra_coords(geom, coords, ncoords, HAS_Z|HAS_M);
}

/// Creates a Polygon geometry that includes Z coordinates.
/// @param poly Input polygon, caller retains ownership.
/// @param coords Array of doubles representing each Z coordinate, caller
/// retains ownership.
/// @param ncoords Number of doubles in array.
/// @return A newly allocated geometry.
/// @return NULL if system is out of memory. 
/// @note The caller is responsible for freeing with tg_geom_free().
/// @see GeometryConstructorsEx
struct tg_geom *tg_geom_new_polygon_z(const struct tg_poly *poly, 
    const double *coords, int ncoords)
{
    struct tg_geom *geom = geom_new(TG_POLYGON);
    if (!geom) return NULL;
    geom->poly = tg_poly_clone(poly);
    return fill_extra_coords(geom, coords, ncoords, HAS_Z);
}

/// Creates a Polygon geometry that includes M coordinates.
/// @param poly Input polygon, caller retains ownership.
/// @param coords Array of doubles representing each M coordinate, caller
/// retains ownership.
/// @param ncoords Number of doubles in array.
/// @return A newly allocated geometry.
/// @return NULL if system is out of memory. 
/// @note The caller is responsible for freeing with tg_geom_free().
/// @see GeometryConstructorsEx
struct tg_geom *tg_geom_new_polygon_m(const struct tg_poly *poly, 
    const double *coords, int ncoords)
{
    struct tg_geom *geom = geom_new(TG_POLYGON);
    if (!geom) return NULL;
    geom->poly = tg_poly_clone(poly);
    return fill_extra_coords(geom, coords, ncoords, HAS_M);
}

/// Creates a Polygon geometry that includes ZM coordinates.
/// @param poly Input polygon, caller retains ownership.
/// @param coords Array of doubles representing each Z and M coordinate, 
/// interleaved. Caller retains ownership.
/// @param ncoords Number of doubles in array.
/// @return A newly allocated geometry.
/// @return NULL if system is out of memory. 
/// @note The caller is responsible for freeing with tg_geom_free().
/// @see GeometryConstructorsEx
struct tg_geom *tg_geom_new_polygon_zm(const struct tg_poly *poly, 
    const double *coords, int ncoords)
{
    struct tg_geom *geom = geom_new(TG_POLYGON);
    if (!geom) return NULL;
    geom->poly = tg_poly_clone(poly);
    return fill_extra_coords(geom, coords, ncoords, HAS_Z|HAS_M);
}

/// Creates a MultiPoint geometry that includes Z coordinates.
/// @param points An array of points, caller retains ownership.
/// @param npoints The number of points in array
/// @param coords Array of doubles representing each Z coordinate, caller
/// retains ownership.
/// @param ncoords Number of doubles in array.
/// @return A newly allocated geometry.
/// @return NULL if system is out of memory. 
/// @note The caller is responsible for freeing with tg_geom_free().
/// @see GeometryConstructorsEx
struct tg_geom *tg_geom_new_multipoint_z(const struct tg_point *points, 
    int npoints, const double *coords, int ncoords)
{
    struct tg_geom *geom = tg_geom_new_multipoint(points, npoints);
    if (!geom) return NULL;
    return fill_extra_coords(geom, coords, ncoords, HAS_Z);
}

/// Creates a MultiPoint geometry that includes M coordinates.
/// @param points An array of points, caller retains ownership.
/// @param npoints The number of points in array
/// @param coords Array of doubles representing each M coordinate, caller
/// retains ownership.
/// @param ncoords Number of doubles in array.
/// @return A newly allocated geometry.
/// @return NULL if system is out of memory. 
/// @note The caller is responsible for freeing with tg_geom_free().
/// @see GeometryConstructorsEx
struct tg_geom *tg_geom_new_multipoint_m(const struct tg_point *points, 
    int npoints, const double *coords, int ncoords)
{
    struct tg_geom *geom = tg_geom_new_multipoint(points, npoints);
    if (!geom) return NULL;
    return fill_extra_coords(geom, coords, ncoords, HAS_M);
}

/// Creates a MultiPoint geometry that includes ZM coordinates.
/// @param points An array of points, caller retains ownership.
/// @param npoints The number of points in array
/// @param coords Array of doubles representing each Z and M coordinate, 
/// interleaved. Caller retains ownership.
/// @param ncoords Number of doubles in array.
/// @return A newly allocated geometry.
/// @return NULL if system is out of memory. 
/// @note The caller is responsible for freeing with tg_geom_free().
/// @see GeometryConstructorsEx
struct tg_geom *tg_geom_new_multipoint_zm(const struct tg_point *points, 
    int npoints, const double *coords, int ncoords)
{
    struct tg_geom *geom = tg_geom_new_multipoint(points, npoints);
    if (!geom) return NULL;
    return fill_extra_coords(geom, coords, ncoords, HAS_Z|HAS_M);
}

/// Creates a MultiLineString geometry that includes Z coordinates.
/// @param lines An array of lines, caller retains ownership.
/// @param nlines The number of lines in array
/// @param coords Array of doubles representing each Z coordinate, caller
/// retains ownership.
/// @param ncoords Number of doubles in array.
/// @return A newly allocated geometry.
/// @return NULL if system is out of memory. 
/// @note The caller is responsible for freeing with tg_geom_free().
/// @see GeometryConstructorsEx
struct tg_geom *tg_geom_new_multilinestring_z(
    const struct tg_line *const lines[], int nlines, const double *coords,
    int ncoords)
{
    struct tg_geom *geom = tg_geom_new_multilinestring(lines, nlines);
    if (!geom) return NULL;
    return fill_extra_coords(geom, coords, ncoords, HAS_Z);
}

/// Creates a MultiLineString geometry that includes M coordinates.
/// @param lines An array of lines, caller retains ownership.
/// @param nlines The number of lines in array
/// @param coords Array of doubles representing each M coordinate, caller
/// retains ownership.
/// @param ncoords Number of doubles in array.
/// @return A newly allocated geometry.
/// @return NULL if system is out of memory. 
/// @note The caller is responsible for freeing with tg_geom_free().
/// @see GeometryConstructorsEx
struct tg_geom *tg_geom_new_multilinestring_m(
    const struct tg_line *const lines[], int nlines,
    const double *coords, int ncoords)
{
    struct tg_geom *geom = tg_geom_new_multilinestring(lines, nlines);
    if (!geom) return NULL;
    return fill_extra_coords(geom, coords, ncoords, HAS_M);
}

/// Creates a MultiLineString geometry that includes ZM coordinates.
/// @param lines An array of lines, caller retains ownership.
/// @param nlines The number of lines in array
/// @param coords Array of doubles representing each Z and M coordinate, 
/// interleaved. Caller retains ownership.
/// @param ncoords Number of doubles in array.
/// @return A newly allocated geometry.
/// @return NULL if system is out of memory. 
/// @note The caller is responsible for freeing with tg_geom_free().
/// @see GeometryConstructorsEx
struct tg_geom *tg_geom_new_multilinestring_zm(
    const struct tg_line *const lines[], int nlines, 
    const double *coords, int ncoords)
{
    struct tg_geom *geom = tg_geom_new_multilinestring(lines, nlines);
    if (!geom) return NULL;
    return fill_extra_coords(geom, coords, ncoords, HAS_Z|HAS_M);
}

/// Creates a MultiPolygon geometry that includes Z coordinates.
/// @param polys An array of polygons, caller retains ownership.
/// @param npolys The number of polygons in array
/// @param coords Array of doubles representing each Z coordinate, caller
/// retains ownership.
/// @param ncoords Number of doubles in array.
/// @return A newly allocated geometry.
/// @return NULL if system is out of memory. 
/// @note The caller is responsible for freeing with tg_geom_free().
/// @see GeometryConstructorsEx
struct tg_geom *tg_geom_new_multipolygon_z(
    const struct tg_poly *const polys[], int npolys,
    const double *coords, int ncoords)
{
    struct tg_geom *geom = tg_geom_new_multipolygon(polys, npolys);
    if (!geom) return NULL;
    return fill_extra_coords(geom, coords, ncoords, HAS_Z);
}

/// Creates a MultiPolygon geometry that includes M coordinates.
/// @param polys An array of polygons, caller retains ownership.
/// @param npolys The number of polygons in array
/// @param coords Array of doubles representing each M coordinate, caller
/// retains ownership.
/// @param ncoords Number of doubles in array.
/// @return A newly allocated geometry.
/// @return NULL if system is out of memory. 
/// @note The caller is responsible for freeing with tg_geom_free().
/// @see GeometryConstructorsEx
struct tg_geom *tg_geom_new_multipolygon_m(
    const struct tg_poly *const polys[], int npolys,
    const double *coords, int ncoords)
{
    struct tg_geom *geom = tg_geom_new_multipolygon(polys, npolys);
    if (!geom) return NULL;
    return fill_extra_coords(geom, coords, ncoords, HAS_M);
}

/// Creates a MultiPolygon geometry that includes ZM coordinates.
/// @param polys An array of polygons, caller retains ownership.
/// @param npolys The number of polygons in array
/// @param coords Array of doubles representing each Z and M coordinate, 
/// interleaved. Caller retains ownership.
/// @param ncoords Number of doubles in array.
/// @return A newly allocated geometry.
/// @return NULL if system is out of memory. 
/// @note The caller is responsible for freeing with tg_geom_free().
/// @see GeometryConstructorsEx
struct tg_geom *tg_geom_new_multipolygon_zm(
    const struct tg_poly *const polys[], int npolys,
    const double *coords, int ncoords)
{
    struct tg_geom *geom = tg_geom_new_multipolygon(polys, npolys);
    if (!geom) return NULL;
    return fill_extra_coords(geom, coords, ncoords, HAS_Z|HAS_M);
}

/// Clones a geometry
/// @param geom Input geometry, caller retains ownership.
/// @return A duplicate of the provided geometry. 
/// @note The caller is responsible for freeing with tg_geom_free().
/// @note This method of cloning uses implicit sharing through an atomic 
/// reference counter.
/// @see GeometryConstructors
struct tg_geom *tg_geom_clone(const struct tg_geom *geom) {
    if (!geom) return NULL;
    struct tg_geom *geom_mut = (struct tg_geom*)geom;
    rc_add(&geom_mut->head.rc);
    return geom_mut;
}

static void geom_free(struct tg_geom *geom) {
    if (rc_sub(&geom->head.rc) > 0) return;
    switch (geom->head.type) {
    case TG_POINT:
        break;
    case TG_LINESTRING:
        tg_line_free(geom->line);
        break;
    case TG_POLYGON:
        tg_poly_free(geom->poly);
        break;
    case TG_MULTIPOINT:
    case TG_MULTILINESTRING:
    case TG_MULTIPOLYGON:
    case TG_GEOMETRYCOLLECTION:
        if (geom->multi) {
            if (geom->multi->geoms) {
                for (int i = 0; i < geom->multi->ngeoms; i++) {
                    tg_geom_free(geom->multi->geoms[i]);
                }
                tg_free(geom->multi->geoms);
            }
            tg_free(geom->multi);
        }
        break;
    }
    if (geom->head.type != TG_POINT && geom->coords) {
        tg_free(geom->coords);
    }
    if (geom->error) {
        // error and xjson share the same memory, so this copy covers both.
        tg_free(geom->error);
    }
    tg_free(geom);
}


/// Releases the memory associated with a geometry.
/// @param geom Input geometry
/// @see GeometryConstructors
void tg_geom_free(struct tg_geom *geom) {
    if (!geom) {
        return;
    }
    switch (geom->head.base) {
    case BASE_GEOM:
        geom_free(geom);
        break;
    case BASE_POINT: 
        boxed_point_free((struct boxed_point*)geom);
        break;
    case BASE_LINE:
        tg_line_free((struct tg_line*)geom);
        break;
    case BASE_RING:
        tg_ring_free((struct tg_ring*)geom);
        break;
    case BASE_POLY:
        tg_poly_free((struct tg_poly*)geom);
        break;
    }
}

/// Returns the geometry type. e.g. TG_POINT, TG_POLYGON, TG_LINESTRING
/// @param geom Input geometry
/// @return The geometry type
/// @see [tg_geom_type](.#tg_geom_type)
/// @see tg_geom_type_string()
/// @see GeometryAccessors
enum tg_geom_type tg_geom_typeof(const struct tg_geom *geom) {
    if (!geom) return 0;
    return geom->head.type;
}

/// Returns true if the geometry is a GeoJSON Feature.
/// @param geom Input geometry
/// @return True or false
/// @see GeometryAccessors
bool tg_geom_is_feature(const struct tg_geom *geom) {
    return geom && (geom->head.flags&IS_FEATURE) == IS_FEATURE;
}

/// Returns true if the geometry is a GeoJSON FeatureCollection.
/// @param geom Input geometry
/// @return True or false
/// @see GeometryAccessors
bool tg_geom_is_featurecollection(const struct tg_geom *geom) {
    return geom && (geom->head.flags&IS_FEATURE_COL) == IS_FEATURE_COL;
}

static struct tg_rect geom_rect(const struct tg_geom *geom) {
    struct tg_rect rect = { 0 };
    switch (geom->head.type) {
    case TG_POINT:
        return tg_point_rect(geom->point);
    case TG_LINESTRING: 
        return tg_line_rect(geom->line);
    case TG_POLYGON:
        return tg_poly_rect(geom->poly);
    case TG_MULTIPOINT:
    case TG_MULTILINESTRING:
    case TG_MULTIPOLYGON:
    case TG_GEOMETRYCOLLECTION:
        if (geom->multi) {
            rect = geom->multi->rect;
        }
    }
    return rect;
}

/// Returns the minimum bounding rectangle of a geometry.
/// @param geom Input geometry
/// @return Minumum bounding rectangle
/// @see tg_rect
/// @see GeometryAccessors
struct tg_rect tg_geom_rect(const struct tg_geom *geom) {
    if (geom) {
        switch (geom->head.base) {
        case BASE_GEOM:
            return geom_rect(geom);
        case BASE_POINT: 
            return tg_point_rect(((struct boxed_point*)geom)->point);
        case BASE_LINE:
            return tg_line_rect((struct tg_line*)geom);
        case BASE_RING:
            return tg_ring_rect((struct tg_ring*)geom);
        case BASE_POLY:
            return tg_poly_rect((struct tg_poly*)geom);
        }
    }
    return (struct tg_rect) { 0 };
}

/// Returns the underlying point for the provided geometry.
/// @param geom Input geometry
/// @return For a TG_POINT geometry, returns the point.
/// @return For everything else returns the center of the geometry's bounding
/// rectangle.
/// @see tg_point
/// @see GeometryAccessors
struct tg_point tg_geom_point(const struct tg_geom *geom) {
    if (!geom) return (struct tg_point) { 0 };
    if (geom->head.base == BASE_POINT) {
        return ((struct boxed_point*)geom)->point;
    }
    if (geom->head.base == BASE_GEOM && geom->head.type == TG_POINT) {
        return geom->point;
    }
    struct tg_rect rect = tg_geom_rect(geom);
    return (struct tg_point) { 
        (rect.min.x + rect.max.x) / 2, 
        (rect.min.y + rect.max.y) / 2,
    };
}

static size_t geom_memsize(const struct tg_geom *geom) {
    size_t size = sizeof(struct tg_geom);
    switch (geom->head.type) {
    case TG_POINT:
        break;
    case TG_LINESTRING:
        size += tg_line_memsize(geom->line);
        break;
    case TG_POLYGON: 
        size += tg_poly_memsize(geom->poly);
        break;
    case TG_MULTIPOINT: 
    case TG_MULTILINESTRING: 
    case TG_MULTIPOLYGON:
    case TG_GEOMETRYCOLLECTION: 
        if (geom->multi) {
            size += sizeof(struct multi);
            size += geom->multi->ngeoms*sizeof(struct tg_geom*);
            for (int i = 0; i < geom->multi->ngeoms; i++) {
                size += tg_geom_memsize(geom->multi->geoms[i]);
            }
        }
        break;
    }
    if (geom->head.type != TG_POINT && geom->coords) {
        size += geom->ncoords*sizeof(double);
    }
    if (geom->xjson) {
        // geom->error shares the same memory stored as a C string.
        size += strlen(geom->xjson)+1;
    }
    return size;
}

/// Returns the allocation size of the geometry.
/// @param geom Input geometry
/// @return Size of geometry in bytes
size_t tg_geom_memsize(const struct tg_geom *geom) {
    if (geom) {
        switch (geom->head.base) {
        case BASE_GEOM:
            return geom_memsize(geom);
        case BASE_POINT: 
            return sizeof(struct boxed_point);
        case BASE_LINE:
            return tg_line_memsize((struct tg_line*)geom);
        case BASE_RING:
            return tg_ring_memsize((struct tg_ring*)geom);
        case BASE_POLY:
            return tg_poly_memsize((struct tg_poly*)geom);
        }
    }
    return 0;
}

/// Returns the underlying line for the provided geometry.
/// @param geom Input geometry
/// @return For a TG_LINESTRING geometry, returns the line.
/// @return For everything else returns NULL.
/// @see tg_line
/// @see GeometryAccessors
const struct tg_line *tg_geom_line(const struct tg_geom *geom) {
    if (!geom) return NULL;
    if (geom->head.base == BASE_LINE) return (struct tg_line*)geom;
    if (geom->head.base == BASE_GEOM && geom->head.type == TG_LINESTRING) {
        return geom->line;
    }
    return NULL;
}

/// Returns the underlying polygon for the provided geometry.
/// @param geom Input geometry
/// @return For a TG_POLYGON geometry, returns the polygon.
/// @return For everything else returns NULL.
/// @see tg_poly
/// @see GeometryAccessors
const struct tg_poly *tg_geom_poly(const struct tg_geom *geom) {
    if (!geom) return NULL;
    if (geom->head.base == BASE_RING) return (struct tg_poly*)geom;
    if (geom->head.base == BASE_POLY) return (struct tg_poly*)geom;
    if (geom->head.base == BASE_GEOM && geom->head.type == TG_POLYGON) {
        return geom->poly;
    }
    return NULL;
}

/// Returns the number of points in a MultiPoint geometry.
/// @param geom Input geometry
/// @return For a TG_MULTIPOINT geometry, returns the number of points.
/// @return For everything else returns zero.
/// @see tg_geom_point_at()
/// @see GeometryAccessors
int tg_geom_num_points(const struct tg_geom *geom) {
    if (!geom) return 0;
    if (geom->head.base == BASE_GEOM && geom->head.type == TG_MULTIPOINT &&
        geom->multi)
    {
        return geom->multi->ngeoms;
    }
    return 0;
}

/// Returns the number of lines in a MultiLineString geometry.
/// @param geom Input geometry
/// @return For a TG_MULTILINESTRING geometry, returns the number of lines.
/// @return For everything else returns zero.
/// @see tg_geom_line_at()
/// @see GeometryAccessors
int tg_geom_num_lines(const struct tg_geom *geom) {
    if (!geom) return 0;
    if (geom->head.base == BASE_GEOM && geom->head.type == TG_MULTILINESTRING &&
        geom->multi)
    {
        return geom->multi->ngeoms;
    }
    return 0;
}

/// Returns the number of polygons in a MultiPolygon geometry.
/// @param geom Input geometry
/// @return For a TG_MULTIPOLYGON geometry, returns the number of polygons.
/// @return For everything else returns zero.
/// @see tg_geom_poly_at()
/// @see GeometryAccessors
int tg_geom_num_polys(const struct tg_geom *geom) {
    if (!geom) return 0;
    if (geom->head.base == BASE_GEOM && geom->head.type == TG_MULTIPOLYGON &&
        geom->multi)
    {
        return geom->multi->ngeoms;
    }
    return 0;
}

/// Returns the number of geometries in a GeometryCollection geometry.
/// @param geom Input geometry
/// @return For a TG_MULTIGEOMETRY geometry, returns the number of geometries.
/// @return For everything else returns zero.
/// @note A geometry that is a GeoJSON FeatureCollection can use this function
/// to get number features in its collection.
/// @see tg_geom_geometry_at()
/// @see tg_geom_is_featurecollection()
/// @see GeometryAccessors
int tg_geom_num_geometries(const struct tg_geom *geom) {
    if (!geom) return 0;
    if (geom->head.base == BASE_GEOM && 
        geom->head.type == TG_GEOMETRYCOLLECTION && geom->multi)
    {
        return geom->multi->ngeoms;
    }
    return 0;
}

/// Returns the point at index for a MultiPoint geometry.
/// @param geom Input geometry
/// @param index Index of point
/// @return The point at index. Returns an empty point if the geometry type is
/// not TG_MULTIPOINT or when the provided index is out of range.
/// @see tg_geom_num_points()
/// @see GeometryAccessors
struct tg_point tg_geom_point_at(const struct tg_geom *geom, int index) {
    if (geom && geom->head.base == BASE_GEOM && 
        geom->head.type == TG_MULTIPOINT && 
        geom->multi &&index >= 0 && index <= geom->multi->ngeoms)
    {
        return ((struct boxed_point*)geom->multi->geoms[index])->point;
    }
    return (struct tg_point) { 0 };
}

/// Returns the line at index for a MultiLineString geometry.
/// @param geom Input geometry
/// @param index Index of line
/// @return The line at index. Returns NULL if the geometry type is not 
/// TG_MULTILINE or when the provided index is out of range.
/// @see tg_geom_num_lines()
/// @see GeometryAccessors
const struct tg_line *tg_geom_line_at(const struct tg_geom *geom, int index) {
    if (geom && geom->head.base == BASE_GEOM && 
        geom->head.type == TG_MULTILINESTRING && 
        geom->multi &&index >= 0 && index <= geom->multi->ngeoms)
    {
        return (struct tg_line*)geom->multi->geoms[index];
    }
    return NULL;
}

/// Returns the polygon at index for a MultiPolygon geometry.
/// @param geom Input geometry
/// @param index Index of polygon
/// @return The polygon at index. Returns NULL if the geometry type is not 
/// TG_MULTIPOLYGON or when the provided index is out of range.
/// @see tg_geom_num_polys()
/// @see GeometryAccessors
const struct tg_poly *tg_geom_poly_at(const struct tg_geom *geom, int index) {
    if (geom && geom->head.base == BASE_GEOM && 
        geom->head.type == TG_MULTIPOLYGON && 
        geom->multi && index >= 0 && index <= geom->multi->ngeoms)
    {
        return (struct tg_poly *)geom->multi->geoms[index];
    }
    return NULL;
}

/// Returns the geometry at index for a GeometryCollection geometry.
/// @param geom Input geometry
/// @param index Index of geometry
/// @return For a TG_MULTIGEOMETRY geometry, returns the number of geometries.
/// @return For everything else returns zero.
/// @note A geometry that is a GeoJSON FeatureCollection can use this
/// function to get number features in its collection.
/// @see tg_geom_geometry_at()
/// @see tg_geom_is_featurecollection()
/// @see GeometryAccessors
const struct tg_geom *tg_geom_geometry_at(const struct tg_geom *geom, 
    int index)
{
    if (geom && geom->head.base == BASE_GEOM && 
        geom->head.type == TG_GEOMETRYCOLLECTION && 
        geom->multi && index >= 0 && index <= geom->multi->ngeoms)
    {
        return geom->multi->geoms[index];
    }
    return NULL;
}

static bool geom_foreach(const struct tg_geom *geom,
    bool(*iter)(const struct tg_geom *geom, void *udata),
    void *udata)
{
    if (!geom) {
        return true;
    }
    if (geom->head.base == BASE_GEOM) {
        switch (geom->head.type) {
        case TG_MULTIPOINT:
        case TG_MULTILINESTRING:
        case TG_MULTIPOLYGON:
        case TG_GEOMETRYCOLLECTION:
            if (geom->multi) {
                for (int i = 0; i < geom->multi->ngeoms; i++) {
                    if (!iter(geom->multi->geoms[i], udata)) return false;
                }
            }
            return true;
        default:
            break;
        }
    }
    return iter(geom, udata);
}

/// tg_geom_foreach flattens a Multi* or GeometryCollection, iterating over
/// every child (or child of child in case of nested geometry collection).
/// If the provided geometry is not a Multi* or GeometryCollection then only 
/// that geometry is returned by the iter.
/// Empty geometries are not returned.
void tg_geom_foreach(const struct tg_geom *geom,
    bool(*iter)(const struct tg_geom *geom, void *udata),
    void *udata)
{
    geom_foreach(geom, iter, udata);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
/////  Intersects
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

static bool point_intersects_geom(struct tg_point point,
    const struct tg_geom *geom);

static bool point_intersects_base_geom(struct tg_point point,
    const struct tg_geom *geom)
{
    if ((geom->head.flags&IS_EMPTY) != IS_EMPTY) {
        switch (geom->head.type) {
        case TG_POINT: 
            return tg_point_intersects_point(point, geom->point); 
        case TG_LINESTRING: 
            return tg_point_intersects_line(point, geom->line);
        case TG_POLYGON: 
            return tg_point_intersects_poly(point, geom->poly);
        case TG_MULTIPOINT: 
        case TG_MULTILINESTRING: 
        case TG_MULTIPOLYGON:
        case TG_GEOMETRYCOLLECTION:
            if (geom->multi) {
                for (int i = 0; i < geom->multi->ngeoms; i++) {
                    if (point_intersects_geom(point, geom->multi->geoms[i])) {
                        return true;
                    }
                }
            }
            return false;
        }
    }
    return false;
}

static bool point_intersects_geom(struct tg_point point,
    const struct tg_geom *geom)
{
    if (geom) {
        switch (geom->head.base) {
        case BASE_GEOM:
            return point_intersects_base_geom(point, geom);
        case BASE_POINT:
            return tg_point_intersects_point(point, 
                ((struct boxed_point*)geom)->point); 
        case BASE_LINE:
            return tg_point_intersects_line(point, ((struct tg_line*)geom));
        case BASE_RING:
            return tg_point_intersects_poly(point, ((struct tg_poly*)geom));
        case BASE_POLY:
            return tg_point_intersects_poly(point, ((struct tg_poly*)geom));
        }
    }
    return false;
}

static bool line_intersects_geom(struct tg_line *line,
    const struct tg_geom *geom);

static bool line_intersects_base_geom(struct tg_line *line,
    const struct tg_geom *geom)
{
    if ((geom->head.flags&IS_EMPTY) != IS_EMPTY) {
        switch (geom->head.type) {
        case TG_POINT: 
            return tg_line_intersects_point(line, geom->point); 
        case TG_LINESTRING: 
            return tg_line_intersects_line(line, geom->line);
        case TG_POLYGON: 
            return tg_line_intersects_poly(line, geom->poly);
        case TG_MULTIPOINT: 
        case TG_MULTILINESTRING: 
        case TG_MULTIPOLYGON:
        case TG_GEOMETRYCOLLECTION: 
            if (geom->multi) {
                for (int i = 0; i < geom->multi->ngeoms; i++) {
                    if (line_intersects_geom(line, geom->multi->geoms[i])) {
                        return true;
                    }
                }
            }
            return false;
        }
    }
    return false;
}

static bool line_intersects_geom(struct tg_line *line,
    const struct tg_geom *geom)
{
    if (geom) {
        switch (geom->head.base) {
        case BASE_GEOM:
            return line_intersects_base_geom(line, geom);
        case BASE_POINT:
            return tg_line_intersects_point(line, 
                ((struct boxed_point*)geom)->point); 
        case BASE_LINE:
            return tg_line_intersects_line(line, ((struct tg_line*)geom));
        case BASE_RING:
            return tg_line_intersects_poly(line, ((struct tg_poly*)geom));
        case BASE_POLY:
            return tg_line_intersects_poly(line, ((struct tg_poly*)geom));
        }
    }
    return false;
}

static bool poly_intersects_geom(struct tg_poly *poly, 
    const struct tg_geom *geom);

static bool poly_intersects_base_geom(struct tg_poly *poly, 
    const struct tg_geom *geom)
{
    if ((geom->head.flags&IS_EMPTY) != IS_EMPTY) {
        switch (geom->head.type) {
        case TG_POINT: 
            return tg_poly_intersects_point(poly, geom->point); 
        case TG_LINESTRING: 
            return tg_poly_intersects_line(poly, geom->line);
        case TG_POLYGON: 
            return tg_poly_intersects_poly(poly, geom->poly);
        case TG_MULTIPOINT: 
        case TG_MULTILINESTRING: 
        case TG_MULTIPOLYGON:
        case TG_GEOMETRYCOLLECTION: 
            if (geom->multi) {
                for (int i = 0; i < geom->multi->ngeoms; i++) {
                    if (poly_intersects_geom(poly, geom->multi->geoms[i])) {
                        return true;
                    }
                }
            }
            return false;
        }
    }
    return false;
}

static bool poly_intersects_geom(struct tg_poly *poly, 
    const struct tg_geom *geom)
{
    if (geom) {
        switch (geom->head.base) {
        case BASE_GEOM:
            return poly_intersects_base_geom(poly, geom);
        case BASE_POINT:
            return tg_poly_intersects_point(poly, 
                ((struct boxed_point*)geom)->point); 
        case BASE_LINE:
            return tg_poly_intersects_line(poly, ((struct tg_line*)geom));
        case BASE_RING:
            return tg_poly_intersects_poly(poly, ((struct tg_poly*)geom));
        case BASE_POLY:
            return tg_poly_intersects_poly(poly, ((struct tg_poly*)geom));
        }
    }
    return false;
}

static bool base_geom_intersects_geom(const struct tg_geom *geom, 
    const struct tg_geom *other)
{
    if ((geom->head.flags&IS_EMPTY) != IS_EMPTY) {
        switch (geom->head.type) {
        case TG_POINT: 
            return point_intersects_geom(geom->point, other);
        case TG_LINESTRING: 
            return line_intersects_geom(geom->line, other);
        case TG_POLYGON: 
            return poly_intersects_geom(geom->poly, other);
        case TG_MULTIPOINT: 
        case TG_MULTILINESTRING: 
        case TG_MULTIPOLYGON:
        case TG_GEOMETRYCOLLECTION: 
            if (geom->multi) {
                for (int i = 0; i < geom->multi->ngeoms; i++) {
                    if (tg_geom_intersects(geom->multi->geoms[i], other)) {
                        return true;
                    }
                }
            }
            return false;
        }
    }
    return false;
}

/// Tests whether two geometries intersect.
/// @see GeometryPredicates
bool tg_geom_intersects(const struct tg_geom *geom, 
    const struct tg_geom *other)
{
    if (geom) {
        switch (geom->head.base) {
        case BASE_GEOM:
            return base_geom_intersects_geom(geom, other);
        case BASE_POINT: 
            return point_intersects_geom(((struct boxed_point*)geom)->point, 
                other);
        case BASE_LINE:
            return line_intersects_geom((struct tg_line*)geom, other);
        case BASE_RING:
            return poly_intersects_geom((struct tg_poly*)geom, other);
        case BASE_POLY:
            return poly_intersects_geom((struct tg_poly*)geom, other);
        }
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
/////  Covers
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

static bool point_covers_geom(struct tg_point point,
    const struct tg_geom *geom);

static bool point_covers_base_geom(struct tg_point point,
    const struct tg_geom *geom)
{
    if ((geom->head.flags&IS_EMPTY) != IS_EMPTY) {
        switch (geom->head.type) {
        case TG_POINT: 
            return tg_point_covers_point(point, geom->point); 
        case TG_LINESTRING: 
            return tg_point_covers_line(point, geom->line);
        case TG_POLYGON: 
            return tg_point_covers_poly(point, geom->poly);
        case TG_MULTIPOINT:
        case TG_MULTILINESTRING:
        case TG_MULTIPOLYGON:
        case TG_GEOMETRYCOLLECTION: 
            if (!geom->multi || geom->multi->ngeoms == 0) return false;
            for (int i = 0; i < geom->multi->ngeoms; i++) {
                if (!point_covers_geom(point, geom->multi->geoms[i])) {
                    return false;
                }
            }
            return true;
        }
    }
    return false;
}

static bool point_covers_geom(struct tg_point point,
    const struct tg_geom *geom)
{
    if (geom) {
        switch (geom->head.base) {
        case BASE_GEOM:
            return point_covers_base_geom(point, geom);
        case BASE_POINT:
            return tg_point_covers_point(point, 
                ((struct boxed_point*)geom)->point); 
        case BASE_LINE:
            return tg_point_covers_line(point, ((struct tg_line*)geom));
        case BASE_RING:
            return tg_point_covers_poly(point, ((struct tg_poly*)geom));
        case BASE_POLY:
            return tg_point_covers_poly(point, ((struct tg_poly*)geom));
        }
    }
    return false;
}

static bool line_covers_geom(struct tg_line *line,
    const struct tg_geom *geom);

static bool line_covers_base_geom(struct tg_line *line,
    const struct tg_geom *geom)
{
    if ((geom->head.flags&IS_EMPTY) != IS_EMPTY) {
        switch (geom->head.type) {
        case TG_POINT: 
            return tg_line_covers_point(line, geom->point); 
        case TG_LINESTRING: 
            return tg_line_covers_line(line, geom->line);
        case TG_POLYGON: 
            return tg_line_covers_poly(line, geom->poly);
        case TG_MULTIPOINT: 
        case TG_MULTILINESTRING: 
        case TG_MULTIPOLYGON:
        case TG_GEOMETRYCOLLECTION: 
            if (!geom->multi || geom->multi->ngeoms == 0) return false;
            for (int i = 0; i < geom->multi->ngeoms; i++) {
                if (!line_covers_geom(line, geom->multi->geoms[i])) {
                    return false;
                }
            }
            return true;
        }
    }
    return false;
}

static bool line_covers_geom(struct tg_line *line,
    const struct tg_geom *geom)
{
    if (geom) {
        switch (geom->head.base) {
        case BASE_GEOM:
            return line_covers_base_geom(line, geom);
        case BASE_POINT:
            return tg_line_covers_point(line, 
                ((struct boxed_point*)geom)->point); 
        case BASE_LINE:
            return tg_line_covers_line(line, ((struct tg_line*)geom));
        case BASE_RING:
            return tg_line_covers_poly(line, ((struct tg_poly*)geom));
        case BASE_POLY:
            return tg_line_covers_poly(line, ((struct tg_poly*)geom));
        }
    }
    return false;
}

static bool poly_covers_geom(struct tg_poly *poly, 
    const struct tg_geom *geom);

static bool poly_covers_base_geom(struct tg_poly *poly, 
    const struct tg_geom *geom)
{
    if ((geom->head.flags&IS_EMPTY) != IS_EMPTY) {
        switch (geom->head.type) {
        case TG_POINT: 
            return tg_poly_covers_point(poly, geom->point); 
        case TG_LINESTRING: 
            return tg_poly_covers_line(poly, geom->line);
        case TG_POLYGON: 
            return tg_poly_covers_poly(poly, geom->poly);
        case TG_MULTIPOINT: 
        case TG_MULTILINESTRING: 
        case TG_MULTIPOLYGON:
        case TG_GEOMETRYCOLLECTION: 
            if (!geom->multi || geom->multi->ngeoms == 0) return false;
            for (int i = 0; i < geom->multi->ngeoms; i++) {
                if (!poly_covers_geom(poly, geom->multi->geoms[i])) {
                    return false;
                }
            }
            return true;
        }
    }
    return false;
}

static bool poly_covers_geom(struct tg_poly *poly, 
    const struct tg_geom *geom)
{
    if (geom) {
        switch (geom->head.base) {
        case BASE_GEOM:
            return poly_covers_base_geom(poly, geom);
        case BASE_POINT:
            return tg_poly_covers_point(poly, 
                ((struct boxed_point*)geom)->point); 
        case BASE_LINE:
            return tg_poly_covers_line(poly, ((struct tg_line*)geom));
        case BASE_RING:
            return tg_poly_covers_poly(poly, ((struct tg_poly*)geom));
        case BASE_POLY:
            return tg_poly_covers_poly(poly, ((struct tg_poly*)geom));
        }
    }
    return false;
}

struct geom_covers_iter_ctx {
    const struct tg_geom *geom;
    bool result;
};

static bool geom_covers_iter0(const struct tg_geom *geom, void *udata) {
    struct geom_covers_iter_ctx *ctx = udata;
    if (tg_geom_covers(geom, ctx->geom)) {
        // found a child object that contains geom, end inner loop
        ctx->result = true;
        return false;
    }
    return true;
}

bool tg_geom_is_empty(const struct tg_geom *geom);

static bool base_geom_deep_empty(const struct tg_geom *geom) {
    switch (geom->head.type) {
    case TG_POINT:
        return false;
    case TG_LINESTRING:
        return tg_line_empty(geom->line);
    case TG_POLYGON:
        return tg_poly_empty(geom->poly);
    case TG_MULTIPOINT:
    case TG_MULTILINESTRING:
    case TG_MULTIPOLYGON:
    case TG_GEOMETRYCOLLECTION:
        if (geom->multi) {
            for (int i = 0; i < geom->multi->ngeoms; i++) {
                if (!tg_geom_is_empty(geom->multi->geoms[i])) {
                    return false;
                }
            }
        }
    }
    return true;
}

/// Tests whether a geometry is empty. An empty geometry is one that has no
/// interior boundary.
/// @param geom Input geometry
/// @return True or false
bool tg_geom_is_empty(const struct tg_geom *geom) {
    if (geom) {
        if ((geom->head.flags&IS_EMPTY) == IS_EMPTY) return true;
        switch (geom->head.base) {
        case BASE_GEOM:
            return base_geom_deep_empty(geom);
        case BASE_POINT:
            return false;
        case BASE_LINE:
            return tg_line_empty((struct tg_line*)geom);
        case BASE_RING:
            return tg_poly_empty((struct tg_poly*)geom);
        case BASE_POLY:
            return tg_poly_empty((struct tg_poly*)geom);
        }
    }
    return true;
}

static bool geom_covers_iter(const struct tg_geom *geom, void *udata) {
    struct geom_covers_iter_ctx *ctx = udata;
    // skip empty geometries
    if (tg_geom_is_empty(geom)) {
        return true;
    }
    struct geom_covers_iter_ctx ctx0 = { .geom = geom };
    tg_geom_foreach(ctx->geom, geom_covers_iter0, &ctx0);
    if (!ctx0.result) {
        // unmark and quit the loop
        ctx->result = false;
        return false;
    }
    // mark that at least one geom is contained
    ctx->result = true;
    return true;
}

static bool base_geom_covers_geom(const struct tg_geom *geom, 
    const struct tg_geom *other)
{
    if ((geom->head.flags&IS_EMPTY) != IS_EMPTY) {
        switch (geom->head.type) {
        case TG_POINT:
            return point_covers_geom(geom->point, other);
        case TG_LINESTRING:
            return line_covers_geom(geom->line, other);
        case TG_POLYGON:
            return poly_covers_geom(geom->poly, other);
        case TG_MULTIPOINT:
        case TG_MULTILINESTRING:
        case TG_MULTIPOLYGON:
        case TG_GEOMETRYCOLLECTION: {
            // all children of 'other' must be fully within 'geom'
            struct geom_covers_iter_ctx ctx = { .geom = geom };
            tg_geom_foreach(other, geom_covers_iter, &ctx);
            return ctx.result;
        }
        }
    }
    return false;
}

/// Tests whether a geometry 'a' fully contains geometry 'b'.
/// @see GeometryPredicates
bool tg_geom_covers(const struct tg_geom *geom, const struct tg_geom *other) {
    if (geom) {
        switch (geom->head.base) {
        case BASE_GEOM:
            return base_geom_covers_geom(geom, other);
        case BASE_POINT: 
            return point_covers_geom(((struct boxed_point*)geom)->point,
                other);
        case BASE_LINE:
            return line_covers_geom((struct tg_line*)geom, other);
        case BASE_RING:
            return poly_covers_geom((struct tg_poly*)geom, other);
        case BASE_POLY:
            return poly_covers_geom((struct tg_poly*)geom, other);
        }
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
/////  Contains
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

static bool poly_contains_geom(struct tg_poly *poly, 
    const struct tg_geom *geom);

static bool poly_contains_base_geom(struct tg_poly *poly, 
    const struct tg_geom *geom)
{
    if ((geom->head.flags&IS_EMPTY) != IS_EMPTY) {
        switch (geom->head.type) {
        case TG_POINT: 
            return tg_poly_contains_point(poly, geom->point); 
        case TG_LINESTRING: 
            return tg_poly_contains_line(poly, geom->line);
        case TG_POLYGON: 
            return tg_poly_contains_poly(poly, geom->poly);
        case TG_MULTIPOINT: 
        case TG_MULTILINESTRING: 
        case TG_MULTIPOLYGON:
        case TG_GEOMETRYCOLLECTION: {
            bool contains = false;
            if (geom->multi && geom->multi->ngeoms > 0) {
                contains = true;
                for (int i = 0; i < geom->multi->ngeoms; i++) {
                    if (!poly_contains_geom(poly, geom->multi->geoms[i])) {
                        contains = false;
                        break;
                    }
                }
            }
            return contains;
        }}
    }
    return false;
}

static bool poly_contains_geom(struct tg_poly *poly, 
    const struct tg_geom *geom)
{
    if (geom) {
        switch (geom->head.base) {
        case BASE_GEOM:
            return poly_contains_base_geom(poly, geom);
        case BASE_POINT:
            return tg_poly_contains_point(poly, 
                ((struct boxed_point*)geom)->point); 
        case BASE_LINE:
            return tg_poly_contains_line(poly, ((struct tg_line*)geom));
        case BASE_RING:
            return tg_poly_contains_poly(poly, ((struct tg_poly*)geom));
        case BASE_POLY:
            return tg_poly_contains_poly(poly, ((struct tg_poly*)geom));
        }
    }
    return false;
}

static bool line_contains_geom(struct tg_line *line,
    const struct tg_geom *geom);

static bool line_contains_base_geom(struct tg_line *line,
    const struct tg_geom *geom)
{
    if ((geom->head.flags&IS_EMPTY) != IS_EMPTY) {
        switch (geom->head.type) {
        case TG_POINT: 
            return tg_line_contains_point(line, geom->point); 
        case TG_LINESTRING: 
            return tg_line_contains_line(line, geom->line);
        case TG_POLYGON: 
            return tg_line_contains_poly(line, geom->poly);
        case TG_MULTIPOINT: 
        case TG_MULTILINESTRING: 
        case TG_MULTIPOLYGON:
        case TG_GEOMETRYCOLLECTION: {
            bool contains = false;
            if (geom->multi && geom->multi->ngeoms > 0) {
                contains = true;
                for (int i = 0; i < geom->multi->ngeoms; i++) {
                    if (!line_contains_geom(line, geom->multi->geoms[i])) {
                        contains = false;
                        break;
                    }
                }
            }
            return contains;
        }}
    }
    return false;
}

static bool line_contains_geom(struct tg_line *line,
    const struct tg_geom *geom)
{
    if (geom) {
        switch (geom->head.base) {
        case BASE_GEOM:
            return line_contains_base_geom(line, geom);
        case BASE_POINT:
            return tg_line_contains_point(line, 
                ((struct boxed_point*)geom)->point); 
        case BASE_LINE:
            return tg_line_contains_line(line, ((struct tg_line*)geom));
        case BASE_RING:
            return tg_line_contains_poly(line, ((struct tg_poly*)geom));
        case BASE_POLY:
            return tg_line_contains_poly(line, ((struct tg_poly*)geom));
        }
    }
    return false;
}


static bool point_contains_geom(struct tg_point point,
    const struct tg_geom *geom);

static bool point_contains_base_geom(struct tg_point point,
    const struct tg_geom *geom)
{
    if ((geom->head.flags&IS_EMPTY) != IS_EMPTY) {
        switch (geom->head.type) {
        case TG_POINT: 
            return tg_point_contains_point(point, geom->point); 
        case TG_LINESTRING: 
            return tg_point_contains_line(point, geom->line);
        case TG_POLYGON: 
            return tg_point_contains_poly(point, geom->poly);
        case TG_MULTIPOINT:
        case TG_MULTILINESTRING:
        case TG_MULTIPOLYGON:
        case TG_GEOMETRYCOLLECTION: {
            bool contains = false;
            if (geom->multi && geom->multi->ngeoms > 0) {
                contains = true;
                for (int i = 0; i < geom->multi->ngeoms; i++) {
                    if (!point_contains_geom(point, geom->multi->geoms[i])) {
                        contains = false;
                        break;
                    }
                }
            }
            return contains;
        }}
    }
    return false;
}

struct geom_contains_iter_ctx {
    const struct tg_geom *geom;
    bool result;
};


static bool geom_contains_iter0(const struct tg_geom *geom, void *udata) {
    struct geom_contains_iter_ctx *ctx = udata;
    if (tg_geom_contains(geom, ctx->geom)) {
        // found a child object that contains geom, end inner loop
        ctx->result = true;
        return false;
    }
    return true;
}

static bool geom_contains_iter(const struct tg_geom *geom, void *udata) {
    struct geom_contains_iter_ctx *ctx = udata;
    // skip empty geometries
    if (!tg_geom_is_empty(geom)) {
        struct geom_contains_iter_ctx ctx0 = { .geom = geom };
        tg_geom_foreach(ctx->geom, geom_contains_iter0, &ctx0);
        if (!ctx0.result) {
            // unmark and quit the loop
            ctx->result = false;
            return false;
        }
        // mark that at least one geom is contained
        ctx->result = true;
    }
    return true;
}

static bool base_geom_contains_geom(const struct tg_geom *geom, 
    const struct tg_geom *other)
{
    if ((geom->head.flags&IS_EMPTY) != IS_EMPTY) {
        switch (geom->head.type) {
        case TG_POINT:
            return point_contains_geom(geom->point, other);
        case TG_LINESTRING:
            return line_contains_geom(geom->line, other);
        case TG_POLYGON:
            return poly_contains_geom(geom->poly, other);
        case TG_MULTIPOINT:
        case TG_MULTILINESTRING:
        case TG_MULTIPOLYGON:
        case TG_GEOMETRYCOLLECTION: {
            // all children of 'other' must be fully within 'geom'
            struct geom_contains_iter_ctx ctx = { .geom = geom };
            tg_geom_foreach(other, geom_contains_iter, &ctx);
            return ctx.result;
        }
        }
    }
    return false;
}

static bool point_contains_geom(struct tg_point point,
    const struct tg_geom *geom)
{
    if (geom) {
        switch (geom->head.base) {
        case BASE_GEOM:
            return point_contains_base_geom(point, geom);
        case BASE_POINT:
            return tg_point_contains_point(point, 
                ((struct boxed_point*)geom)->point); 
        case BASE_LINE:
            return tg_point_contains_line(point, ((struct tg_line*)geom));
        case BASE_RING:
            return tg_point_contains_poly(point, ((struct tg_poly*)geom));
        case BASE_POLY:
            return tg_point_contains_poly(point, ((struct tg_poly*)geom));
        }
    }
    return false;
}

/// Tests whether 'a' contains 'b', and 'b' is not touching the boundary of 'a'.
/// @note Works the same as `tg_geom_within(b, a)`
/// @warning This predicate returns **false** when geometry 'b' is *on* or
/// *touching* the boundary of geometry 'a'. Such as when a point is on the
/// edge of a polygon.  
/// For full coverage, consider using @ref tg_geom_covers.
/// @see GeometryPredicates
bool tg_geom_contains(const struct tg_geom *geom, const struct tg_geom *other) {
    if (geom) {
        switch (geom->head.base) {
        case BASE_GEOM:
            return base_geom_contains_geom(geom, other);
        case BASE_POINT: 
            return point_contains_geom(((struct boxed_point*)geom)->point,
                other);
        case BASE_LINE:
            return line_contains_geom((struct tg_line*)geom, other);
        case BASE_RING:
            return poly_contains_geom((struct tg_poly*)geom, other);
        case BASE_POLY:
            return poly_contains_geom((struct tg_poly*)geom, other);
        }
    }
    return false;
}

bool tg_poly_contains_geom(struct tg_poly *a, const struct tg_geom *b) {
    return poly_contains_geom(a, b);
}

bool tg_line_contains_geom(struct tg_line *a, const struct tg_geom *b) {
    return line_contains_geom(a, b);
}

bool tg_point_contains_geom(struct tg_point a, const struct tg_geom *b) {
    return point_contains_geom(a, b);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
/////  Touches
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

static bool point_touches_geom(struct tg_point point,
    const struct tg_geom *geom);

static bool point_touches_base_geom(struct tg_point point,
    const struct tg_geom *geom)
{
    if ((geom->head.flags&IS_EMPTY) != IS_EMPTY) {
        switch (geom->head.type) {
        case TG_POINT: 
            return tg_point_touches_point(point, geom->point); 
        case TG_LINESTRING: 
            return tg_point_touches_line(point, geom->line);
        case TG_POLYGON: 
            return tg_point_touches_poly(point, geom->poly);
        case TG_MULTIPOINT: 
        case TG_MULTILINESTRING: 
        case TG_MULTIPOLYGON:
        case TG_GEOMETRYCOLLECTION:
            if (geom->multi) {
                for (int i = 0; i < geom->multi->ngeoms; i++) {
                    if (point_touches_geom(point, geom->multi->geoms[i])) {
                        return true;
                    }
                }
            }
            return false;
        }
    }
    return false;
}

static bool point_touches_geom(struct tg_point point,
    const struct tg_geom *geom)
{
    if (geom) {
        switch (geom->head.base) {
        case BASE_GEOM:
            return point_touches_base_geom(point, geom);
        case BASE_POINT:
            return tg_point_touches_point(point, 
                ((struct boxed_point*)geom)->point); 
        case BASE_LINE:
            return tg_point_touches_line(point, ((struct tg_line*)geom));
        case BASE_RING:
            return tg_point_touches_poly(point, ((struct tg_poly*)geom));
        case BASE_POLY:
            return tg_point_touches_poly(point, ((struct tg_poly*)geom));
        }
    }
    return false;
}

static bool line_touches_geom(struct tg_line *line,
    const struct tg_geom *geom);

static bool line_touches_base_geom(struct tg_line *line,
    const struct tg_geom *geom)
{
    if ((geom->head.flags&IS_EMPTY) != IS_EMPTY) {
        switch (geom->head.type) {
        case TG_POINT: 
            return tg_line_touches_point(line, geom->point); 
        case TG_LINESTRING: 
            return tg_line_touches_line(line, geom->line);
        case TG_POLYGON: 
            return tg_line_touches_poly(line, geom->poly);
        case TG_MULTIPOINT: 
        case TG_MULTILINESTRING: 
        case TG_MULTIPOLYGON:
        case TG_GEOMETRYCOLLECTION: 
            if (geom->multi) {
                for (int i = 0; i < geom->multi->ngeoms; i++) {
                    if (line_touches_geom(line, geom->multi->geoms[i])) {
                        return true;
                    }
                }
            }
            return false;
        }
    }
    return false;
}

static bool line_touches_geom(struct tg_line *line,
    const struct tg_geom *geom)
{
    if (geom) {
        switch (geom->head.base) {
        case BASE_GEOM:
            return line_touches_base_geom(line, geom);
        case BASE_POINT:
            return tg_line_touches_point(line, 
                ((struct boxed_point*)geom)->point); 
        case BASE_LINE:
            return tg_line_touches_line(line, ((struct tg_line*)geom));
        case BASE_RING:
            return tg_line_touches_poly(line, ((struct tg_poly*)geom));
        case BASE_POLY:
            return tg_line_touches_poly(line, ((struct tg_poly*)geom));
        }
    }
    return false;
}

static bool poly_touches_geom(struct tg_poly *poly, 
    const struct tg_geom *geom);

static bool poly_touches_base_geom(struct tg_poly *poly, 
    const struct tg_geom *geom)
{
    if ((geom->head.flags&IS_EMPTY) != IS_EMPTY) {
        switch (geom->head.type) {
        case TG_POINT: 
            return tg_poly_touches_point(poly, geom->point); 
        case TG_LINESTRING: 
            return tg_poly_touches_line(poly, geom->line);
        case TG_POLYGON: 
            return tg_poly_touches_poly(poly, geom->poly);
        case TG_MULTIPOINT: 
        case TG_MULTILINESTRING: 
        case TG_MULTIPOLYGON:
        case TG_GEOMETRYCOLLECTION: {
            bool touches = false;
            if (geom->multi) {
                for (int i = 0; i < geom->multi->ngeoms; i++) {
                    const struct tg_geom *child = geom->multi->geoms[i];
                    if (poly_touches_geom(poly, child)) {
                        touches = true;
                    } else if (poly_intersects_geom(poly, child)) {
                        return false;
                    }
                }
            }
            return touches;
        }}
    }
    return false;
}

static bool poly_touches_geom(struct tg_poly *poly, 
    const struct tg_geom *geom)
{
    if (geom) {
        switch (geom->head.base) {
        case BASE_GEOM:
            return poly_touches_base_geom(poly, geom);
        case BASE_POINT:
            return tg_poly_touches_point(poly, 
                ((struct boxed_point*)geom)->point); 
        case BASE_LINE:
            return tg_poly_touches_line(poly, ((struct tg_line*)geom));
        case BASE_RING:
            return tg_poly_touches_poly(poly, ((struct tg_poly*)geom));
        case BASE_POLY:
            return tg_poly_touches_poly(poly, ((struct tg_poly*)geom));
        }
    }
    return false;
}

static bool base_geom_touches_geom(const struct tg_geom *geom, 
    const struct tg_geom *other)
{
    if ((geom->head.flags&IS_EMPTY) != IS_EMPTY) {
        switch (geom->head.type) {
        case TG_POINT: 
            return point_touches_geom(geom->point, other);
        case TG_LINESTRING: 
            return line_touches_geom(geom->line, other);
        case TG_POLYGON: 
            return poly_touches_geom(geom->poly, other);
        case TG_MULTIPOINT: 
        case TG_MULTILINESTRING: 
        case TG_MULTIPOLYGON:
        case TG_GEOMETRYCOLLECTION: {
            bool touches = false;
            if (geom->multi) {
                for (int i = 0; i < geom->multi->ngeoms; i++) {
                    const struct tg_geom *child = geom->multi->geoms[i];
                    if (tg_geom_touches(child, other)) {
                        touches = true;
                    } else if (tg_geom_intersects(child, other)) {
                        return false;
                    }
                }
            }
            return touches;
         }}
    }
    return false;
}

/// Tests whether a geometry 'a' touches 'b'. 
/// They have at least one point in common, but their interiors do not
/// intersect.
/// @see GeometryPredicates
bool tg_geom_touches(const struct tg_geom *geom, const struct tg_geom *other) {
    if (geom) {
        switch (geom->head.base) {
        case BASE_GEOM:
            return base_geom_touches_geom(geom, other);
        case BASE_POINT: 
            return point_touches_geom(((struct boxed_point*)geom)->point,
                other);
        case BASE_LINE:
            return line_touches_geom((struct tg_line*)geom, other);
        case BASE_RING:
            return poly_touches_geom((struct tg_poly*)geom, other);
        case BASE_POLY:
            return poly_touches_geom((struct tg_poly*)geom, other);
        }
    }
    return false;
}

bool tg_poly_touches_geom(struct tg_poly *a, const struct tg_geom *b) {
    return poly_touches_geom(a, b);
}

bool tg_line_touches_geom(struct tg_line *a, const struct tg_geom *b) {
    return line_touches_geom(a, b);
}

bool tg_point_touches_geom(struct tg_point a, const struct tg_geom *b) {
    return point_touches_geom(a, b);
}

/// Tests whether a geometry 'a' fully contains point 'b'.
/// @see GeometryPredicates
bool tg_geom_covers_point(const struct tg_geom *a, struct tg_point b) {
    struct boxed_point bpoint = {
        .head = { .base = BASE_POINT, .type = TG_POINT },
        .point = b,
    };
    return tg_geom_covers(a, (struct tg_geom*)&bpoint);
}

/// Tests whether a geometry fully contains a point using xy coordinates.
/// @see GeometryPredicates
bool tg_geom_covers_xy(const struct tg_geom *a, double x, double y) {
    return tg_geom_covers_point(a, (struct tg_point){ .x = x, .y = y });
}

/// Tests whether a geometry 'a' intersects point 'b'.
/// @see GeometryPredicates
bool tg_geom_intersects_point(const struct tg_geom *a, struct tg_point b) {
    struct boxed_point bpoint = {
        .head = { .base = BASE_POINT, .type = TG_POINT },
        .point = b,
    };
    return tg_geom_intersects(a, (struct tg_geom*)&bpoint);
}

/// Tests whether a geometry intersects a point using xy coordinates.
/// @see GeometryPredicates
bool tg_geom_intersects_xy(const struct tg_geom *a, double x, double y) {
    return tg_geom_intersects_point(a, (struct tg_point){ .x = x, .y = y });
}

/// Get the extra coordinates for a geometry.
/// @param geom Input geometry
/// @return Array of coordinates
/// @return NULL if there are no extra coordinates 
/// @note These are the raw coodinates provided by a constructor like 
/// tg_geom_new_polygon_z() or from a parsed source like WKT "POLYGON Z ...".
/// @see tg_geom_num_extra_coords()
const double *tg_geom_extra_coords(const struct tg_geom *geom) {
    if (!geom || geom->head.base != BASE_GEOM || geom->head.type == TG_POINT) {
        return NULL;
    }
    return geom->coords;
}

/// Get the number of extra coordinates for a geometry
/// @param geom Input geometry
/// @return The number of extra coordinates, or zero if none.
/// @see tg_geom_extra_coords()
int tg_geom_num_extra_coords(const struct tg_geom *geom) {
    if (!geom || geom->head.base != BASE_GEOM || geom->head.type == TG_POINT) {
        return 0;
    }
    return geom->ncoords;
}

/// Get the number of dimensions for a geometry.
/// @param geom Input geometry
/// @return 2 for standard geometries
/// @return 3 when geometry has Z or M coordinates
/// @return 4 when geometry has Z and M coordinates
/// @return 0 when input is NULL
int tg_geom_dims(const struct tg_geom *geom) {
    if (!geom) return 0;
    int dims = 2;
    if ((geom->head.flags&HAS_Z) == HAS_Z) dims++;
    if ((geom->head.flags&HAS_M) == HAS_M) dims++;
    return dims;
}

/// Tests whether a geometry has Z coordinates.
/// @param geom Input geometry
/// @return True or false
bool tg_geom_has_z(const struct tg_geom *geom) {
    return (geom && (geom->head.flags&HAS_Z) == HAS_Z);
}

/// Tests whether a geometry has M coordinates.
/// @param geom Input geometry
/// @return True or false
bool tg_geom_has_m(const struct tg_geom *geom) {
    return (geom && (geom->head.flags&HAS_M) == HAS_M);
}

/// Get the Z coordinate of a Point geometry.
/// @param geom Input geometry
/// @return For a TG_POINT geometry, returns the Z coodinate.
/// @return For everything else returns zero.
double tg_geom_z(const struct tg_geom *geom) {
    if (!geom || geom->head.base != BASE_GEOM || geom->head.type != TG_POINT) {
        return 0;
    }
    return geom->z;
}

/// Get the M coordinate of a Point geometry.
/// @param geom Input geometry
/// @return For a TG_POINT geometry, returns the M coodinate.
/// @return For everything else returns zero.
double tg_geom_m(const struct tg_geom *geom) {
    if (!geom || geom->head.base != BASE_GEOM || geom->head.type != TG_POINT) {
        return 0;
    }
    return geom->m;
}

/// Returns the indexing spread for a ring.
/// 
/// The "spread" is the number of segments or rectangles that are grouped 
/// together to produce a unioned rectangle that is stored at a higher level.
/// 
/// For a tree based structure, this would be the number of items per node.
///
/// @param ring Input ring
/// @return The spread, default is 16
/// @return Zero if ring has no indexing
/// @see tg_ring_index_num_levels()
/// @see tg_ring_index_level_num_rects()
/// @see tg_ring_index_level_rect()
/// @see RingFuncs
int tg_ring_index_spread(const struct tg_ring *ring) {
    if (!ring || !ring->index) return 0;
    return ring->index->spread;
}

/// Returns the number of levels.
/// @param ring Input ring
/// @return The number of levels
/// @return Zero if ring has no indexing
/// @see tg_ring_index_spread()
/// @see tg_ring_index_level_num_rects()
/// @see tg_ring_index_level_rect()
/// @see RingFuncs
int tg_ring_index_num_levels(const struct tg_ring *ring) {
    if (!ring || !ring->index) return 0;
    return ring->index->nlevels;
}

/// Returns the number of rectangles at level.
/// @param ring Input ring
/// @param levelidx The index of level
/// @return The number of index levels
/// @return Zero if ring has no indexing or levelidx is out of bounds.
/// @see tg_ring_index_spread()
/// @see tg_ring_index_num_levels()
/// @see tg_ring_index_level_rect()
/// @see RingFuncs
int tg_ring_index_level_num_rects(const struct tg_ring *ring, int levelidx) {
    if (!ring || !ring->index) return 0;
    if (levelidx < 0 || levelidx >= ring->index->nlevels) return 0;
    return ring->index->levels[levelidx].nrects;
}

/// Returns a specific level rectangle.
/// @param ring Input ring
/// @param levelidx The index of level
/// @param rectidx The index of rectangle
/// @return The rectangle
/// @return Empty rectangle if ring has no indexing, or levelidx or rectidx
/// is out of bounds.
/// @see tg_ring_index_spread()
/// @see tg_ring_index_num_levels()
/// @see tg_ring_index_level_num_rects()
/// @see RingFuncs
struct tg_rect tg_ring_index_level_rect(const struct tg_ring *ring, 
    int levelidx, int rectidx)
{
    if (!ring || !ring->index) return (struct tg_rect) { 0 };
    if (levelidx < 0 || levelidx >= ring->index->nlevels) {
        return (struct tg_rect) { 0 };
    }
    struct level *level = &ring->index->levels[levelidx];
    if (rectidx < 0 || rectidx >= level->nrects) {
        return (struct tg_rect) { 0 };
    }
    struct tg_rect rect;
    ixrect_to_tg_rect(&level->rects[rectidx], &rect);
    return rect;
}

/// Get the string representation of a geometry type. 
/// e.g. "Point", "Polygon", "LineString".
/// @param type Input geometry type
/// @return A string representing the type
/// @note The returned string does not need to be freed.
/// @see tg_geom_typeof()
/// @see GeometryAccessors
const char *tg_geom_type_string(enum tg_geom_type type) {
    switch (type) {
    case TG_POINT: return "Point";
    case TG_LINESTRING: return "LineString";
    case TG_POLYGON: return "Polygon";
    case TG_MULTIPOINT: return "MultiPoint";
    case TG_MULTILINESTRING: return "MultiLineString";
    case TG_MULTIPOLYGON: return "MultiPolygon";
    case TG_GEOMETRYCOLLECTION: return "GeometryCollection";
    default: return "Unknown";
    }
}

/////////////////////////////////////////////
// Formats -- GeoJSON, WKT, WKB, HEX
/////////////////////////////////////////////

// maximum depth for all recursive input formats such as wkt, wkb, and json.
#define MAXDEPTH 1024

#ifndef JSON_MAXDEPTH
#define JSON_MAXDEPTH MAXDEPTH
#endif

#ifdef TG_NOAMALGA

#include "deps/json.h"

#else

#define JSON_STATIC

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#endif

// BEGIN json.c
// https://github.com/tidwall/json.c
//
// Copyright 2023 Joshua J Baker. All rights reserved.
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file.
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#ifndef JSON_STATIC
#include "json.h"
#else
enum json_type { 
    JSON_NULL,
    JSON_FALSE,
    JSON_NUMBER,
    JSON_STRING,
    JSON_TRUE,
    JSON_ARRAY,
    JSON_OBJECT,
};
struct json { void *priv[4]; };

struct json_valid {
    bool valid;
    size_t pos;
};

#define JSON_EXTERN static
#endif

#ifndef JSON_EXTERN
#define JSON_EXTERN
#endif

#ifndef JSON_MAXDEPTH
#define JSON_MAXDEPTH 1024
#endif

struct vutf8res { int n; uint32_t cp; };

// parse and validate a single utf8 codepoint.
// The first byte has already been checked from the vstring function.
static inline struct vutf8res vutf8(const uint8_t data[], int64_t len) {
    uint32_t cp;
    int n = 0;
    if (data[0]>>4 == 14) {
        if (len < 3) goto fail;
        if (((data[1]>>6)|(data[2]>>6<<2)) != 10) goto fail;
        cp = ((uint32_t)(data[0]&15)<<12)|((uint32_t)(data[1]&63)<<6)|
             ((uint32_t)(data[2]&63));
        n = 3;
    } else if (data[0]>>3 == 30) {
        if (len < 4) goto fail;
        if (((data[1]>>6)|(data[2]>>6<<2)|(data[3]>>6<<4)) != 42) goto fail;
        cp = ((uint32_t)(data[0]&7)<<18)|((uint32_t)(data[1]&63)<<12)|
             ((uint32_t)(data[2]&63)<<6)|((uint32_t)(data[3]&63));
        n = 4;
    } else if (data[0]>>5 == 6) {
        if (len < 2) goto fail;
        if (data[1]>>6 != 2) goto fail;
        cp = ((uint32_t)(data[0]&31)<<6)|((uint32_t)(data[1]&63));
        n = 2;
    } else {
        goto fail;
    }
    if (cp < 128) goto fail; // don't allow multibyte ascii characters
    if (cp >= 0x10FFFF) goto fail; // restricted to utf-16
    if (cp >= 0xD800 && cp <= 0xDFFF) goto fail; // needs surrogate pairs
    return (struct vutf8res) { .n = n, .cp = cp };
fail:
    return (struct vutf8res) { 0 };
}

static inline int64_t vesc(const uint8_t *json, int64_t jlen, int64_t i) {
    // The first byte has already been checked from the vstring function.
    i += 1;
    if (i == jlen) return -(i+1);
    switch (json[i]) {
    case '"': case '\\': case '/': case 'b': case 'f': case 'n': 
    case 'r': case 't': return i+1;
    case 'u':
        for (int j = 0; j < 4; j++) {
            i++;
            if (i == jlen) return -(i+1);
            if (!((json[i] >= '0' && json[i] <= '9') ||
                  (json[i] >= 'a' && json[i] <= 'f') ||
                  (json[i] >= 'A' && json[i] <= 'F'))) return -(i+1);
        }
        return i+1;
    }
    return -(i+1);
}

#ifndef ludo
#define ludo
#define ludo1(i,f) f; i++;
#define ludo2(i,f) ludo1(i,f); ludo1(i,f);
#define ludo4(i,f) ludo2(i,f); ludo2(i,f);
#define ludo8(i,f) ludo4(i,f); ludo4(i,f);
#define ludo16(i,f) ludo8(i,f); ludo8(i,f);
#define ludo32(i,f) ludo16(i,f); ludo16(i,f);
#define ludo64(i,f) ludo32(i,f); ludo32(i,f);
#define for1(i,n,f) while(i+1<=(n)) { ludo1(i,f); }
#define for2(i,n,f) while(i+2<=(n)) { ludo2(i,f); } for1(i,n,f);
#define for4(i,n,f) while(i+4<=(n)) { ludo4(i,f); } for1(i,n,f);
#define for8(i,n,f) while(i+8<=(n)) { ludo8(i,f); } for1(i,n,f);
#define for16(i,n,f) while(i+16<=(n)) { ludo16(i,f); } for1(i,n,f);
#define for32(i,n,f) while(i+32<=(n)) { ludo32(i,f); } for1(i,n,f);
#define for64(i,n,f) while(i+64<=(n)) { ludo64(i,f); } for1(i,n,f);
#endif

static const uint8_t strtoksu[256] = {
    6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
    0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
#ifndef JSON_NOVALIDATEUTF8
    6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
    6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
    3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
    4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,5,6,6,6,6,6,6,6,6,
    //0=ascii, 1=quote, 2=escape, 3=utf82, 4=utf83, 5=utf84, 6=error
#endif
};

static int64_t vstring(const uint8_t *json, int64_t jlen, int64_t i) {
    while (1) {
        for8(i, jlen, { if (strtoksu[json[i]]) goto tok; })
        break;
    tok:
        if (json[i] == '"') {
            return i+1;
#ifndef JSON_NOVALIDATEUTF8
        } else if (json[i] > 127) {
            struct vutf8res res = vutf8(json+i, jlen-i);
            if (res.n == 0) break;
            i += res.n;
#endif
        } else if (json[i] == '\\') {
            if ((i = vesc(json, jlen, i)) < 0) break;
        } else {
            break;
        }
    } 
    return -(i+1);
}

static int64_t vnumber(const uint8_t *data, int64_t dlen, int64_t i) {
    i--;
    // sign
    if (data[i] == '-') {
        i++;
        if (i == dlen) return -(i+1);
        if (data[i] < '0' || data[i] > '9') return -(i+1);
    }
    // int 
    if (data[i] == '0') {
        i++;
    } else {
        for (; i < dlen; i++) {
            if (data[i] >= '0' && data[i] <= '9') continue;
            break;
        }
    }
    // frac
    if (i == dlen) return i;
    if (data[i] == '.') {
        i++;
        if (i == dlen) return -(i+1);
        if (data[i] < '0' || data[i] > '9') return -(i+1);
        i++;
        for (; i < dlen; i++) {
            if (data[i] >= '0' && data[i] <= '9') continue;
            break;
        }
    }
    // exp
    if (i == dlen) return i;
    if (data[i] == 'e' || data[i] == 'E') {
        i++;
        if (i == dlen) return -(i+1);
        if (data[i] == '+' || data[i] == '-') i++;
        if (i == dlen) return -(i+1);
        if (data[i] < '0' || data[i] > '9') return -(i+1);
        i++;
        for (; i < dlen; i++) {
            if (data[i] >= '0' && data[i] <= '9') continue;
            break;
        }
    }
    return i;
}

static int64_t vnull(const uint8_t *data, int64_t dlen, int64_t i) {
    return i+3 <= dlen && data[i] == 'u' && data[i+1] == 'l' &&
        data[i+2] == 'l' ? i+3 : -(i+1);
}

static int64_t vtrue(const uint8_t *data, int64_t dlen, int64_t i) {
    return i+3 <= dlen && data[i] == 'r' && data[i+1] == 'u' &&
        data[i+2] == 'e' ? i+3 : -(i+1);
}

static int64_t vfalse(const uint8_t *data, int64_t dlen, int64_t i) {
    return i+4 <= dlen && data[i] == 'a' && data[i+1] == 'l' &&
        data[i+2] == 's' && data[i+3] == 'e' ? i+4 : -(i+1);
}

static int64_t vcolon(const uint8_t *json, int64_t len, int64_t i) {
    if (i == len) return -(i+1);
    if (json[i] == ':') return i+1;
    do {
        switch (json[i]) {
        case ' ': case '\t': case '\n': case '\r': continue;
        case ':': return i+1;
        default: return -(i+1);
        }
    } while (++i < len);
    return -(i+1);
}

static int64_t vcomma(const uint8_t *json, int64_t len, int64_t i, uint8_t end)
{
    if (i == len) return -(i+1);
    if (json[i] == ',') return i;
    do {
        switch (json[i]) {
        case ' ': case '\t': case '\n': case '\r': continue;
        case ',': return i;
        default: return json[i] == end ? i : -(i+1);
        }
    } while (++i < len);
    return -(i+1);
}

static int64_t vany(const uint8_t *data, int64_t dlen, int64_t i, int depth);

static int64_t varray(const uint8_t *data, int64_t dlen, int64_t i, int depth) {
    for (; i < dlen; i++) {
        switch (data[i]) {
        case ' ': case '\t': case '\n': case '\r': continue;
        case ']': return i+1;
        default:
            for (; i < dlen; i++) {
                if ((i = vany(data, dlen, i, depth+1)) < 0) return i;
                if ((i = vcomma(data, dlen, i, ']')) < 0) return i;
                if (data[i] == ']') return i+1;
            }
        }
    }
    return -(i+1);
}

static int64_t vkey(const uint8_t *json, int64_t len, int64_t i) {
    for16(i, len, { if (strtoksu[json[i]]) goto tok; })
    return -(i+1);
tok:
    if (json[i] == '"') return i+1;
    return vstring(json, len, i);
}

static int64_t vobject(const uint8_t *data, int64_t dlen, int64_t i, int depth)
{
    for (; i < dlen; i++) {
        switch (data[i]) {
        case '"':
        key:
            if ((i = vkey(data, dlen, i+1)) < 0) return i;
            if ((i = vcolon(data, dlen, i)) < 0) return i;
            if ((i = vany(data, dlen, i, depth+1)) < 0) return i;
            if ((i = vcomma(data, dlen, i, '}')) < 0) return i;
            if (data[i] == '}') return i+1;
            i++;
            for (; i < dlen; i++) {
                switch (data[i]) {
                case ' ': case '\t': case '\n': case '\r': continue;
                case '"': goto key;
                default: return -(i+1);
                }
            }
            return -(i+1);
        case ' ': case '\t': case '\n': case '\r': continue;
        case '}': return i+1;
        default:
            return -(i+1);
        }
    }
    return -(i+1);
}

static int64_t vany(const uint8_t *data, int64_t dlen, int64_t i, int depth) {
    if (depth > JSON_MAXDEPTH) return -(i+1);
    for (; i < dlen; i++) {
        switch (data[i]) {
        case ' ': case '\t': case '\n': case '\r': continue;
        case '{': return vobject(data, dlen, i+1, depth);
        case '[': return varray(data, dlen, i+1, depth);
        case '"': return vstring(data, dlen, i+1);
        case 't': return vtrue(data, dlen, i+1);
        case 'f': return vfalse(data, dlen, i+1);
        case 'n': return vnull(data, dlen, i+1);
        case '-': case '0': case '1': case '2': case '3': case '4': 
        case '5': case '6': case '7': case '8': case '9':
            return vnumber(data, dlen, i+1);
        }
        break;
    }
    return -(i+1);
}

static int64_t vpayload(const uint8_t *data, int64_t dlen, int64_t i) {
    for (; i < dlen; i++) {
        switch (data[i]) {
        case ' ': case '\t': case '\n': case '\r': continue;
        default:
            if ((i = vany(data, dlen, i, 1)) < 0) return i;
            for (; i < dlen; i++) {
                switch (data[i]) {
                case ' ': case '\t': case '\n': case '\r': continue;
                default: return -(i+1);
                }
            }
            return i;
        }
    }
    return -(i+1);
}

JSON_EXTERN
struct json_valid json_validn_ex(const char *json_str, size_t len, int opts) {
    (void)opts; // for future use
    int64_t ilen = len;
    if (ilen < 0) return (struct json_valid) { 0 };
    int64_t pos = vpayload((uint8_t*)json_str, len, 0);
    if (pos > 0) return (struct json_valid) { .valid = true };
    return (struct json_valid) { .pos = (-pos)-1 };
}

JSON_EXTERN struct json_valid json_valid_ex(const char *json_str, int opts) {
    return json_validn_ex(json_str, json_str?strlen(json_str):0, opts);
}

JSON_EXTERN bool json_validn(const char *json_str, size_t len) {
    return json_validn_ex(json_str, len, 0).valid;
}

JSON_EXTERN bool json_valid(const char *json_str) {
    return json_validn(json_str, json_str?strlen(json_str):0);
}

// don't changes these flags without changing the numtoks table too.
enum iflags { IESC = 1, IDOT = 2, ISCI = 4, ISIGN = 8 };

#define jmake(info, raw, end, len) ((struct json) { .priv = { \
    (void*)(uintptr_t)(info), (void*)(uintptr_t)(raw), \
    (void*)(uintptr_t)(end), (void*)(uintptr_t)(len) } })
#define jinfo(json) ((int)(uintptr_t)((json).priv[0]))
#define jraw(json) ((uint8_t*)(uintptr_t)((json).priv[1]))
#define jend(json) ((uint8_t*)(uintptr_t)((json).priv[2]))
#define jlen(json) ((size_t)(uintptr_t)((json).priv[3]))

static const uint8_t strtoksa[256] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,
};

static inline size_t count_string(uint8_t *raw, uint8_t *end, int *infoout) {
    size_t len = end-raw;
    size_t i = 1;
    int info = 0;
    bool e = false;
    while (1) {
        for8(i, len, {
            if (strtoksa[raw[i]]) goto tok;
            e = false;
        });
        break;
    tok:
        if (raw[i] == '"') {
            i++;
            if (!e) {
                break;
            }
            e = false;
            continue;
        }
        if (raw[i] == '\\') {
            info |= IESC;
            e = !e;
        }
        i++;
    }
    *infoout = info;
    return i;
}

static struct json take_string(uint8_t *raw, uint8_t *end) {
    int info = 0;
    size_t i = count_string(raw, end, &info);
    return jmake(info, raw, end, i);
}

static const uint8_t numtoks[256] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,1,0,1,3,0,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,
    0,0,0,0,0,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    // don't changes these flags without changing enum iflags too.
};

static struct json take_number(uint8_t *raw, uint8_t *end) {
    int64_t len = end-raw;
    int info = raw[0] == '-' ? ISIGN : 0;
    int64_t i = 1;
    for16(i, len, {
        if (!numtoks[raw[i]]) goto done;
        info |= (numtoks[raw[i]]-1);
    });
done:
    return jmake(info, raw, end, i);
}

static const uint8_t nesttoks[256] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,0,2,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,0,2,0,0,
};

static size_t count_nested(uint8_t *raw, uint8_t *end) {
    size_t len = end-raw;
    size_t i = 1;
    int depth = 1;
    int kind = 0;
    if (i >= len) return i;
    while (depth) {
        for16(i, len, { if (nesttoks[raw[i]]) goto tok0; });
        break;
    tok0:
        kind = nesttoks[raw[i]];
        i++;
        if (kind-1) {
            depth += kind-3;
        } else {
            while (1) {
                for16(i, len, { if (raw[i]=='"') goto tok1; });
                break;
            tok1:
                i++;
                if (raw[i-2] == '\\') {
                    size_t j = i-3;
                    size_t e = 1;
                    while (j > 0 && raw[j] == '\\') {
                        e = (e+1)&1;
                        j--;
                    }
                    if (e) continue;
                }
                break;
            }
        }
    }
    return i;
}

static struct json take_literal(uint8_t *raw, uint8_t *end, size_t litlen) {
    size_t rlen = end-raw;
    return jmake(0, raw, end, rlen < litlen ? rlen : litlen);
}

static struct json peek_any(uint8_t *raw, uint8_t *end) {
    while (raw < end) {
        switch (*raw){
        case '}': case ']': return (struct json){ 0 };
        case '{': case '[': return jmake(0, raw, end, 0);
        case '"': return take_string(raw, end);
        case 'n': return take_literal(raw, end, 4);
        case 't': return take_literal(raw, end, 4);
        case 'f': return take_literal(raw, end, 5);
        case '-': case '0': case '1': case '2': case '3': case '4': case '5': 
        case '6': case '7': case '8': case '9': return take_number(raw, end);
        }
        raw++;
    }
    return (struct json){ 0 };
}

JSON_EXTERN struct json json_first(struct json json) {
    uint8_t *raw = jraw(json);
    uint8_t *end = jend(json);
    if (end <= raw || (*raw != '{' &&  *raw != '[')) return (struct json){0};
    return peek_any(raw+1, end);
}

JSON_EXTERN struct json json_next(struct json json) {
    uint8_t *raw = jraw(json);
    uint8_t *end = jend(json);
    if (end <= raw) return (struct json){ 0 };
    raw += jlen(json) == 0 ? count_nested(raw, end): jlen(json);
    return peek_any(raw, end);
}

JSON_EXTERN struct json json_parsen(const char *json_str, size_t len) {
    if (len > 0 && (json_str[0] == '[' || json_str[0] == '{')) {
        return jmake(0, json_str, json_str+len, 0);
    }
    if (len == 0) return (struct json){ 0 };
    return peek_any((uint8_t*)json_str, (uint8_t*)json_str+len);
}

JSON_EXTERN struct json json_parse(const char *json_str) {
    return json_parsen(json_str, json_str?strlen(json_str):0);
}

JSON_EXTERN bool json_exists(struct json json) {
    return jraw(json) && jend(json);
}

JSON_EXTERN const char *json_raw(struct json json) {
    return (char*)jraw(json);
}

JSON_EXTERN size_t json_raw_length(struct json json) {
    if (jlen(json)) return jlen(json);
    if (jraw(json) < jend(json)) return count_nested(jraw(json), jend(json));
    return 0;
}

static const uint8_t typetoks[256] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,3,0,0,0,0,0,0,0,0,0,0,2,0,0,2,2,2,2,2,2,2,2,2,2,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,0,0,0,0,
    0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,6,0,0,0,0,
};

JSON_EXTERN enum json_type json_type(struct json json) {
    return jraw(json) < jend(json) ? typetoks[*jraw(json)] : JSON_NULL;
}

JSON_EXTERN struct json json_ensure(struct json json) {
    return jmake(jinfo(json), jraw(json), jend(json), json_raw_length(json));
}

static int strcmpn(const char *a, size_t alen, const char *b, size_t blen) {
    size_t n = alen < blen ? alen : blen;
    int cmp = strncmp(a, b, n);
    if (cmp == 0) {
        cmp = alen < blen ? -1 : alen > blen ? 1 : 0;
    }
    return cmp;
}

static const uint8_t hextoks[256] = { 
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,2,3,4,5,6,7,8,9,0,0,0,0,0,0,
    0,10,11,12,13,14,15,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,10,11,12,13,14,15,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

static uint32_t decode_hex(const uint8_t *str) {
    return (((int)hextoks[str[0]])<<12) | (((int)hextoks[str[1]])<<8) |
           (((int)hextoks[str[2]])<<4) | (((int)hextoks[str[3]])<<0);
}

static bool is_surrogate(uint32_t cp) {
    return cp > 55296 && cp < 57344;
}

static uint32_t decode_codepoint(uint32_t cp1, uint32_t cp2) {
    return cp1 > 55296  && cp1 < 56320 && cp2 > 56320 && cp2 < 57344 ?
        ((cp1 - 55296) << 10) | ((cp2 - 56320) + 65536) :
        65533;
}

static inline int encode_codepoint(uint8_t dst[], uint32_t cp) {
    if (cp < 128) {
        dst[0] = cp;
        return 1;
    } else if (cp < 2048) {
        dst[0] = 192 | (cp >> 6);
        dst[1] = 128 | (cp & 63);
        return 2;
    } else if (cp > 1114111 || is_surrogate(cp)) {
        cp = 65533; // error codepoint
    }
    if (cp < 65536) {
        dst[0] = 224 | (cp >> 12);
        dst[1] = 128 | ((cp >> 6) & 63);
        dst[2] = 128 | (cp & 63);
        return 3;
    }
    dst[0] = 240 | (cp >> 18);
    dst[1] = 128 | ((cp >> 12) & 63);
    dst[2] = 128 | ((cp >> 6) & 63);
    dst[3] = 128 | (cp & 63);
    return 4;
}

// for_each_utf8 iterates over each UTF-8 bytes in jstr, unescaping along the
// way. 'f' is a loop expression that will make available the 'ch' char which 
// is just a single byte in a UTF-8 series.
#define for_each_utf8(jstr, len, f) { \
    size_t nn = (len); \
    int ch = 0; \
    (void)ch; \
    for (size_t ii = 0; ii < nn; ii++) { \
        if ((jstr)[ii] != '\\') { \
            ch = (jstr)[ii]; \
            if (1) f \
            continue; \
        }; \
        ii++; \
        if (ii == nn) break; \
        switch  ((jstr)[ii]) { \
        case '\\': ch = '\\'; break; \
        case '/' : ch = '/';  break; \
        case 'b' : ch = '\b'; break; \
        case 'f' : ch = '\f'; break; \
        case 'n' : ch = '\n'; break; \
        case 'r' : ch = '\r'; break; \
        case 't' : ch = '\t'; break; \
        case '"' : ch = '"';  break; \
        case 'u' : \
            if (ii+5 > nn) { nn = 0; continue; }; \
            uint32_t cp = decode_hex((jstr)+ii+1); \
            ii += 5; \
            if (is_surrogate(cp)) { \
                if (nn-ii >= 6 && (jstr)[ii] == '\\' && (jstr)[ii+1] == 'u') { \
                    cp = decode_codepoint(cp, decode_hex((jstr)+ii+2)); \
                    ii += 6; \
                } \
            } \
            uint8_t _bytes[4]; \
            int _n = encode_codepoint(_bytes, cp); \
            for (int _j = 0; _j < _n; _j++) { \
                ch = _bytes[_j]; \
                if (1) f \
            } \
            ii--; \
            continue; \
        default: \
            continue; \
        }; \
        if (1) f \
    } \
}

JSON_EXTERN 
int json_raw_comparen(struct json json, const char *str, size_t len) {
    char *raw = (char*)jraw(json);
    if (!raw) raw = "";
    size_t rlen = json_raw_length(json);
    return strcmpn(raw, rlen, str, len);
}

JSON_EXTERN int json_raw_compare(struct json json, const char *str) {
    return json_raw_comparen(json, str, strlen(str));
}

JSON_EXTERN size_t json_string_length(struct json json) {
    size_t len = json_raw_length(json);
    if (json_type(json) != JSON_STRING) {
        return len;
    }
    len = len < 2 ? 0 : len - 2;
    if ((jinfo(json)&IESC) != IESC) {
        return len;
    }
    uint8_t *raw = jraw(json)+1;
    size_t count = 0;
    for_each_utf8(raw, len, { count++; });
    return count;
}

JSON_EXTERN 
int json_string_comparen(struct json json, const char *str, size_t slen) {
    if (json_type(json) != JSON_STRING) {
        return json_raw_comparen(json, str, slen);
    }
    uint8_t *raw = jraw(json);
    size_t rlen = json_raw_length(json);
    raw++;
    rlen = rlen < 2 ? 0 : rlen - 2;
    if ((jinfo(json)&IESC) != IESC) {
        return strcmpn((char*)raw, rlen, str, slen);
    }
    int cmp = 0;
    uint8_t *sp = (uint8_t*)(str ? str : "");
    for_each_utf8(raw, rlen, {
        if (!*sp || ch > *sp) {
            cmp = 1;
            goto done;
        } else if (ch < *sp) {
            cmp = -1;
            goto done;
        }
        sp++;
    });
done:
    if (cmp == 0 && *sp) cmp = -1;
    return cmp;
}

JSON_EXTERN 
int json_string_compare(struct json json, const char *str) {
    return json_string_comparen(json, str, str?strlen(str):0);
}

JSON_EXTERN size_t json_string_copy(struct json json, char *str, size_t n) {
    size_t len = json_raw_length(json);
    uint8_t *raw = jraw(json);
    bool isjsonstr = json_type(json) == JSON_STRING;
    bool isesc = false;
    if (isjsonstr) {
        raw++;
        len = len < 2 ? 0 : len - 2;
        isesc = (jinfo(json)&IESC) == IESC;
    }
    if (!isesc) {
        if (n == 0) return len;
        n = n-1 < len ? n-1 : len;
        memcpy(str, raw, n);
        str[n] = '\0';
        return len;
    }
    size_t count = 0;
    for_each_utf8(raw, len, {
        if (count < n) str[count] = ch;
        count++;
    });
    if (n > count) str[count] = '\0';
    else if (n > 0) str[n-1] = '\0';
    return count;
}

JSON_EXTERN size_t json_array_count(struct json json) {
    size_t count = 0;
    if (json_type(json) == JSON_ARRAY) {
        json = json_first(json);
        while (json_exists(json)) {
            count++;
            json = json_next(json);
        }
    }
    return count;
}

JSON_EXTERN struct json json_array_get(struct json json, size_t index) {
    if (json_type(json) == JSON_ARRAY) {
        json = json_first(json);
        while (json_exists(json)) {
            if (index == 0) return json;
            json = json_next(json);
            index--;
        }
    }
    return (struct json) { 0 };
}

JSON_EXTERN
struct json json_object_getn(struct json json, const char *key, size_t len) {
    if (json_type(json) == JSON_OBJECT) {
        json = json_first(json);
        while (json_exists(json)) {
            if (json_string_comparen(json, key, len) == 0) {
                return json_next(json);
            }
            json = json_next(json_next(json));
        }
    }
    return (struct json) { 0 };
}

JSON_EXTERN struct json json_object_get(struct json json, const char *key) {
    return json_object_getn(json, key, key?strlen(key):0);
}

static double stod(const uint8_t *str, size_t len, char *buf) {
    memcpy(buf, str, len);
    buf[len] = '\0';
    char *ptr;
    double x = strtod(buf, &ptr);
    return (size_t)(ptr-buf) == len ? x : 0;
}

static double parse_double_big(const uint8_t *str, size_t len) {
    char buf[512];
    if (len >= sizeof(buf)) return 0;
    return stod(str, len, buf);
}

static double parse_double(const uint8_t *str, size_t len) {
    char buf[32];
    if (len >= sizeof(buf)) return parse_double_big(str, len);
    return stod(str, len, buf);
}

static int64_t parse_int64(const uint8_t *s, size_t len) {
    char buf[21];
    double y;
    if (len == 0) return 0;
    if (len < sizeof(buf) && sizeof(long long) == sizeof(int64_t)) {
        memcpy(buf, s, len);
        buf[len] = '\0';
        char *ptr = NULL;
        int64_t x = strtoll(buf, &ptr, 10);
        if ((size_t)(ptr-buf) == len) return x;
        y = strtod(buf, &ptr);
        if ((size_t)(ptr-buf) == len) goto clamp;
    }
    y = parse_double(s, len);
clamp:
    if (y < (double)INT64_MIN) return INT64_MIN;
    if (y > (double)INT64_MAX) return INT64_MAX;
    return y;
}

static uint64_t parse_uint64(const uint8_t *s, size_t len) {
    char buf[21];
    double y;
    if (len == 0) return 0;
    if (len < sizeof(buf) && sizeof(long long) == sizeof(uint64_t) &&
        s[0] != '-')
    {
        memcpy(buf, s, len);
        buf[len] = '\0';
        char *ptr = NULL;
        uint64_t x = strtoull(buf, &ptr, 10);
        if ((size_t)(ptr-buf) == len) return x;
        y = strtod(buf, &ptr);
        if ((size_t)(ptr-buf) == len) goto clamp;
    }
    y = parse_double(s, len);
clamp:
    if (y < 0) return 0;
    if (y > (double)UINT64_MAX) return UINT64_MAX;
    return y;
}

JSON_EXTERN double json_double(struct json json) {
    switch (json_type(json)) {
    case JSON_TRUE:
        return 1;
    case JSON_STRING:
        if (jlen(json) < 3) return 0.0;
        return parse_double(jraw(json)+1, jlen(json)-2);
    case JSON_NUMBER:
        return parse_double(jraw(json), jlen(json));
    default:
        return 0.0;
    }
}

JSON_EXTERN int64_t json_int64(struct json json) {
    switch (json_type(json)) {
    case JSON_TRUE:
        return 1;
    case JSON_STRING:
        if (jlen(json) < 2) return 0;
        return parse_int64(jraw(json)+1, jlen(json)-2);
    case JSON_NUMBER:
        return parse_int64(jraw(json), jlen(json));
    default:
        return 0;
    }
}

JSON_EXTERN int json_int(struct json json) {
    int64_t x = json_int64(json);
    if (x < (int64_t)INT_MIN) return INT_MIN;
    if (x > (int64_t)INT_MAX) return INT_MAX;
    return x;
}

JSON_EXTERN uint64_t json_uint64(struct json json) {
    switch (json_type(json)) {
    case JSON_TRUE:
        return 1;
    case JSON_STRING:
        if (jlen(json) < 2) return 0;
        return parse_uint64(jraw(json)+1, jlen(json)-2);
    case JSON_NUMBER:
        return parse_uint64(jraw(json), jlen(json));
    default:
        return 0;
    }
}

JSON_EXTERN bool json_bool(struct json json) {
    switch (json_type(json)) {
    case JSON_TRUE:
        return true;
    case JSON_NUMBER:
         return json_double(json) != 0.0; 
    case JSON_STRING: {
        char *trues[] = { "1", "t", "T", "true", "TRUE", "True" };
        for (size_t i = 0; i < sizeof(trues)/sizeof(char*); i++) {
            if (json_string_compare(json, trues[i]) == 0) return true;
        }
        return false;
    }
    default:
        return false;
    }
}

struct jesc_buf { 
    uint8_t *esc;
    size_t esclen;
    size_t count;
};

static void jesc_append(struct jesc_buf *buf, uint8_t ch) {
    if (buf->esclen > 1) { 
        *(buf->esc++) = ch;
        buf->esclen--; 
    }
    buf->count++;
}
static void jesc_append2(struct jesc_buf *buf, uint8_t c1, uint8_t c2) {
    jesc_append(buf, c1);
    jesc_append(buf, c2);
}

static const uint8_t hexchars[] = "0123456789abcdef";

static void 
jesc_append_ux(struct jesc_buf *buf, uint8_t c1, uint8_t c2, uint16_t x) {
    jesc_append2(buf, c1, c2);
    jesc_append2(buf, hexchars[x>>12&0xF], hexchars[x>>8&0xF]);
    jesc_append2(buf, hexchars[x>>4&0xF], hexchars[x>>0&0xF]);
}

JSON_EXTERN
size_t json_escapen(const char *str, size_t len, char *esc, size_t n) {
    uint8_t cpbuf[4];
    struct jesc_buf buf  = { .esc = (uint8_t*)esc, .esclen = n };
    jesc_append(&buf, '"');
    for (size_t i = 0; i < len; i++) {
        uint32_t c = (uint8_t)str[i];
        if (c < ' ') {
            switch (c) {
            case '\n': jesc_append2(&buf, '\\', 'n'); break;
            case '\b': jesc_append2(&buf, '\\', 'b'); break;
            case '\f': jesc_append2(&buf, '\\', 'f'); break;
            case '\r': jesc_append2(&buf, '\\', 'r'); break;
            case '\t': jesc_append2(&buf, '\\', 't'); break;
            default: jesc_append_ux(&buf, '\\', 'u', c);
            }
        } else if (c == '>' || c == '<' || c == '&') {
            // make web safe
            jesc_append_ux(&buf, '\\', 'u', c);
        } else if (c == '\\') {
            jesc_append2(&buf, '\\', '\\');
        } else if (c == '"') {
            jesc_append2(&buf, '\\', '"');
        } else if (c > 127) {
            struct vutf8res res = vutf8((uint8_t*)(str+i), len-i);
            if (res.n == 0) {
                res.n = 1;
                res.cp = 0xfffd;
            }
            int cpn = encode_codepoint(cpbuf, res.cp);
            for (int j = 0; j < cpn; j++) {
                jesc_append(&buf, cpbuf[j]);
            }
            i = i + res.n - 1;
        } else {
            jesc_append(&buf, str[i]);
        }
    }
    jesc_append(&buf, '"');
    if (buf.esclen > 0) {
        // add to null terminator
        *(buf.esc++) = '\0';
        buf.esclen--;
    }
    return buf.count;
}

JSON_EXTERN size_t json_escape(const char *str, char *esc, size_t n) {
    return json_escapen(str, str?strlen(str):0, esc, n);
}

JSON_EXTERN
struct json json_getn(const char *json_str, size_t len, const char *path) {
    if (!path) return (struct json) { 0 };
    struct json json = json_parsen(json_str, len);
    int i = 0;
    bool end = false;
    char *p = (char*)path;
    for (; !end && json_exists(json); i++) {
        // get the next component
        const char *key = p;
        while (*p && *p != '.') p++;
        size_t klen = p-key;
        if (*p == '.') p++;
        else if (!*p) end = true;
        enum json_type type = json_type(json);
        if (type == JSON_OBJECT) {
            json = json_object_getn(json, key, klen);
        } else if (type == JSON_ARRAY) {
            if (klen == 0) { i = 0; break; }
            char *end;
            size_t index = strtol(key, &end, 10);
            if (*end && *end != '.') { i = 0; break; }
            json = json_array_get(json, index);
        } else {
            i = 0; 
            break;
        }
    }
    return i == 0 ? (struct json) { 0 } : json;
}

JSON_EXTERN struct json json_get(const char *json_str, const char *path) {
    return json_getn(json_str, json_str?strlen(json_str):0, path);
}

JSON_EXTERN bool json_string_is_escaped(struct json json) {
    return (jinfo(json)&IESC) == IESC;
}
// END json.c

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#endif

static struct tg_geom *make_parse_error(const char *format, ...) {
    char *error = NULL;
    va_list args;
    va_start(args, format);
    int n = vsnprintf(NULL, 0, format, args);
    va_end(args);
    if (n >= 0) {
        error = tg_malloc(n+1);
        if (error) {
            va_start(args, format);
            vsnprintf(error, n+1, format, args);
            va_end(args);
        }
    }
    if (!error) return NULL;
    struct tg_geom *geom = geom_new_empty(TG_POINT);
    if (!geom) {
        tg_free(error);
        return NULL;
    }
    geom->head.flags |= IS_ERROR;
    geom->error = error;
    return geom;
}

/// Return a parsing error.
///
/// Parsing functions, such as tg_parse_geojson(), may fail due to invalid
/// input data.
///
/// It's important to **always** check for errors after parsing.
///
/// **Example**
///
/// ```
/// struct tg_geom *geom = tg_parse_geojson(input);
/// if (tg_geom_error(geom)) {
///     // The parsing failed due to invalid input or out of memory.
///
///     // Get the error message.
///     const char *err = tg_geom_error(geom);
///
///     // Do something with the error, such as log it.
///     printf("[err] %s\n", err);
///
///     // Make sure to free the error geom and it's resources.
///     tg_geom_free(geom);
///
///     // !!
///     // DO NOT use the return value of tg_geom_error() after calling 
///     // tg_geom_free(). If you need to hang onto the error for a longer
///     // period of time then you must copy it before freeing.
///     // !!
///
///     return;
/// } else {
///     // ... Parsing succeeded 
/// }
/// ```
///
/// @return A string describing the error
/// @return NULL if there was no error
/// @see tg_geom_free()
/// @see tg_parse_geojson()
/// @see tg_parse_wkt()
/// @see tg_parse_wkb()
/// @see tg_parse_hex()
/// @see GeometryParsing
const char *tg_geom_error(const struct tg_geom *geom) {
    if (!geom) return "no memory";
    return (geom->head.flags&IS_ERROR) == IS_ERROR ? geom->error : NULL;
}

static bool buf_append_json_pair(struct buf *buf, struct json key, 
    struct json val)
{
    size_t len = buf->len;
    if (!buf_append_byte(buf, buf->len == 0 ? '{' : ',') ||
        !buf_append_bytes(buf, (uint8_t*)json_raw(key), json_raw_length(key)) || 
        !buf_append_byte(buf, ':') || 
        !buf_append_bytes(buf, (uint8_t*)json_raw(val), json_raw_length(val)))
    {
        buf->len = len;
        return false;
    }
    return true;
}

// returns an error message constant if there's an error. Do not free.
static const char *take_basic_geojson(struct json json, 
    const char *target_name, struct json *targetout, 
    enum flags *flagsout, char **extraout, bool *okout
) {
    enum flags flags = 0;
    bool is_feat = strcmp(target_name, "geometry") == 0;
    const char *err = NULL;
    bool ok = false;
    bool has_props = false;
    bool has_id = false;
    struct buf extra = { 0 };
    struct json target = { 0 };
    struct json key = json_first(json);
    struct json val = json_next(key);
    while (json_exists(key)) {
        if (json_string_compare(key, "type") == 0) {
            // .. do nothing, the caller should already know its type ...
        } else if (json_string_compare(key, target_name) == 0) {
            val = json_ensure(val);
            target = val;
        } else {
            bool skip_val = false;
            if (is_feat) {
                if (json_string_compare(key, "properties") == 0) {
                    if (!has_props) {
                        if (json_type(val) == JSON_NULL) {
                            flags |= HAS_NULL_PROPS;
                            skip_val = true;
                        } else if (json_type(val) == JSON_OBJECT) {
                            skip_val = !json_exists(json_first(val));
                        } else {
                            err = "'properties' must be an object or null";
                            goto fail;
                        }
                    } else {
                        skip_val = true;
                    }
                    has_props = true;
                } else if (json_string_compare(key, "id") == 0) {
                    if (has_id) {
                        skip_val = true;
                    } else {
                        if (json_type(val) != JSON_STRING && 
                            json_type(val) != JSON_NUMBER)
                        {
                            err = "'id' must be a string or number";
                            goto fail;
                        }
                    }
                    has_id = true;
                }
            }
            if (!skip_val) {
                if (!buf_append_json_pair(&extra, key, val)) goto fail;
            }
        }
        key = json_next(val);
        val = json_next(key);
    }
    if (!json_exists(target)) {
        if (strcmp(target_name, "geometry") == 0) {
            err = "missing 'geometry'";
        } else if (strcmp(target_name, "geometries") == 0) {
            err = "missing 'geometries'";
        } else if (strcmp(target_name, "features") == 0) {
            err = "missing 'features'";
        } else { // "coordinates"
            err = "missing 'coordinates'";
        }
        goto fail;
    }
    enum json_type ttype = json_type(target);
    if (is_feat) {
        if (ttype != JSON_OBJECT) {
            if (ttype == JSON_NULL) {
                // unlocated
                // https://www.rfc-editor.org/rfc/rfc7946#section-3.2
                flags |= IS_EMPTY;
                flags |= IS_UNLOCATED;
            } else {
                err = "'geometry' must be an object or null";
                goto fail;
            }
        }
        #ifdef GEOJSON_REQ_PROPERTIES
        if (!has_props) {
            err = "missing 'properties'";
            goto fail;
        }
        #endif
    } else if (ttype != JSON_ARRAY) {
        if (strcmp(target_name, "geometries") == 0) {
            err = "'geometries' must be an array";
        } else if (strcmp(target_name, "features") == 0) {
            err = "'features' must be an array";
        } else { // "coordinates"
            err = "'coordinates' must be an array";
        }
        goto fail;
    } else if (!json_exists(json_first(target))) {
        // null object
        // https://www.rfc-editor.org/rfc/rfc7946#section-3.1
        flags |= IS_EMPTY;
    }
    if (extra.len > 0) {
        if (!buf_append_byte(&extra, '}')) goto fail;
        if (!buf_append_byte(&extra, '\0')) goto fail;
        if (!buf_trunc(&extra)) goto fail;
    }
    ok = true;
fail:
    if (!ok) {
        if (extra.data) tg_free(extra.data);
        *extraout = NULL;
    } else {
        *extraout = (char*)extra.data;
    }
    *targetout = target;
    *okout = ok;
    *flagsout = flags;
    return err;
}

#define def_vec(name, type, append_name, start_cap) \
name { \
    type *data; \
    size_t len; \
    size_t cap; \
}; \
static bool append_name(name *vec, type f) { \
    if (vec->len == vec->cap) { \
        size_t cap = vec->cap; \
        cap = grow_cap(cap, start_cap); \
        type *data = tg_realloc(vec->data, cap*sizeof(type)); \
        if (!data) return false; \
        vec->data = data; \
        vec->cap = cap; \
    } \
    vec->data[vec->len++] = f; \
    return true; \
} \

// some vectors are better than no vectors, i guess.
def_vec(struct dvec, double,          dvec_append, 8)
def_vec(struct rvec, struct tg_ring*, rvec_append, 1)
def_vec(struct lvec, struct tg_line*, lvec_append, 1)
def_vec(struct pvec, struct tg_poly*, pvec_append, 1)
def_vec(struct gvec, struct tg_geom*, gvec_append, 1)

#define PARSE_GEOJSON_BASIC_HEAD(target_name) \
    struct tg_geom *gerr = NULL; \
    struct tg_geom *geom = NULL; \
    struct json target; \
    enum flags flags; \
    char *extra; \
    bool ok; \
    const char *err_ = take_basic_geojson(json, target_name, \
        &target, &flags, &extra, &ok); \
    if (!ok) { \
        gerr = err_ ? make_parse_error("%s", err_) : NULL; \
        goto fail; \
    }

#define PARSE_GEOJSON_BASIC_TAIL(cleanup) \
    goto done; \
done: \
    if (!geom) goto fail; \
    geom->head.flags |= flags; \
    if (extra) geom->xjson = extra; \
    cleanup; \
    return geom; \
fail: \
    tg_geom_free(geom); \
    if (extra) tg_free(extra); \
    cleanup; \
    return gerr;

static const char *err_for_geojson_depth(int depth) {
    if (depth == 1) {
        return "'coordinates' must be an array of positions";
    } else if (depth == 2) {
        return "'coordinates' must be a two deep nested array of positions";
    } else {
        return "'coordinates' must be a three deep nested array of positions";
    }
}

static struct tg_geom *parse_geojson_point(struct json json, bool req_geom,
    enum tg_index ix)
{
    (void)ix;
    PARSE_GEOJSON_BASIC_HEAD("coordinates")
    if ((flags&IS_EMPTY) == IS_EMPTY) {
        geom = tg_geom_new_point_empty();
        if (!geom) goto fail;
        goto done;
    }
    double posn[4];
    int dims = 0;
    struct json val = json_first(target);
    while (json_exists(val)) {
        if (json_type(val) != JSON_NUMBER) {
            gerr = make_parse_error("'coordinates' must only contain numbers");
            goto fail;
        }
        if (dims < 4) {
            // i don't care about more than 4 dimensions
            posn[dims] = json_double(val);
            dims++;
        }
        val = json_next(val);
    }
    if (dims < 2) {
        gerr = make_parse_error("'coordinates' must have two or more numbers");
        goto fail;
    }
    struct tg_point xy = { posn[0], posn[1] };
    double z = posn[2];
    double m = posn[3];
    if (!req_geom && !extra && dims == 2) {
        geom = tg_geom_new_point(xy);
    } else {
        switch (dims) {
        case 2: 
            geom = geom_new(TG_POINT);
            if (!geom) goto fail;
            geom->point = xy;
            break;
        case 3: 
            geom = tg_geom_new_point_z(xy, z);
            break;
        default: 
            geom = tg_geom_new_point_zm(xy, z, m);
            break;
        }
    }
    PARSE_GEOJSON_BASIC_TAIL()
}

static bool check_parse_posns(enum base base, double *posns, int nposns,
    const char **err)
{
    // nposns must be an even number.
    const struct tg_point *points = (void*)posns;
    int npoints = nposns / 2;
    if (base == BASE_LINE) {
        if (npoints < 2) {
            *err = "lines must have two or more positions";
            return false;
        }
    } else if (base == BASE_RING) {
        if (npoints < 3) {
            *err = "rings must have three or more positions";
            return false;
        }
        struct tg_point p0 = points[0];
        struct tg_point p1 = points[npoints-1];
        if (!(p0.x == p1.x && p0.y == p1.y)) {
            *err = "rings must have matching first and last positions";
            return false;
        }
    }
    return true;
}

// return the dims or -1 if error
static int parse_geojson_posns(enum base base, int dims, int depth, 
    struct json coords, struct dvec *posns, struct dvec *xcoords, 
    const char **err)
{
    struct json val0 = json_first(coords);
    while (json_exists(val0)) {
        if (json_type(val0) != JSON_ARRAY) {
            *err = err_for_geojson_depth(depth);
            return -1;
        }
        struct json val1 = json_first(val0);
        double posn[4];
        int pdims = 0;
        while (json_exists(val1)) {
            if (json_type(val1) != JSON_NUMBER) {
                *err = "each element in a position must be a number";
                return -1;
            }
            if (pdims < 4) {
                // we don't care more that 4
                posn[pdims] = json_double(val1);
                pdims++;
            }
            val1 = json_next(val1);
        }
        if (dims == 0) {
            dims = pdims;
        }
        if (pdims < 2) {
            *err = "each position must have two or more numbers";
            return -1;
        } else if (pdims != dims) {
            *err = "each position must have the same number of dimensions";
            return -1;
        }
        if (!dvec_append(posns, posn[0]) || !dvec_append(posns, posn[1])) {
            return -1;
        }
        for (int i = 2; i < dims; i++) {
            if (i >= pdims || !dvec_append(xcoords, posn[i])) return -1;
            // if (!dvec_append(xcoords, i < pdims ? posn[i] : 0)) return -1;
        }
        val0 = json_next(val0);
    }
    if (!check_parse_posns(base, posns->data, posns->len, err)) return -1;
    return dims;
}

static struct tg_geom *parse_geojson_linestring(struct json json, 
    bool req_geom, enum tg_index ix)
{
    struct tg_line *line = NULL;
    struct dvec posns = { 0 };
    struct dvec xcoords = { 0 };
    const char *err = NULL;
    PARSE_GEOJSON_BASIC_HEAD("coordinates")
    if ((flags&IS_EMPTY) == IS_EMPTY) {
        geom = tg_geom_new_linestring_empty();
        goto done;
    }
    int dims = parse_geojson_posns(BASE_LINE, 0, 1, target, &posns, &xcoords,
        &err);
    if (dims == -1) {
        gerr = err ? make_parse_error("%s", err) : NULL;
        goto fail;
    }
    line = tg_line_new_ix((struct tg_point*)posns.data, posns.len / 2, ix);
    if (!line) goto fail;
    if (!req_geom && !extra && dims == 2) {
        geom = tg_geom_new_linestring(line);
    } else {
        switch (dims) {
        case 2: 
            geom = geom_new(TG_LINESTRING);
            if (geom) geom->line = tg_line_clone(line);
            break;
        case 3: 
            geom = tg_geom_new_linestring_z(line, xcoords.data, xcoords.len);
            break;
        default: 
            geom = tg_geom_new_linestring_zm(line, xcoords.data, xcoords.len);
            break;
        }
    }
    PARSE_GEOJSON_BASIC_TAIL({
        if (posns.data) tg_free(posns.data);
        if (xcoords.data) tg_free(xcoords.data);
        if (line) tg_line_free(line);
    })
}

// returns the dims or -1 if error
static int parse_geojson_multi_posns(enum base base, int dims, int depth, 
    struct json coords, struct dvec *posns, struct rvec *rings,  
    struct tg_poly **poly, struct dvec *xcoords, enum tg_index ix,
    const char **err)
{
    struct json val0 = json_first(coords);
    while (json_exists(val0)) {
        if (json_type(val0) != JSON_ARRAY) {
            *err = "'coordinates' must be a nested array";
            return -1;
        }
        posns->len = 0;
        dims = parse_geojson_posns(base, dims, depth, val0, posns, 
            xcoords, err);
        if (dims == -1) return -1;
        struct tg_ring *ring = tg_ring_new_ix((struct tg_point*)posns->data, 
            posns->len / 2, ix);
        if (!ring) return -1;
        if (!rvec_append(rings, ring)) {
            tg_ring_free(ring);
            return -1;
        }
        val0 = json_next(val0);
    }
    if (rings->len == 0) {
        *err = "polygons must have one or more rings";
        return -1;
    }
    *poly = tg_poly_new(rings->data[0], 
        (struct tg_ring const*const*)rings->data+1, rings->len-1);
    if (!*poly) return -1;
    for (size_t i = 0; i < rings->len; i++) {
        tg_ring_free(rings->data[i]);
    }
    rings->len = 0;
    return dims;
}

static struct tg_geom *parse_geojson_polygon(struct json json, bool req_geom,
    enum tg_index ix)
{
    struct tg_poly *poly = NULL;
    struct dvec posns = { 0 };
    struct dvec xcoords = { 0 };
    struct rvec rings = { 0 };
    const char *err = NULL;
    PARSE_GEOJSON_BASIC_HEAD("coordinates")
    if ((flags&IS_EMPTY) == IS_EMPTY) {
        geom = tg_geom_new_polygon_empty();
        goto done;
    }
    int dims = parse_geojson_multi_posns(BASE_RING, 0, 2, target, &posns, 
        &rings, &poly, &xcoords, ix, &err);
    if (dims == -1) {
        gerr = err ? make_parse_error("%s", err) : NULL;
        goto fail;
    }
    if (!req_geom && !extra && dims == 2) {
        geom = tg_geom_new_polygon(poly);
    } else {
        switch (dims) {
        case 2: 
            geom = geom_new(TG_POLYGON);
            if (geom) geom->poly = tg_poly_clone(poly);
            break;
        case 3: 
            geom = tg_geom_new_polygon_z(poly, xcoords.data, xcoords.len);
            break;
        default: 
            geom = tg_geom_new_polygon_zm(poly, xcoords.data, xcoords.len);
            break;
        }
    }
    PARSE_GEOJSON_BASIC_TAIL({
        if (posns.data) tg_free(posns.data);
        if (xcoords.data) tg_free(xcoords.data);
        if (rings.data) {
            for (size_t i = 0; i < rings.len; i++) {
                tg_ring_free(rings.data[i]);
            }
            tg_free(rings.data);
        }
        if (poly) tg_poly_free(poly);
    })
}

static struct tg_geom *parse_geojson_multipoint(struct json json,
    enum tg_index ix)
{
    (void)ix;
    struct dvec posns = { 0 };
    struct dvec xcoords = { 0 };
    const char *err = NULL;
    PARSE_GEOJSON_BASIC_HEAD("coordinates")
    int dims = parse_geojson_posns(BASE_POINT, 0, 1, target, &posns, &xcoords,
        &err);
    if (dims == -1) {
        gerr = err ? make_parse_error("%s", err) : NULL;
        goto fail;
    }
    const struct tg_point *points = (struct tg_point*)posns.data;
    int npoints = posns.len/2;
    switch (dims) {
    case 2: 
        geom = tg_geom_new_multipoint(points, npoints);
        break;
    case 3: 
        geom = tg_geom_new_multipoint_z(points, npoints, 
            xcoords.data, xcoords.len);
        break;
    default: 
        geom = tg_geom_new_multipoint_zm(points, npoints, 
            xcoords.data, xcoords.len);
        break;
    }
    PARSE_GEOJSON_BASIC_TAIL({
        if (posns.data) tg_free(posns.data);
        if (xcoords.data) tg_free(xcoords.data);
    })
}

static struct tg_geom *parse_geojson_multilinestring(struct json json,
    enum tg_index ix)
{
    struct dvec posns = { 0 };
    struct dvec xcoords = { 0 };
    struct lvec lines = { 0 };
    const char *err = NULL;
    int dims = 0;
    PARSE_GEOJSON_BASIC_HEAD("coordinates")
    struct json val0 = json_first(target);
    while (json_exists(val0)) {
        if (json_type(val0) != JSON_ARRAY) {
            gerr = make_parse_error("%s", err_for_geojson_depth(2));
            goto fail;
        }
        posns.len = 0;
        dims = parse_geojson_posns(BASE_LINE, dims, 2, val0, &posns, &xcoords,
            &err);
        if (dims == -1) {
            gerr = err ? make_parse_error("%s", err) : NULL;
            goto fail;
        }
        struct tg_line *line = tg_line_new_ix((struct tg_point*)posns.data, 
            posns.len / 2, ix);
        if (!line) goto fail;
        if (!lvec_append(&lines, line)) {
            tg_line_free(line);
            goto fail;
        }
        val0 = json_next(val0);
    }
    switch (dims) {
    case 2: 
        geom = tg_geom_new_multilinestring(
                (struct tg_line const*const*)lines.data, lines.len);
        break;
    case 3: 
        geom = tg_geom_new_multilinestring_z(
            (struct tg_line const*const*)lines.data, lines.len,
            xcoords.data, xcoords.len);
        break;
    default: 
        geom = tg_geom_new_multilinestring_zm(
            (struct tg_line const*const*)lines.data, lines.len,
            xcoords.data, xcoords.len);
        break;
    }
    PARSE_GEOJSON_BASIC_TAIL({
        if (posns.data) tg_free(posns.data);
        if (xcoords.data) tg_free(xcoords.data);
        if (lines.data) {
            for (size_t i = 0; i < lines.len; i++) {
                tg_line_free(lines.data[i]);
            }
            tg_free(lines.data);
        }
    })
}

static struct tg_geom *parse_geojson_multipolygon(struct json json,
    enum tg_index ix)
{
    struct tg_poly *poly = NULL;
    struct dvec posns = { 0 };
    struct dvec xcoords = { 0 };
    struct rvec rings = { 0 };
    struct pvec polys = { 0 };
    const char *err = NULL;
    int dims = 0;
    PARSE_GEOJSON_BASIC_HEAD("coordinates")
    struct json val0 = json_first(target);
    while (json_exists(val0)) {
        if (json_type(val0) != JSON_ARRAY) {
            gerr = make_parse_error("%s", err_for_geojson_depth(3));
            goto fail;
        }
        posns.len = 0;
        rings.len = 0;
        dims = parse_geojson_multi_posns(BASE_RING, dims, 3, val0, &posns, 
            &rings, &poly, &xcoords, ix, &err);
        if (dims == -1) {
            gerr = err ? make_parse_error("%s", err) : NULL;
            goto fail;
        }
        if (!pvec_append(&polys, poly)) {
            tg_poly_free(poly);
            goto fail;
        }
        val0 = json_next(val0);
    }
    switch (dims) {
    case 2: 
        geom = tg_geom_new_multipolygon(
                (struct tg_poly const*const*)polys.data, polys.len);
        break;
    case 3: 
        geom = tg_geom_new_multipolygon_z(
            (struct tg_poly const*const*)polys.data, polys.len,
            xcoords.data, xcoords.len);
        break;
    default: 
        geom = tg_geom_new_multipolygon_zm(
            (struct tg_poly const*const*)polys.data, polys.len,
            xcoords.data, xcoords.len);
        break;
    }
    PARSE_GEOJSON_BASIC_TAIL({
        if (posns.data) tg_free(posns.data);
        if (xcoords.data) tg_free(xcoords.data);
        if (rings.data) {
            for (size_t i = 0; i < rings.len; i++) {
                tg_ring_free(rings.data[i]);
            }
            tg_free(rings.data);
        }
        if (polys.data) {
            for (size_t i = 0; i < polys.len; i++) {
                tg_poly_free(polys.data[i]);
            }
            tg_free(polys.data);
        }
    })
}

static struct tg_geom *parse_geojson(struct json json, bool req_geom, 
    enum tg_index ix);

static struct tg_geom *parse_geojson_geometrycollection(struct json json,
    enum tg_index ix) 
{
    struct gvec geoms = { 0 };
    PARSE_GEOJSON_BASIC_HEAD("geometries")
    struct json val0 = json_first(target);
    while (json_exists(val0)) {
        struct tg_geom *child = parse_geojson(val0, false, ix);
        if (!child) goto fail;
        if (tg_geom_error(child)) {
            gerr = child; 
            child = NULL;
            goto fail;
        }
        if ((child->head.flags&IS_FEATURE) == IS_FEATURE ||
            (child->head.flags&IS_FEATURE_COL) == IS_FEATURE_COL)
        {
            gerr = make_parse_error("'geometries' must only contain objects "
                    "with the 'type' of Point, LineString, Polygon, "
                    "MultiPoint, MultiLineString, MultiPolygon, or "
                    "GeometryCollection");
            tg_geom_free(child);
            goto fail;
        }
        if (!gvec_append(&geoms, child)) {
            tg_geom_free(child);
            goto fail;
        }
        val0 = json_next(val0);
    }
    geom = tg_geom_new_geometrycollection(
        (struct tg_geom const*const*)geoms.data, geoms.len);
    PARSE_GEOJSON_BASIC_TAIL({
        if (geoms.data) {
            for (size_t i = 0; i < geoms.len; i++) {
                tg_geom_free(geoms.data[i]);
            }
            tg_free(geoms.data);
        }
    })
}

static struct tg_geom *parse_geojson_feature(struct json json, enum tg_index ix)
{
    struct buf combined = { 0 };
    PARSE_GEOJSON_BASIC_HEAD("geometry")
    if ((flags&IS_EMPTY) == IS_EMPTY) {
        geom = tg_geom_new_point_empty();
    } else {
        geom = parse_geojson(target, extra != NULL, ix);
    }
    if (!geom) goto fail;
    if (tg_geom_error(geom)) {
        gerr = geom;
        geom = NULL;
        goto fail;
    }
    if ((geom->head.flags&IS_FEATURE) == IS_FEATURE ||
        (geom->head.flags&IS_FEATURE_COL) == IS_FEATURE_COL)
    {
        gerr = make_parse_error("'geometry' must only contain an object with "
            "the 'type' of Point, LineString, Polygon, MultiPoint, "
            "MultiLineString, MultiPolygon, or GeometryCollection");
        goto fail;
    }
    geom->head.flags |= IS_FEATURE;
    if (geom->head.base == BASE_GEOM && geom->xjson) {
        // combine the two together as '[feature-extra,geometry-extra]'
        size_t xn0 = extra ? strlen(extra) : 0;
        size_t xn1 = strlen(geom->xjson);
        if (!buf_append_byte(&combined, '[') || 
            !buf_append_bytes(&combined, 
                (uint8_t*)(xn0 ? extra : "{}"), (xn0 ? xn0 : 2)) ||
            !buf_append_byte(&combined, ',') || 
            !buf_append_bytes(&combined, (uint8_t*)geom->xjson, xn1) ||
            !buf_append_byte(&combined, ']') ||
            !buf_append_byte(&combined, '\0'))
        { goto fail; }
        if (!buf_trunc(&combined)) goto fail;
        if (geom->xjson) tg_free(geom->xjson);
        geom->xjson = NULL;
        if (extra) tg_free(extra);
        extra = (char*)combined.data;
        combined = (struct buf) { 0 };
    }
        
    PARSE_GEOJSON_BASIC_TAIL({
        if (combined.data) tg_free(combined.data);
    })
}

static struct tg_geom *parse_geojson_featurecollection(struct json json,
    enum tg_index ix)
{
    struct gvec geoms = { 0 };
    PARSE_GEOJSON_BASIC_HEAD("features")
    struct json val0 = json_first(target);
    while (json_exists(val0)) {
        struct tg_geom *child = parse_geojson(val0, false, ix);
        if (!child) goto fail;
        if (tg_geom_error(child)) {
            gerr = child;
            goto fail;
        }
        if ((child->head.flags&IS_FEATURE) != IS_FEATURE) {
            gerr = make_parse_error("'features' must only contain objects "
                "with the 'type' of Feature");
            tg_geom_free(child);
            goto fail;
        }
        if (!gvec_append(&geoms, child)) {
            tg_geom_free(child);
            goto fail;
        }
        val0 = json_next(val0);
    }
    geom = tg_geom_new_geometrycollection(
        (struct tg_geom const*const*)geoms.data, geoms.len);
    if (geom) geom->head.flags |= IS_FEATURE_COL;
    PARSE_GEOJSON_BASIC_TAIL({
        if (geoms.data) {
            for (size_t i = 0; i < geoms.len; i++) {
                tg_geom_free(geoms.data[i]);
            }
            tg_free(geoms.data);
        }
    })
}

static struct tg_geom *parse_geojson(struct json json, bool req_geom,
    enum tg_index ix)
{
    if (json_type(json) != JSON_OBJECT) {
        return make_parse_error("expected an object");
    }
    struct json jtype = json_object_get(json, "type");
    if (!json_exists(jtype)) {
        return make_parse_error("'type' is required");
    }
    char type[24];
    json_string_copy(jtype, type, sizeof(type));
    struct tg_geom *geom = NULL;
    if (strcmp(type, "Point") == 0) {
        geom = parse_geojson_point(json, req_geom, ix);
    } else if (strcmp(type, "LineString") == 0) {
        geom = parse_geojson_linestring(json, req_geom, ix);
    } else if (strcmp(type, "Polygon") == 0) {
        geom = parse_geojson_polygon(json, req_geom, ix);
    } else if (strcmp(type, "MultiPoint") == 0) {
        geom = parse_geojson_multipoint(json, ix);
    } else if (strcmp(type, "MultiLineString") == 0) {
        geom = parse_geojson_multilinestring(json, ix);
    } else if (strcmp(type, "MultiPolygon") == 0) {
        geom = parse_geojson_multipolygon(json, ix);
    } else if (strcmp(type, "GeometryCollection") == 0) {
        geom = parse_geojson_geometrycollection(json, ix);
    } else if (strcmp(type, "Feature") == 0) {
        geom = parse_geojson_feature(json, ix);
    } else if (strcmp(type, "FeatureCollection") == 0) {
        geom = parse_geojson_featurecollection(json, ix);
    } else {
        geom = make_parse_error("unknown type '%s'", type);
    }
    return geom;
}

/// Parse geojson with an included data length.
/// @param geojson Geojson data. Must be UTF8.
/// @param len Length of data
/// @returns A geometry or an error. Use tg_geom_error() after parsing to check
/// for errors. 
/// @see tg_parse_geojson()
/// @see GeometryParsing
struct tg_geom *tg_parse_geojsonn(const char *geojson, size_t len) {
    return tg_parse_geojsonn_ix(geojson, len, TG_DEFAULT);
}

/// Parse geojson.
///
/// Supports [GeoJSON](https://datatracker.ietf.org/doc/html/rfc7946) standard,
/// including Features, FeaturesCollection, ZM coordinates, properties, and
/// arbritary JSON members.
/// @param geojson A geojson string. Must be UTF8 and null-terminated.
/// @returns A geometry or an error. Use tg_geom_error() after parsing to check
/// for errors. 
/// @see tg_parse_geojsonn()
/// @see tg_parse_geojson_ix()
/// @see tg_parse_geojsonn_ix()
/// @see tg_geom_error()
/// @see tg_geom_geojson()
/// @see GeometryParsing
struct tg_geom *tg_parse_geojson(const char *geojson) {
    return tg_parse_geojsonn_ix(geojson, geojson?strlen(geojson):0, TG_DEFAULT);
}

/// Parse geojson using provided indexing option.
/// @param geojson A geojson string. Must be UTF8 and null-terminated.
/// @param ix Indexing option, e.g. TG_NONE, TG_NATURAL, TG_YSTRIPES
/// @returns A geometry or an error. Use tg_geom_error() after parsing to check
/// for errors. 
/// @see [tg_index](.#tg_index)
/// @see tg_parse_geojson()
/// @see tg_parse_geojsonn_ix()
/// @see GeometryParsing
struct tg_geom *tg_parse_geojson_ix(const char *geojson, enum tg_index ix) {
    return tg_parse_geojsonn_ix(geojson, geojson?strlen(geojson):0, ix);
}

/// Parse geojson using provided indexing option. 
/// @param geojson Geojson data. Must be UTF8.
/// @param len Length of data
/// @param ix Indexing option, e.g. TG_NONE, TG_NATURAL, TG_YSTRIPES
/// @returns A geometry or an error. Use tg_geom_error() after parsing to check
/// for errors. 
/// @see [tg_index](.#tg_index)
/// @see tg_parse_geojson()
/// @see tg_parse_geojson_ix()
/// @see GeometryParsing
struct tg_geom *tg_parse_geojsonn_ix(const char *geojson, size_t len, 
    enum tg_index ix)
{
    struct tg_geom *geom = NULL;
    struct json_valid is = json_validn_ex(geojson, len, 0);
    if (!is.valid) {
        geom = make_parse_error("invalid json");
    } else {
        struct json json = json_parsen(geojson, len);
        geom = parse_geojson(json, false, ix);
    }
    if (!geom) return NULL;
    if ((geom->head.flags&IS_ERROR) == IS_ERROR) {
        struct tg_geom *gerr = make_parse_error("ParseError: %s", geom->error);
        tg_geom_free(geom);
        return gerr;
    }
    return geom;
}

struct writer {
    uint8_t *dst;
    size_t n;
    size_t count;
};

union raw_double {
    uint64_t u;
    double d;
};

static void write_nullterm(struct writer *wr) {
    if (wr->n > wr->count) {
        wr->dst[wr->count] = '\0';
    } else if (wr->n > 0) {
        wr->dst[wr->n-1] = '\0';
    }
}

static void write_byte(struct writer *wr, uint8_t b) {
    if (wr->count < wr->n) {
        wr->dst[wr->count] = b;
    }
    wr->count++;
}

static void write_char(struct writer *wr, char ch) {
    write_byte(wr, ch);
}

static void write_uint32le(struct writer *wr, uint32_t x) {
    for (int i = 0; i < 4; i++) {
        write_byte(wr, x>>(i*8));
    }
}

static void write_uint64le(struct writer *wr, uint64_t x) {
    for (int i = 0; i < 8; i++) {
        write_byte(wr, x>>(i*8));
    }
}

static void write_doublele(struct writer *wr, double x) {
    write_uint64le(wr, ((union raw_double){.d=x}).u);
}

static void write_string(struct writer *wr, const char *str) {
    char *p = (char*)str;
    while (*p) write_char(wr, *(p++));
}

static void write_stringn(struct writer *wr, const char *str, size_t n) {
    for (size_t i = 0; i < n; i++) {
        write_char(wr, str[i]);
    }
}

#ifdef TG_NOAMALGA

#include "deps/ryu.h"

#else

#define RYU_STATIC
#define RYU_NOWRITER

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#endif

// BEGIN ryu.c
// The "ryu_print" function is a lightweight wrapper around the original
// ryu d2s_buffered function.

// https://github.com/tidwall/ryu
//
// Copyright 2023 Joshua J Baker. All rights reserved.
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file.
//

// https://github.com/ulfjack/ryu
//
// Copyright 2018 Ulf Adams
//
// The contents of this file may be used under the terms of the Apache License,
// Version 2.0.
//
//    (See accompanying file LICENSE-Apache or copy at
//     http://www.apache.org/licenses/LICENSE-2.0)
//
// Alternatively, the contents of this file may be used under the terms of
// the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE-Boost or copy at
//     https://www.boost.org/LICENSE_1_0.txt)
//
// Unless required by applicable law or agreed to in writing, this software
// is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.

// Runtime compiler options:
// -DRYU_DEBUG Generate verbose debugging output to stdout.
//
// -DRYU_ONLY_64_BIT_OPS Avoid using uint128_t or 64-bit intrinsics. Slower,
//     depending on your compiler.
//
// -DRYU_OPTIMIZE_SIZE Use smaller lookup tables. Instead of storing every
//     required power of 5, only store every 26th entry, and compute
//     intermediate values with a multiplication. This reduces the lookup table
//     size by about 10x (only one case, and only double) at the cost of some
//     performance. Currently requires MSVC intrinsics.

#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#ifdef RYU_STATIC
#define RYU_EXTERN static
#endif

#ifndef RYU_EXTERN
#define RYU_EXTERN
#endif

#ifdef RYU_DEBUG
#include <inttypes.h>
#include <stdio.h>
#endif

#if defined(_M_IX86) || defined(_M_ARM)
#define RYU_32_BIT_PLATFORM
#endif

// Returns e == 0 ? 1 : ceil(log_2(5^e)); requires 0 <= e <= 3528.
static inline int32_t pow5bits(const int32_t e) {
    // This approximation works up to the point that the multiplication
    // overflows at e = 3529.
    // If the multiplication were done in 64 bits, it would fail at 5^4004
    // which is just greater than 2^9297.
    assert(e >= 0);
    assert(e <= 3528);
    return (int32_t) (((((uint32_t) e) * 1217359) >> 19) + 1);
}

// Returns floor(log_10(2^e)); requires 0 <= e <= 1650.
static inline uint32_t log10Pow2(const int32_t e) {
    // The first value this approximation fails for is 2^1651 which is just
    // greater than 10^297.
    assert(e >= 0);
    assert(e <= 1650);
    return (((uint32_t) e) * 78913) >> 18;
}

// Returns floor(log_10(5^e)); requires 0 <= e <= 2620.
static inline uint32_t log10Pow5(const int32_t e) {
    // The first value this approximation fails for is 5^2621 which is just
    // greater than 10^1832.
    assert(e >= 0);
    assert(e <= 2620);
    return (((uint32_t) e) * 732923) >> 20;
}

static inline int copy_special_str(char * const result, const bool sign, 
    const bool exponent, const bool mantissa)
{
    if (mantissa) {
        memcpy(result, "NaN", 3);
        return 3;
    }
    if (sign) {
        result[0] = '-';
    }
    if (exponent) {
        memcpy(result + sign, "Infinity", 8);
        return sign + 8;
    }
    memcpy(result + sign, "0E0", 3);
    return sign + 3;
}

static inline uint64_t double_to_bits(const double d) {
    uint64_t bits = 0;
    memcpy(&bits, &d, sizeof(double));
    return bits;
}

// A table of all two-digit numbers. This is used to speed up decimal digit
// generation by copying pairs of digits into the final output.
static const char DIGIT_TABLE[200] = {
    '0','0','0','1','0','2','0','3','0','4','0','5','0','6','0','7','0','8',
    '0','9','1','0','1','1','1','2','1','3','1','4','1','5','1','6','1','7',
    '1','8','1','9','2','0','2','1','2','2','2','3','2','4','2','5','2','6',
    '2','7','2','8','2','9','3','0','3','1','3','2','3','3','3','4','3','5',
    '3','6','3','7','3','8','3','9','4','0','4','1','4','2','4','3','4','4',
    '4','5','4','6','4','7','4','8','4','9','5','0','5','1','5','2','5','3',
    '5','4','5','5','5','6','5','7','5','8','5','9','6','0','6','1','6','2',
    '6','3','6','4','6','5','6','6','6','7','6','8','6','9','7','0','7','1',
    '7','2','7','3','7','4','7','5','7','6','7','7','7','8','7','9','8','0',
    '8','1','8','2','8','3','8','4','8','5','8','6','8','7','8','8','8','9',
    '9','0','9','1','9','2','9','3','9','4','9','5','9','6','9','7','9','8',
    '9','9',
};

// Defines RYU_32_BIT_PLATFORM if applicable.

// ABSL avoids uint128_t on Win32 even if __SIZEOF_INT128__ is defined.
// Let's do the same for now.
#if defined(__SIZEOF_INT128__) && !defined(_MSC_VER) && \
    !defined(RYU_ONLY_64_BIT_OPS)
#define HAS_UINT128
#elif defined(_MSC_VER) && !defined(RYU_ONLY_64_BIT_OPS) && defined(_M_X64)
#define HAS_64_BIT_INTRINSICS
#endif

#if defined(HAS_UINT128)
typedef __uint128_t uint128_t;
#endif

#if defined(HAS_64_BIT_INTRINSICS)

#include <intrin.h>

static inline uint64_t umul128(const uint64_t a, const uint64_t b, uint64_t* 
    const productHi)
{
    return _umul128(a, b, productHi);
}

// Returns the lower 64 bits of (hi*2^64 + lo) >> dist, with 0 < dist < 64.
static inline uint64_t shiftright128(const uint64_t lo, const uint64_t hi, 
    const uint32_t dist)
{
    // For the __shiftright128 intrinsic, the shift value is always
    // modulo 64.
    // In the current implementation of the double-precision version
    // of Ryu, the shift value is always < 64. (In the case
    // RYU_OPTIMIZE_SIZE == 0, the shift value is in the range [49, 58].
    // Otherwise in the range [2, 59].)
    // However, this function is now also called by s2d, which requires
    // supporting the larger shift range (TODO: what is the actual range?).
    // Check this here in case a future change requires larger shift
    // values. In this case this function needs to be adjusted.
    assert(dist < 64);
    return __shiftright128(lo, hi, (unsigned char) dist);
}

#else // defined(HAS_64_BIT_INTRINSICS)

static inline uint64_t umul128(const uint64_t a, const uint64_t b, uint64_t*
    const productHi)
{
    // The casts here help MSVC to avoid calls to the __allmul library function.
    const uint32_t aLo = (uint32_t)a;
    const uint32_t aHi = (uint32_t)(a >> 32);
    const uint32_t bLo = (uint32_t)b;
    const uint32_t bHi = (uint32_t)(b >> 32);

    const uint64_t b00 = (uint64_t)aLo * bLo;
    const uint64_t b01 = (uint64_t)aLo * bHi;
    const uint64_t b10 = (uint64_t)aHi * bLo;
    const uint64_t b11 = (uint64_t)aHi * bHi;

    const uint32_t b00Lo = (uint32_t)b00;
    const uint32_t b00Hi = (uint32_t)(b00 >> 32);

    const uint64_t mid1 = b10 + b00Hi;
    const uint32_t mid1Lo = (uint32_t)(mid1);
    const uint32_t mid1Hi = (uint32_t)(mid1 >> 32);

    const uint64_t mid2 = b01 + mid1Lo;
    const uint32_t mid2Lo = (uint32_t)(mid2);
    const uint32_t mid2Hi = (uint32_t)(mid2 >> 32);

    const uint64_t pHi = b11 + mid1Hi + mid2Hi;
    const uint64_t pLo = ((uint64_t)mid2Lo << 32) | b00Lo;

    *productHi = pHi;
    return pLo;
}

static inline uint64_t shiftright128(const uint64_t lo, const uint64_t hi,
    const uint32_t dist)
{
    // We don't need to handle the case dist >= 64 here (see above).
    assert(dist < 64);
    assert(dist > 0);
    return (hi << (64 - dist)) | (lo >> dist);
}

#endif // defined(HAS_64_BIT_INTRINSICS)

#if defined(RYU_32_BIT_PLATFORM)

// Returns the high 64 bits of the 128-bit product of a and b.
static inline uint64_t umulh(const uint64_t a, const uint64_t b) {
    // Reuse the umul128 implementation.
    // Optimizers will likely eliminate the instructions used to compute the
    // low part of the product.
    uint64_t hi;
    umul128(a, b, &hi);
    return hi;
}

// On 32-bit platforms, compilers typically generate calls to library
// functions for 64-bit divisions, even if the divisor is a constant.
//
// E.g.:
// https://bugs.llvm.org/show_bug.cgi?id=37932
// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=17958
// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=37443
//
// The functions here perform division-by-constant using multiplications
// in the same way as 64-bit compilers would do.
//
// NB:
// The multipliers and shift values are the ones generated by clang x64
// for expressions like x/5, x/10, etc.

static inline uint64_t div5(const uint64_t x) {
    return umulh(x, 0xCCCCCCCCCCCCCCCDu) >> 2;
}

static inline uint64_t div10(const uint64_t x) {
    return umulh(x, 0xCCCCCCCCCCCCCCCDu) >> 3;
}

static inline uint64_t div100(const uint64_t x) {
    return umulh(x >> 2, 0x28F5C28F5C28F5C3u) >> 2;
}

static inline uint64_t div1e8(const uint64_t x) {
    return umulh(x, 0xABCC77118461CEFDu) >> 26;
}

static inline uint64_t div1e9(const uint64_t x) {
    return umulh(x >> 9, 0x44B82FA09B5A53u) >> 11;
}

static inline uint32_t mod1e9(const uint64_t x) {
    // Avoid 64-bit math as much as possible.
    // Returning (uint32_t) (x - 1000000000 * div1e9(x)) would
    // perform 32x64-bit multiplication and 64-bit subtraction.
    // x and 1000000000 * div1e9(x) are guaranteed to differ by
    // less than 10^9, so their highest 32 bits must be identical,
    // so we can truncate both sides to uint32_t before subtracting.
    // We can also simplify (uint32_t) (1000000000 * div1e9(x)).
    // We can truncate before multiplying instead of after, as multiplying
    // the highest 32 bits of div1e9(x) can't affect the lowest 32 bits.
    return ((uint32_t) x) - 1000000000 * ((uint32_t) div1e9(x));
}

#else // defined(RYU_32_BIT_PLATFORM)

static inline uint64_t div5(const uint64_t x) {
    return x / 5;
}

static inline uint64_t div10(const uint64_t x) {
    return x / 10;
}

static inline uint64_t div100(const uint64_t x) {
    return x / 100;
}

static inline uint64_t div1e8(const uint64_t x) {
    return x / 100000000;
}

static inline uint64_t div1e9(const uint64_t x) {
    return x / 1000000000;
}

static inline uint32_t mod1e9(const uint64_t x) {
    return (uint32_t) (x - 1000000000 * div1e9(x));
}

#endif // defined(RYU_32_BIT_PLATFORM)

static inline uint32_t pow5Factor(uint64_t value) {
    const uint64_t m_inv_5 = 14757395258967641293u;
    const uint64_t n_div_5 = 3689348814741910323u;
    uint32_t count = 0;
    for (;;) {
        assert(value != 0);
        value *= m_inv_5;
        if (value > n_div_5)
            break;
        ++count;
    }
    return count;
}

// Returns true if value is divisible by 5^p.
static inline bool multipleOfPowerOf5(const uint64_t value, const uint32_t p) {
   // I tried a case distinction on p, but there was no performance difference.
   return pow5Factor(value) >= p;
}

// Returns true if value is divisible by 2^p.
static inline bool multipleOfPowerOf2(const uint64_t value, const uint32_t p) {
    assert(value != 0);
    assert(p < 64);
    // __builtin_ctzll doesn't appear to be faster here.
    return (value & ((1ull << p) - 1)) == 0;
}

// We need a 64x128-bit multiplication and a subsequent 128-bit shift.
// Multiplication:
//   The 64-bit factor is variable and passed in, the 128-bit factor comes
//   from a lookup table. We know that the 64-bit factor only has 55
//   significant bits (i.e., the 9 topmost bits are zeros). The 128-bit
//   factor only has 124 significant bits (i.e., the 4 topmost bits are
//   zeros).
// Shift:
//   In principle, the multiplication result requires 55 + 124 = 179 bits to
//   represent. However, we then shift this value to the right by j, which is
//   at least j >= 115, so the result is guaranteed to fit into 179 - 115 = 64
//   bits. This means that we only need the topmost 64 significant bits of
//   the 64x128-bit multiplication.
//
// There are several ways to do this:
// 1. Best case: the compiler exposes a 128-bit type.
//    We perform two 64x64-bit multiplications, add the higher 64 bits of the
//    lower result to the higher result, and shift by j - 64 bits.
//
//    We explicitly cast from 64-bit to 128-bit, so the compiler can tell
//    that these are only 64-bit inputs, and can map these to the best
//    possible sequence of assembly instructions.
//    x64 machines happen to have matching assembly instructions for
//    64x64-bit multiplications and 128-bit shifts.
//
// 2. Second best case: the compiler exposes intrinsics for the x64 assembly
//    instructions mentioned in 1.
//
// 3. We only have 64x64 bit instructions that return the lower 64 bits of
//    the result, i.e., we have to use plain C.
//    Our inputs are less than the full width, so we have three options:
//    a. Ignore this fact and just implement the intrinsics manually.
//    b. Split both into 31-bit pieces, which guarantees no internal overflow,
//       but requires extra work upfront (unless we change the lookup table).
//    c. Split only the first factor into 31-bit pieces, which also guarantees
//       no internal overflow, but requires extra work since the intermediate
//       results are not perfectly aligned.
#if defined(HAS_UINT128)

// Best case: use 128-bit type.
static inline uint64_t mulShift64(const uint64_t m, const uint64_t* const mul, 
    const int32_t j)
{
    const uint128_t b0 = ((uint128_t) m) * mul[0];
    const uint128_t b2 = ((uint128_t) m) * mul[1];
    return (uint64_t) (((b0 >> 64) + b2) >> (j - 64));
}

static inline uint64_t mulShiftAll64(const uint64_t m, const uint64_t*
    const mul, const int32_t j, uint64_t* const vp, uint64_t* const vm, 
    const uint32_t mmShift) 
{
    *vp = mulShift64(4 * m + 2, mul, j);
    *vm = mulShift64(4 * m - 1 - mmShift, mul, j);
    return mulShift64(4 * m, mul, j);
}

#elif defined(HAS_64_BIT_INTRINSICS)

static inline uint64_t mulShift64(const uint64_t m, const uint64_t* const mul, 
    const int32_t j) 
{
    // m is maximum 55 bits
    uint64_t high1;                                   // 128
    const uint64_t low1 = umul128(m, mul[1], &high1); // 64
    uint64_t high0;                                   // 64
    umul128(m, mul[0], &high0);                       // 0
    const uint64_t sum = high0 + low1;
    if (sum < high0) {
        ++high1; // overflow into high1
    }
    return shiftright128(sum, high1, j - 64);
}

static inline uint64_t mulShiftAll64(const uint64_t m, const uint64_t* 
    const mul, const int32_t j, uint64_t* const vp, uint64_t* const vm, 
    const uint32_t mmShift)
{
    *vp = mulShift64(4 * m + 2, mul, j);
    *vm = mulShift64(4 * m - 1 - mmShift, mul, j);
    return mulShift64(4 * m, mul, j);
}

#else // !defined(HAS_UINT128) && !defined(HAS_64_BIT_INTRINSICS)

// This is faster if we don't have a 64x64->128-bit multiplication.
static inline uint64_t mulShiftAll64(uint64_t m, const uint64_t* const mul, 
    const int32_t j, uint64_t* const vp, uint64_t* const vm, 
    const uint32_t mmShift)
{
    m <<= 1;
    // m is maximum 55 bits
    uint64_t tmp;
    const uint64_t lo = umul128(m, mul[0], &tmp);
    uint64_t hi;
    const uint64_t mid = tmp + umul128(m, mul[1], &hi);
    hi += mid < tmp; // overflow into hi
  
    const uint64_t lo2 = lo + mul[0];
    const uint64_t mid2 = mid + mul[1] + (lo2 < lo);
    const uint64_t hi2 = hi + (mid2 < mid);
    *vp = shiftright128(mid2, hi2, (uint32_t) (j - 64 - 1));
  
    if (mmShift == 1) {
        const uint64_t lo3 = lo - mul[0];
        const uint64_t mid3 = mid - mul[1] - (lo3 > lo);
        const uint64_t hi3 = hi - (mid3 > mid);
        *vm = shiftright128(mid3, hi3, (uint32_t) (j - 64 - 1));
    } else {
        const uint64_t lo3 = lo + lo;
        const uint64_t mid3 = mid + mid + (lo3 < lo);
        const uint64_t hi3 = hi + hi + (mid3 < mid);
        const uint64_t lo4 = lo3 - mul[0];
        const uint64_t mid4 = mid3 - mul[1] - (lo4 > lo3);
        const uint64_t hi4 = hi3 - (mid4 > mid3);
        *vm = shiftright128(mid4, hi4, (uint32_t) (j - 64));
    }
  
    return shiftright128(mid, hi, (uint32_t) (j - 64 - 1));
}

#endif // HAS_64_BIT_INTRINSICS

// Include either the small or the full lookup tables depending on the mode.
#if defined(RYU_OPTIMIZE_SIZE)

// These tables are generated by PrintDoubleLookupTable.
#define DOUBLE_POW5_INV_BITCOUNT 125
#define DOUBLE_POW5_BITCOUNT 125

static const uint64_t DOUBLE_POW5_INV_SPLIT2[15][2] = {
    {                    1u, 2305843009213693952u },
    {  5955668970331000884u, 1784059615882449851u },
    {  8982663654677661702u, 1380349269358112757u },
    {  7286864317269821294u, 2135987035920910082u },
    {  7005857020398200553u, 1652639921975621497u },
    { 17965325103354776697u, 1278668206209430417u },
    {  8928596168509315048u, 1978643211784836272u },
    { 10075671573058298858u, 1530901034580419511u },
    {   597001226353042382u, 1184477304306571148u },
    {  1527430471115325346u, 1832889850782397517u },
    { 12533209867169019542u, 1418129833677084982u },
    {  5577825024675947042u, 2194449627517475473u },
    { 11006974540203867551u, 1697873161311732311u },
    { 10313493231639821582u, 1313665730009899186u },
    { 12701016819766672773u, 2032799256770390445u }
};
static const uint32_t POW5_INV_OFFSETS[19] = {
    0x54544554, 0x04055545, 0x10041000, 0x00400414, 0x40010000, 0x41155555,
    0x00000454, 0x00010044, 0x40000000, 0x44000041, 0x50454450, 0x55550054,
    0x51655554, 0x40004000, 0x01000001, 0x00010500, 0x51515411, 0x05555554,
    0x00000000
};

static const uint64_t DOUBLE_POW5_SPLIT2[13][2] = {
    {                    0u, 1152921504606846976u },
    {                    0u, 1490116119384765625u },
    {  1032610780636961552u, 1925929944387235853u },
    {  7910200175544436838u, 1244603055572228341u },
    { 16941905809032713930u, 1608611746708759036u },
    { 13024893955298202172u, 2079081953128979843u },
    {  6607496772837067824u, 1343575221513417750u },
    { 17332926989895652603u, 1736530273035216783u },
    { 13037379183483547984u, 2244412773384604712u },
    {  1605989338741628675u, 1450417759929778918u },
    {  9630225068416591280u, 1874621017369538693u },
    {   665883850346957067u, 1211445438634777304u },
    { 14931890668723713708u, 1565756531257009982u }
};
static const uint32_t POW5_OFFSETS[21] = {
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x40000000, 0x59695995,
    0x55545555, 0x56555515, 0x41150504, 0x40555410, 0x44555145, 0x44504540,
    0x45555550, 0x40004000, 0x96440440, 0x55565565, 0x54454045, 0x40154151,
    0x55559155, 0x51405555, 0x00000105
};

#define POW5_TABLE_SIZE 26
static const uint64_t DOUBLE_POW5_TABLE[POW5_TABLE_SIZE] = {
    1ull, 5ull, 25ull, 125ull, 625ull, 3125ull, 15625ull, 78125ull, 390625ull,
    1953125ull, 9765625ull, 48828125ull, 244140625ull, 1220703125ull,
    6103515625ull, 30517578125ull, 152587890625ull, 762939453125ull, 
    3814697265625ull, 19073486328125ull, 95367431640625ull,
    476837158203125ull, 2384185791015625ull, 11920928955078125ull, 
    59604644775390625ull, 298023223876953125ull //, 1490116119384765625ull
};

#if defined(HAS_UINT128)

// Computes 5^i in the form required by Ryu, and stores it in the given pointer.
static inline void double_computePow5(const uint32_t i, uint64_t* const result)
{
    const uint32_t base = i / POW5_TABLE_SIZE;
    const uint32_t base2 = base * POW5_TABLE_SIZE;
    const uint32_t offset = i - base2;
    const uint64_t* const mul = DOUBLE_POW5_SPLIT2[base];
    if (offset == 0) {
        result[0] = mul[0];
        result[1] = mul[1];
        return;
    }
    const uint64_t m = DOUBLE_POW5_TABLE[offset];
    const uint128_t b0 = ((uint128_t) m) * mul[0];
    const uint128_t b2 = ((uint128_t) m) * mul[1];
    const uint32_t delta = pow5bits(i) - pow5bits(base2);
    const uint128_t shiftedSum = (b0 >> delta) + (b2 << (64 - delta)) + 
        ((POW5_OFFSETS[i / 16] >> ((i % 16) << 1)) & 3);
    result[0] = (uint64_t) shiftedSum;
    result[1] = (uint64_t) (shiftedSum >> 64);
}

// Computes 5^-i in the form required by Ryu, and stores it in the given pointer.
static inline void double_computeInvPow5(const uint32_t i, 
    uint64_t* const result)
{
    const uint32_t base = (i + POW5_TABLE_SIZE - 1) / POW5_TABLE_SIZE;
    const uint32_t base2 = base * POW5_TABLE_SIZE;
    const uint32_t offset = base2 - i;
    const uint64_t* const mul = DOUBLE_POW5_INV_SPLIT2[base]; // 1/5^base2
    if (offset == 0) {
        result[0] = mul[0];
        result[1] = mul[1];
        return;
    }
    const uint64_t m = DOUBLE_POW5_TABLE[offset]; // 5^offset
    const uint128_t b0 = ((uint128_t) m) * (mul[0] - 1);
    const uint128_t b2 = ((uint128_t) m) * mul[1]; 
    const uint32_t delta = pow5bits(base2) - pow5bits(i);
    const uint128_t shiftedSum = ((b0 >> delta) + (b2 << (64 - delta))) + 1 + 
        ((POW5_INV_OFFSETS[i / 16] >> ((i % 16) << 1)) & 3);
    result[0] = (uint64_t) shiftedSum;
    result[1] = (uint64_t) (shiftedSum >> 64);
}

#else // defined(HAS_UINT128)

// Computes 5^i in the form required by Ryu, and stores it in the given pointer.
static inline void double_computePow5(const uint32_t i, uint64_t* const result)
{
    const uint32_t base = i / POW5_TABLE_SIZE;
    const uint32_t base2 = base * POW5_TABLE_SIZE;
    const uint32_t offset = i - base2;
    const uint64_t* const mul = DOUBLE_POW5_SPLIT2[base];
    if (offset == 0) {
        result[0] = mul[0];
        result[1] = mul[1];
        return;
    }
    const uint64_t m = DOUBLE_POW5_TABLE[offset];
    uint64_t high1;
    const uint64_t low1 = umul128(m, mul[1], &high1);
    uint64_t high0;
    const uint64_t low0 = umul128(m, mul[0], &high0);
    const uint64_t sum = high0 + low1;
    if (sum < high0) {
        ++high1; // overflow into high1
    }
    // high1 | sum | low0
    const uint32_t delta = pow5bits(i) - pow5bits(base2);
    result[0] = shiftright128(low0, sum, delta) + 
        ((POW5_OFFSETS[i / 16] >> ((i % 16) << 1)) & 3);
    result[1] = shiftright128(sum, high1, delta);
}

// Computes 5^-i in the form required by Ryu, and stores it in the given
// pointer.
static inline void double_computeInvPow5(const uint32_t i,
    uint64_t* const result)
{
    const uint32_t base = (i + POW5_TABLE_SIZE - 1) / POW5_TABLE_SIZE;
    const uint32_t base2 = base * POW5_TABLE_SIZE;
    const uint32_t offset = base2 - i;
    const uint64_t* const mul = DOUBLE_POW5_INV_SPLIT2[base]; // 1/5^base2
    if (offset == 0) {
        result[0] = mul[0];
        result[1] = mul[1];
        return;
    }
    const uint64_t m = DOUBLE_POW5_TABLE[offset];
    uint64_t high1;
    const uint64_t low1 = umul128(m, mul[1], &high1);
    uint64_t high0;
    const uint64_t low0 = umul128(m, mul[0] - 1, &high0);
    const uint64_t sum = high0 + low1;
    if (sum < high0) {
        ++high1; // overflow into high1
    }
    // high1 | sum | low0
    const uint32_t delta = pow5bits(base2) - pow5bits(i);
    result[0] = shiftright128(low0, sum, delta) + 1 + 
        ((POW5_INV_OFFSETS[i / 16] >> ((i % 16) << 1)) & 3);
    result[1] = shiftright128(sum, high1, delta);
}

#endif // defined(HAS_UINT128)

#else
// These tables are generated by PrintDoubleLookupTable.
#define DOUBLE_POW5_INV_BITCOUNT 125
#define DOUBLE_POW5_BITCOUNT 125

#define DOUBLE_POW5_INV_TABLE_SIZE 342
#define DOUBLE_POW5_TABLE_SIZE 326

static const uint64_t DOUBLE_POW5_INV_SPLIT[DOUBLE_POW5_INV_TABLE_SIZE][2] = {
    {                    1u, 2305843009213693952u }, 
    { 11068046444225730970u, 1844674407370955161u },
    {  5165088340638674453u, 1475739525896764129u }, 
    {  7821419487252849886u, 1180591620717411303u },
    {  8824922364862649494u, 1888946593147858085u },
    {  7059937891890119595u, 1511157274518286468u },
    { 13026647942995916322u, 1208925819614629174u },
    {  9774590264567735146u, 1934281311383406679u },
    { 11509021026396098440u, 1547425049106725343u },
    { 16585914450600699399u, 1237940039285380274u },
    { 15469416676735388068u, 1980704062856608439u },
    { 16064882156130220778u, 1584563250285286751u },
    {  9162556910162266299u, 1267650600228229401u },
    {  7281393426775805432u, 2028240960365167042u },
    { 16893161185646375315u, 1622592768292133633u },
    {  2446482504291369283u, 1298074214633706907u },
    {  7603720821608101175u, 2076918743413931051u },
    {  2393627842544570617u, 1661534994731144841u },
    { 16672297533003297786u, 1329227995784915872u },
    { 11918280793837635165u, 2126764793255865396u },
    {  5845275820328197809u, 1701411834604692317u },
    { 15744267100488289217u, 1361129467683753853u },
    {  3054734472329800808u, 2177807148294006166u },
    { 17201182836831481939u, 1742245718635204932u },
    {  6382248639981364905u, 1393796574908163946u },
    {  2832900194486363201u, 2230074519853062314u },
    {  5955668970331000884u, 1784059615882449851u },
    {  1075186361522890384u, 1427247692705959881u },
    { 12788344622662355584u, 2283596308329535809u },
    { 13920024512871794791u, 1826877046663628647u },
    {  3757321980813615186u, 1461501637330902918u },
    { 10384555214134712795u, 1169201309864722334u },
    {  5547241898389809503u, 1870722095783555735u },
    {  4437793518711847602u, 1496577676626844588u },
    { 10928932444453298728u, 1197262141301475670u },
    { 17486291911125277965u, 1915619426082361072u },
    {  6610335899416401726u, 1532495540865888858u },
    { 12666966349016942027u, 1225996432692711086u },
    { 12888448528943286597u, 1961594292308337738u },
    { 17689456452638449924u, 1569275433846670190u },
    { 14151565162110759939u, 1255420347077336152u },
    {  7885109000409574610u, 2008672555323737844u },
    {  9997436015069570011u, 1606938044258990275u },
    {  7997948812055656009u, 1285550435407192220u },
    { 12796718099289049614u, 2056880696651507552u },
    {  2858676849947419045u, 1645504557321206042u },
    { 13354987924183666206u, 1316403645856964833u },
    { 17678631863951955605u, 2106245833371143733u },
    {  3074859046935833515u, 1684996666696914987u },
    { 13527933681774397782u, 1347997333357531989u },
    { 10576647446613305481u, 2156795733372051183u },
    { 15840015586774465031u, 1725436586697640946u },
    {  8982663654677661702u, 1380349269358112757u },
    { 18061610662226169046u, 2208558830972980411u },
    { 10759939715039024913u, 1766847064778384329u },
    { 12297300586773130254u, 1413477651822707463u },
    { 15986332124095098083u, 2261564242916331941u },
    {  9099716884534168143u, 1809251394333065553u },
    { 14658471137111155161u, 1447401115466452442u },
    {  4348079280205103483u, 1157920892373161954u },
    { 14335624477811986218u, 1852673427797059126u },
    {  7779150767507678651u, 1482138742237647301u },
    {  2533971799264232598u, 1185710993790117841u },
    { 15122401323048503126u, 1897137590064188545u },
    { 12097921058438802501u, 1517710072051350836u },
    {  5988988032009131678u, 1214168057641080669u },
    { 16961078480698431330u, 1942668892225729070u },
    { 13568862784558745064u, 1554135113780583256u },
    {  7165741412905085728u, 1243308091024466605u },
    { 11465186260648137165u, 1989292945639146568u },
    { 16550846638002330379u, 1591434356511317254u },
    { 16930026125143774626u, 1273147485209053803u },
    {  4951948911778577463u, 2037035976334486086u },
    {   272210314680951647u, 1629628781067588869u },
    {  3907117066486671641u, 1303703024854071095u },
    {  6251387306378674625u, 2085924839766513752u },
    { 16069156289328670670u, 1668739871813211001u },
    {  9165976216721026213u, 1334991897450568801u },
    {  7286864317269821294u, 2135987035920910082u },
    { 16897537898041588005u, 1708789628736728065u },
    { 13518030318433270404u, 1367031702989382452u },
    {  6871453250525591353u, 2187250724783011924u },
    {  9186511415162383406u, 1749800579826409539u },
    { 11038557946871817048u, 1399840463861127631u },
    { 10282995085511086630u, 2239744742177804210u },
    {  8226396068408869304u, 1791795793742243368u },
    { 13959814484210916090u, 1433436634993794694u },
    { 11267656730511734774u, 2293498615990071511u },
    {  5324776569667477496u, 1834798892792057209u },
    {  7949170070475892320u, 1467839114233645767u },
    { 17427382500606444826u, 1174271291386916613u },
    {  5747719112518849781u, 1878834066219066582u },
    { 15666221734240810795u, 1503067252975253265u },
    { 12532977387392648636u, 1202453802380202612u },
    {  5295368560860596524u, 1923926083808324180u },
    {  4236294848688477220u, 1539140867046659344u },
    {  7078384693692692099u, 1231312693637327475u },
    { 11325415509908307358u, 1970100309819723960u },
    {  9060332407926645887u, 1576080247855779168u },
    { 14626963555825137356u, 1260864198284623334u },
    { 12335095245094488799u, 2017382717255397335u },
    {  9868076196075591040u, 1613906173804317868u },
    { 15273158586344293478u, 1291124939043454294u },
    { 13369007293925138595u, 2065799902469526871u },
    {  7005857020398200553u, 1652639921975621497u },
    { 16672732060544291412u, 1322111937580497197u },
    { 11918976037903224966u, 2115379100128795516u },
    {  5845832015580669650u, 1692303280103036413u },
    { 12055363241948356366u, 1353842624082429130u },
    {   841837113407818570u, 2166148198531886609u },
    {  4362818505468165179u, 1732918558825509287u },
    { 14558301248600263113u, 1386334847060407429u },
    { 12225235553534690011u, 2218135755296651887u },
    {  2401490813343931363u, 1774508604237321510u },
    {  1921192650675145090u, 1419606883389857208u },
    { 17831303500047873437u, 2271371013423771532u },
    {  6886345170554478103u, 1817096810739017226u },
    {  1819727321701672159u, 1453677448591213781u },
    { 16213177116328979020u, 1162941958872971024u },
    { 14873036941900635463u, 1860707134196753639u },
    { 15587778368262418694u, 1488565707357402911u },
    {  8780873879868024632u, 1190852565885922329u },
    {  2981351763563108441u, 1905364105417475727u },
    { 13453127855076217722u, 1524291284333980581u },
    {  7073153469319063855u, 1219433027467184465u },
    { 11317045550910502167u, 1951092843947495144u },
    { 12742985255470312057u, 1560874275157996115u },
    { 10194388204376249646u, 1248699420126396892u },
    {  1553625868034358140u, 1997919072202235028u },
    {  8621598323911307159u, 1598335257761788022u },
    { 17965325103354776697u, 1278668206209430417u },
    { 13987124906400001422u, 2045869129935088668u },
    {   121653480894270168u, 1636695303948070935u },
    {    97322784715416134u, 1309356243158456748u },
    { 14913111714512307107u, 2094969989053530796u },
    {  8241140556867935363u, 1675975991242824637u },
    { 17660958889720079260u, 1340780792994259709u },
    { 17189487779326395846u, 2145249268790815535u },
    { 13751590223461116677u, 1716199415032652428u },
    { 18379969808252713988u, 1372959532026121942u },
    { 14650556434236701088u, 2196735251241795108u },
    {   652398703163629901u, 1757388200993436087u },
    { 11589965406756634890u, 1405910560794748869u },
    {  7475898206584884855u, 2249456897271598191u },
    {  2291369750525997561u, 1799565517817278553u },
    {  9211793429904618695u, 1439652414253822842u },
    { 18428218302589300235u, 2303443862806116547u },
    {  7363877012587619542u, 1842755090244893238u },
    { 13269799239553916280u, 1474204072195914590u },
    { 10615839391643133024u, 1179363257756731672u },
    {  2227947767661371545u, 1886981212410770676u },
    { 16539753473096738529u, 1509584969928616540u },
    { 13231802778477390823u, 1207667975942893232u },
    {  6413489186596184024u, 1932268761508629172u },
    { 16198837793502678189u, 1545815009206903337u },
    {  5580372605318321905u, 1236652007365522670u },
    {  8928596168509315048u, 1978643211784836272u },
    { 18210923379033183008u, 1582914569427869017u },
    {  7190041073742725760u, 1266331655542295214u },
    {   436019273762630246u, 2026130648867672343u },
    {  7727513048493924843u, 1620904519094137874u },
    {  9871359253537050198u, 1296723615275310299u },
    {  4726128361433549347u, 2074757784440496479u },
    {  7470251503888749801u, 1659806227552397183u },
    { 13354898832594820487u, 1327844982041917746u },
    { 13989140502667892133u, 2124551971267068394u },
    { 14880661216876224029u, 1699641577013654715u },
    { 11904528973500979224u, 1359713261610923772u },
    {  4289851098633925465u, 2175541218577478036u },
    { 18189276137874781665u, 1740432974861982428u },
    {  3483374466074094362u, 1392346379889585943u },
    {  1884050330976640656u, 2227754207823337509u },
    {  5196589079523222848u, 1782203366258670007u },
    { 15225317707844309248u, 1425762693006936005u },
    {  5913764258841343181u, 2281220308811097609u },
    {  8420360221814984868u, 1824976247048878087u },
    { 17804334621677718864u, 1459980997639102469u },
    { 17932816512084085415u, 1167984798111281975u },
    { 10245762345624985047u, 1868775676978051161u },
    {  4507261061758077715u, 1495020541582440929u },
    {  7295157664148372495u, 1196016433265952743u },
    {  7982903447895485668u, 1913626293225524389u },
    { 10075671573058298858u, 1530901034580419511u },
    {  4371188443704728763u, 1224720827664335609u },
    { 14372599139411386667u, 1959553324262936974u },
    { 15187428126271019657u, 1567642659410349579u },
    { 15839291315758726049u, 1254114127528279663u },
    {  3206773216762499739u, 2006582604045247462u },
    { 13633465017635730761u, 1605266083236197969u },
    { 14596120828850494932u, 1284212866588958375u },
    {  4907049252451240275u, 2054740586542333401u },
    {   236290587219081897u, 1643792469233866721u },
    { 14946427728742906810u, 1315033975387093376u },
    { 16535586736504830250u, 2104054360619349402u },
    {  5849771759720043554u, 1683243488495479522u },
    { 15747863852001765813u, 1346594790796383617u },
    { 10439186904235184007u, 2154551665274213788u },
    { 15730047152871967852u, 1723641332219371030u },
    { 12584037722297574282u, 1378913065775496824u },
    {  9066413911450387881u, 2206260905240794919u },
    { 10942479943902220628u, 1765008724192635935u },
    {  8753983955121776503u, 1412006979354108748u },
    { 10317025513452932081u, 2259211166966573997u },
    {   874922781278525018u, 1807368933573259198u },
    {  8078635854506640661u, 1445895146858607358u },
    { 13841606313089133175u, 1156716117486885886u },
    { 14767872471458792434u, 1850745787979017418u },
    {   746251532941302978u, 1480596630383213935u },
    {   597001226353042382u, 1184477304306571148u },
    { 15712597221132509104u, 1895163686890513836u },
    {  8880728962164096960u, 1516130949512411069u },
    { 10793931984473187891u, 1212904759609928855u },
    { 17270291175157100626u, 1940647615375886168u },
    {  2748186495899949531u, 1552518092300708935u },
    {  2198549196719959625u, 1242014473840567148u },
    { 18275073973719576693u, 1987223158144907436u },
    { 10930710364233751031u, 1589778526515925949u },
    { 12433917106128911148u, 1271822821212740759u },
    {  8826220925580526867u, 2034916513940385215u },
    {  7060976740464421494u, 1627933211152308172u },
    { 16716827836597268165u, 1302346568921846537u },
    { 11989529279587987770u, 2083754510274954460u },
    {  9591623423670390216u, 1667003608219963568u },
    { 15051996368420132820u, 1333602886575970854u },
    { 13015147745246481542u, 2133764618521553367u },
    {  3033420566713364587u, 1707011694817242694u },
    {  6116085268112601993u, 1365609355853794155u },
    {  9785736428980163188u, 2184974969366070648u },
    { 15207286772667951197u, 1747979975492856518u },
    {  1097782973908629988u, 1398383980394285215u },
    {  1756452758253807981u, 2237414368630856344u },
    {  5094511021344956708u, 1789931494904685075u },
    {  4075608817075965366u, 1431945195923748060u },
    {  6520974107321544586u, 2291112313477996896u },
    {  1527430471115325346u, 1832889850782397517u },
    { 12289990821117991246u, 1466311880625918013u },
    { 17210690286378213644u, 1173049504500734410u },
    {  9090360384495590213u, 1876879207201175057u },
    { 18340334751822203140u, 1501503365760940045u },
    { 14672267801457762512u, 1201202692608752036u },
    { 16096930852848599373u, 1921924308174003258u },
    {  1809498238053148529u, 1537539446539202607u },
    { 12515645034668249793u, 1230031557231362085u },
    {  1578287981759648052u, 1968050491570179337u },
    { 12330676829633449412u, 1574440393256143469u },
    { 13553890278448669853u, 1259552314604914775u },
    {  3239480371808320148u, 2015283703367863641u },
    { 17348979556414297411u, 1612226962694290912u },
    {  6500486015647617283u, 1289781570155432730u },
    { 10400777625036187652u, 2063650512248692368u },
    { 15699319729512770768u, 1650920409798953894u },
    { 16248804598352126938u, 1320736327839163115u },
    {  7551343283653851484u, 2113178124542660985u },
    {  6041074626923081187u, 1690542499634128788u },
    { 12211557331022285596u, 1352433999707303030u },
    {  1091747655926105338u, 2163894399531684849u },
    {  4562746939482794594u, 1731115519625347879u },
    {  7339546366328145998u, 1384892415700278303u },
    {  8053925371383123274u, 2215827865120445285u },
    {  6443140297106498619u, 1772662292096356228u },
    { 12533209867169019542u, 1418129833677084982u },
    {  5295740528502789974u, 2269007733883335972u },
    { 15304638867027962949u, 1815206187106668777u },
    {  4865013464138549713u, 1452164949685335022u },
    { 14960057215536570740u, 1161731959748268017u },
    {  9178696285890871890u, 1858771135597228828u },
    { 14721654658196518159u, 1487016908477783062u },
    {  4398626097073393881u, 1189613526782226450u },
    {  7037801755317430209u, 1903381642851562320u },
    {  5630241404253944167u, 1522705314281249856u },
    {   814844308661245011u, 1218164251424999885u },
    {  1303750893857992017u, 1949062802279999816u },
    { 15800395974054034906u, 1559250241823999852u },
    {  5261619149759407279u, 1247400193459199882u },
    { 12107939454356961969u, 1995840309534719811u },
    {  5997002748743659252u, 1596672247627775849u },
    {  8486951013736837725u, 1277337798102220679u },
    {  2511075177753209390u, 2043740476963553087u },
    { 13076906586428298482u, 1634992381570842469u },
    { 14150874083884549109u, 1307993905256673975u },
    {  4194654460505726958u, 2092790248410678361u },
    { 18113118827372222859u, 1674232198728542688u },
    {  3422448617672047318u, 1339385758982834151u },
    { 16543964232501006678u, 2143017214372534641u },
    {  9545822571258895019u, 1714413771498027713u },
    { 15015355686490936662u, 1371531017198422170u },
    {  5577825024675947042u, 2194449627517475473u },
    { 11840957649224578280u, 1755559702013980378u },
    { 16851463748863483271u, 1404447761611184302u },
    { 12204946739213931940u, 2247116418577894884u },
    { 13453306206113055875u, 1797693134862315907u },
    {  3383947335406624054u, 1438154507889852726u },
    { 16482362180876329456u, 2301047212623764361u },
    {  9496540929959153242u, 1840837770099011489u },
    { 11286581558709232917u, 1472670216079209191u },
    {  5339916432225476010u, 1178136172863367353u },
    {  4854517476818851293u, 1885017876581387765u },
    {  3883613981455081034u, 1508014301265110212u },
    { 14174937629389795797u, 1206411441012088169u },
    { 11611853762797942306u, 1930258305619341071u },
    {  5600134195496443521u, 1544206644495472857u },
    { 15548153800622885787u, 1235365315596378285u },
    {  6430302007287065643u, 1976584504954205257u },
    { 16212288050055383484u, 1581267603963364205u },
    { 12969830440044306787u, 1265014083170691364u },
    {  9683682259845159889u, 2024022533073106183u },
    { 15125643437359948558u, 1619218026458484946u },
    {  8411165935146048523u, 1295374421166787957u },
    { 17147214310975587960u, 2072599073866860731u },
    { 10028422634038560045u, 1658079259093488585u },
    {  8022738107230848036u, 1326463407274790868u },
    {  9147032156827446534u, 2122341451639665389u },
    { 11006974540203867551u, 1697873161311732311u },
    {  5116230817421183718u, 1358298529049385849u },
    { 15564666937357714594u, 2173277646479017358u },
    {  1383687105660440706u, 1738622117183213887u },
    { 12174996128754083534u, 1390897693746571109u },
    {  8411947361780802685u, 2225436309994513775u },
    {  6729557889424642148u, 1780349047995611020u },
    {  5383646311539713719u, 1424279238396488816u },
    {  1235136468979721303u, 2278846781434382106u },
    { 15745504434151418335u, 1823077425147505684u },
    { 16285752362063044992u, 1458461940118004547u },
    {  5649904260166615347u, 1166769552094403638u },
    {  5350498001524674232u, 1866831283351045821u },
    {   591049586477829062u, 1493465026680836657u },
    { 11540886113407994219u, 1194772021344669325u },
    {    18673707743239135u, 1911635234151470921u },
    { 14772334225162232601u, 1529308187321176736u },
    {  8128518565387875758u, 1223446549856941389u },
    {  1937583260394870242u, 1957514479771106223u },
    {  8928764237799716840u, 1566011583816884978u },
    { 14521709019723594119u, 1252809267053507982u },
    {  8477339172590109297u, 2004494827285612772u },
    { 17849917782297818407u, 1603595861828490217u },
    {  6901236596354434079u, 1282876689462792174u },
    { 18420676183650915173u, 2052602703140467478u },
    {  3668494502695001169u, 1642082162512373983u },
    { 10313493231639821582u, 1313665730009899186u },
    {  9122891541139893884u, 2101865168015838698u },
    { 14677010862395735754u, 1681492134412670958u },
    {   673562245690857633u, 1345193707530136767u }
};

static const uint64_t DOUBLE_POW5_SPLIT[DOUBLE_POW5_TABLE_SIZE][2] = {
    {                    0u, 1152921504606846976u }, 
    {                    0u, 1441151880758558720u },
    {                    0u, 1801439850948198400u }, 
    {                    0u, 2251799813685248000u },
    {                    0u, 1407374883553280000u }, 
    {                    0u, 1759218604441600000u },
    {                    0u, 2199023255552000000u }, 
    {                    0u, 1374389534720000000u },
    {                    0u, 1717986918400000000u }, 
    {                    0u, 2147483648000000000u },
    {                    0u, 1342177280000000000u }, 
    {                    0u, 1677721600000000000u },
    {                    0u, 2097152000000000000u }, 
    {                    0u, 1310720000000000000u },
    {                    0u, 1638400000000000000u }, 
    {                    0u, 2048000000000000000u },
    {                    0u, 1280000000000000000u }, 
    {                    0u, 1600000000000000000u },
    {                    0u, 2000000000000000000u }, 
    {                    0u, 1250000000000000000u },
    {                    0u, 1562500000000000000u }, 
    {                    0u, 1953125000000000000u },
    {                    0u, 1220703125000000000u }, 
    {                    0u, 1525878906250000000u },
    {                    0u, 1907348632812500000u }, 
    {                    0u, 1192092895507812500u },
    {                    0u, 1490116119384765625u }, 
    {  4611686018427387904u, 1862645149230957031u },
    {  9799832789158199296u, 1164153218269348144u }, 
    { 12249790986447749120u, 1455191522836685180u },
    { 15312238733059686400u, 1818989403545856475u }, 
    { 14528612397897220096u, 2273736754432320594u },
    { 13692068767113150464u, 1421085471520200371u }, 
    { 12503399940464050176u, 1776356839400250464u },
    { 15629249925580062720u, 2220446049250313080u }, 
    {  9768281203487539200u, 1387778780781445675u },
    {  7598665485932036096u, 1734723475976807094u }, 
    {   274959820560269312u, 2168404344971008868u },
    {  9395221924704944128u, 1355252715606880542u }, 
    {  2520655369026404352u, 1694065894508600678u },
    { 12374191248137781248u, 2117582368135750847u }, 
    { 14651398557727195136u, 1323488980084844279u },
    { 13702562178731606016u, 1654361225106055349u }, 
    {  3293144668132343808u, 2067951531382569187u },
    { 18199116482078572544u, 1292469707114105741u }, 
    {  8913837547316051968u, 1615587133892632177u },
    { 15753982952572452864u, 2019483917365790221u }, 
    { 12152082354571476992u, 1262177448353618888u },
    { 15190102943214346240u, 1577721810442023610u }, 
    {  9764256642163156992u, 1972152263052529513u },
    { 17631875447420442880u, 1232595164407830945u }, 
    {  8204786253993389888u, 1540743955509788682u },
    {  1032610780636961552u, 1925929944387235853u }, 
    {  2951224747111794922u, 1203706215242022408u },
    {  3689030933889743652u, 1504632769052528010u }, 
    { 13834660704216955373u, 1880790961315660012u },
    { 17870034976990372916u, 1175494350822287507u }, 
    { 17725857702810578241u, 1469367938527859384u },
    {  3710578054803671186u, 1836709923159824231u }, 
    {    26536550077201078u, 2295887403949780289u },
    { 11545800389866720434u, 1434929627468612680u }, 
    { 14432250487333400542u, 1793662034335765850u },
    {  8816941072311974870u, 2242077542919707313u }, 
    { 17039803216263454053u, 1401298464324817070u },
    { 12076381983474541759u, 1751623080406021338u }, 
    {  5872105442488401391u, 2189528850507526673u },
    { 15199280947623720629u, 1368455531567204170u }, 
    {  9775729147674874978u, 1710569414459005213u },
    { 16831347453020981627u, 2138211768073756516u }, 
    {  1296220121283337709u, 1336382355046097823u },
    { 15455333206886335848u, 1670477943807622278u }, 
    { 10095794471753144002u, 2088097429759527848u },
    {  6309871544845715001u, 1305060893599704905u }, 
    { 12499025449484531656u, 1631326116999631131u },
    { 11012095793428276666u, 2039157646249538914u }, 
    { 11494245889320060820u, 1274473528905961821u },
    {   532749306367912313u, 1593091911132452277u }, 
    {  5277622651387278295u, 1991364888915565346u },
    {  7910200175544436838u, 1244603055572228341u }, 
    { 14499436237857933952u, 1555753819465285426u },
    {  8900923260467641632u, 1944692274331606783u }, 
    { 12480606065433357876u, 1215432671457254239u },
    { 10989071563364309441u, 1519290839321567799u }, 
    {  9124653435777998898u, 1899113549151959749u },
    {  8008751406574943263u, 1186945968219974843u }, 
    {  5399253239791291175u, 1483682460274968554u },
    { 15972438586593889776u, 1854603075343710692u }, 
    {   759402079766405302u, 1159126922089819183u },
    { 14784310654990170340u, 1448908652612273978u }, 
    {  9257016281882937117u, 1811135815765342473u },
    { 16182956370781059300u, 2263919769706678091u }, 
    {  7808504722524468110u, 1414949856066673807u },
    {  5148944884728197234u, 1768687320083342259u }, 
    {  1824495087482858639u, 2210859150104177824u },
    {  1140309429676786649u, 1381786968815111140u }, 
    {  1425386787095983311u, 1727233711018888925u },
    {  6393419502297367043u, 2159042138773611156u }, 
    { 13219259225790630210u, 1349401336733506972u },
    { 16524074032238287762u, 1686751670916883715u }, 
    { 16043406521870471799u, 2108439588646104644u },
    {   803757039314269066u, 1317774742903815403u }, 
    { 14839754354425000045u, 1647218428629769253u },
    {  4714634887749086344u, 2059023035787211567u }, 
    {  9864175832484260821u, 1286889397367007229u },
    { 16941905809032713930u, 1608611746708759036u }, 
    {  2730638187581340797u, 2010764683385948796u },
    { 10930020904093113806u, 1256727927116217997u }, 
    { 18274212148543780162u, 1570909908895272496u },
    {  4396021111970173586u, 1963637386119090621u }, 
    {  5053356204195052443u, 1227273366324431638u },
    { 15540067292098591362u, 1534091707905539547u }, 
    { 14813398096695851299u, 1917614634881924434u },
    { 13870059828862294966u, 1198509146801202771u }, 
    { 12725888767650480803u, 1498136433501503464u },
    { 15907360959563101004u, 1872670541876879330u }, 
    { 14553786618154326031u, 1170419088673049581u },
    {  4357175217410743827u, 1463023860841311977u }, 
    { 10058155040190817688u, 1828779826051639971u },
    {  7961007781811134206u, 2285974782564549964u }, 
    { 14199001900486734687u, 1428734239102843727u },
    { 13137066357181030455u, 1785917798878554659u }, 
    { 11809646928048900164u, 2232397248598193324u },
    { 16604401366885338411u, 1395248280373870827u }, 
    { 16143815690179285109u, 1744060350467338534u },
    { 10956397575869330579u, 2180075438084173168u }, 
    {  6847748484918331612u, 1362547148802608230u },
    { 17783057643002690323u, 1703183936003260287u }, 
    { 17617136035325974999u, 2128979920004075359u },
    { 17928239049719816230u, 1330612450002547099u }, 
    { 17798612793722382384u, 1663265562503183874u },
    { 13024893955298202172u, 2079081953128979843u }, 
    {  5834715712847682405u, 1299426220705612402u },
    { 16516766677914378815u, 1624282775882015502u }, 
    { 11422586310538197711u, 2030353469852519378u },
    { 11750802462513761473u, 1268970918657824611u }, 
    { 10076817059714813937u, 1586213648322280764u },
    { 12596021324643517422u, 1982767060402850955u }, 
    {  5566670318688504437u, 1239229412751781847u },
    {  2346651879933242642u, 1549036765939727309u }, 
    {  7545000868343941206u, 1936295957424659136u },
    {  4715625542714963254u, 1210184973390411960u }, 
    {  5894531928393704067u, 1512731216738014950u },
    { 16591536947346905892u, 1890914020922518687u }, 
    { 17287239619732898039u, 1181821263076574179u },
    { 16997363506238734644u, 1477276578845717724u }, 
    {  2799960309088866689u, 1846595723557147156u },
    { 10973347230035317489u, 1154122327223216972u }, 
    { 13716684037544146861u, 1442652909029021215u },
    { 12534169028502795672u, 1803316136286276519u }, 
    { 11056025267201106687u, 2254145170357845649u },
    { 18439230838069161439u, 1408840731473653530u }, 
    { 13825666510731675991u, 1761050914342066913u },
    {  3447025083132431277u, 2201313642927583642u }, 
    {  6766076695385157452u, 1375821026829739776u },
    {  8457595869231446815u, 1719776283537174720u }, 
    { 10571994836539308519u, 2149720354421468400u },
    {  6607496772837067824u, 1343575221513417750u }, 
    { 17482743002901110588u, 1679469026891772187u },
    { 17241742735199000331u, 2099336283614715234u }, 
    { 15387775227926763111u, 1312085177259197021u },
    {  5399660979626290177u, 1640106471573996277u }, 
    { 11361262242960250625u, 2050133089467495346u },
    { 11712474920277544544u, 1281333180917184591u }, 
    { 10028907631919542777u, 1601666476146480739u },
    {  7924448521472040567u, 2002083095183100924u }, 
    { 14176152362774801162u, 1251301934489438077u },
    {  3885132398186337741u, 1564127418111797597u }, 
    {  9468101516160310080u, 1955159272639746996u },
    { 15140935484454969608u, 1221974545399841872u }, 
    {   479425281859160394u, 1527468181749802341u },
    {  5210967620751338397u, 1909335227187252926u }, 
    { 17091912818251750210u, 1193334516992033078u },
    { 12141518985959911954u, 1491668146240041348u }, 
    { 15176898732449889943u, 1864585182800051685u },
    { 11791404716994875166u, 1165365739250032303u }, 
    { 10127569877816206054u, 1456707174062540379u },
    {  8047776328842869663u, 1820883967578175474u }, 
    {   836348374198811271u, 2276104959472719343u },
    {  7440246761515338900u, 1422565599670449589u }, 
    { 13911994470321561530u, 1778206999588061986u },
    {  8166621051047176104u, 2222758749485077483u }, 
    {  2798295147690791113u, 1389224218428173427u },
    { 17332926989895652603u, 1736530273035216783u }, 
    { 17054472718942177850u, 2170662841294020979u },
    {  8353202440125167204u, 1356664275808763112u }, 
    { 10441503050156459005u, 1695830344760953890u },
    {  3828506775840797949u, 2119787930951192363u }, 
    {    86973725686804766u, 1324867456844495227u },
    { 13943775212390669669u, 1656084321055619033u }, 
    {  3594660960206173375u, 2070105401319523792u },
    {  2246663100128858359u, 1293815875824702370u }, 
    { 12031700912015848757u, 1617269844780877962u },
    {  5816254103165035138u, 2021587305976097453u }, 
    {  5941001823691840913u, 1263492066235060908u },
    {  7426252279614801142u, 1579365082793826135u }, 
    {  4671129331091113523u, 1974206353492282669u },
    {  5225298841145639904u, 1233878970932676668u }, 
    {  6531623551432049880u, 1542348713665845835u },
    {  3552843420862674446u, 1927935892082307294u }, 
    { 16055585193321335241u, 1204959932551442058u },
    { 10846109454796893243u, 1506199915689302573u }, 
    { 18169322836923504458u, 1882749894611628216u },
    { 11355826773077190286u, 1176718684132267635u }, 
    {  9583097447919099954u, 1470898355165334544u },
    { 11978871809898874942u, 1838622943956668180u }, 
    { 14973589762373593678u, 2298278679945835225u },
    {  2440964573842414192u, 1436424174966147016u }, 
    {  3051205717303017741u, 1795530218707683770u },
    { 13037379183483547984u, 2244412773384604712u }, 
    {  8148361989677217490u, 1402757983365377945u },
    { 14797138505523909766u, 1753447479206722431u }, 
    { 13884737113477499304u, 2191809349008403039u },
    { 15595489723564518921u, 1369880843130251899u }, 
    { 14882676136028260747u, 1712351053912814874u },
    {  9379973133180550126u, 2140438817391018593u }, 
    { 17391698254306313589u, 1337774260869386620u },
    {  3292878744173340370u, 1672217826086733276u }, 
    {  4116098430216675462u, 2090272282608416595u },
    {   266718509671728212u, 1306420176630260372u }, 
    {   333398137089660265u, 1633025220787825465u },
    {  5028433689789463235u, 2041281525984781831u }, 
    { 10060300083759496378u, 1275800953740488644u },
    { 12575375104699370472u, 1594751192175610805u }, 
    {  1884160825592049379u, 1993438990219513507u },
    { 17318501580490888525u, 1245899368887195941u }, 
    {  7813068920331446945u, 1557374211108994927u },
    {  5154650131986920777u, 1946717763886243659u }, 
    {   915813323278131534u, 1216698602428902287u },
    { 14979824709379828129u, 1520873253036127858u }, 
    {  9501408849870009354u, 1901091566295159823u },
    { 12855909558809837702u, 1188182228934474889u }, 
    {  2234828893230133415u, 1485227786168093612u },
    {  2793536116537666769u, 1856534732710117015u }, 
    {  8663489100477123587u, 1160334207943823134u },
    {  1605989338741628675u, 1450417759929778918u }, 
    { 11230858710281811652u, 1813022199912223647u },
    {  9426887369424876662u, 2266277749890279559u }, 
    { 12809333633531629769u, 1416423593681424724u },
    { 16011667041914537212u, 1770529492101780905u }, 
    {  6179525747111007803u, 2213161865127226132u },
    { 13085575628799155685u, 1383226165704516332u }, 
    { 16356969535998944606u, 1729032707130645415u },
    { 15834525901571292854u, 2161290883913306769u }, 
    {  2979049660840976177u, 1350806802445816731u },
    { 17558870131333383934u, 1688508503057270913u }, 
    {  8113529608884566205u, 2110635628821588642u },
    {  9682642023980241782u, 1319147268013492901u }, 
    { 16714988548402690132u, 1648934085016866126u },
    { 11670363648648586857u, 2061167606271082658u }, 
    { 11905663298832754689u, 1288229753919426661u },
    {  1047021068258779650u, 1610287192399283327u }, 
    { 15143834390605638274u, 2012858990499104158u },
    {  4853210475701136017u, 1258036869061940099u }, 
    {  1454827076199032118u, 1572546086327425124u },
    {  1818533845248790147u, 1965682607909281405u }, 
    {  3442426662494187794u, 1228551629943300878u },
    { 13526405364972510550u, 1535689537429126097u }, 
    {  3072948650933474476u, 1919611921786407622u },
    { 15755650962115585259u, 1199757451116504763u }, 
    { 15082877684217093670u, 1499696813895630954u },
    {  9630225068416591280u, 1874621017369538693u }, 
    {  8324733676974063502u, 1171638135855961683u },
    {  5794231077790191473u, 1464547669819952104u }, 
    {  7242788847237739342u, 1830684587274940130u },
    { 18276858095901949986u, 2288355734093675162u }, 
    { 16034722328366106645u, 1430222333808546976u },
    {  1596658836748081690u, 1787777917260683721u }, 
    {  6607509564362490017u, 2234722396575854651u },
    {  1823850468512862308u, 1396701497859909157u }, 
    {  6891499104068465790u, 1745876872324886446u },
    { 17837745916940358045u, 2182346090406108057u }, 
    {  4231062170446641922u, 1363966306503817536u },
    {  5288827713058302403u, 1704957883129771920u }, 
    {  6611034641322878003u, 2131197353912214900u },
    { 13355268687681574560u, 1331998346195134312u }, 
    { 16694085859601968200u, 1664997932743917890u },
    { 11644235287647684442u, 2081247415929897363u }, 
    {  4971804045566108824u, 1300779634956185852u },
    {  6214755056957636030u, 1625974543695232315u }, 
    {  3156757802769657134u, 2032468179619040394u },
    {  6584659645158423613u, 1270292612261900246u }, 
    { 17454196593302805324u, 1587865765327375307u },
    { 17206059723201118751u, 1984832206659219134u }, 
    {  6142101308573311315u, 1240520129162011959u },
    {  3065940617289251240u, 1550650161452514949u }, 
    {  8444111790038951954u, 1938312701815643686u },
    {   665883850346957067u, 1211445438634777304u }, 
    {   832354812933696334u, 1514306798293471630u },
    { 10263815553021896226u, 1892883497866839537u }, 
    { 17944099766707154901u, 1183052186166774710u },
    { 13206752671529167818u, 1478815232708468388u }, 
    { 16508440839411459773u, 1848519040885585485u },
    { 12623618533845856310u, 1155324400553490928u }, 
    { 15779523167307320387u, 1444155500691863660u },
    {  1277659885424598868u, 1805194375864829576u }, 
    {  1597074856780748586u, 2256492969831036970u },
    {  5609857803915355770u, 1410308106144398106u }, 
    { 16235694291748970521u, 1762885132680497632u },
    {  1847873790976661535u, 2203606415850622041u }, 
    { 12684136165428883219u, 1377254009906638775u },
    { 11243484188358716120u, 1721567512383298469u }, 
    {   219297180166231438u, 2151959390479123087u },
    {  7054589765244976505u, 1344974619049451929u }, 
    { 13429923224983608535u, 1681218273811814911u },
    { 12175718012802122765u, 2101522842264768639u }, 
    { 14527352785642408584u, 1313451776415480399u },
    { 13547504963625622826u, 1641814720519350499u }, 
    { 12322695186104640628u, 2052268400649188124u },
    { 16925056528170176201u, 1282667750405742577u }, 
    {  7321262604930556539u, 1603334688007178222u },
    { 18374950293017971482u, 2004168360008972777u }, 
    {  4566814905495150320u, 1252605225005607986u },
    { 14931890668723713708u, 1565756531257009982u }, 
    {  9441491299049866327u, 1957195664071262478u },
    {  1289246043478778550u, 1223247290044539049u }, 
    {  6223243572775861092u, 1529059112555673811u },
    {  3167368447542438461u, 1911323890694592264u }, 
    {  1979605279714024038u, 1194577431684120165u },
    {  7086192618069917952u, 1493221789605150206u }, 
    { 18081112809442173248u, 1866527237006437757u },
    { 13606538515115052232u, 1166579523129023598u }, 
    {  7784801107039039482u, 1458224403911279498u },
    {   507629346944023544u, 1822780504889099373u }, 
    {  5246222702107417334u, 2278475631111374216u },
    {  3278889188817135834u, 1424047269444608885u }, 
    {  8710297504448807696u, 1780059086805761106u }
};

#endif

#define DOUBLE_MANTISSA_BITS 52
#define DOUBLE_EXPONENT_BITS 11
#define DOUBLE_BIAS 1023

static inline uint32_t decimalLength17(const uint64_t v) {
    // This is slightly faster than a loop.
    // The average output length is 16.38 digits, so we check high-to-low.
    // Function precondition: v is not an 18, 19, or 20-digit number.
    // (17 digits are sufficient for round-tripping.)
    assert(v < 100000000000000000L);
    if (v >= 10000000000000000L) { return 17; }
    if (v >= 1000000000000000L) { return 16; }
    if (v >= 100000000000000L) { return 15; }
    if (v >= 10000000000000L) { return 14; }
    if (v >= 1000000000000L) { return 13; }
    if (v >= 100000000000L) { return 12; }
    if (v >= 10000000000L) { return 11; }
    if (v >= 1000000000L) { return 10; }
    if (v >= 100000000L) { return 9; }
    if (v >= 10000000L) { return 8; }
    if (v >= 1000000L) { return 7; }
    if (v >= 100000L) { return 6; }
    if (v >= 10000L) { return 5; }
    if (v >= 1000L) { return 4; }
    if (v >= 100L) { return 3; }
    if (v >= 10L) { return 2; }
    return 1;
}

// A floating decimal representing m * 10^e.
typedef struct floating_decimal_64 {
    uint64_t mantissa;
    // Decimal exponent's range is -324 to 308
    // inclusive, and can fit in a short if needed.
    int32_t exponent;
} floating_decimal_64;

static inline floating_decimal_64 d2d(const uint64_t ieeeMantissa,
    const uint32_t ieeeExponent)
{
    int32_t e2;
    uint64_t m2;
    if (ieeeExponent == 0) {
        // We subtract 2 so that the bounds computation has 2 additional bits.
        e2 = 1 - DOUBLE_BIAS - DOUBLE_MANTISSA_BITS - 2;
        m2 = ieeeMantissa;
    } else {
        e2 = (int32_t) ieeeExponent - DOUBLE_BIAS - DOUBLE_MANTISSA_BITS - 2;
        m2 = (1ull << DOUBLE_MANTISSA_BITS) | ieeeMantissa;
    }
    const bool even = (m2 & 1) == 0;
    const bool acceptBounds = even;

#ifdef RYU_DEBUG
    printf("-> %" PRIu64 " * 2^%d\n", m2, e2 + 2);
#endif

    // Step 2: Determine the interval of valid decimal representations.
    const uint64_t mv = 4 * m2;
    // Implicit bool -> int conversion. True is 1, false is 0.
    const uint32_t mmShift = ieeeMantissa != 0 || ieeeExponent <= 1;
    // We would compute mp and mm like this:
    // uint64_t mp = 4 * m2 + 2;
    // uint64_t mm = mv - 1 - mmShift;

    // Step 3: Convert to a decimal power base using 128-bit arithmetic.
    uint64_t vr, vp, vm;
    int32_t e10;
    bool vmIsTrailingZeros = false;
    bool vrIsTrailingZeros = false;
    if (e2 >= 0) {
        // I tried special-casing q == 0, but there was no effect on
        // performance.
        // This expression is slightly faster than max(0, log10Pow2(e2) - 1).
        const uint32_t q = log10Pow2(e2) - (e2 > 3);
        e10 = (int32_t) q;
        const int32_t k = DOUBLE_POW5_INV_BITCOUNT + pow5bits((int32_t) q) - 1;
        const int32_t i = -e2 + (int32_t) q + k;
#if defined(RYU_OPTIMIZE_SIZE)
        uint64_t pow5[2];
        double_computeInvPow5(q, pow5);
        vr = mulShiftAll64(m2, pow5, i, &vp, &vm, mmShift);
#else
        vr = mulShiftAll64(m2, DOUBLE_POW5_INV_SPLIT[q], i, &vp, &vm, mmShift);
#endif
#ifdef RYU_DEBUG
        printf("%" PRIu64 " * 2^%d / 10^%u\n", mv, e2, q);
        printf("V+=%" PRIu64 "\nV =%" PRIu64 "\nV-=%" PRIu64 "\n", vp, vr, vm);
#endif
        if (q <= 21) {
            // This should use q <= 22, but I think 21 is also safe. Smaller 
            // values may still be safe, but it's more difficult to reason
            // about them. Only one of mp, mv, and mm can be a multiple of 5,
            // if any.
            const uint32_t mvMod5 = ((uint32_t) mv) - 5 * ((uint32_t) div5(mv));
            if (mvMod5 == 0) {
                vrIsTrailingZeros = multipleOfPowerOf5(mv, q);
            } else if (acceptBounds) {
                // Same as min(e2 + (~mm & 1), pow5Factor(mm)) >= q
                // <=> e2 + (~mm & 1) >= q && pow5Factor(mm) >= q
                // <=> true && pow5Factor(mm) >= q, since e2 >= q.
                vmIsTrailingZeros = multipleOfPowerOf5(mv - 1 - mmShift, q);
            } else {
                // Same as min(e2 + 1, pow5Factor(mp)) >= q.
                vp -= multipleOfPowerOf5(mv + 2, q);
            }
        }
    } else {
        // This expression is slightly faster than max(0, log10Pow5(-e2) - 1).
        const uint32_t q = log10Pow5(-e2) - (-e2 > 1);
        e10 = (int32_t) q + e2;
        const int32_t i = -e2 - (int32_t) q;
        const int32_t k = pow5bits(i) - DOUBLE_POW5_BITCOUNT;
        const int32_t j = (int32_t) q - k;
#if defined(RYU_OPTIMIZE_SIZE)
        uint64_t pow5[2];
        double_computePow5(i, pow5);
        vr = mulShiftAll64(m2, pow5, j, &vp, &vm, mmShift);
#else
        vr = mulShiftAll64(m2, DOUBLE_POW5_SPLIT[i], j, &vp, &vm, mmShift);
#endif
#ifdef RYU_DEBUG
        printf("%" PRIu64 " * 5^%d / 10^%u\n", mv, -e2, q);
        printf("%u %d %d %d\n", q, i, k, j);
        printf("V+=%" PRIu64 "\nV =%" PRIu64 "\nV-=%" PRIu64 "\n", vp, vr, vm);
#endif
        if (q <= 1) {
            // {vr,vp,vm} is trailing zeros if {mv,mp,mm} has at least q
            // trailing 0 bits. mv = 4 * m2, so it always has at least two
            // trailing 0 bits.
            vrIsTrailingZeros = true;
            if (acceptBounds) {
                // mm = mv - 1 - mmShift, so it has 1 trailing 0 bit iff 
                // mmShift == 1.
                vmIsTrailingZeros = mmShift == 1;
            } else {
                // mp = mv + 2, so it always has at least one trailing 0 bit.
                --vp;
            }
        } else if (q < 63) { // TODO(ulfjack): Use a tighter bound here.
            // We want to know if the full product has at least q trailing 
            // zeros.
            // We need to compute min(p2(mv), p5(mv) - e2) >= q
            // <=> p2(mv) >= q && p5(mv) - e2 >= q
            // <=> p2(mv) >= q (because -e2 >= q)
            vrIsTrailingZeros = multipleOfPowerOf2(mv, q);
#ifdef RYU_DEBUG
            printf("vr is trailing zeros=%s\n", vrIsTrailingZeros ? "true" : 
                "false");
#endif
        }
    }
#ifdef RYU_DEBUG
    printf("e10=%d\n", e10);
    printf("V+=%" PRIu64 "\nV =%" PRIu64 "\nV-=%" PRIu64 "\n", vp, vr, vm);
    printf("vm is trailing zeros=%s\n", vmIsTrailingZeros ? "true" : "false");
    printf("vr is trailing zeros=%s\n", vrIsTrailingZeros ? "true" : "false");
#endif

      // Step 4: Find the shortest decimal representation in the interval of 
      // valid representations.
      int32_t removed = 0;
      uint8_t lastRemovedDigit = 0;
      uint64_t output;
      // On average, we remove ~2 digits.
      if (vmIsTrailingZeros || vrIsTrailingZeros) {
          // General case, which happens rarely (~0.7%).
          for (;;) {
              const uint64_t vpDiv10 = div10(vp);
              const uint64_t vmDiv10 = div10(vm);
              if (vpDiv10 <= vmDiv10) {
                  break;
              }
              const uint32_t vmMod10 = ((uint32_t) vm) - 10 * 
                  ((uint32_t) vmDiv10);
              const uint64_t vrDiv10 = div10(vr);
              const uint32_t vrMod10 = ((uint32_t) vr) - 10 *
                  ((uint32_t) vrDiv10);
              vmIsTrailingZeros &= vmMod10 == 0;
              vrIsTrailingZeros &= lastRemovedDigit == 0;
              lastRemovedDigit = (uint8_t) vrMod10;
              vr = vrDiv10;
              vp = vpDiv10;
              vm = vmDiv10;
              ++removed;
          }
#ifdef RYU_DEBUG
        printf("V+=%" PRIu64 "\nV =%" PRIu64 "\nV-=%" PRIu64 "\n", vp, vr, vm);
        printf("d-10=%s\n", vmIsTrailingZeros ? "true" : "false");
#endif
        if (vmIsTrailingZeros) {
            for (;;) {
                const uint64_t vmDiv10 = div10(vm);
                const uint32_t vmMod10 = ((uint32_t) vm) - 10 * 
                    ((uint32_t) vmDiv10);
                if (vmMod10 != 0) {
                    break;
                }
                const uint64_t vpDiv10 = div10(vp);
                const uint64_t vrDiv10 = div10(vr);
                const uint32_t vrMod10 = ((uint32_t) vr) - 10 * 
                    ((uint32_t) vrDiv10);
                vrIsTrailingZeros &= lastRemovedDigit == 0;
                lastRemovedDigit = (uint8_t) vrMod10;
                vr = vrDiv10;
                vp = vpDiv10;
                vm = vmDiv10;
                ++removed;
            }
        }
#ifdef RYU_DEBUG
        printf("%" PRIu64 " %d\n", vr, lastRemovedDigit);
        printf("vr is trailing zeros=%s\n", vrIsTrailingZeros ? "true" : 
            "false");
#endif
        if (vrIsTrailingZeros && lastRemovedDigit == 5 && vr % 2 == 0) {
            // Round even if the exact number is .....50..0.
            lastRemovedDigit = 4;
        }
        // We need to take vr + 1 if vr is outside bounds or we need to round up.
        output = vr + ((vr == vm && (!acceptBounds || !vmIsTrailingZeros)) || 
            lastRemovedDigit >= 5);
    } else {
        // Specialized for the common case (~99.3%). Percentages below are 
        // relative to this.
        bool roundUp = false;
        const uint64_t vpDiv100 = div100(vp);
        const uint64_t vmDiv100 = div100(vm);
        if (vpDiv100 > vmDiv100) { // Opt: remove two digits at a time (~86.2%).
            const uint64_t vrDiv100 = div100(vr);
            const uint32_t vrMod100 = ((uint32_t) vr) - 100 * 
                ((uint32_t) vrDiv100);
            roundUp = vrMod100 >= 50;
            vr = vrDiv100;
            vp = vpDiv100;
            vm = vmDiv100;
            removed += 2;
        }
        // Loop iterations below (approximately), without optimization above:
        // 0: 0.03%, 1: 13.8%, 2: 70.6%, 3: 14.0%, 4: 1.40%, 5: 0.14%, 6+: 0.02%
        // Loop iterations below (approximately), with optimization above:
        // 0: 70.6%, 1: 27.8%, 2: 1.40%, 3: 0.14%, 4+: 0.02%
        for (;;) {
            const uint64_t vpDiv10 = div10(vp);
            const uint64_t vmDiv10 = div10(vm);
            if (vpDiv10 <= vmDiv10) {
                break;
            }
            const uint64_t vrDiv10 = div10(vr);
            const uint32_t vrMod10 = ((uint32_t) vr) - 10 * 
                ((uint32_t) vrDiv10);
            roundUp = vrMod10 >= 5;
            vr = vrDiv10;
            vp = vpDiv10;
            vm = vmDiv10;
            ++removed;
        }
#ifdef RYU_DEBUG
        printf("%" PRIu64 " roundUp=%s\n", vr, roundUp ? "true" : "false");
        printf("vr is trailing zeros=%s\n", vrIsTrailingZeros ? "true" : 
            "false");
#endif
        // We need to take vr + 1 if vr is outside bounds or we need to round 
        // up.
        output = vr + (vr == vm || roundUp);
    }
    const int32_t exp = e10 + removed;

#ifdef RYU_DEBUG
    printf("V+=%" PRIu64 "\nV =%" PRIu64 "\nV-=%" PRIu64 "\n", vp, vr, vm);
    printf("O=%" PRIu64 "\n", output);
    printf("EXP=%d\n", exp);
#endif

    floating_decimal_64 fd;
    fd.exponent = exp;
    fd.mantissa = output;
    return fd;
}

static inline int to_chars(const floating_decimal_64 v, const bool sign, 
    char* const result)
{
    // Step 5: Print the decimal representation.
    int index = 0;
    if (sign) {
        result[index++] = '-';
    }

    uint64_t output = v.mantissa;
    const uint32_t olength = decimalLength17(output);

#ifdef RYU_DEBUG
    printf("DIGITS=%" PRIu64 "\n", v.mantissa);
    printf("OLEN=%u\n", olength);
    printf("EXP=%u\n", v.exponent + olength);
#endif

    // Print the decimal digits.
    // The following code is equivalent to:
    // for (uint32_t i = 0; i < olength - 1; ++i) {
    //   const uint32_t c = output % 10; output /= 10;
    //   result[index + olength - i] = (char) ('0' + c);
    // }
    // result[index] = '0' + output % 10;

    uint32_t i = 0;
    // We prefer 32-bit operations, even on 64-bit platforms.
    // We have at most 17 digits, and uint32_t can store 9 digits.
    // If output doesn't fit into uint32_t, we cut off 8 digits,
    // so the rest will fit into uint32_t.
    if ((output >> 32) != 0) {
        // Expensive 64-bit division.
        const uint64_t q = div1e8(output);
        uint32_t output2 = ((uint32_t) output) - 100000000 * ((uint32_t) q);
        output = q;

        const uint32_t c = output2 % 10000;
        output2 /= 10000;
        const uint32_t d = output2 % 10000;
        const uint32_t c0 = (c % 100) << 1;
        const uint32_t c1 = (c / 100) << 1;
        const uint32_t d0 = (d % 100) << 1;
        const uint32_t d1 = (d / 100) << 1;
        memcpy(result + index + olength - i - 1, DIGIT_TABLE + c0, 2);
        memcpy(result + index + olength - i - 3, DIGIT_TABLE + c1, 2);
        memcpy(result + index + olength - i - 5, DIGIT_TABLE + d0, 2);
        memcpy(result + index + olength - i - 7, DIGIT_TABLE + d1, 2);
        i += 8;
    }
    uint32_t output2 = (uint32_t) output;
    while (output2 >= 10000) {
#ifdef __clang__ // https://bugs.llvm.org/show_bug.cgi?id=38217
        const uint32_t c = output2 - 10000 * (output2 / 10000);
#else
        const uint32_t c = output2 % 10000;
#endif
        output2 /= 10000;
        const uint32_t c0 = (c % 100) << 1;
        const uint32_t c1 = (c / 100) << 1;
        memcpy(result + index + olength - i - 1, DIGIT_TABLE + c0, 2);
        memcpy(result + index + olength - i - 3, DIGIT_TABLE + c1, 2);
        i += 4;
    }
    if (output2 >= 100) {
        const uint32_t c = (output2 % 100) << 1;
        output2 /= 100;
        memcpy(result + index + olength - i - 1, DIGIT_TABLE + c, 2);
        i += 2;
    }
    if (output2 >= 10) {
        const uint32_t c = output2 << 1;
        // We can't use memcpy here: the decimal dot goes between these two 
        // digits.
        result[index + olength - i] = DIGIT_TABLE[c + 1];
        result[index] = DIGIT_TABLE[c];
    } else {
        result[index] = (char) ('0' + output2);
    }

    // Print decimal point if needed.
    if (olength > 1) {
        result[index + 1] = '.';
        index += olength + 1;
    } else {
        ++index;
    }

    // Print the exponent.
    result[index++] = 'E';
    int32_t exp = v.exponent + (int32_t) olength - 1;
    if (exp < 0) {
        result[index++] = '-';
        exp = -exp;
    }

    if (exp >= 100) {
        const int32_t c = exp % 10;
        memcpy(result + index, DIGIT_TABLE + 2 * (exp / 10), 2);
        result[index + 2] = (char) ('0' + c);
        index += 3;
    } else if (exp >= 10) {
        memcpy(result + index, DIGIT_TABLE + 2 * exp, 2);
        index += 2;
    } else {
        result[index++] = (char) ('0' + exp);
    }

    return index;
}

static inline bool d2d_small_int(const uint64_t ieeeMantissa,
    const uint32_t ieeeExponent, floating_decimal_64* const v)
{
    const uint64_t m2 = (1ull << DOUBLE_MANTISSA_BITS) | ieeeMantissa;
    const int32_t e2 = (int32_t) ieeeExponent - DOUBLE_BIAS - 
        DOUBLE_MANTISSA_BITS;

    if (e2 > 0) {
        // f = m2 * 2^e2 >= 2^53 is an integer.
        // Ignore this case for now.
        return false;
    }

    if (e2 < -52) {
        // f < 1.
        return false;
    }

    // Since 2^52 <= m2 < 2^53 and 0 <= -e2 <= 52: 1 <= f = m2 / 2^-e2 < 2^53.
    // Test if the lower -e2 bits of the significand are 0, i.e. whether the
    // fraction is 0.
    const uint64_t mask = (1ull << -e2) - 1;
    const uint64_t fraction = m2 & mask;
    if (fraction != 0) {
        return false;
    }

    // f is an integer in the range [1, 2^53).
    // Note: mantissa might contain trailing (decimal) 0's.
    // Note: since 2^53 < 10^16, there is no need to adjust decimalLength17().
    v->mantissa = m2 >> -e2;
    v->exponent = 0;
    return true;
}

static int d2s_buffered_n(double f, char* result) {
    // Step 1: Decode the floating-point number, and unify normalized and
    // subnormal cases.
    const uint64_t bits = double_to_bits(f);

#ifdef RYU_DEBUG
    printf("IN=");
    for (int32_t bit = 63; bit >= 0; --bit) {
        printf("%d", (int) ((bits >> bit) & 1));
    }
    printf("\n");
#endif

    // Decode bits into sign, mantissa, and exponent.
    const bool ieeeSign = ((bits >> (DOUBLE_MANTISSA_BITS + 
        DOUBLE_EXPONENT_BITS)) & 1) != 0;
    const uint64_t ieeeMantissa = bits & ((1ull << DOUBLE_MANTISSA_BITS) - 1);
    const uint32_t ieeeExponent = (uint32_t) ((bits >> DOUBLE_MANTISSA_BITS) & 
        ((1u << DOUBLE_EXPONENT_BITS) - 1));
    // Case distinction; exit early for the easy cases.
    if (ieeeExponent == ((1u << DOUBLE_EXPONENT_BITS) - 1u) || 
        (ieeeExponent == 0 && ieeeMantissa == 0))
    {
        return copy_special_str(result, ieeeSign, ieeeExponent, ieeeMantissa);
    }

    floating_decimal_64 v;
    const bool isSmallInt = d2d_small_int(ieeeMantissa, ieeeExponent, &v);
    if (isSmallInt) {
        // For small integers in the range [1, 2^53), v.mantissa might contain 
        // trailing (decimal) zeros.
        // For scientific notation we need to move these zeros into the 
        // exponent.
        // (This is not needed for fixed-point notation, so it might be
        // beneficial to trim
        // trailing zeros in to_chars only if needed - once fixed-point
        // notation output is implemented.)
        for (;;) {
            const uint64_t q = div10(v.mantissa);
            const uint32_t r = ((uint32_t) v.mantissa) - 10 * ((uint32_t) q);
            if (r != 0) {
                break;
            }
            v.mantissa = q;
            ++v.exponent;
        }
    } else {
        v = d2d(ieeeMantissa, ieeeExponent);
    }

    return to_chars(v, ieeeSign, result);
}

static void d2s_buffered(double f, char* result) {
    (void)umul128; (void)shiftright128; (void)mod1e9;
    const int index = d2s_buffered_n(f, result);

    // Terminate the string.
    result[index] = '\0';
}

#ifndef RYU_NOWRITER
struct writer {
    uint8_t *dst;
    size_t n;
    size_t count;
};

static void write_nullterm(struct writer *wr) {
    if (wr->n > wr->count) wr->dst[wr->count] = '\0';
    else if (wr->n > 0) wr->dst[wr->n-1] = '\0';
}

static void write_char(struct writer *wr, char b) {
    if (wr->count < wr->n) wr->dst[wr->count] = b;
    wr->count++;
}
#endif

RYU_EXTERN
size_t ryu_string(double d, char fmt, char dst[], size_t nbytes) {
    struct writer wr = { .dst = (uint8_t*)dst, .n = nbytes };
    char buf[25];
    bool f = true;
    bool g = false;
    bool j = false;
    char ech = 'e';
    switch (fmt) {
    case 'j': case 'J':
        fmt -= 3;
        j = true;
        // fall through
    case 'g': case 'G':
        fmt -= 2;
        g = true;
        // fall through
    case 'e': case 'E':
        f = g;
        if (fmt == 'E') ech = 'E';
        // fall through
    case 'f':
        d2s_buffered(d, buf);
        break;
    default:
        buf[0] = '\0';
    }
    bool neg = false;
    char *p = buf;
    if (p[0] == '-') {
        write_char(&wr, '-');
        p++;
        neg = true;
    }
    char *end = p;
    char *e = NULL;
    while (*end) {
        if (*end >= 'E' && (*end == 'E' || *end == 'e')) {
            e = end;
        }
        end++;
    } 
    if (!e) {
        if (*p == 'i' || *p == 'I') {
            strcpy(p, "Infinity");
        } else if (*p == 'n' || *p == 'N') {
            strcpy(p, "NaN");
        } else {
            *p = '\0';
        }
        while (*p) write_char(&wr, *(p++));
        write_nullterm(&wr);
        return wr.count;
    }
    if (!f) {
        *e = '\0';
        while (*p) write_char(&wr, *(p++));
        write_char(&wr, ech);
        p++;
        if (j && *p != '-') write_char(&wr, '+');
        while (*p) write_char(&wr, *(p++));
        write_nullterm(&wr);
        return wr.count;
    }
    int en = atoi(e+1);
    *e = '\0';
    if (en < 0) {
        write_char(&wr, '0');
        write_char(&wr, '.');
        en = -en;
        for (int i = 0; i < en-1; i++) {
            write_char(&wr, '0');
        }
        write_char(&wr, *(p++));
        if (*p) {
            p++;
            while (*p) write_char(&wr, *(p++));
        }
    } else {
        write_char(&wr, *(p++));
        if (*p) p++;
        for (int i = 0; i < en; i++) {
            if (*p) {
                write_char(&wr, *(p++));
            } else {
                write_char(&wr, '0');
            }
        }
        if (*p && !(*p == '0' && *(p+1) == '\0')) {
            write_char(&wr, '.');
            while (*p) write_char(&wr, *(p++));
        }
    }
    write_nullterm(&wr);
    if (g) {
        bool rewrite = false;
        if (j) {
            rewrite = neg ? wr.count > 22 : wr.count > 21;
        } else {
            rewrite = (size_t)(end-buf) < wr.count;
        }
        if (rewrite) {
            // rewind and rewrite the buffer
            wr = (struct writer){ .dst = (uint8_t*)dst, .n = nbytes };
            p = buf;
            *e = '\0';
            while (*p) write_char(&wr, *(p++));
            write_char(&wr, ech);
            p++;
            if (j && *p != '-') write_char(&wr, '+');
            while (*p) write_char(&wr, *(p++));
            write_nullterm(&wr);
        }
    }
    return wr.count;
}
// END ryu.c

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#endif

static void write_string_double(struct writer *wr, double f) {
    if (!isnormal(f)) {
        write_char(wr, '0');
        return;
    }
    size_t dstsz = wr->count < wr->n ? wr->n - wr->count : 0;
    wr->count += ryu_string(f, 'f', (char*)wr->dst+wr->count, dstsz);
}

static void write_posn_geojson(struct writer *wr, struct tg_point posn) {
    write_char(wr, '[');
    write_string_double(wr, posn.x);
    write_char(wr, ',');
    write_string_double(wr, posn.y);
    write_char(wr, ']');
}

static void write_posn_geojson_3(struct writer *wr, struct tg_point posn, 
    double z)
{
    write_char(wr, '[');
    write_string_double(wr, posn.x);
    write_char(wr, ',');
    write_string_double(wr, posn.y);
    write_char(wr, ',');
    write_string_double(wr, z);
    write_char(wr, ']');
}
static void write_posn_geojson_4(struct writer *wr, struct tg_point posn, 
    double z, double m)
{
    write_char(wr, '[');
    write_string_double(wr, posn.x);
    write_char(wr, ',');
    write_string_double(wr, posn.y);
    write_char(wr, ',');
    write_string_double(wr, z);
    write_char(wr, ',');
    write_string_double(wr, m);
    write_char(wr, ']');
}

static int write_ring_points_geojson(struct writer *wr,
    const struct tg_ring *ring)
{
    write_char(wr, '[');
    for (int i = 0 ; i < ring->npoints; i++) {
        if (i > 0) write_char(wr, ',');
        write_posn_geojson(wr, ring->points[i]);
    }
    write_char(wr, ']');
    return ring->npoints;
}

static int write_ring_points_geojson_3(struct writer *wr, 
    const struct tg_ring *ring, const double *coords, int ncoords)
{
    double z;
    write_char(wr, '[');
    int j = 0;
    for (int i = 0 ; i < ring->npoints; i++) {
        if (i > 0) write_char(wr, ',');
        z = (j < ncoords) ? coords[j++] : 0;
        write_posn_geojson_3(wr, ring->points[i], z);
    }
    write_char(wr, ']');
    return ring->npoints;
}

static int write_ring_points_geojson_4(struct writer *wr, 
    const struct tg_ring *ring, const double *coords, int ncoords)
{
    double z, m;
    write_char(wr, '[');
    int j = 0;
    for (int i = 0 ; i < ring->npoints; i++) {
        if (i > 0) write_char(wr, ',');
        z = (j < ncoords) ? coords[j++] : 0;
        m = (j < ncoords) ? coords[j++] : 0;
        write_posn_geojson_4(wr, ring->points[i], z, m);
    }
    write_char(wr, ']');
    return ring->npoints;
}

static int write_poly_points_geojson(struct writer *wr, 
    const struct tg_poly *poly)
{
    int count = 0;
    write_char(wr, '[');
    write_ring_points_geojson(wr, tg_poly_exterior(poly));
    int nholes = tg_poly_num_holes(poly);
    for (int i = 0 ; i < nholes; i++) {
        write_char(wr, ',');
        count += write_ring_points_geojson(wr, tg_poly_hole_at(poly, i));
    }
    write_char(wr, ']');
    return count;
}

static int write_poly_points_geojson_3(struct writer *wr, 
    const struct tg_poly *poly, const double *coords, int ncoords)
{
    int count = 0;
    double *pcoords = (double*)coords;
    write_char(wr, '[');
    const struct tg_ring *exterior = tg_poly_exterior(poly);
    int n = write_ring_points_geojson_3(wr, exterior, pcoords, ncoords);
    count += n;
    ncoords -= n;
    if (ncoords < 0) ncoords = 0;
    pcoords = ncoords == 0 ? NULL : pcoords+n;
    int nholes = tg_poly_num_holes(poly);
    for (int i = 0 ; i < nholes; i++) {
        write_char(wr, ',');
        const struct tg_ring *hole = tg_poly_hole_at(poly, i);
        int n = write_ring_points_geojson_3(wr, hole, pcoords, ncoords);
        count += n;
        ncoords -= n;
        if (ncoords < 0) ncoords = 0;
        pcoords = ncoords == 0 ? NULL : pcoords+n;
    }
    write_char(wr, ']');
    return count;
}

static int write_poly_points_geojson_4(struct writer *wr, 
    const struct tg_poly *poly, const double *coords, int ncoords)
{
    int count = 0;
    double *pcoords = (double*)coords;
    write_char(wr, '[');
    const struct tg_ring *exterior = tg_poly_exterior(poly);
    int n = write_ring_points_geojson_4(wr, exterior, pcoords, ncoords);
    count += n;
    ncoords -= n*2;
    if (ncoords < 0) ncoords = 0;
    pcoords = ncoords == 0 ? NULL : pcoords+(n*2);
    int nholes = tg_poly_num_holes(poly);
    for (int i = 0 ; i < nholes; i++) {
        write_char(wr, ',');
        const struct tg_ring *hole = tg_poly_hole_at(poly, i);
        int n = write_ring_points_geojson_4(wr, hole, pcoords, ncoords);
        count += n;
        ncoords -= n*2;
        if (ncoords < 0) ncoords = 0;
        pcoords = ncoords == 0 ? NULL : pcoords+(n*2);
    }
    write_char(wr, ']');
    return count;
}

static void write_geom_point_geojson(const struct tg_geom *geom,
    struct writer *wr)
{
    write_string(wr, "{\"type\":\"Point\",\"coordinates\":");
    if ((geom->head.flags&IS_EMPTY) == IS_EMPTY) {
        write_string(wr, "[]");
    } else {
        if ((geom->head.flags&HAS_Z) == HAS_Z) {
            if ((geom->head.flags&HAS_M) == HAS_M) {
                write_posn_geojson_4(wr, geom->point, geom->z, geom->m);
            } else {
                write_posn_geojson_3(wr, geom->point, geom->z);
            }
        } else if ((geom->head.flags&HAS_M) == HAS_M) {
            write_posn_geojson_3(wr, geom->point, geom->m);
        } else {
            write_posn_geojson(wr, geom->point);
        }
    }
    write_char(wr, '}');
}

static void write_geom_linestring_geojson(const struct tg_geom *geom,
    struct writer *wr)
{
    write_string(wr, "{\"type\":\"LineString\",\"coordinates\":");
    if ((geom->head.flags&IS_EMPTY) == IS_EMPTY) {
        write_string(wr, "[]");
    } else {
        switch (tg_geom_dims(geom)) {
        case 3:
            write_ring_points_geojson_3(wr, (struct tg_ring*)geom->line, 
                geom->coords, geom->ncoords);
            break;
        case 4:
            write_ring_points_geojson_4(wr, (struct tg_ring*)geom->line, 
                geom->coords, geom->ncoords);
            break;
        default:
            write_ring_points_geojson(wr, (struct tg_ring*)geom->line);
            break;
        }
    }
    write_char(wr, '}');
}

static void write_geom_polygon_geojson(const struct tg_geom *geom,
    struct writer *wr)
{
    write_string(wr, "{\"type\":\"Polygon\",\"coordinates\":");
    if ((geom->head.flags&IS_EMPTY) == IS_EMPTY) {
        write_string(wr, "[]");
    } else {
        switch (tg_geom_dims(geom)) {
        case 3:
            write_poly_points_geojson_3(wr, geom->poly, 
                geom->coords, geom->ncoords);
            break;
        case 4:
            write_poly_points_geojson_4(wr, geom->poly, 
                geom->coords, geom->ncoords);
            break;
        default: // 2
            write_poly_points_geojson(wr, geom->poly);
            break;
        }
    }
    write_char(wr, '}');
}

static void write_geom_multipoint_geojson(const struct tg_geom *geom,
    struct writer *wr)
{
    write_string(wr, "{\"type\":\"MultiPoint\",\"coordinates\":[");
    if (geom->multi) {
        int dims = tg_geom_dims(geom);
        double z, m;
        double *coords = (double *)geom->coords;
        int ncoords = geom->ncoords;
        int j = 0;
        for (int i = 0; i < geom->multi->ngeoms; i++) {
            struct tg_point point = tg_geom_point(geom->multi->geoms[i]);
            if (i > 0) write_char(wr, ',');
            switch (dims) {
            case 3:
                z = (j < ncoords) ? coords[j++] : 0;
                write_posn_geojson_3(wr, point, z);
                break;
            case 4:
                z = (j < ncoords) ? coords[j++] : 0;
                m = (j < ncoords) ? coords[j++] : 0;
                write_posn_geojson_4(wr, point, z, m);
                break;
            default: // 2
                write_posn_geojson(wr, point);
                break;
            }
        }
    }
    write_char(wr, ']');
    write_char(wr, '}');
}

static void write_geom_multilinestring_geojson(const struct tg_geom *geom,
    struct writer *wr)
{
    write_string(wr, "{\"type\":\"MultiLineString\",\"coordinates\":[");
    if (geom->multi) {
        double *pcoords = (double *)geom->coords;
        int ncoords = geom->ncoords;
        int n;
        for (int i = 0; i < geom->multi->ngeoms; i++) {
            const struct tg_line *line = tg_geom_line(geom->multi->geoms[i]);
            const struct tg_ring *ring = (struct tg_ring*)line;
            if (i > 0) write_char(wr, ',');
            switch (tg_geom_dims(geom)) {
            case 3:
                n = write_ring_points_geojson_3(wr, ring, pcoords, ncoords);
                ncoords -= n;
                if (ncoords < 0) ncoords = 0;
                pcoords = ncoords == 0 ? NULL : pcoords+n;
                break;
            case 4:
                n = write_ring_points_geojson_4(wr, ring, pcoords, ncoords);
                ncoords -= n*2;
                if (ncoords < 0) ncoords = 0;
                pcoords = ncoords == 0 ? NULL : pcoords+(n*2);
                break;
            default: // 2
                write_ring_points_geojson(wr, ring);
                break;
            }
        }
    }
    write_char(wr, ']');
    write_char(wr, '}');
}

static void write_geom_multipolygon_geojson(const struct tg_geom *geom,
    struct writer *wr)
{
    write_string(wr, "{\"type\":\"MultiPolygon\",\"coordinates\":[");
    if (geom->multi) {
        double *pcoords = (double *)geom->coords;
        int ncoords = geom->ncoords;
        int n;
        for (int i = 0; i < geom->multi->ngeoms; i++) {
            const struct tg_poly *poly = tg_geom_poly(geom->multi->geoms[i]);
            if (i > 0) write_char(wr, ',');
            switch (tg_geom_dims(geom)) {
            case 3:
                n = write_poly_points_geojson_3(wr, poly, pcoords, ncoords);
                ncoords -= n;
                if (ncoords < 0) ncoords = 0;
                pcoords = ncoords == 0 ? NULL : pcoords+n;
                break;
            case 4:
                n = write_poly_points_geojson_4(wr, poly, pcoords, ncoords);
                ncoords -= n*2;
                if (ncoords < 0) ncoords = 0;
                pcoords = ncoords == 0 ? NULL : pcoords+(n*2);
                break;
            default: // 2
                write_poly_points_geojson(wr, poly);
                break;
            }
        }
    }
    write_char(wr, ']');
    write_char(wr, '}');
}

static void write_geom_geojson(const struct tg_geom *geom, struct writer *wr);

static void write_geom_geometrycollection_geojson(const struct tg_geom *geom,
    struct writer *wr)
{
    if ((geom->head.flags&IS_FEATURE_COL) == IS_FEATURE_COL) {
        write_string(wr, "{\"type\":\"FeatureCollection\",\"features\":[");
    } else {
        write_string(wr, "{\"type\":\"GeometryCollection\",\"geometries\":[");
    }
    int ngeoms = tg_geom_num_geometries(geom);
    for (int i = 0; i < ngeoms; i++) {
        if (i > 0) write_char(wr, ',');
        write_geom_geojson(tg_geom_geometry_at(geom, i), wr);
    }
    write_char(wr, ']');
    write_char(wr, '}');
}

static void write_geom_geojson(const struct tg_geom *geom, struct writer *wr) {
    if ((geom->head.flags&IS_ERROR) == IS_ERROR) {
        // sigh, just write us an empty point ...
        write_string(wr, "{\"type\":\"Point\",\"coordinates\":[]}");
        return;
    }
    bool is_feat = (geom->head.flags&IS_FEATURE) == IS_FEATURE;
    struct json fjson = { 0 };
    struct json gjson = { 0 };
    const char *xjson = geom->head.base == BASE_GEOM ? geom->xjson : NULL;
    if (is_feat) {
        if (xjson) {
            struct json json = json_parse(xjson);
            if (json_type(json) == JSON_ARRAY) {
                fjson = json_ensure(json_first(json));
                gjson = json_ensure(json_next(fjson));
            } else if (json_type(json) == JSON_OBJECT) {
                fjson = json_ensure(json);
            }
        }
        write_string(wr, "{\"type\":\"Feature\",");
        struct json id = json_object_get(fjson, "id");
        if (json_exists(id)) {
            write_string(wr, "\"id\":");
            write_stringn(wr, json_raw(id), json_raw_length(id));
            write_char(wr, ',');
        }
        write_string(wr, "\"geometry\":");
    } else {
        if (xjson) {
            gjson = json_ensure(json_parse(xjson));
        }
    }
    if (is_feat && (geom->head.flags&IS_UNLOCATED) == IS_UNLOCATED) {
        write_string(wr, "null");
    } else {
        switch (geom->head.type) {
        case TG_POINT:
            write_geom_point_geojson(geom, wr);
            break;
        case TG_LINESTRING:
            write_geom_linestring_geojson(geom, wr);
            break;
        case TG_POLYGON:
            write_geom_polygon_geojson(geom, wr);
            break;
        case TG_MULTIPOINT:
            write_geom_multipoint_geojson(geom, wr);
            break;
        case TG_MULTILINESTRING:
            write_geom_multilinestring_geojson(geom, wr);
            break;
        case TG_MULTIPOLYGON:
            write_geom_multipolygon_geojson(geom, wr);
            break;
        case TG_GEOMETRYCOLLECTION:
            write_geom_geometrycollection_geojson(geom, wr);
            break;
        }
    }
    if (json_type(gjson) == JSON_OBJECT) {
        if (json_exists(json_first(gjson))) {
            long len = json_raw_length(gjson)-1;
            if (len > 1) {
                // rewind one byte
                wr->count--;
                write_char(wr, ',');
                write_stringn(wr, (char*)(json_raw(gjson)+1), len);
            }
        }
    }
    if (is_feat) {
        bool wrote_props = false;
        if (json_type(fjson) == JSON_OBJECT) {
            struct json key = json_first(fjson);
            struct json val = json_next(key);
            while (json_exists(key)) {
                if (json_raw_compare(key, "\"id\"") != 0) {
                    write_char(wr, ',');
                    write_stringn(wr, json_raw(key), json_raw_length(key));
                    write_char(wr, ':');
                    write_stringn(wr, json_raw(val), json_raw_length(val));
                    if (!wrote_props && 
                        json_raw_compare(key, "\"properties\"") == 0)
                    {
                        wrote_props = true;
                    }
                }
                key = json_next(val);
                val = json_next(key);
            }
        }
        if (!wrote_props) {
            write_string(wr, ",\"properties\":");
            if ((geom->head.flags&HAS_NULL_PROPS)== HAS_NULL_PROPS){
                write_string(wr, "null");
            } else {
                write_string(wr, "{}");
            }
        }
        write_char(wr, '}');
    }
}

static void write_point_geojson(const struct boxed_point *point,
    struct writer *wr)
{
    if ((point->head.flags&IS_FEATURE) == IS_FEATURE) {
        write_string(wr, "{\"type\":\"Feature\",\"geometry\":");
    }
    write_string(wr, "{\"type\":\"Point\",\"coordinates\":");
    write_posn_geojson(wr, point->point);
    write_char(wr, '}');
    if ((point->head.flags&IS_FEATURE) == IS_FEATURE) {
        write_string(wr, ",\"properties\":");
        if ((point->head.flags&HAS_NULL_PROPS)== HAS_NULL_PROPS){
            write_string(wr, "null}");
        } else {
            write_string(wr, "{}}");
        }
    }
}

static void write_line_geojson(const struct tg_line *line, struct writer *wr) {
    struct tg_ring *ring = (struct tg_ring*)line;
    if ((ring->head.flags&IS_FEATURE) == IS_FEATURE) {
        write_string(wr, "{\"type\":\"Feature\",\"geometry\":");
    }
    write_string(wr, "{\"type\":\"LineString\",\"coordinates\":");
    write_ring_points_geojson(wr, ring);
    write_char(wr, '}');
    if ((ring->head.flags&IS_FEATURE) == IS_FEATURE) {
        write_string(wr, ",\"properties\":");
        if ((ring->head.flags&HAS_NULL_PROPS)== HAS_NULL_PROPS){
            write_string(wr, "null}");
        } else {
            write_string(wr, "{}}");
        }
    }
}

static void write_ring_geojson(const struct tg_ring *ring, struct writer *wr) {
    if ((ring->head.flags&IS_FEATURE) == IS_FEATURE) {
        write_string(wr, "{\"type\":\"Feature\",\"geometry\":");
    }
    write_string(wr, "{\"type\":\"Polygon\",\"coordinates\":[");
    write_ring_points_geojson(wr, ring);
    write_string(wr, "]}");
    if ((ring->head.flags&IS_FEATURE) == IS_FEATURE) {
        write_string(wr, ",\"properties\":");
        if ((ring->head.flags&HAS_NULL_PROPS)== HAS_NULL_PROPS){
            write_string(wr, "null}");
        } else {
            write_string(wr, "{}}");
        }
    }
}

static void write_poly_geojson(const struct tg_poly *poly, struct writer *wr) {
    if ((poly->head.flags&IS_FEATURE) == IS_FEATURE) {
        write_string(wr, "{\"type\":\"Feature\",\"geometry\":");
    }
    write_string(wr, "{\"type\":\"Polygon\",\"coordinates\":");
    write_poly_points_geojson(wr, poly);
    write_char(wr, '}');
    if ((poly->head.flags&IS_FEATURE) == IS_FEATURE) {
        write_string(wr, ",\"properties\":");
        if ((poly->head.flags&HAS_NULL_PROPS)== HAS_NULL_PROPS){
            write_string(wr, "null}");
        } else {
            write_string(wr, "{}}");
        }
    }
}

/// Writes a GeoJSON representation of a geometry.
///
/// The content is stored as a C string in the buffer pointed to by dst.
/// A terminating null character is automatically appended after the
/// content written.
///
/// @param geom Input geometry
/// @param dst Buffer where the resulting content is stored.
/// @param n Maximum number of bytes to be used in the buffer.
/// @return  The number of characters, not including the null-terminator, 
/// needed to store the content into the C string buffer.
/// If the returned length is greater than n-1, then only a parital copy
/// occurred, for example:
///
/// ```
/// char str[64];
/// size_t len = tg_geom_geojson(geom, str, sizeof(str));
/// if (len > sizeof(str)-1) {
///     // ... write did not complete ...
/// }
/// ```
///
/// @see tg_geom_wkt()
/// @see tg_geom_wkb()
/// @see tg_geom_hex()
/// @see GeometryWriting
size_t tg_geom_geojson(const struct tg_geom *geom, char *dst, size_t n) {
    if (!geom) return 0;
    struct writer wr = { .dst = (uint8_t*)dst, .n = n };
    switch (geom->head.base) {
    case BASE_GEOM:
        write_geom_geojson(geom, &wr);
        break;
    case BASE_POINT:
        write_point_geojson((struct boxed_point*)geom, &wr);
        break;
    case BASE_LINE:
        write_line_geojson((struct tg_line*)geom, &wr);
        break;
    case BASE_RING:
        write_ring_geojson((struct tg_ring*)geom, &wr);
        break;
    case BASE_POLY:
        write_poly_geojson((struct tg_poly*)geom, &wr);
        break;
    }
    write_nullterm(&wr);
    return wr.count;
}

/// Returns a string that represents any extra JSON from a parsed GeoJSON
/// geometry. Such as the "id" or "properties" fields.
/// @param geom Input geometry
/// @return Returns a valid JSON object as a string, or NULL if the geometry
/// did not come from GeoJSON or there is no extra JSON.
/// @note The returned string does not need to be freed.
/// @see tg_parse_geojson()
const char *tg_geom_extra_json(const struct tg_geom *geom) {
    return geom && geom->head.base == BASE_GEOM &&
          (geom->head.flags&IS_ERROR) != IS_ERROR ? geom->xjson : NULL;
}

//////////////////
// wkt
//////////////////

static const char *wkt_invalid_err(const char *inner) {
    (void)inner;
    return "invalid text";
}

static bool isws(char c) {
    return c <= ' ' && (c == ' ' || c == '\t' || c == '\n' || c == '\r');
}

static bool isnum(char c) {
    return c == '-' || (c >= '0' && c <= '9');
}

static long wkt_trim_ws(const char *wkt, long len, long i) {
    for (; i < len; i++) {
        if (!isws(wkt[i])) break;
    }
    return i;
}

// returns -1 for invalid 'Z', 'M', 'ZM' or 'EMPTY'
static enum tg_geom_type wkt2type(const char *wkt, long len, 
    bool *z, bool *m, bool *empty)
{
    *z = false;
    *m = false;
    *empty = false;
    char name[32];
    long i = 0;
    long j = 0;
    int nws = 0;
    for (; i < len; i++) {
        if (j == sizeof(name)-1) {
            goto bad_type;
        }
        if (isws(wkt[i])) {
            if (j > 0 && name[j-1] == ' ') continue;
            name[j] = ' ';
            nws++;
            if (nws > 2) return -1;
        } else if (wkt[i] >= 'a' && wkt[i] <= 'z') {
            name[j] = wkt[i]-32;
        } else {
            name[j] = wkt[i];
        }
        j++;
    }
    name[j] = '\0';
    if (j > 6) {
        // look for " EMPTY" suffix
        if (strcmp(name+j-6, " EMPTY") == 0) {
            j -= 6;
            name[j] = '\0';
            *empty = true;
        }
    }
    if (j > 3) {
        // look for " ZM", " Z", " M" and smash it
        if (name[j-2] == 'Z' && name[j-1] == 'M') {
            j -= 2;
            *z = true;
            *m = true;
        } else if (name[j-1] == 'Z') {
            j -= 1;
            *z = true;
        } else if (name[j-1] == 'M') {
            j -= 1;
            *m = true;
        }
        if (name[j-1] == ' ') {
            j -= 1;
        }
        name[j] = '\0';
    }
    if (j == 0) return 0;
    if (!strcmp(name, "POINT")) return TG_POINT;
    if (!strcmp(name, "LINESTRING")) return TG_LINESTRING;
    if (!strcmp(name, "POLYGON")) return TG_POLYGON;
    if (!strcmp(name, "MULTIPOINT")) return TG_MULTIPOINT;
    if (!strcmp(name, "MULTILINESTRING")) return TG_MULTILINESTRING;
    if (!strcmp(name, "MULTIPOLYGON")) return TG_MULTIPOLYGON;
    if (!strcmp(name, "GEOMETRYCOLLECTION")) return TG_GEOMETRYCOLLECTION;
    if (strchr(name, ' ')) return -1;
bad_type:
    // determine the length of the bad type
    i = 0;
    for (; i < len; i++) {
        char c = wkt[i];
        if (isws(c)) break;
        if (c >= 'a' && c <= 'z') c -= 32;
        if (c < 'A' || c > 'Z') break;
    }
    return -(i+1);
}

static long wkt_balance_coords(const char *wkt, long len, long i) {
    i++; // first '(' already checked by caller
    long depth = 1;
    long maxdepth = 1;
    for (; i < len; i++) {
        if (wkt[i] == '(') {
            depth++;
            maxdepth++;
        } else if (wkt[i] == ')') {
            depth--;
            if (depth == 0) {
                if (maxdepth > MAXDEPTH) {
                    return -(i+1); 
                }
                return i+1;
            }
        }
    }
    return -(i+1); 
}

static long wkt_vnumber(const char *data, long dlen, long i) {
    // sign
    if (data[i] == '-') {
        i++;
        if (i == dlen) return -(i+1);
    }
    // int
    if ((data[i] < '0' || data[i] > '9') && data[i] != '.') return -(i+1);
    for (; i < dlen; i++) {
        if (data[i] >= '0' && data[i] <= '9') continue;
        break;
    }
    // frac
    if (i == dlen) return i;
    if (data[i] == '.') {
        i++;
        if (i == dlen) return -(i+1);
        if (data[i] < '0' || data[i] > '9') return -(i+1);
        i++;
        for (; i < dlen; i++) {
            if (data[i] >= '0' && data[i] <= '9') continue;
            break;
        }
    }
    // exp
    if (i == dlen) return i;
    if (data[i] == 'e' || data[i] == 'E') {
        i++;
        if (i == dlen) return -(i+1);
        if (data[i] == '+' || data[i] == '-') i++;
        if (i == dlen) return -(i+1);
        if (data[i] < '0' || data[i] > '9') return -(i+1);
        i++;
        for (; i < dlen; i++) {
            if (data[i] >= '0' && data[i] <= '9') continue;
            break;
        }
    }
    return i;
}

static const char *err_for_wkt_posn(int dims) {
    if (dims == 2) {
        return "each position must have two numbers";
    } else if (dims == 3) {
        return "each position must have three numbers";
    } else if (dims == 4) {
        return "each position must have four numbers";
    } else {
        return "each position must have two to four numbers";
    }
}

static struct tg_geom *parse_wkt_point(const char *wkt, long len, 
    bool z, bool m, enum tg_index ix)
{
    (void)ix;
    int dims = z ? m ? 4 : 3 : m ? 3 : 0;
    long i = 0;
    double posn[4] = { 0 };
    int pdims = 0;
    i = wkt_trim_ws(wkt, len, i);
    if (i == len) goto bad_dims;
    while (1) {
        long s = i;
        if ((i = wkt_vnumber(wkt, len, i)) < 0) {
            return make_parse_error(wkt_invalid_err("invalid number"));
        }
        if (pdims < 4) {
            posn[pdims++] = strtod(wkt+s, NULL);
        } else goto bad_dims;
        if (i == len) break;
        if (isws(wkt[i])) {
            i = wkt_trim_ws(wkt, len, i);
            if (i == len) break;
        } else {
            return make_parse_error(wkt_invalid_err("invalid number"));
        }
    }
    enum flags flags = 0;
    if (dims == 0) {
        if (pdims < 2) goto bad_dims;
    } else {
        if (pdims != dims) goto bad_dims;
    }
    dims = pdims;
    struct tg_point pt = { posn[0], posn[1] };
    struct tg_geom *geom = NULL;
    if (dims == 2) {
        geom = tg_geom_new_point(pt);
    } else if (dims == 3) {
        if (m) {
            geom = tg_geom_new_point_m(pt, posn[2]);
        } else {
            geom = tg_geom_new_point_z(pt, posn[2]);
        }
    } else {
        geom = tg_geom_new_point_zm(pt, posn[2], posn[3]);
    }
    if (geom) geom->head.flags |= flags;
    return geom;
bad_dims:
    return make_parse_error("%s", err_for_wkt_posn(dims));
}

static int parse_wkt_posns(enum base base, int dims, int depth, const char *wkt, 
    long len, struct dvec *posns, struct dvec *xcoords, const char **err)
{
    (void)depth; // TODO: return correct depth errors
    double posn[4] = { 0 };
    int pdims = 0;
    long i = 0;
    i = wkt_trim_ws(wkt, len, i);
    if (i == len) {
        // err: expected numbers
        goto exp_nums;
    }
    bool xparens = false;
    if (base == BASE_POINT && wkt[i] == '(') {
        // The multipoint is using the format 'MULTIPOINT ((1 2),(3 4))'.
        // While not standard, it must be supported.
        xparens = true;
    }
    while (i < len) {
        if (xparens) {
            if (i == len || wkt[i] != '(') {
                // err: expected '('
                // if (wkt[i] == 'e' || wkt[i] == 'E') {
                //     (wkt[i+1] == 'm' || wkt[i+1] == 'M')
                //     (wkt[i+2] == 'p' || wkt[i+2] == 'P')
                //     (wkt[i+3] == 't' || wkt[i+3] == 'T')
                //     (wkt[i+4] == 'y' || wkt[i+4] == 'Y')
                //     if (i+5
                //     // EMPTY ?
                // }
                // printf(">>> %c\n", wkt[i]);
                *err = wkt_invalid_err("expected '('");
                return -1;
            }
            i = wkt_trim_ws(wkt, len, i+1);
        }
        // read each number, delimted by whitespace
        while (i < len) {
            double num;
            if (isnum(wkt[i])) {
                long s = i;
                i = wkt_vnumber(wkt, len, i);
                if (i < 0) {
                    *err = wkt_invalid_err("invalid number");
                    return -1;
                }
                num = strtod(wkt+s, NULL);
            } else if (wkt[i] == ')') {
                // err: expected a number, got ')'
                *err = wkt_invalid_err("expected number, got '('");
                return -1;
            } else if (wkt[i] == ',') {
                *err = wkt_invalid_err("expected number, got ','");
                return -1;
            } else {
                *err = wkt_invalid_err("expected a number");
                return -1;
            }
            if (pdims == 4) {
                *err = err_for_wkt_posn(dims);
                return -1;
            }
            posn[pdims++] = num;
            if (i == len || !isws(wkt[i])) break;
            if ((i = wkt_trim_ws(wkt, len, i+1)) == len) break;
            if (wkt[i] == ')' || wkt[i] == ',') break;
        }
        if (xparens) {
            if (i == len || wkt[i] != ')') {
                *err = wkt_invalid_err("expected ')'");
                return -1;
            }
            i = wkt_trim_ws(wkt, len, i+1);
        }
        if (i < len) {
            if (wkt[i] != ',') {
                *err = wkt_invalid_err("expected ','");
                return -1;
            }
            i = wkt_trim_ws(wkt, len, i+1);
            if (i == len) {
                *err = wkt_invalid_err("expected position, got end of stream");
                return -1;
            }
        }
        if (dims != pdims) {
            if (dims == 0 && pdims >= 2) {
                dims = pdims;
            } else {
                *err = err_for_wkt_posn(dims);
                return -1;
            }
        }
        if (!dvec_append(posns, posn[0]) || !dvec_append(posns, posn[1])) {
            return -1;
        }
        for (int i = 2; i < dims; i++) {
            if (i >= pdims || !dvec_append(xcoords, posn[i])) return -1;
        }
        pdims = 0;
    }
exp_nums:
    if (!check_parse_posns(base, posns->data, posns->len, err)) return -1;
    return dims;
}

static int parse_wkt_multi_posns(enum base base, int dims, int depth, 
    const char *wkt, long len, struct dvec *posns, struct rvec *rings,  
    struct tg_poly **poly, struct dvec *xcoords, enum tg_index ix,
    const char **err)
{
    long i = 0;
    i = wkt_trim_ws(wkt, len, i);
    while (i < len) {
        if (wkt[i] != '(') {
            *err = wkt_invalid_err("expected '('");
            return -1;
        }
        long j = wkt_balance_coords(wkt, len, i);
        const char *grp_wkt = wkt+i+1;
        long grp_len = j-i-2;
        i = j;
        posns->len = 0;
        dims = parse_wkt_posns(base, dims, depth, grp_wkt, grp_len, posns, 
            xcoords, err);
        if (dims == -1) return -1;
        struct tg_ring *ring = tg_ring_new_ix((struct tg_point*)posns->data, 
            posns->len / 2, ix);
        if (!ring) return -1;
        if (!rvec_append(rings, ring)) {
            tg_ring_free(ring);
            return -1;
        }
        i = wkt_trim_ws(wkt, len, i);
        if (i == len) break;
        if (wkt[i] != ',') {
            *err = wkt_invalid_err("expected ','");
            return -1;
        }
        i = wkt_trim_ws(wkt, len, i+1);
        if (i == len) {
            *err = wkt_invalid_err("expected '(', got end of stream");
            return -1;
        }
    }
    if (rings->len == 0) {
        *err = "polygons must have one or more rings";
        return -1;
    }
    *poly = tg_poly_new(rings->data[0], 
        (struct tg_ring const*const*)rings->data+1, rings->len-1);
    if (!*poly) return -1;
    for (size_t i = 0; i < rings->len; i++) {
        tg_ring_free(rings->data[i]);
    }
    rings->len = 0;
    return dims;
}

static struct tg_geom *parse_wkt_linestring(const char *wkt, long len, 
    bool z, bool m, enum tg_index ix)
{
    struct tg_geom *geom = NULL;
    struct tg_geom *gerr = NULL;
    struct tg_line *line = NULL;
    struct dvec posns = { 0 };
    struct dvec xcoords = { 0 };
    const char *err = NULL;

    int dims = z ? m ? 4 : 3 : m ? 3 : 0;
    dims = parse_wkt_posns(BASE_LINE, dims, 1, wkt, len, &posns, &xcoords,
        &err);
    if (dims == -1) {
        gerr = err ? make_parse_error("%s", err) : NULL;
        goto fail;
    }
    line = tg_line_new_ix((struct tg_point*)posns.data, posns.len / 2, ix);
    if (!line) goto fail;
    switch (dims) {
    case 2: 
        geom = tg_geom_new_linestring(line);
        break;
    case 3:
        if (m) {
            geom = tg_geom_new_linestring_m(line, xcoords.data, xcoords.len);
        } else {
            geom = tg_geom_new_linestring_z(line, xcoords.data, xcoords.len);
        }
        break;
    default: 
        geom = tg_geom_new_linestring_zm(line, xcoords.data, xcoords.len);
        break;
    }
cleanup:
    tg_line_free(line);
    if (posns.data) tg_free(posns.data);
    if (xcoords.data) tg_free(xcoords.data);
    return geom;
fail:
    tg_geom_free(geom);
    geom = gerr;
    gerr = NULL;
    goto cleanup;
}

static struct tg_geom *parse_wkt_polygon(const char *wkt, long len, 
    bool z, bool m, enum tg_index ix)
{
    struct tg_geom *geom = NULL;
    struct tg_geom *gerr = NULL;
    struct dvec posns = { 0 };
    struct dvec xcoords = { 0 };
    struct tg_poly *poly = NULL;
    struct rvec rings = { 0 };
    const char *err = NULL;

    int dims = z ? m ? 4 : 3 : m ? 3 : 0;
    dims = parse_wkt_multi_posns(BASE_RING, dims, 2, wkt, len, &posns, 
        &rings, &poly, &xcoords, ix, &err);
    if (dims == -1) {
        gerr = err ? make_parse_error("%s", err) : NULL;
        goto fail;
    }
    switch (dims) {
    case 2: 
        geom = tg_geom_new_polygon(poly);
        break;
    case 3:
        if (m) {
            geom = tg_geom_new_polygon_m(poly, xcoords.data, xcoords.len);
        } else {
            geom = tg_geom_new_polygon_z(poly, xcoords.data, xcoords.len);
        }
        break;
    default: 
        geom = tg_geom_new_polygon_zm(poly, xcoords.data, xcoords.len);
        break;
    }
cleanup:
    tg_poly_free(poly);
    if (posns.data) tg_free(posns.data);
    if (xcoords.data) tg_free(xcoords.data);
    if (rings.data) {
        for (size_t i = 0; i < rings.len; i++) {
            tg_ring_free(rings.data[i]);
        }
        tg_free(rings.data);
    }
    return geom;
fail:
    tg_geom_free(geom);
    geom = gerr;
    gerr = NULL;
    goto cleanup;
}

static struct tg_geom *parse_wkt_multipoint(const char *wkt, long len, 
    bool z, bool m, enum tg_index ix)
{
    (void)ix;
    struct tg_geom *geom = NULL;
    struct tg_geom *gerr = NULL;
    struct dvec posns = { 0 };
    struct dvec xcoords = { 0 };
    const char *err = NULL;

    int dims = z ? m ? 4 : 3 : m ? 3 : 0;
    dims = parse_wkt_posns(BASE_POINT, dims, 1, wkt, len, &posns, &xcoords,
        &err);
    if (dims == -1) {
        gerr = err ? make_parse_error("%s", err) : NULL;
        goto fail;
    }
    const struct tg_point *points = (struct tg_point*)posns.data;
    int npoints = posns.len/2;
    switch (dims) {
    case 2: 
        geom = tg_geom_new_multipoint(points, npoints);
        break;
    case 3: 
        if (m) {
            geom = tg_geom_new_multipoint_m(points, npoints, 
                xcoords.data, xcoords.len);
        } else {
            geom = tg_geom_new_multipoint_z(points, npoints, 
                xcoords.data, xcoords.len);
        }
        break;
    default: 
        geom = tg_geom_new_multipoint_zm(points, npoints, 
            xcoords.data, xcoords.len);
        break;
    }
cleanup:
    if (posns.data) tg_free(posns.data);
    if (xcoords.data) tg_free(xcoords.data);
    return geom;
fail:
    tg_geom_free(geom);
    geom = gerr;
    gerr = NULL;
    goto cleanup;
}

static struct tg_geom *parse_wkt_multilinestring(const char *wkt, long len, 
    bool z, bool m, enum tg_index ix)
{
    int dims = z ? m ? 4 : 3 : m ? 3 : 0;
    struct tg_geom *geom = NULL;
    struct tg_geom *gerr = NULL;
    struct dvec posns = { 0 };
    struct dvec xcoords = { 0 };
    struct lvec lines = { 0 };
    const char *err = NULL;
    long i = wkt_trim_ws(wkt, len, 0);
    while (i < len) {
        if (wkt[i] != '(') {
            gerr = make_parse_error(wkt_invalid_err("expected '('"));
            goto fail;
        }
        long j = wkt_balance_coords(wkt, len, i);
        const char *grp_wkt = wkt+i+1;
        long grp_len = j-i-2;
        i = j;
        posns.len = 0;
        dims = parse_wkt_posns(BASE_LINE, dims, 2, grp_wkt, grp_len, &posns,
            &xcoords, &err);
        if (dims == -1) {
            gerr = err ? make_parse_error("%s", err) : NULL;
            goto fail;
        }
        struct tg_line *line = tg_line_new_ix((struct tg_point*)posns.data, 
            posns.len / 2, ix);
        if (!line) goto fail;
        if (!lvec_append(&lines, line)) {
            tg_line_free(line);
            goto fail;
        }
        i = wkt_trim_ws(wkt, len, i);
        if (i == len) break;
        if (wkt[i] != ',') {
            gerr = make_parse_error(wkt_invalid_err("expected ','"));
            goto fail;
        }
        i = wkt_trim_ws(wkt, len, i+1);
        if (i == len) {
            gerr = make_parse_error(wkt_invalid_err("expected '('"));
            goto fail;
        }
    }
    switch (dims) {
    case 2:
        geom = tg_geom_new_multilinestring(
                (struct tg_line const*const*)lines.data, lines.len);
        break;
    case 3:
        if (m) {
            geom = tg_geom_new_multilinestring_m(
                (struct tg_line const*const*)lines.data, lines.len,
                xcoords.data, xcoords.len);
        } else {
            geom = tg_geom_new_multilinestring_z(
                (struct tg_line const*const*)lines.data, lines.len,
                xcoords.data, xcoords.len);
        }
        break;
    default:
        geom = tg_geom_new_multilinestring_zm(
            (struct tg_line const*const*)lines.data, lines.len,
            xcoords.data, xcoords.len);
        break;
    }
cleanup:
    if (posns.data) tg_free(posns.data);
    if (xcoords.data) tg_free(xcoords.data);
    if (lines.data) {
        for (size_t i = 0; i < lines.len; i++) {
            tg_line_free(lines.data[i]);
        }
        tg_free(lines.data);
    }
    return geom;
fail:
    tg_geom_free(geom);
    geom = gerr;
    gerr = NULL;
    goto cleanup;
}

static struct tg_geom *parse_wkt_multipolygon(const char *wkt, long len, 
    bool z, bool m, enum tg_index ix)
{
    int dims = z ? m ? 4 : 3 : m ? 3 : 0;
    struct tg_geom *geom = NULL;
    struct tg_geom *gerr = NULL;
    struct tg_poly *poly = NULL;
    struct dvec posns = { 0 };
    struct dvec xcoords = { 0 };
    struct rvec rings = { 0 };
    struct pvec polys = { 0 };
    const char *err = NULL;
    long i = wkt_trim_ws(wkt, len, 0);
    while (i < len) {
        if (wkt[i] != '(') {
            gerr = make_parse_error(wkt_invalid_err("expected '('"));
            goto fail;
        }
        long j = wkt_balance_coords(wkt, len, i);
        const char *grp_wkt = wkt+i+1;
        long grp_len = j-i-2;
        i = j;
        posns.len = 0;
        rings.len = 0;
        dims = parse_wkt_multi_posns(BASE_RING, dims, 3, grp_wkt, grp_len,
            &posns, &rings, &poly, &xcoords, ix, &err);
        if (dims == -1) {
            gerr = err ? make_parse_error("%s", err) : NULL;
            goto fail;
        }
        if (!pvec_append(&polys, poly)) {
            tg_poly_free(poly);
            goto fail;
        }
        i = wkt_trim_ws(wkt, len, i);
        if (i == len) break;
        if (wkt[i] != ',') {
            gerr = make_parse_error(wkt_invalid_err("expected ','"));
            goto fail;
        }
        i = wkt_trim_ws(wkt, len, i+1);
        if (i == len) {
            gerr = make_parse_error(wkt_invalid_err("expected '('"));
            goto fail;
        }
    }
    switch (dims) {
    case 2:
        geom = tg_geom_new_multipolygon(
                (struct tg_poly const*const*)polys.data, polys.len);
        break;
    case 3:
        if (m) {
            geom = tg_geom_new_multipolygon_m(
                (struct tg_poly const*const*)polys.data, polys.len,
                xcoords.data, xcoords.len);
        } else {
            geom = tg_geom_new_multipolygon_z(
                (struct tg_poly const*const*)polys.data, polys.len,
                xcoords.data, xcoords.len);
        }
        break;
    default:
        geom = tg_geom_new_multipolygon_zm(
            (struct tg_poly const*const*)polys.data, polys.len,
            xcoords.data, xcoords.len);
        break;
    }
cleanup:
    if (posns.data) tg_free(posns.data);
    if (xcoords.data) tg_free(xcoords.data);
    if (polys.data) {
        for (size_t i = 0; i < polys.len; i++) {
            tg_poly_free(polys.data[i]);
        }
        tg_free(polys.data);
    }
    if (rings.data) {
        for (size_t i = 0; i < rings.len; i++) {
            tg_ring_free(rings.data[i]);
        }
        tg_free(rings.data);
    }
    return geom;
fail:
    tg_geom_free(geom);
    geom = gerr;
    gerr = NULL;
    goto cleanup;
}

static long wkt_next_geometry(const char *wkt, long len, long i) {
    for (;i < len;i++) {
        if (wkt[i] == ',') break;
        if (wkt[i] == '(') {
            return wkt_balance_coords(wkt, len, i);
        }
    }
    return i;
}

static struct tg_geom *parse_wkt(const char *wkt, long len, enum tg_index ix);

static struct tg_geom *parse_wkt_geometrycollection(const char *wkt, long len, 
    bool z, bool m, enum tg_index ix)
{
    (void)z; (void)m; // not used
    struct tg_geom *geom = NULL;
    struct tg_geom *gerr = NULL;
    struct gvec geoms = { 0 };
    long i = 0;
    size_t commas = 0;
    while (i < len) {
        long s = i;
        i = wkt_next_geometry(wkt, len, i);
        if (i-s > 0) {
            struct tg_geom *child = parse_wkt(wkt+s, i-s, ix);
            if (!child) goto fail;
            if (tg_geom_error(child)) {
                gerr = child; 
                child = NULL;
                goto fail;
            }
            if (!gvec_append(&geoms, child)) {
                tg_geom_free(child);
                goto fail;
            }
        }
        i = wkt_trim_ws(wkt, len, i);
        if (i == len) break;
        if (wkt[i] != ',') {
            gerr = make_parse_error(wkt_invalid_err("expected ','"));
            goto fail;
        }
        i = wkt_trim_ws(wkt, len, i);
        commas++;
        i++;
    }
    if (commas+1 != geoms.len) {
        // err: missing last geometry
        gerr = make_parse_error("missing type");
        goto fail;
    }
    geom = tg_geom_new_geometrycollection(
        (struct tg_geom const*const*)geoms.data, geoms.len);
cleanup:
    if (geoms.data) {
        for (size_t i = 0; i < geoms.len; i++) {
            tg_geom_free(geoms.data[i]);
        }
        tg_free(geoms.data);
    }
    return geom;
fail:
    tg_geom_free(geom);
    geom = gerr;
    gerr = NULL;
    goto cleanup;
}

static struct tg_geom *parse_wkt(const char *wkt, long len, enum tg_index ix) {
    if (len == 0) {
        return make_parse_error("missing type");
    }
    long i = wkt_trim_ws(wkt, len, 0);
    long s = i;
    for (; i < len; i++) if (wkt[i] == '(') break;
    long e = i;
    while (e-1 > s && isws(wkt[e-1])) e--;
    bool z, m, is_empty;
    enum tg_geom_type type = wkt2type(wkt+s, e-s, &z, &m, &is_empty);
    if ((int)type <= 0) {
        int n = (int)type;
        if (n == 0) {
            return make_parse_error("missing type");
        } else if (n == -1) {
            return make_parse_error("invalid type specifier, "
                "expected 'Z', 'M', 'ZM', or 'EMPTY'");
        } else {
            n = (-n)-1;
            return make_parse_error("unknown type '%.*s'", n, wkt+s);
        }
    }
    if (is_empty) {
        switch (type) {
        case TG_POINT: return tg_geom_new_point_empty();
        case TG_LINESTRING: return tg_geom_new_linestring_empty();
        case TG_POLYGON: return tg_geom_new_polygon_empty();
        case TG_MULTIPOINT: return tg_geom_new_multipoint_empty();
        case TG_MULTILINESTRING: return tg_geom_new_multilinestring_empty();
        case TG_MULTIPOLYGON: return tg_geom_new_multipolygon_empty();
        default: return tg_geom_new_geometrycollection_empty();
        }
    }
    if (i == len || wkt[i] != '(') {
        return make_parse_error(wkt_invalid_err("expected '('"));
    }
    long j = wkt_balance_coords(wkt, len, i);
    if (j <= 0) {
        return make_parse_error(wkt_invalid_err("unbalanced '()'"));
    }
    long k = j;
    for (; k < len; k++) {
        if (!isws(wkt[k])) {
            return make_parse_error(
                wkt_invalid_err("too much data after last ')'"));
        }
    }
    // Only use the inner parts of the group. Do not include the parens.
    wkt = wkt+i+1;
    len = j-i-2;
    switch (type) {
    case TG_POINT: return parse_wkt_point(wkt, len, z, m, ix);
    case TG_LINESTRING: return parse_wkt_linestring(wkt, len, z, m, ix);
    case TG_POLYGON: return parse_wkt_polygon(wkt, len, z, m, ix);
    case TG_MULTIPOINT: return parse_wkt_multipoint(wkt, len, z, m, ix);
    case TG_MULTILINESTRING: 
        return parse_wkt_multilinestring(wkt, len, z, m, ix);
    case TG_MULTIPOLYGON: return parse_wkt_multipolygon(wkt, len, z, m, ix);
    default: return parse_wkt_geometrycollection(wkt, len, z, m, ix);
    }
}

/// Parse Well-known text (WKT) with an included data length.
/// @param wkt WKT data
/// @param len Length of data
/// @returns A geometry or an error. Use tg_geom_error() after parsing to check
/// for errors. 
/// @see tg_parse_wkt()
/// @see GeometryParsing
struct tg_geom *tg_parse_wktn(const char *wkt, size_t len) {
    return tg_parse_wktn_ix(wkt, len, TG_DEFAULT);
}

/// Parse Well-known text (WKT).
/// @param wkt A WKT string. Must be null-terminated
/// @returns A geometry or an error. Use tg_geom_error() after parsing to check
/// for errors. 
/// @see tg_parse_wktn()
/// @see tg_parse_wkt_ix()
/// @see tg_parse_wktn_ix()
/// @see tg_geom_error()
/// @see tg_geom_wkt()
/// @see GeometryParsing
struct tg_geom *tg_parse_wkt(const char *wkt) {
    return tg_parse_wktn_ix(wkt, wkt?strlen(wkt):0, TG_DEFAULT);
}

/// Parse Well-known text (WKT) using provided indexing option.
/// @param wkt A WKT string. Must be null-terminated
/// @param ix Indexing option, e.g. TG_NONE, TG_NATURAL, TG_YSTRIPES
/// @returns A geometry or an error. Use tg_geom_error() after parsing to check
/// for errors. 
/// @see tg_parse_wkt()
/// @see tg_parse_wktn_ix()
/// @see GeometryParsing
struct tg_geom *tg_parse_wkt_ix(const char *wkt, enum tg_index ix) {
    return tg_parse_wktn_ix(wkt, wkt?strlen(wkt):0, ix);
}

/// Parse Well-known text (WKT) using provided indexing option. 
/// @param wkt WKT data
/// @param len Length of data
/// @param ix Indexing option, e.g. TG_NONE, TG_NATURAL, TG_YSTRIPES
/// @returns A geometry or an error. Use tg_geom_error() after parsing to check
/// for errors. 
/// @see tg_parse_wkt()
/// @see tg_parse_wkt_ix()
/// @see GeometryParsing
struct tg_geom *tg_parse_wktn_ix(const char *wkt, size_t len, 
    enum tg_index ix)
{
    struct tg_geom *geom = parse_wkt(wkt, len, ix);
    if (!geom) return NULL;
    if ((geom->head.flags&IS_ERROR) == IS_ERROR) {
        struct tg_geom *gerr = make_parse_error("ParseError: %s", geom->error);
        tg_geom_free(geom);
        return gerr;
    }
    return geom;
}

static void write_posn_wkt(struct writer *wr, struct tg_point posn) {
    write_string_double(wr, posn.x);
    write_char(wr, ' ');
    write_string_double(wr, posn.y);
}

static void write_posn_wkt_3(struct writer *wr, struct tg_point posn, 
    double z)
{
    write_string_double(wr, posn.x);
    write_char(wr, ' ');
    write_string_double(wr, posn.y);
    write_char(wr, ' ');
    write_string_double(wr, z);
}

static void write_posn_wkt_4(struct writer *wr, struct tg_point posn, 
    double z, double m)
{
    write_string_double(wr, posn.x);
    write_char(wr, ' ');
    write_string_double(wr, posn.y);
    write_char(wr, ' ');
    write_string_double(wr, z);
    write_char(wr, ' ');
    write_string_double(wr, m);
}

static void write_point_wkt(const struct boxed_point *point, 
    struct writer *wr)
{
    write_string(wr, "POINT(");
    write_posn_wkt(wr, point->point);
    write_char(wr, ')');
}

static int write_ring_points_wkt(struct writer *wr, const struct tg_ring *ring)
{
    for (int i = 0 ; i < ring->npoints; i++) {
        if (i > 0) write_char(wr, ',');
        write_posn_wkt(wr, ring->points[i]);
    }
    return ring->npoints;
}


static int write_ring_points_wkt_3(struct writer *wr, 
    const struct tg_ring *ring, const double *coords, int ncoords)
{
    double z;
    int j = 0;
    for (int i = 0 ; i < ring->npoints; i++) {
        if (i > 0) write_char(wr, ',');
        z = (j < ncoords) ? coords[j++] : 0;
        write_posn_wkt_3(wr, ring->points[i], z);
    }
    return ring->npoints;
}

static int write_ring_points_wkt_4(struct writer *wr, 
    const struct tg_ring *ring, const double *coords, int ncoords)
{
    double z, m;
    int j = 0;
    for (int i = 0 ; i < ring->npoints; i++) {
        if (i > 0) write_char(wr, ',');
        z = (j < ncoords) ? coords[j++] : 0;
        m = (j < ncoords) ? coords[j++] : 0;
        write_posn_wkt_4(wr, ring->points[i], z, m);
    }
    return ring->npoints;
}

static void write_line_wkt(const struct tg_line *line, struct writer *wr) {
    struct tg_ring *ring = (struct tg_ring*)line;
    write_string(wr, "LINESTRING(");
    write_ring_points_wkt(wr, ring);
    write_char(wr, ')');
}

static void write_ring_wkt(const struct tg_ring *ring, struct writer *wr) {
    write_string(wr, "POLYGON((");
    write_ring_points_wkt(wr, ring);
    write_string(wr, "))");
}


static int write_poly_points_wkt(struct writer *wr, 
    const struct tg_poly *poly)
{
    int count = 0;
    write_char(wr, '(');
    write_ring_points_wkt(wr, tg_poly_exterior(poly));
    write_char(wr, ')');
    int nholes = tg_poly_num_holes(poly);
    for (int i = 0 ; i < nholes; i++) {
        write_char(wr, ',');
        write_char(wr, '(');
        count += write_ring_points_wkt(wr, tg_poly_hole_at(poly, i));
        write_char(wr, ')');
    }
    
    return count;
}

static int write_poly_points_wkt_3(struct writer *wr, 
    const struct tg_poly *poly, const double *coords, int ncoords)
{
    int count = 0;
    double *pcoords = (double*)coords;
    const struct tg_ring *exterior = tg_poly_exterior(poly);
    write_char(wr, '(');
    int n = write_ring_points_wkt_3(wr, exterior, pcoords, ncoords);
    write_char(wr, ')');
    count += n;
    ncoords -= n;
    if (ncoords < 0) ncoords = 0;
    pcoords = ncoords == 0 ? NULL : pcoords+n;
    int nholes = tg_poly_num_holes(poly);
    for (int i = 0 ; i < nholes; i++) {
        write_char(wr, ',');
        const struct tg_ring *hole = tg_poly_hole_at(poly, i);
        write_char(wr, '(');
        int n = write_ring_points_wkt_3(wr, hole, pcoords, ncoords);
        write_char(wr, ')');
        count += n;
        ncoords -= n;
        if (ncoords < 0) ncoords = 0;
        pcoords = ncoords == 0 ? NULL : pcoords+n;
    }
    return count;
}

static int write_poly_points_wkt_4(struct writer *wr, 
    const struct tg_poly *poly, const double *coords, int ncoords)
{
    int count = 0;
    double *pcoords = (double*)coords;
    const struct tg_ring *exterior = tg_poly_exterior(poly);
    write_char(wr, '(');
    int n = write_ring_points_wkt_4(wr, exterior, pcoords, ncoords);
    write_char(wr, ')');
    count += n;
    ncoords -= n*2;
    if (ncoords < 0) ncoords = 0;
    pcoords = ncoords == 0 ? NULL : pcoords+(n*2);
    int nholes = tg_poly_num_holes(poly);
    for (int i = 0 ; i < nholes; i++) {
        write_char(wr, ',');
        const struct tg_ring *hole = tg_poly_hole_at(poly, i);
        write_char(wr, '(');
        int n = write_ring_points_wkt_4(wr, hole, pcoords, ncoords);
        write_char(wr, ')');
        count += n;
        ncoords -= n*2;
        if (ncoords < 0) ncoords = 0;
        pcoords = ncoords == 0 ? NULL : pcoords+(n*2);
    }
    return count;
}

static void write_poly_wkt(const struct tg_poly *poly, struct writer *wr) {
    write_string(wr, "POLYGON(");
    write_poly_points_wkt(wr, poly);
    write_char(wr, ')');
}

static void write_zm_def_wkt(struct writer *wr, const struct tg_geom *geom) {
    if ((geom->head.flags&HAS_M) == HAS_M && 
               (geom->head.flags&HAS_Z) != HAS_Z)
    {
        write_string(wr, " M");
    }
}

static void write_geom_point_wkt(const struct tg_geom *geom, 
    struct writer *wr)
{
    write_string(wr, "POINT");
    write_zm_def_wkt(wr, geom);
    if ((geom->head.flags&IS_EMPTY) == IS_EMPTY) {
        write_string(wr, " EMPTY");
    } else {
        write_char(wr, '(');
        if ((geom->head.flags&HAS_Z) == HAS_Z) {
            if ((geom->head.flags&HAS_M) == HAS_M) {
                write_posn_wkt_4(wr, geom->point, geom->z, geom->m);
            } else {
                write_posn_wkt_3(wr, geom->point, geom->z);
            }
        } else if ((geom->head.flags&HAS_M) == HAS_M) {
            write_posn_wkt_3(wr, geom->point, geom->m);
        } else {
            write_posn_wkt(wr, geom->point);
        }
        write_char(wr, ')');
    }
}

static void write_geom_linestring_wkt(const struct tg_geom *geom,
    struct writer *wr)
{
    write_string(wr, "LINESTRING");
    write_zm_def_wkt(wr, geom);
    if ((geom->head.flags&IS_EMPTY) == IS_EMPTY) {
        write_string(wr, " EMPTY");
        return;
    }
    write_char(wr, '(');
    switch (tg_geom_dims(geom)) {
    case 3:
        write_ring_points_wkt_3(wr, (struct tg_ring*)geom->line, 
            geom->coords, geom->ncoords);
        break;
    case 4:
        write_ring_points_wkt_4(wr, (struct tg_ring*)geom->line, 
            geom->coords, geom->ncoords);
        break;
    default:
        write_ring_points_wkt(wr, (struct tg_ring*)geom->line);
        break;
    }
    write_char(wr, ')');
}

static void write_geom_polygon_wkt(const struct tg_geom *geom,
    struct writer *wr)
{
    write_string(wr, "POLYGON");
    write_zm_def_wkt(wr, geom);
    if ((geom->head.flags&IS_EMPTY) == IS_EMPTY) {
        write_string(wr, " EMPTY");
        return;
    }
    write_char(wr, '(');
    switch (tg_geom_dims(geom)) {
    case 3:
        write_poly_points_wkt_3(wr, geom->poly, 
            geom->coords, geom->ncoords);
        break;
    case 4:
        write_poly_points_wkt_4(wr, geom->poly, 
            geom->coords, geom->ncoords);
        break;
    default: // 2
        write_poly_points_wkt(wr, geom->poly);
        break;
    }
    write_char(wr, ')');
}

static void write_geom_multipoint_wkt(const struct tg_geom *geom,
    struct writer *wr)
{
    write_string(wr, "MULTIPOINT");
    write_zm_def_wkt(wr, geom);
    if ((geom->head.flags&IS_EMPTY) == IS_EMPTY || !geom->multi ||
        !geom->multi->ngeoms)
    {
        write_string(wr, " EMPTY");
        return;
    }
    write_char(wr, '(');
    int dims = tg_geom_dims(geom);
    double z, m;
    double *coords = (double *)geom->coords;
    int ncoords = geom->ncoords;
    int j = 0;
    for (int i = 0; i < geom->multi->ngeoms; i++) {
        struct tg_point point = tg_geom_point(geom->multi->geoms[i]);
        if (i > 0) write_char(wr, ',');
        switch (dims) {
        case 3:
            z = (j < ncoords) ? coords[j++] : 0;
            write_posn_wkt_3(wr, point, z);
            break;
        case 4:
            z = (j < ncoords) ? coords[j++] : 0;
            m = (j < ncoords) ? coords[j++] : 0;
            write_posn_wkt_4(wr, point, z, m);
            break;
        default: // 2
            write_posn_wkt(wr, point);
            break;
        }
    }
    write_char(wr, ')');
}

static void write_geom_multilinestring_wkt(const struct tg_geom *geom,
    struct writer *wr)
{
    write_string(wr, "MULTILINESTRING");
    write_zm_def_wkt(wr, geom);
    if ((geom->head.flags&IS_EMPTY) == IS_EMPTY || !geom->multi ||
        !geom->multi->ngeoms)
    {
        write_string(wr, " EMPTY");
        return;
    }
    write_char(wr, '(');
    double *pcoords = (double *)geom->coords;
    int ncoords = geom->ncoords;
    int n;
    for (int i = 0; i < geom->multi->ngeoms; i++) {
        const struct tg_line *line = tg_geom_line(
                geom->multi->geoms[i]);
        const struct tg_ring *ring = (struct tg_ring*)line;
        if (i > 0) write_char(wr, ',');
        write_char(wr, '(');
        switch (tg_geom_dims(geom)) {
        case 3:
            n = write_ring_points_wkt_3(wr, ring, pcoords, ncoords);
            ncoords -= n;
            if (ncoords < 0) ncoords = 0;
            pcoords = ncoords == 0 ? NULL : pcoords+n;
            break;
        case 4:
            n = write_ring_points_wkt_4(wr, ring, pcoords, ncoords);
            ncoords -= n*2;
            if (ncoords < 0) ncoords = 0;
            pcoords = ncoords == 0 ? NULL : pcoords+(n*2);
            break;
        default: // 2
            write_ring_points_wkt(wr, ring);
            break;
        }
        write_char(wr, ')');
    }
    write_char(wr, ')');
}

static void write_geom_multipolygon_wkt(const struct tg_geom *geom,
    struct writer *wr)
{
    write_string(wr, "MULTIPOLYGON");
    write_zm_def_wkt(wr, geom);
    if ((geom->head.flags&IS_EMPTY) == IS_EMPTY || !geom->multi ||
        !geom->multi->ngeoms)
    {
        write_string(wr, " EMPTY");
        return;
    }
    write_char(wr, '(');
    double *pcoords = (double *)geom->coords;
    int ncoords = geom->ncoords;
    int n;
    for (int i = 0; i < geom->multi->ngeoms; i++) {
        const struct tg_poly *poly = tg_geom_poly(
                geom->multi->geoms[i]);
        if (i > 0) write_char(wr, ',');
        write_char(wr, '(');
        switch (tg_geom_dims(geom)) {
        case 3:
            n = write_poly_points_wkt_3(wr, poly, pcoords, ncoords);
            ncoords -= n;
            if (ncoords < 0) ncoords = 0;
            pcoords = ncoords == 0 ? NULL : pcoords+n;
            break;
        case 4:
            n = write_poly_points_wkt_4(wr, poly, pcoords, ncoords);
            ncoords -= n*2;
            if (ncoords < 0) ncoords = 0;
            pcoords = ncoords == 0 ? NULL : pcoords+(n*2);
            break;
        default: // 2
            write_poly_points_wkt(wr, poly);
            break;
        }
        write_char(wr, ')');
    }
    write_char(wr, ')');
}

static void write_geom_wkt(const struct tg_geom *geom, struct writer *wr);

static void write_geom_geometrycollection_wkt(const struct tg_geom *geom,
    struct writer *wr)
{
    write_string(wr, "GEOMETRYCOLLECTION");
    write_zm_def_wkt(wr, geom);
    if ((geom->head.flags&IS_EMPTY) == IS_EMPTY || !geom->multi ||
        !geom->multi->ngeoms)
    {
        write_string(wr, " EMPTY");
        return;
    }
    write_char(wr, '(');
    int ngeoms = tg_geom_num_geometries(geom);
    for (int i = 0; i < ngeoms; i++) {
        if (i > 0) write_char(wr, ',');
        write_geom_wkt(tg_geom_geometry_at(geom, i), wr);
    }
    write_char(wr, ')');
}

static void write_geom_wkt(const struct tg_geom *geom, struct writer *wr) {
    switch (geom->head.type) {
    case TG_POINT:
        write_geom_point_wkt(geom, wr);
        break;
    case TG_LINESTRING:
        write_geom_linestring_wkt(geom, wr);
        break;
    case TG_POLYGON:
        write_geom_polygon_wkt(geom, wr);
        break;
    case TG_MULTIPOINT:
        write_geom_multipoint_wkt(geom, wr);
        break;
    case TG_MULTILINESTRING:
        write_geom_multilinestring_wkt(geom, wr);
        break;
    case TG_MULTIPOLYGON:
        write_geom_multipolygon_wkt(geom, wr);
        break;
    case TG_GEOMETRYCOLLECTION:
        write_geom_geometrycollection_wkt(geom, wr);
        break;
    }
}

/// Writes a Well-known text (WKT) representation of a geometry.
///
/// The content is stored as a C string in the buffer pointed to by dst.
/// A terminating null character is automatically appended after the
/// content written.
///
/// @param geom Input geometry
/// @param dst Buffer where the resulting content is stored.
/// @param n Maximum number of bytes to be used in the buffer.
/// @return  The number of characters, not including the null-terminator, 
/// needed to store the content into the C string buffer.
/// If the returned length is greater than n-1, then only a parital copy
/// occurred, for example:
///
/// ```
/// char str[64];
/// size_t len = tg_geom_wkt(geom, str, sizeof(str));
/// if (len > sizeof(str)-1) {
///     // ... write did not complete ...
/// }
/// ```
///
/// @see tg_geom_geojson()
/// @see tg_geom_wkb()
/// @see tg_geom_hex()
/// @see GeometryWriting
size_t tg_geom_wkt(const struct tg_geom *geom, char *dst, size_t n) {
    if (!geom) return 0;
    struct writer wr = { .dst = (uint8_t*)dst, .n = n };
    switch (geom->head.base) {
    case BASE_GEOM:
        write_geom_wkt(geom, &wr);
        break;
    case BASE_POINT:
        write_point_wkt((struct boxed_point*)geom, &wr);
        break;
    case BASE_LINE:
        write_line_wkt((struct tg_line*)geom, &wr);
        break;
    case BASE_RING:
        write_ring_wkt((struct tg_ring*)geom, &wr);
        break;
    case BASE_POLY:
        write_poly_wkt((struct tg_poly*)geom, &wr);
        break;
    }
    write_nullterm(&wr);
    return wr.count;
}

// wkb

static const char *wkb_invalid_child_type(void) {
    return "invalid child type";
}

static uint32_t read_uint32(const uint8_t *data, bool swap) {
    uint32_t x = (((uint32_t)data[0])<<0)|(((uint32_t)data[1])<<8)|
                 (((uint32_t)data[2])<<16)|(((uint32_t)data[3])<<24);
    return swap ? __builtin_bswap32(x) : x;
}

static uint64_t read_uint64(const uint8_t *data, bool swap) {
    uint64_t x = (((uint64_t)data[0])<<0)|(((uint64_t)data[1])<<8)|
                 (((uint64_t)data[2])<<16)|(((uint64_t)data[3])<<24)|
                 (((uint64_t)data[4])<<32)|(((uint64_t)data[5])<<40)|
                 (((uint64_t)data[6])<<48)|(((uint64_t)data[7])<<56);
    return swap ? __builtin_bswap64(x) : x;
}

static double read_double(const uint8_t *data, bool le) {
    return ((union raw_double){.u=read_uint64(data, le)}).d;
}

#define read_uint32(name) { \
    if (i+4 > len) goto invalid; \
    uint32_t _val_ = read_uint32(wkb+i, swap); \
    i += 4; \
    (name) = _val_; \
}

#define read_posn(posn) { \
    if (i+(8*dims) > len) goto invalid; \
    for (int j = 0; j < dims; j++) { \
        (posn)[j] = read_double(wkb+i, swap); \
        i += 8; \
    } \
}

static const char *wkb_invalid_err(void) {
    return "invalid binary";
}

#define PARSE_FAIL SIZE_MAX

// returns the updated wkb index.
static size_t parse_wkb_posns(enum base base, int dims, 
    const uint8_t *wkb, size_t len, size_t i, bool swap,
    struct dvec *posns, struct dvec *xcoords,
    struct tg_point **points, int *npoints,
    const char **err)
{
    *err = NULL;
    double posn[4];
    uint32_t count;
    read_uint32(count);
    if (count == 0) return i;
    if (dims == 2 && !swap && len-i >= count*2*8) {
        // Use the point data directly. No allocations. 
        *points = (void*)(wkb+i);
        *npoints = count;
        i += count*2*8; 
    } else {
        for (uint32_t j = 0 ; j < count; j++) {
            read_posn(posn);
            for (int i = 0; i < 2; i++) {
                if (!dvec_append(posns, posn[i])) {
                    return PARSE_FAIL;
                }
            }
            for (int i = 2; i < dims; i++) {
                if (!dvec_append(xcoords, posn[i])) {
                    return PARSE_FAIL;
                }
            }
        }
        *points = (void*)posns->data;
        *npoints = posns->len / 2;
    }
    if (!check_parse_posns(base, (void*)(*points), (*npoints) * 2, err)) {
        return PARSE_FAIL;
    }
    return i;
invalid:
    *err = wkb_invalid_err();
    return PARSE_FAIL;
}

static size_t parse_wkb_multi_posns(enum base base, int dims, 
    const uint8_t *wkb, size_t len, size_t i, bool swap, struct dvec *posns, 
    struct rvec *rings,  struct tg_poly **poly, struct dvec *xcoords, 
    enum tg_index ix, const char **err)
{
    *err = NULL;
    uint32_t count;
    read_uint32(count);
    if (count == 0) return i;
    for (uint32_t j = 0 ; j < count; j++) {
        struct tg_point *points = NULL;
        int npoints = 0;
        posns->len = 0;
        i = parse_wkb_posns(base, dims, wkb, len, i, swap, posns, xcoords, 
            &points, &npoints, err);
        if (i == PARSE_FAIL) return PARSE_FAIL;
        struct tg_ring *ring = tg_ring_new_ix(points, npoints, ix);
        if (!ring) return PARSE_FAIL;
        if (!rvec_append(rings, ring)) {
            tg_ring_free(ring);
            return PARSE_FAIL;
        }
    }
    *poly = tg_poly_new(rings->data[0], 
        (struct tg_ring const*const*)rings->data+1, rings->len-1);
    if (!*poly) return PARSE_FAIL;
    for (size_t i = 0; i < rings->len; i++) {
        tg_ring_free(rings->data[i]);
    }
    rings->len = 0;
    return i;
invalid:
    *err = wkb_invalid_err();
    return PARSE_FAIL;
}

static size_t parse_wkb_point(const uint8_t *wkb, size_t len, size_t i,
    bool swap, bool z, bool m, int depth, enum tg_index ix, 
    struct tg_geom **gout)
{
    (void)depth; (void)ix;
    int dims = z ? m ? 4 : 3 : m ? 3 : 2;
    double posn[4];
    read_posn(posn);
    struct tg_geom *geom = NULL;
    if (isnan(posn[0])) {
        bool empty = true;
        for (int i = 1; i < dims; i++) {
            if (!isnan(posn[i])) {
                empty = false;
                break;
            }
        }
        if (empty) {
            geom = tg_geom_new_point_empty();
            *gout = geom;
            return i;
        }
    }
    struct tg_point pt = { posn[0], posn[1] };
    if (dims == 2) {
        geom = tg_geom_new_point(pt);
    } else if (dims == 3) {
        if (m) {
            geom = tg_geom_new_point_m(pt, posn[2]);
        } else {
            geom = tg_geom_new_point_z(pt, posn[2]);
        }
    } else {
        geom = tg_geom_new_point_zm(pt, posn[2], posn[3]);
    }
    *gout = geom;
    return i;
invalid:
    *gout = make_parse_error(wkb_invalid_err());
    return PARSE_FAIL;
}

static size_t parse_wkb_linestring(const uint8_t *wkb, size_t len, size_t i, 
    bool swap, bool z, bool m, int depth, enum tg_index ix, 
    struct tg_geom **gout)
{
    (void)depth;
    struct tg_geom *gerr = NULL;
    struct tg_geom *geom = NULL;
    struct tg_line *line = NULL;
    struct dvec posns = { 0 };
    struct dvec xcoords = { 0 };
    struct tg_point *points = NULL;
    int npoints = 0;
    const char *err = NULL;
    int dims = z ? m ? 4 : 3 : m ? 3 : 2;
    i = parse_wkb_posns(BASE_LINE, dims, wkb, len, i, swap, &posns, &xcoords, 
        &points, &npoints, &err);
    if (i == PARSE_FAIL) {
        gerr = err ? make_parse_error("%s", err) : NULL;
        goto fail;
    }
    if (npoints == 0) {
        geom = tg_geom_new_linestring_empty();
        goto cleanup;
    }
    line = tg_line_new_ix(points, npoints, ix);
    if (!line) goto fail;
    switch (dims) {
    case 2: 
        geom = tg_geom_new_linestring(line);
        break;
    case 3:
        if (m) {
            geom = tg_geom_new_linestring_m(line, xcoords.data, xcoords.len);
        } else {
            geom = tg_geom_new_linestring_z(line, xcoords.data, xcoords.len);
        }
        break;
    default: 
        geom = tg_geom_new_linestring_zm(line, xcoords.data, xcoords.len);
        break;
    }
cleanup:
    tg_line_free(line);
    if (posns.data) tg_free(posns.data);
    if (xcoords.data) tg_free(xcoords.data);
    *gout = geom;
    return i;
fail:
    tg_geom_free(geom);
    geom = gerr;
    gerr = NULL;
    goto cleanup;
}

static size_t parse_wkb_polygon(const uint8_t *wkb, size_t len,
    size_t i, bool swap, bool z, bool m, int depth, enum tg_index ix,
    struct tg_geom **gout)
{
    (void)depth;
    struct tg_geom *geom = NULL;
    struct tg_geom *gerr = NULL;
    struct dvec posns = { 0 };
    struct dvec xcoords = { 0 };
    struct tg_poly *poly = NULL;
    struct rvec rings = { 0 };
    const char *err = NULL;
    int dims = z ? m ? 4 : 3 : m ? 3 : 2;
    i = parse_wkb_multi_posns(BASE_RING, dims, wkb, len, i, swap, &posns, 
        &rings, &poly, &xcoords, ix, &err);
    if (i == PARSE_FAIL) {
        gerr = err ? make_parse_error("%s", err) : NULL;
        goto fail;
    }
    if (!poly) {
        geom = tg_geom_new_polygon_empty();
        goto cleanup;
    }
    switch (dims) {
    case 2: 
        geom = tg_geom_new_polygon(poly);
        break;
    case 3:
        if (m) {
            geom = tg_geom_new_polygon_m(poly, xcoords.data, xcoords.len);
        } else {
            geom = tg_geom_new_polygon_z(poly, xcoords.data, xcoords.len);
        }
        break;
    default: 
        geom = tg_geom_new_polygon_zm(poly, xcoords.data, xcoords.len);
        break;
    }
cleanup:
    tg_poly_free(poly);
    if (posns.data) tg_free(posns.data);
    if (xcoords.data) tg_free(xcoords.data);
    if (rings.data) {
        for (size_t i = 0; i < rings.len; i++) {
            tg_ring_free(rings.data[i]);
        }
        tg_free(rings.data);
    }
    *gout = geom;
    return i;
fail:
    tg_geom_free(geom);
    geom = gerr;
    gerr = NULL;
    goto cleanup;
}

static size_t parse_wkb(const uint8_t *wkb, size_t len, size_t i, int depth,
    enum tg_index ix, struct tg_geom **g);

static bool wkb_type_match(const struct tg_geom *child, enum tg_geom_type type, 
    bool z, bool m)
{
    bool child_has_z = (child->head.flags&HAS_Z) == HAS_Z;
    bool child_has_m = (child->head.flags&HAS_M) == HAS_M;
    return child->head.type == type && child_has_z == z && child_has_m == m;
}

static size_t parse_wkb_multipoint(const uint8_t *wkb, size_t len, size_t i,
    bool swap, bool z, bool m, int depth, enum tg_index ix, 
    struct tg_geom **gout)
{
    int dims = z ? m ? 4 : 3 : m ? 3 : 2;

    struct dvec posns = { 0 };
    struct dvec xcoords = { 0 };
    struct tg_geom *geom = NULL;
    struct tg_geom *gerr = NULL;
    struct tg_geom *point = NULL;

    uint32_t count;
    read_uint32(count);
    for (size_t j = 0; j < count; j++) {
        i = parse_wkb(wkb, len, i, depth+1, ix, &point);
        if (!point || i == PARSE_FAIL ||  tg_geom_error(point)) {
            gerr = point;
            point = NULL;
            goto fail;
        }
        if (!wkb_type_match(point, TG_POINT, z, m)) {
            gerr = make_parse_error(wkb_invalid_child_type());
            goto fail;
        }
        struct tg_point pt = tg_geom_point(point);
        if (!dvec_append(&posns, pt.x) || !dvec_append(&posns, pt.y)) goto fail;
        if (z || m) {
            if (z && m) {
                if (!dvec_append(&xcoords, tg_geom_z(point))||
                    !dvec_append(&xcoords, tg_geom_m(point))) {
                        goto fail;
                    }
            } else if (z) {
                if (!dvec_append(&xcoords, tg_geom_z(point))) goto fail;
            } else if (m) {
                if (!dvec_append(&xcoords, tg_geom_m(point))) goto fail;
            }
        }
        tg_geom_free(point);
        point = NULL;
    }

    const struct tg_point *points = (struct tg_point*)posns.data;
    int npoints = posns.len/2;
    switch (dims) {
    case 2: 
        geom = tg_geom_new_multipoint(points, npoints);
        break;
    case 3: 
        if (m) {
            geom = tg_geom_new_multipoint_m(points, npoints, 
                xcoords.data, xcoords.len);
        } else {
            geom = tg_geom_new_multipoint_z(points, npoints, 
                xcoords.data, xcoords.len);
        }
        break;
    default: 
        geom = tg_geom_new_multipoint_zm(points, npoints, 
            xcoords.data, xcoords.len);
        break;
    }
cleanup:
    if (point) tg_geom_free(point);
    if (posns.data) tg_free(posns.data);
    if (xcoords.data) tg_free(xcoords.data);
    *gout = geom;
    return i;
fail:
    tg_geom_free(geom);
    geom = gerr;
    gerr = NULL;
    goto cleanup;
invalid:
    gerr = make_parse_error(wkb_invalid_err());
    goto fail;
}

static size_t parse_wkb_multilinestring(const uint8_t *wkb, size_t len,
    size_t i, bool swap, bool z, bool m, int depth, enum tg_index ix,
    struct tg_geom **gout)
{
    int dims = z ? m ? 4 : 3 : m ? 3 : 2;

    struct dvec xcoords = { 0 };
    struct tg_geom *geom = NULL;
    struct tg_geom *gerr = NULL;
    struct tg_geom *child = NULL;
    struct lvec lines = { 0 };

    uint32_t count;
    read_uint32(count);
    for (size_t j = 0; j < count; j++) {
        i = parse_wkb(wkb, len, i, depth+1, ix, &child);
        if (!child || i == PARSE_FAIL || tg_geom_error(child)) {
            gerr = child;
            child = NULL;
            goto fail;
        }
        if (!wkb_type_match(child, TG_LINESTRING, z, m)) {
            gerr = make_parse_error(wkb_invalid_child_type());
            goto fail;
        }
        struct tg_line *line = tg_line_clone(tg_geom_line(child));
        if (!lvec_append(&lines, line)) {
            tg_line_free(line);
            goto fail;
        }
        const double *coords = tg_geom_extra_coords(child);
        int ncoords = tg_geom_num_extra_coords(child);
        for (int i = 0; i < ncoords; i++) {
            if (!dvec_append(&xcoords, coords[i])) goto fail;
        }
        tg_geom_free(child);
        child = NULL;
    }
    switch (dims) {
    case 2:
        geom = tg_geom_new_multilinestring(
                (struct tg_line const*const*)lines.data, lines.len);
        break;
    case 3:
        if (m) {
            geom = tg_geom_new_multilinestring_m(
                (struct tg_line const*const*)lines.data, lines.len,
                xcoords.data, xcoords.len);
        } else {
            geom = tg_geom_new_multilinestring_z(
                (struct tg_line const*const*)lines.data, lines.len,
                xcoords.data, xcoords.len);
        }
        break;
    default:
        geom = tg_geom_new_multilinestring_zm(
            (struct tg_line const*const*)lines.data, lines.len,
            xcoords.data, xcoords.len);
        break;
    }
cleanup:
    if (child) tg_geom_free(child);
    if (xcoords.data) tg_free(xcoords.data);
    if (lines.data) {
        for (size_t i = 0; i < lines.len; i++) {
            tg_line_free(lines.data[i]);
        }
        tg_free(lines.data);
    }
    *gout = geom;
    return i;
fail:
    tg_geom_free(geom);
    geom = gerr;
    gerr = NULL;
    goto cleanup;
invalid:
    gerr = make_parse_error(wkb_invalid_err());
    goto fail;
}

static size_t parse_wkb_multipolygon(const uint8_t *wkb, size_t len,
    size_t i, bool swap, bool z, bool m, int depth, enum tg_index ix, 
    struct tg_geom **gout)
{
    int dims = z ? m ? 4 : 3 : m ? 3 : 2;

    struct dvec xcoords = { 0 };
    struct tg_geom *geom = NULL;
    struct tg_geom *gerr = NULL;
    struct tg_geom *child = NULL;
    struct pvec polys = { 0 };

    uint32_t count;
    read_uint32(count);
    for (size_t j = 0; j < count; j++) {
        i = parse_wkb(wkb, len, i, depth+1, ix, &child);
        if (!child || i == PARSE_FAIL || tg_geom_error(child)) {
            gerr = child;
            child = NULL;
            goto fail;
        }
        if (!wkb_type_match(child, TG_POLYGON, z, m)) {
            gerr = make_parse_error(wkb_invalid_child_type());
            goto fail;
        }
        struct tg_poly *poly = tg_poly_clone(tg_geom_poly(child));
        if (!pvec_append(&polys, poly)) {
            tg_poly_free(poly);
            goto fail;
        }
        const double *coords = tg_geom_extra_coords(child);
        int ncoords = tg_geom_num_extra_coords(child);
        for (int i = 0; i < ncoords; i++) {
            if (!dvec_append(&xcoords, coords[i])) goto fail;
        }
        tg_geom_free(child);
        child = NULL;
    }
    switch (dims) {
    case 2:
        geom = tg_geom_new_multipolygon(
                (struct tg_poly const*const*)polys.data, polys.len);
        break;
    case 3:
        if (m) {
            geom = tg_geom_new_multipolygon_m(
                (struct tg_poly const*const*)polys.data, polys.len,
                xcoords.data, xcoords.len);
        } else {
            geom = tg_geom_new_multipolygon_z(
                (struct tg_poly const*const*)polys.data, polys.len,
                xcoords.data, xcoords.len);
        }
        break;
    default:
        geom = tg_geom_new_multipolygon_zm(
            (struct tg_poly const*const*)polys.data, polys.len,
            xcoords.data, xcoords.len);
        break;
    }
cleanup:
    if (child) tg_geom_free(child);
    if (xcoords.data) tg_free(xcoords.data);
    if (polys.data) {
        for (size_t i = 0; i < polys.len; i++) {
            tg_poly_free(polys.data[i]);
        }
        tg_free(polys.data);
    }
    *gout = geom;
    return i;
fail:
    tg_geom_free(geom);
    geom = gerr;
    gerr = NULL;
    goto cleanup;
invalid:
    gerr = make_parse_error(wkb_invalid_err());
    goto fail;
}

static size_t parse_wkb_geometrycollection(const uint8_t *wkb, size_t len, 
    size_t i, bool swap, bool z, bool m, int depth, enum tg_index ix,
    struct tg_geom **gout)
{
    (void)z; (void)m; // not used
    struct tg_geom *geom = NULL;
    struct tg_geom *gerr = NULL;
    struct gvec geoms = { 0 };

    uint32_t count;
    read_uint32(count);
    for (size_t j = 0; j < count; j++) {
        struct tg_geom *child = NULL;
        i = parse_wkb(wkb, len, i, depth+1, ix, &child);
        if (!child || i == PARSE_FAIL || tg_geom_error(child)) {
            gerr = child;
            goto fail;
        }
        if (!gvec_append(&geoms, child)) {
            tg_geom_free(child);
            goto fail;
        }
    }
    geom = tg_geom_new_geometrycollection(
        (struct tg_geom const*const*)geoms.data, geoms.len);
cleanup:
    if (geoms.data) {
        for (size_t i = 0; i < geoms.len; i++) {
            tg_geom_free(geoms.data[i]);
        }
        tg_free(geoms.data);
    }
    *gout = geom;
    return i;
fail:
    tg_geom_free(geom);
    geom = gerr;
    gerr = NULL;
    goto cleanup;
invalid:
    gerr = make_parse_error(wkb_invalid_err());
    goto fail;
}

static size_t parse_wkb(const uint8_t *wkb, size_t len, size_t i, int depth,
    enum tg_index ix, struct tg_geom **g)
{
    if (i == len) goto invalid;
    if (wkb[i] >> 1) goto invalid; // not 1 or 0
    
    int d = depth;
    if (d > MAXDEPTH) goto invalid;

    // Set the 'swap' bool which indicates that the wkb numbers need swapping
    // to match the host endianness.
#if BYTE_ORDER == BIG_ENDIAN
    bool swap = wkb[i] == 1;
#elif BYTE_ORDER == LITTLE_ENDIAN
    bool swap = wkb[i] == 0;
#else
    #error "cannot determine byte order"
#endif
    i++;

    uint32_t type;
    read_uint32(type);

    bool has_srid = !!(type & 0x20000000);
    type &= 0xFFFF;
    int srid = 0;
    if (has_srid) {
        // Read the SRID from the extended wkb format.
        uint32_t usrid;
        read_uint32(usrid);
        srid = (int)usrid;
    }
    (void)srid; // Now throw it away.

    bool s = swap;
    switch (type) {
    case    1: return parse_wkb_point(wkb, len, i, s, 0, 0, d, ix, g);
    case 1001: return parse_wkb_point(wkb, len, i, s, 1, 0, d, ix, g);
    case 2001: return parse_wkb_point(wkb, len, i, s, 0, 1, d, ix, g);
    case 3001: return parse_wkb_point(wkb, len, i, s, 1, 1, d, ix, g);
    case    2: return parse_wkb_linestring(wkb, len, i, s, 0, 0, d, ix, g);
    case 1002: return parse_wkb_linestring(wkb, len, i, s, 1, 0, d, ix, g);
    case 2002: return parse_wkb_linestring(wkb, len, i, s, 0, 1, d, ix, g);
    case 3002: return parse_wkb_linestring(wkb, len, i, s, 1, 1, d, ix, g);
    case    3: return parse_wkb_polygon(wkb, len, i, s, 0, 0, d, ix, g); 
    case 1003: return parse_wkb_polygon(wkb, len, i, s, 1, 0, d, ix, g); 
    case 2003: return parse_wkb_polygon(wkb, len, i, s, 0, 1, d, ix, g); 
    case 3003: return parse_wkb_polygon(wkb, len, i, s, 1, 1, d, ix, g); 
    case    4: return parse_wkb_multipoint(wkb, len, i, s, 0, 0, d, ix, g);
    case 1004: return parse_wkb_multipoint(wkb, len, i, s, 1, 0, d, ix, g);
    case 2004: return parse_wkb_multipoint(wkb, len, i, s, 0, 1, d, ix, g);
    case 3004: return parse_wkb_multipoint(wkb, len, i, s, 1, 1, d, ix, g);
    case    5: return parse_wkb_multilinestring(wkb, len, i, s, 0, 0, d, ix, g);
    case 1005: return parse_wkb_multilinestring(wkb, len, i, s, 1, 0, d, ix, g);
    case 2005: return parse_wkb_multilinestring(wkb, len, i, s, 0, 1, d, ix, g);
    case 3005: return parse_wkb_multilinestring(wkb, len, i, s, 1, 1, d, ix, g);
    case    6: return parse_wkb_multipolygon(wkb, len, i, s, 0, 0, d, ix, g);
    case 1006: return parse_wkb_multipolygon(wkb, len, i, s, 1, 0, d, ix, g);
    case 2006: return parse_wkb_multipolygon(wkb, len, i, s, 0, 1, d, ix, g);
    case 3006: return parse_wkb_multipolygon(wkb, len, i, s, 1, 1, d, ix, g);
    case    7: case 1007: case 2007: case 3007: 
        return parse_wkb_geometrycollection(wkb, len, i, s, 0, 0, d, ix, g);
    default: 
        *g = make_parse_error("invalid type");
        return PARSE_FAIL;
    }
invalid:
    *g = make_parse_error("invalid binary");
    return PARSE_FAIL;
}

/// Parse Well-known binary (WKB).
/// @param wkb WKB data
/// @param len Length of data
/// @returns A geometry or an error. Use tg_geom_error() after parsing to check
/// for errors. 
/// @see tg_parse_wkb_ix()
/// @see tg_geom_error()
/// @see tg_geom_wkb()
/// @see GeometryParsing
struct tg_geom *tg_parse_wkb(const uint8_t *wkb, size_t len) {
    return tg_parse_wkb_ix(wkb, len, 0);
}

/// Parse Well-known binary (WKB) using provided indexing option.
/// @param wkb WKB data
/// @param len Length of data
/// @param ix Indexing option, e.g. TG_NONE, TG_NATURAL, TG_YSTRIPES
/// @returns A geometry or an error. Use tg_geom_error() after parsing to check
/// for errors. 
/// @see tg_parse_wkb()
struct tg_geom *tg_parse_wkb_ix(const uint8_t *wkb, size_t len, 
    enum tg_index ix)
{
    struct tg_geom *geom = NULL;
    parse_wkb(wkb, len, 0, 0, ix, &geom);
    if (!geom) return NULL;
    if ((geom->head.flags&IS_ERROR) == IS_ERROR) {
        struct tg_geom *gerr = make_parse_error("ParseError: %s", geom->error);
        tg_geom_free(geom);
        return gerr;
    }
    return geom;
}

static void write_wkb_type(struct writer *wr, const struct head *head) {
    uint32_t type = head->type;
    if ((head->flags&HAS_Z) == HAS_Z) {
        if ((head->flags&HAS_M) == HAS_M) {
            type += 3000;
        } else {
            type += 1000;
        }
    } else if ((head->flags&HAS_M) == HAS_M) {
        type += 2000;
    }
    write_byte(wr, 1);
    write_uint32le(wr, type);
}

static void write_posn_wkb(struct writer *wr, struct tg_point posn) {
#if BYTE_ORDER == LITTLE_ENDIAN
    if (wr->count+16 < wr->n) {
        memcpy(wr->dst+wr->count, &posn, 16);
        wr->count += 16;
        return;
    }
#endif
    write_doublele(wr, posn.x);
    write_doublele(wr, posn.y);
}

static void write_posn_wkb_3(struct writer *wr, struct tg_point posn, double z)
{
#if BYTE_ORDER == LITTLE_ENDIAN
    if (wr->count+24 < wr->n) {
        memcpy(wr->dst+wr->count, ((double[3]){posn.x, posn.y, z}), 24);
        wr->count += 24;
        return;
    }
#endif
    write_doublele(wr, posn.x);
    write_doublele(wr, posn.y);
    write_doublele(wr, z);
}

static void write_posn_wkb_4(struct writer *wr, struct tg_point posn, 
    double z, double m)
{
#if BYTE_ORDER == LITTLE_ENDIAN
    if (wr->count+32 < wr->n) {
        memcpy(wr->dst+wr->count, ((double[4]){posn.x, posn.y, z, m}), 32);
        wr->count += 32;
        return;
    }
#endif
    write_doublele(wr, posn.x);
    write_doublele(wr, posn.y);
    write_doublele(wr, z);
    write_doublele(wr, m);
}

static int write_ring_points_wkb(struct writer *wr, const struct tg_ring *ring)
{
    write_uint32le(wr, ring->npoints);
    size_t needed = ring->npoints*16;
#if BYTE_ORDER == LITTLE_ENDIAN
    if (wr->count+needed <= wr->n) {
        memcpy(wr->dst+wr->count, ring->points, needed);
        wr->count += needed;
        return ring->npoints;
    }
#endif
    if (wr->count >= wr->n) {
        wr->count += needed;
    } else {
        for (int i = 0 ; i < ring->npoints; i++) {
            write_posn_wkb(wr, ring->points[i]);
        }
    }
    return ring->npoints;
}

static int write_ring_points_wkb_3(struct writer *wr, 
    const struct tg_ring *ring, const double *coords, int ncoords)
{
    write_uint32le(wr, ring->npoints);
    size_t needed = ring->npoints*24;
    if (wr->count >= wr->n) {
        wr->count += needed;
    } else {
        double z;
        int j = 0;
        for (int i = 0 ; i < ring->npoints; i++) {
            z = (j < ncoords) ? coords[j++] : 0;
            write_posn_wkb_3(wr, ring->points[i], z);
        }
    }
    return ring->npoints;
}

static int write_ring_points_wkb_4(struct writer *wr, 
    const struct tg_ring *ring, const double *coords, int ncoords)
{
    write_uint32le(wr, ring->npoints);
    size_t needed = ring->npoints*32;
    if (wr->count >= wr->n) {
        wr->count += needed;
    } else {
        double z, m;
        int j = 0;
        for (int i = 0 ; i < ring->npoints; i++) {
            z = (j < ncoords) ? coords[j++] : 0;
            m = (j < ncoords) ? coords[j++] : 0;
            write_posn_wkb_4(wr, ring->points[i], z, m);
        }
    }
    return ring->npoints;
}

static int write_poly_points_wkb(struct writer *wr, 
    const struct tg_poly *poly)
{
    int count = 0;
    int nholes = tg_poly_num_holes(poly);
    write_uint32le(wr, 1+nholes);
    write_ring_points_wkb(wr, tg_poly_exterior(poly));
    for (int i = 0 ; i < nholes; i++) {
        count += write_ring_points_wkb(wr, tg_poly_hole_at(poly, i));
    }    
    return count;
}

static int write_poly_points_wkb_3(struct writer *wr, 
    const struct tg_poly *poly, const double *coords, int ncoords)
{
    int count = 0;
    double *pcoords = (double*)coords;
    int nholes = tg_poly_num_holes(poly);
    write_uint32le(wr, 1+nholes);
    const struct tg_ring *exterior = tg_poly_exterior(poly);
    int n = write_ring_points_wkb_3(wr, exterior, pcoords, ncoords);
    count += n;
    ncoords -= n;
    if (ncoords < 0) ncoords = 0;
    pcoords = ncoords == 0 ? NULL : pcoords+n;
    for (int i = 0 ; i < nholes; i++) {
        const struct tg_ring *hole = tg_poly_hole_at(poly, i);
        int n = write_ring_points_wkb_3(wr, hole, pcoords, ncoords);
        count += n;
        ncoords -= n;
        if (ncoords < 0) ncoords = 0;
        pcoords = ncoords == 0 ? NULL : pcoords+n;
    }
    return count;
}

static int write_poly_points_wkb_4(struct writer *wr, 
    const struct tg_poly *poly, const double *coords, int ncoords)
{
    int count = 0;
    double *pcoords = (double*)coords;
    int nholes = tg_poly_num_holes(poly);
    write_uint32le(wr, 1+nholes);
    const struct tg_ring *exterior = tg_poly_exterior(poly);
    int n = write_ring_points_wkb_4(wr, exterior, pcoords, ncoords);
    count += n;
    ncoords -= n*2;
    if (ncoords < 0) ncoords = 0;
    pcoords = ncoords == 0 ? NULL : pcoords+(n*2);
    for (int i = 0 ; i < nholes; i++) {
        const struct tg_ring *hole = tg_poly_hole_at(poly, i);
        int n = write_ring_points_wkb_4(wr, hole, pcoords, ncoords);
        count += n;
        ncoords -= n*2;
        if (ncoords < 0) ncoords = 0;
        pcoords = ncoords == 0 ? NULL : pcoords+(n*2);
    }
    return count;
}

static void write_point_wkb(struct boxed_point *point, struct writer *wr) {
    write_wkb_type(wr, &point->head);
    write_posn_wkb(wr, point->point);
}

static void write_line_wkb(struct tg_line *line, struct writer *wr) {
    struct tg_ring *ring = (struct tg_ring*)line;
    write_wkb_type(wr, &ring->head);
    write_ring_points_wkb(wr, ring);
}

static void write_ring_wkb(struct tg_ring *ring, struct writer *wr) {
    (void)ring; (void)wr;
    write_wkb_type(wr, &ring->head);
    write_uint32le(wr, 1);
    write_ring_points_wkb(wr, ring);
}

static void write_poly_wkb(struct tg_poly *poly, struct writer *wr) {
    (void)poly; (void)wr;
    write_wkb_type(wr, &poly->head);
    write_poly_points_wkb(wr, poly);
}

static void write_geom_point_wkb(const struct tg_geom *geom, struct writer *wr)
{
    write_wkb_type(wr, &geom->head);
    if ((geom->head.flags&IS_EMPTY) == IS_EMPTY) {
        write_posn_wkb(wr, (struct tg_point){ NAN, NAN });
    } else {
        if ((geom->head.flags&HAS_Z) == HAS_Z) {
            if ((geom->head.flags&HAS_M) == HAS_M) {
                write_posn_wkb_4(wr, geom->point, geom->z, geom->m);
            } else {
                write_posn_wkb_3(wr, geom->point, geom->z);
            }
        } else if ((geom->head.flags&HAS_M) == HAS_M) {
            write_posn_wkb_3(wr, geom->point, geom->m);
        } else {
            write_posn_wkb(wr, geom->point);
        }
    }
}

static void write_geom_linestring_wkb(const struct tg_geom *geom,
    struct writer *wr)
{
    write_wkb_type(wr, &geom->head);
    if ((geom->head.flags&IS_EMPTY) == IS_EMPTY) {
        write_uint32le(wr, 0);
        return;
    }
    switch (tg_geom_dims(geom)) {
    case 3:
        write_ring_points_wkb_3(wr, (struct tg_ring*)geom->line, 
            geom->coords, geom->ncoords);
        break;
    case 4:
        write_ring_points_wkb_4(wr, (struct tg_ring*)geom->line, 
            geom->coords, geom->ncoords);
        break;
    default:
        write_ring_points_wkb(wr, (struct tg_ring*)geom->line);
        break;
    }
}

static void write_geom_polygon_wkb(const struct tg_geom *geom,
    struct writer *wr)
{
    write_wkb_type(wr, &geom->head);
    if ((geom->head.flags&IS_EMPTY) == IS_EMPTY) {
        write_uint32le(wr, 0);
        return;
    }
    switch (tg_geom_dims(geom)) {
    case 3:
        write_poly_points_wkb_3(wr, geom->poly, 
            geom->coords, geom->ncoords);
        break;
    case 4:
        write_poly_points_wkb_4(wr, geom->poly, 
            geom->coords, geom->ncoords);
        break;
    default: // 2
        write_poly_points_wkb(wr, geom->poly);
        break;
    }
}

static void write_geom_multipoint_wkb(const struct tg_geom *geom,
    struct writer *wr)
{
    write_wkb_type(wr, &geom->head);
    if (!geom->multi || (geom->head.flags&IS_EMPTY) == IS_EMPTY) {
        write_uint32le(wr, 0);
        return;
    }
    int dims = tg_geom_dims(geom);
    double z, m;
    double *coords = (double *)geom->coords;
    int ncoords = geom->ncoords;
    int j = 0;
    write_uint32le(wr, geom->multi->ngeoms);
    struct head head = { 
        .type = TG_POINT, 
        .flags = (geom->head.flags&(HAS_Z|HAS_M)),
    };
    for (int i = 0; i < geom->multi->ngeoms; i++) {
        struct tg_point point = tg_geom_point(geom->multi->geoms[i]);
        write_wkb_type(wr, &head);
        switch (dims) {
        case 3:
            z = (j < ncoords) ? coords[j++] : 0;
            write_posn_wkb_3(wr, point, z);
            break;
        case 4:
            z = (j < ncoords) ? coords[j++] : 0;
            m = (j < ncoords) ? coords[j++] : 0;
            write_posn_wkb_4(wr, point, z, m);
            break;
        default: // 2
            write_posn_wkb(wr, point);
            break;
        }
    }
}

static void write_geom_multilinestring_wkb(const struct tg_geom *geom, 
    struct writer *wr)
{
    write_wkb_type(wr, &geom->head);
    if (!geom->multi || (geom->head.flags&IS_EMPTY) == IS_EMPTY) {
        write_uint32le(wr, 0);
        return;
    }
    int dims = tg_geom_dims(geom);
    write_uint32le(wr, geom->multi->ngeoms);
    struct head head = { 
        .type = TG_LINESTRING, 
        .flags = (geom->head.flags&(HAS_Z|HAS_M)),
    };
    const double *pcoords = geom->coords;
    int ncoords = geom->ncoords;
    int n;
    for (int i = 0; i < geom->multi->ngeoms; i++) {
        const struct tg_line *line = tg_geom_line(geom->multi->geoms[i]);
        const struct tg_ring *ring = (struct tg_ring*)line;
        write_wkb_type(wr, &head);
        switch (dims) {
        case 3:
            n = write_ring_points_wkb_3(wr, ring, pcoords, ncoords);
            ncoords -= n;
            if (ncoords < 0) ncoords = 0;
            pcoords = ncoords == 0 ? NULL : pcoords+n;
            break;
        case 4:
            n = write_ring_points_wkb_4(wr, ring, pcoords, ncoords);
            ncoords -= n*2;
            if (ncoords < 0) ncoords = 0;
            pcoords = ncoords == 0 ? NULL : pcoords+(n*2);
            break;
        default: // 2
            write_ring_points_wkb(wr, ring);
            break;
        }
    }
}

static void write_geom_multipolygon_wkb(const struct tg_geom *geom,
    struct writer *wr)
{
    write_wkb_type(wr, &geom->head);
    if (!geom->multi || (geom->head.flags&IS_EMPTY) == IS_EMPTY) {
        write_uint32le(wr, 0);
        return;
    }
    int dims = tg_geom_dims(geom);
    write_uint32le(wr, geom->multi->ngeoms);
    struct head head = { 
        .type = TG_POLYGON, 
        .flags = (geom->head.flags&(HAS_Z|HAS_M)),
    };
    const double *pcoords = geom->coords;
    int ncoords = geom->ncoords;
    int n;
    for (int i = 0; i < geom->multi->ngeoms; i++) {
        const struct tg_poly *poly = tg_geom_poly(geom->multi->geoms[i]);
        write_wkb_type(wr, &head);
        switch (dims) {
        case 3:
            n = write_poly_points_wkb_3(wr, poly, pcoords, ncoords);
            ncoords -= n;
            if (ncoords < 0) ncoords = 0;
            pcoords = ncoords == 0 ? NULL : pcoords+n;
            break;
        case 4:
            n = write_poly_points_wkb_4(wr, poly, pcoords, ncoords);
            ncoords -= n*2;
            if (ncoords < 0) ncoords = 0;
            pcoords = ncoords == 0 ? NULL : pcoords+(n*2);
            break;
        default: // 2
            write_poly_points_wkb(wr, poly);
            break;
        }
    }
}

static void write_geom_wkb(const struct tg_geom *geom, struct writer *wr);

static void write_geom_geometrycollection_wkb(const struct tg_geom *geom, 
    struct writer *wr)
{
    write_wkb_type(wr, &geom->head);
    if (!geom->multi || (geom->head.flags&IS_EMPTY) == IS_EMPTY) {
        write_uint32le(wr, 0);
        return;
    }
    write_uint32le(wr, geom->multi->ngeoms);
    for (int i = 0; i < geom->multi->ngeoms; i++) {
        write_geom_wkb(geom->multi->geoms[i], wr);
    }
}

static void write_geom_wkb(const struct tg_geom *geom, struct writer *wr) {
    switch (geom->head.type) {
    case TG_POINT:
        write_geom_point_wkb(geom, wr);
        break;
    case TG_LINESTRING:
        write_geom_linestring_wkb(geom, wr);
        break;
    case TG_POLYGON:
        write_geom_polygon_wkb(geom, wr);
        break;
    case TG_MULTIPOINT:
        write_geom_multipoint_wkb(geom, wr);
        break;
    case TG_MULTILINESTRING:
        write_geom_multilinestring_wkb(geom, wr);
        break;
    case TG_MULTIPOLYGON:
        write_geom_multipolygon_wkb(geom, wr);
        break;
    case TG_GEOMETRYCOLLECTION:
        write_geom_geometrycollection_wkb(geom, wr);
        break;
    }
}

/// Writes a Well-known binary (WKB) representation of a geometry.
///
/// The content is stored in the buffer pointed by dst.
///
/// @param geom Input geometry
/// @param dst Buffer where the resulting content is stored.
/// @param n Maximum number of bytes to be used in the buffer.
/// @return  The number of characters needed to store the content into the
/// buffer.
/// If the returned length is greater than n, then only a parital copy
/// occurred, for example:
///
/// ```
/// uint8_t buf[64];
/// size_t len = tg_geom_wkb(geom, buf, sizeof(buf));
/// if (len > sizeof(buf)) {
///     // ... write did not complete ...
/// }
/// ```
///
/// @see tg_geom_geojson()
/// @see tg_geom_wkt()
/// @see tg_geom_hex()
/// @see GeometryWriting
size_t tg_geom_wkb(const struct tg_geom *geom, uint8_t *dst, size_t n) {
    if (!geom) return 0;
    struct writer wr = { .dst = dst, .n = n };
    switch (geom->head.base) {
    case BASE_GEOM:
        write_geom_wkb(geom, &wr);
        break;
    case BASE_POINT:
        write_point_wkb((struct boxed_point*)geom, &wr);
        break;
    case BASE_LINE:
        write_line_wkb((struct tg_line*)geom, &wr);
        break;
    case BASE_RING:
        write_ring_wkb((struct tg_ring*)geom, &wr);
        break;
    case BASE_POLY:
        write_poly_wkb((struct tg_poly*)geom, &wr);
        break;
    }
    return wr.count;
}

/// Writes a hex encoded Well-known binary (WKB) representation of a geometry.
///
/// The content is stored as a C string in the buffer pointed to by dst.
/// A terminating null character is automatically appended after the
/// content written.
///
/// @param geom Input geometry
/// @param dst Buffer where the resulting content is stored.
/// @param n Maximum number of bytes to be used in the buffer.
/// @return  The number of characters, not including the null-terminator, 
/// needed to store the content into the C string buffer.
/// If the returned length is greater than n-1, then only a parital copy
/// occurred, for example:
///
/// ```
/// char str[64];
/// size_t len = tg_geom_hex(geom, str, sizeof(str));
/// if (len > sizeof(str)-1) {
///     // ... write did not complete ...
/// }
/// ```
///
/// @see tg_geom_geojson()
/// @see tg_geom_wkt()
/// @see tg_geom_wkb()
/// @see GeometryWriting
size_t tg_geom_hex(const struct tg_geom *geom, char *dst, size_t n) {
    // Geom to hex is done by first writing as wkb, then rewrite as hex.
    // This is done by scanning the wkb in reverse, overwriting the data
    // along the way.
    static const uint8_t hexchars[] = "0123456789ABCDEF";
    size_t count = tg_geom_wkb(geom, (uint8_t*)dst, n);
    if (count == 0) {
        if (n > 0) dst[0] = '\0';
        return 0;
    }
    size_t i = count - 1;
    size_t j = count*2 - 1;
    while (1) {
        if (i < n) {
            uint8_t ch = dst[i];
            if (j < n) dst[j] = hexchars[ch&15];
            if (j-1 < n) dst[j-1] = hexchars[(ch>>4)&15];
        }
        if (i == 0) break;
        i -= 1;
        j -= 2;
    }
    if (count*2 < n) dst[count*2] = '\0';
    else if (n > 0) dst[n-1] = '\0';
    return count*2;
}

static struct tg_geom *parse_hex(const char *hex, size_t len, enum tg_index ix)
{
    const uint8_t _ = 0;
    static const uint8_t hextoks[256] = { 
        _,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,
        _,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,01,2,3,4,5,6,7,8,9,10,_,_,_,_,_,
        _,_,11,12,13,14,15,16,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,
        _,_,_,_,_,11,12,13,14,15,16,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,
    };
    uint8_t *dst = NULL;
    bool must_free = false;
    if (len == 0 || (len&1) == 1) goto invalid;
    uint8_t smallfry[128];
    if (len/2 > sizeof(smallfry)) {
        dst = tg_malloc(len/2);
        if (!dst) return NULL;
        must_free = true;
    } else {
        dst = smallfry;
    }
    size_t j = 0;
    for (size_t i = 0; i < len; i += 2) {
        uint8_t b0 = hextoks[(uint8_t)hex[i+0]];
        uint8_t b1 = hextoks[(uint8_t)hex[i+1]];
        if (b0 == _ || b1 == _) goto invalid;
        dst[j] = ((b0-1)<<4)|(b1-1);
        j++;
    }
    struct tg_geom *geom;
    parse_wkb(dst, len/2, 0, 0, ix, &geom);
    if (must_free) tg_free(dst);
    return geom;
invalid:
    if (must_free) tg_free(dst);
    return make_parse_error(wkb_invalid_err());
}

/// Parse hex encoded Well-known binary (WKB) using provided indexing option.
/// @param hex Hex data
/// @param len Length of data
/// @param ix Indexing option, e.g. TG_NONE, TG_NATURAL, TG_YSTRIPES
/// @returns A geometry or an error. Use tg_geom_error() after parsing to check
/// for errors. 
/// @see tg_parse_hex()
/// @see tg_parse_hex_ix()
/// @see GeometryParsing
struct tg_geom *tg_parse_hexn_ix(const char *hex, size_t len, 
    enum tg_index ix)
{
    struct tg_geom *geom = parse_hex(hex, len, ix);
    if (!geom) return NULL;
    if ((geom->head.flags&IS_ERROR) == IS_ERROR) {
        struct tg_geom *gerr = make_parse_error("ParseError: %s", geom->error);
        tg_geom_free(geom);
        return gerr;
    }
    return geom;
}

/// Parse hex encoded Well-known binary (WKB) using provided indexing option.
/// @param hex Hex string. Must be null-terminated
/// @param ix Indexing option, e.g. TG_NONE, TG_NATURAL, TG_YSTRIPES
/// @returns A geometry or an error. Use tg_geom_error() after parsing to check
/// for errors. 
/// @see tg_parse_hex()
/// @see tg_parse_hexn_ix()
/// @see GeometryParsing
struct tg_geom *tg_parse_hex_ix(const char *hex, enum tg_index ix) {
    return tg_parse_hexn_ix(hex, hex?strlen(hex):0, ix);
}

/// Parse hex encoded Well-known binary (WKB) with an included data length.
/// @param hex Hex data
/// @param len Length of data
/// @returns A geometry or an error. Use tg_geom_error() after parsing to check
/// for errors. 
/// @see tg_parse_hex()
/// @see GeometryParsing
struct tg_geom *tg_parse_hexn(const char *hex, size_t len) {
    return tg_parse_hexn_ix(hex, len, TG_DEFAULT);
}

/// Parse hex encoded Well-known binary (WKB).
/// @param hex A hex string. Must be null-terminated
/// @returns A geometry or an error. Use tg_geom_error() after parsing to check
/// for errors. 
/// @see tg_parse_hexn()
/// @see tg_parse_hex_ix()
/// @see tg_parse_hexn_ix()
/// @see tg_geom_error()
/// @see tg_geom_hex()
/// @see GeometryParsing
struct tg_geom *tg_parse_hex(const char *hex) {
    return tg_parse_hexn_ix(hex, hex?strlen(hex):0, TG_DEFAULT);
}

static double ring_area(const struct tg_ring *ring) {
    if (tg_ring_empty(ring)) return 0;
    // The ring area has already been calculated by process_points.
    return ring->area;
}

static double ring_perimeter(const struct tg_ring *ring) {
    if (tg_ring_empty(ring)) return 0;
    int nsegs = tg_ring_num_segments(ring);
    double perim = 0;
    for (int i = 0; i < nsegs; i++) {
        struct tg_point a = ring->points[i];
        struct tg_point b = ring->points[i+1];
        perim += length(a.x, a.y, b.x, b.y);
    }
    return perim;
}

double tg_ring_polsby_popper_score(const struct tg_ring *ring) {
    // Calculate the polsby-popper score for the ring or line
    // https://en.wikipedia.org/wiki/Polsby–Popper_test 
    //
    // The score is calculated by multiplying the polygon area by 4pi
    // and dividing by the perimeter squared. A perfect circle has a score of 1
    // and all other shapes will be smaller. Itty bitty scores mean the
    // polygon is really something nuts or has bad data.
    double score = 0.0;
    double perim = ring_perimeter(ring);
    double area = ring_area(ring);
    if (perim > 0) {
        score = (area * M_PI * 4) / (perim * perim);
    }
    return score;
}

double tg_line_polsby_popper_score(const struct tg_line *line) {
    const struct tg_ring *ring = (const struct tg_ring*)line;
    return tg_ring_polsby_popper_score(ring);    
}

struct tg_ring *tg_circle_new_ix(struct tg_point center, double radius, 
    int steps, enum tg_index ix)
{
    steps--; 
    radius = radius < 0 ? 0 : radius;
    steps = steps < 3 ? 3 : steps;
    struct tg_ring *ring = NULL;
    struct tg_point *points = tg_malloc(sizeof(struct tg_point)*(steps+1));
    if (points) {
        int i = 0;
        for (double th = 0.0; th <= 360.0; th += 360.0 / (double)steps) {
            double radians = (M_PI / 180.0) * th;
            double x = center.x + radius * cos(radians);
            double y = center.y + radius * sin(radians);
            points[i] = (struct tg_point) { x, y };
            i++;
        }
        // add last connecting point, make a total of steps
        points[steps] = points[0];
        ring = tg_ring_new_ix(points, steps+1, ix);
        tg_free(points);
    }
    return ring;
}

struct tg_ring *tg_circle_new(struct tg_point center, double radius, int steps)
{
    return tg_circle_new_ix(center, radius, steps, 0);
}

double tg_point_distance_segment(struct tg_point p, struct tg_segment s) {
    double a = p.x - s.a.x;
    double b = p.y - s.a.y;
    double c = s.b.x - s.a.x;
    double d = s.b.y - s.a.y;
    double e = c * c + d * d;
    double f = e ? (a * c + b * d) / e : 0.0;
    double g = fclamp0(f, 0, 1);
    double dx = p.x - (s.a.x + g * c);
    double dy = p.y - (s.a.y + g * d);
    return sqrt(dx * dx + dy * dy);
}

double tg_rect_distance_rect(struct tg_rect a, struct tg_rect b) {
    double dx = fmax0(fmax0(a.min.x, b.min.x) - fmin0(a.max.x, b.max.x), 0);
    double dy = fmax0(fmax0(a.min.y, b.min.y) - fmin0(a.max.y, b.max.y), 0);
    return sqrt(dx * dx + dy * dy);
}

double tg_point_distance_rect(struct tg_point p, struct tg_rect r) {
    return tg_rect_distance_rect((struct tg_rect) { p, p }, r);
}

double tg_point_distance_point(struct tg_point a, struct tg_point b) {
    return sqrt((a.x-b.x) * (a.x-b.x) + (a.y-b.y) * (a.y-b.y));
}

enum nqentry_kind {
    NQUEUE_KIND_SEGMENT,
    NQUEUE_KIND_RECT
};

struct nqentry {
    double dist;
    enum nqentry_kind kind:8;
    int rect_level:8;
    int more;
    union {
        int seg_index;
        int rect_index;
    };
};

// nqueue is a priority queue using a binary heap type structure for ordering
// segments needed by the tg_ring_nearest function.
struct nqueue {
    bool oom;    // nqueue operation failure
    bool onheap; // items array is on the heap
    struct nqentry spare;
    struct nqentry *items;
    size_t len;
    size_t cap;
    size_t maxlen;
};

#define NQUEUE_INIT(queue, init_cap) \
    struct nqentry init_items[init_cap]; \
    queue = (struct nqueue) { \
        .cap = (init_cap), \
        .items = init_items \
    }; \

#define NQUEUE_DESTROY(queue) \
    if (queue.onheap) { \
        tg_free(queue.items); \
    } \

static void nqswap(struct nqueue *queue, size_t i, size_t j) {
    queue->spare = queue->items[i];
    queue->items[i] = queue->items[j];
    queue->items[j] = queue->spare;
}

static int nqcompare(struct nqueue *queue, size_t i, size_t j) {
    const struct nqentry *a = &queue->items[i];
    const struct nqentry *b = &queue->items[j];
    return a->dist < b->dist ? -1 : a->dist > b->dist;
}

static void nqueue_push(struct nqueue *queue, const struct nqentry *item) {
    if (queue->oom) {
        return;
    }
    size_t elsize = sizeof(struct nqentry);
    if (queue->len == queue->cap) {
        size_t cap = queue->cap < 1000 ? queue->cap*2 : queue->cap*1.25;
        struct nqentry *items;
        if (!queue->onheap) {
            items = tg_malloc(elsize*cap);
            if (!items) goto oom;
            memcpy(items, queue->items, elsize*queue->len);
        } else {
            items = tg_realloc(queue->items, elsize*cap);
            if (!items) goto oom;
        }
        queue->items = items;
        queue->cap = cap;
        queue->onheap = true;
    }
    queue->items[queue->len++] = *item;
    size_t i = queue->len - 1;
    while (i != 0) {
        size_t parent = (i - 1) / 2;
        if (!(nqcompare(queue, parent, i) > 0)) break;
        nqswap(queue, parent, i);
        i = parent;
    }
    queue->maxlen = fmax0(queue->maxlen, queue->len);
    return;
oom:
    queue->oom = true;
}

static const struct nqentry *nqueue_pop(struct nqueue *queue) {
    if (queue->len == 0 || queue->oom) {
        return NULL;
    }
    nqswap(queue, 0, queue->len-1);
    queue->len--;
    const struct nqentry *item = &queue->items[queue->len];
    size_t i = 0;
    while (1) {
        size_t smallest = i;
        size_t left = i * 2 + 1;
        size_t right = i * 2 + 2;
        if (left < queue->len && nqcompare(queue, left, smallest) <= 0) {
            smallest = left;
        }
        if (right < queue->len && nqcompare(queue, right, smallest) <= 0) {
            smallest = right;
        }
        if (smallest == i) {
            break;
        }
        nqswap(queue, smallest, i);
        i = smallest;
    }
    return item;
}

/// Iterates over segments from nearest to farthest.
///
/// This is a kNN operation.
/// The caller must provide their own "rect_dist" and "seg_dist" callbacks to
/// do the actual distance calculations.
///
/// @param ring Input ring
/// @param rect_dist Callback that returns the distance to a tg_rect.
/// @param seg_dist Callback that returns the distance to a tg_segment.
/// @param iter Callback that returns each segment in the ring in order of
/// nearest to farthest. Caller must return true to continue to the next
/// segment, or return false to stop iterating.
/// @param udata User-defined data
/// @return True if operation succeeded, false if out of memory.
/// @note Though not typical, this operation may need to allocate memory.
/// It's recommended to check the return value for success.
/// @note The `*more` argument is an optional ref-value that is used for
/// performing partial step-based or probability-based calculations. A detailed
/// description of its use is outside the scope of this document. Ignoring it 
/// altogether is the preferred behavior.
/// @see RingFuncs
bool tg_ring_nearest_segment(const struct tg_ring *ring, 
    double (*rect_dist)(struct tg_rect rect, int *more, void *udata),
    double (*seg_dist)(struct tg_segment seg, int *more, void *udata),
    bool (*iter)(struct tg_segment seg, double dist, int index, void *udata),
    void *udata)
{
    if (!ring || !seg_dist) return true;
    struct nqueue queue;
    NQUEUE_INIT(queue, 256);
    struct index *ix = ring->index;
    int ixspread = ix ? ring->index->spread : 0;
    if (rect_dist && ix) {
        // Gather root rectangles
        for (int i = 0; i < ix->levels[0].nrects; i++) {
            int more = 0;
            struct tg_rect rect;
            ixrect_to_tg_rect(&ix->levels[0].rects[i], &rect);
            double dist = rect_dist(rect, &more, udata);
            struct nqentry entry = {
                .kind = NQUEUE_KIND_RECT,
                .dist = dist,
                .more = more,
                .rect_level = 0,
                .rect_index = i,
            };
            nqueue_push(&queue, &entry);
        }
    } else {
        // Gather all segments
        for (int i = 0; i < ring->nsegs; i++) {
            struct tg_segment seg = { 
                ring->points[i+0],
                ring->points[i+1]
            };
            int more = 0;
            double dist = seg_dist(seg, &more, udata);
            struct nqentry entry = {
                .kind = NQUEUE_KIND_SEGMENT,
                .dist = dist,
                .more = more,
                .seg_index = i,
            };
            nqueue_push(&queue, &entry);
        }
    }
    while (1) {
        const struct nqentry *ientry = nqueue_pop(&queue);
        if (!ientry) break;
        if (ientry->kind == NQUEUE_KIND_SEGMENT) {
            struct tg_segment seg = { 
                ring->points[ientry->seg_index+0], 
                ring->points[ientry->seg_index+1],
            };
            if (ientry->more) {
                // Reinsert the segment
                struct nqentry entry = *ientry;
                entry.dist = seg_dist(seg, &entry.more, udata);
                nqueue_push(&queue, &entry);
            } else {
                // Segments are sent back to the caller.
                if (!iter(seg, ientry->dist, ientry->seg_index, udata)) {
                    break;
                }
            }
            continue;
        }
        if (ientry->more) {
            // Reinsert the rectangle
            struct tg_rect rect;
            ixrect_to_tg_rect(&ix->levels[ientry->rect_level]
                .rects[ientry->rect_index], &rect);
            struct nqentry entry = *ientry;
            entry.dist = rect_dist(rect, &entry.more, udata);
            nqueue_push(&queue, &entry);
            continue;
        }
        int lvl = ientry->rect_level + 1;
        int start = ientry->rect_index*ixspread;
        ientry = NULL; // no longer need this
        if (lvl == ix->nlevels) {
            // Gather leaf segments
            int nsegs = ring->nsegs;
            int i = start;
            int e = i+ixspread;
            if (e > nsegs) e = nsegs;
            for (; i < e; i++) {
                struct tg_segment seg = {
                    ring->points[i+0],
                    ring->points[i+1]
                };
                int more = 0;
                double dist = seg_dist(seg, &more, udata);
                struct nqentry entry = {
                    .more = more,
                    .dist = dist,
                    .kind = NQUEUE_KIND_SEGMENT,
                    .seg_index = i,
                };
                nqueue_push(&queue, &entry);
            };
        } else {
            // Gather branch rectangles
            const struct level *level = &ix->levels[lvl];
            int i = start;
            int e = i+ixspread;
            if (e > level->nrects) e = level->nrects;
            for (; i < e; i++) {
                int more = 0;
                struct tg_rect rect;
                ixrect_to_tg_rect(&level->rects[i], &rect);
                double dist = rect_dist(rect, &more, udata);
                struct nqentry entry = {
                    .more = more,
                    .dist = dist,
                    .kind = NQUEUE_KIND_RECT,
                    .rect_level = lvl,
                    .rect_index = i,
                };
                nqueue_push(&queue, &entry);
            };
        }
    }
    bool oom = queue.oom;
    NQUEUE_DESTROY(queue);
    return !oom;
}

/// Iterates over segments from nearest to farthest.
/// @see tg_ring_nearest_segment(), which shares the same interface, for a 
/// detailed description.
/// @see LineFuncs
bool tg_line_nearest_segment(const struct tg_line *line, 
    double (*rect_dist)(struct tg_rect rect, int *more, void *udata),
    double (*seg_dist)(struct tg_segment seg, int *more, void *udata),
    bool (*iter)(struct tg_segment seg, double dist, int index, void *udata),
    void *udata)
{
    struct tg_ring *ring = (struct tg_ring *)line;
    return tg_ring_nearest_segment(ring, rect_dist, seg_dist, iter, udata);
}

////////////////////////////////////////////////////////////////////////////////
//  Spatial predicates
////////////////////////////////////////////////////////////////////////////////

/// Tests whether two geometries are topologically equal.
/// @see GeometryPredicates
bool tg_geom_equals(const struct tg_geom *a, const struct tg_geom *b) {
    return tg_geom_within(a, b) && tg_geom_contains(a, b);
}

/// Tests whether 'a' is fully contained inside of 'b'.
/// @note Works the same as `tg_geom_covers(b, a)`
/// @see GeometryPredicates
bool tg_geom_coveredby(const struct tg_geom *a, const struct tg_geom *b) {
    return tg_geom_covers(b, a);
}

/// Tests whether 'a' and 'b' have no point in common, and are fully
/// disconnected geometries.
/// @note Works the same as `!tg_geom_intersects(a, b)`
/// @see GeometryPredicates
bool tg_geom_disjoint(const struct tg_geom *a, const struct tg_geom *b) {
    return !tg_geom_intersects(a, b);
}

/// Tests whether 'a' is contained inside of 'b' and not touching the boundary
/// of 'b'.
/// @note Works the same as `tg_geom_contains(b, a)`
/// @warning This predicate returns **false** when geometry 'a' is *on* or
/// *touching* the boundary of geometry 'b'. Such as when a point is on the
/// edge of a polygon.  
/// For full coverage, consider using @ref tg_geom_coveredby.
/// @see GeometryPredicates
bool tg_geom_within(const struct tg_geom *a, const struct tg_geom *b) {
    return tg_geom_contains(b, a);
}

bool tg_geom_crosses(const struct tg_geom *a, const struct tg_geom *b) {
    (void)a; (void)b;
    // unsupported
    return false;
}

bool tg_geom_overlaps(const struct tg_geom *a, const struct tg_geom *b) {
    (void)a; (void)b;
    // unsupported
    return false;
}

int tg_geom_de9im_dims(const struct tg_geom *geom) {
    int dims = -1;
    if (geom) {
        switch (geom->head.base) {
        case BASE_POINT: return 0;
        case BASE_LINE:  return 1;
        case BASE_RING:  return 2;
        case BASE_POLY:  return 2;
        case BASE_GEOM:
            switch (geom->head.type) {
            case TG_POINT:           return 0;
            case TG_LINESTRING:      return 1;
            case TG_POLYGON:         return 2;
            case TG_MULTIPOINT:      return 0;
            case TG_MULTILINESTRING: return 1;
            case TG_MULTIPOLYGON:    return 2;
            case TG_GEOMETRYCOLLECTION: {
                int ngeoms = tg_geom_num_geometries(geom);
                for (int i = 0; i < ngeoms; i++) {
                    const struct tg_geom *child = tg_geom_geometry_at(geom, i);
                    int child_dims = tg_geom_de9im_dims(child);
                    if (child_dims > dims) {
                        dims = child_dims;
                    }
                }
            }}
        }
    }
    return dims;
}

/// Copies a ring
/// @param ring Input ring, caller retains ownership.
/// @return A duplicate of the provided ring. 
/// @return NULL if out of memory
/// @note The caller is responsible for freeing with tg_ring_free().
/// @note This method performs a deep copy of the entire geometry to new memory.
/// @see RingFuncs
struct tg_ring *tg_ring_copy(const struct tg_ring *ring) {
    if (!ring) {
        return NULL;
    }
    size_t size = ring_alloc_size(ring);
    struct tg_ring *ring2 = tg_malloc(size);
    if (!ring2) {
        return NULL;
    }
    memcpy(ring2, ring, size);
    ring2->head.rc = 0;
    if (ring->ystripes) {
        ring2->ystripes = tg_malloc(ring->ystripes->memsz);
        if (!ring2->ystripes) {
            tg_free(ring2);
            return NULL;
        }
        memcpy(ring2->ystripes, ring->ystripes, ring->ystripes->memsz);
    }
    return ring2;
}

/// Copies a line
/// @param line Input line, caller retains ownership.
/// @return A duplicate of the provided line. 
/// @return NULL if out of memory
/// @note The caller is responsible for freeing with tg_line_free().
/// @note This method performs a deep copy of the entire geometry to new memory.
/// @see LineFuncs
struct tg_line *tg_line_copy(const struct tg_line *line) {
    return (struct tg_line*)tg_ring_copy((struct tg_ring*)line);
}

/// Copies a polygon.
/// @param poly Input polygon, caller retains ownership.
/// @return A duplicate of the provided polygon. 
/// @return NULL if out of memory
/// @note The caller is responsible for freeing with tg_poly_free().
/// @note This method performs a deep copy of the entire geometry to new memory.
/// @see PolyFuncs
struct tg_poly *tg_poly_copy(const struct tg_poly *poly) {
    if (!poly) {
        return NULL;
    }
    if (poly->head.base == BASE_RING) {
        return (struct tg_poly*)tg_ring_copy((struct tg_ring*)poly);
    }
    struct tg_poly *poly2 = tg_malloc(sizeof(struct tg_poly));
    if (!poly2) {
        goto fail;
    }
    memset(poly2, 0, sizeof(struct tg_poly));
    memcpy(&poly2->head, &poly->head, sizeof(struct head));
    poly2->head.rc = 0;
    poly2->exterior = tg_ring_copy(poly->exterior);
    if (!poly2->exterior) {
        goto fail;
    }
    if (poly->nholes > 0) {
        poly2->holes = tg_malloc(sizeof(struct tg_ring*)*poly->nholes);
        if (!poly2->holes) {
            goto fail;
        }
        poly2->nholes = poly->nholes;
        memset(poly2->holes, 0, sizeof(struct tg_ring*)*poly2->nholes);
        for (int i = 0; i < poly2->nholes; i++) {
            poly2->holes[i] = tg_ring_copy(poly->holes[i]);
            if (!poly2->holes[i]) {
                goto fail;
            }
        }
    }
    return poly2;
fail:
    tg_poly_free(poly2);
    return NULL;
}

static struct tg_geom *geom_copy(const struct tg_geom *geom) {
    struct tg_geom *geom2 = tg_malloc(sizeof(struct tg_geom));
    if (!geom2) {
        return NULL;
    }
    memset(geom2, 0, sizeof(struct tg_geom));
    memcpy(&geom2->head, &geom->head, sizeof(struct head));
    geom2->head.rc = 0;
    switch (geom->head.type) {
    case TG_POINT:
        geom2->point.x = geom->point.x;
        geom2->point.y = geom->point.y;
        geom2->z = geom->z;
        geom2->m = geom->m;
        break;
    case TG_LINESTRING:
        geom2->line = tg_line_copy(geom->line);
        if (!geom2->line) {
            goto fail;
        }
        break;
    case TG_POLYGON:
        geom2->poly = tg_poly_copy(geom->poly);
        if (!geom2->poly) {
            goto fail;
        }
        break;
    case TG_MULTIPOINT:
    case TG_MULTILINESTRING:
    case TG_MULTIPOLYGON:
    case TG_GEOMETRYCOLLECTION:
        if (geom->multi) {
            geom2->multi = tg_malloc(sizeof(struct multi));
            if (!geom2->multi) {
                goto fail;
            }
            memset(geom2->multi, 0, sizeof(struct multi));
            if (geom->multi->geoms) {
                size_t gsize = sizeof(struct tg_geom*)*geom->multi->ngeoms;
                geom2->multi->geoms = tg_malloc(gsize);
                if (!geom2->multi->geoms) {
                    goto fail;
                }
                geom2->multi->ngeoms = geom->multi->ngeoms;
                memset(geom2->multi->geoms, 0, gsize);
                for (int i = 0; i < geom->multi->ngeoms; i++) {
                    const struct tg_geom *child = geom->multi->geoms[i];
                    geom2->multi->geoms[i] = tg_geom_copy(child);
                    if (!geom2->multi->geoms[i]) {
                        goto fail;
                    }
                }
            }
        }
        break;
    }
    if (geom->head.type != TG_POINT && geom->coords) {
        geom2->coords = tg_malloc(sizeof(double)*geom->ncoords);
        if (!geom2->coords) {
            goto fail;
        }
        geom2->ncoords = geom->ncoords;
        memcpy(geom2->coords, geom->coords, sizeof(double)*geom->ncoords);
    }
    if (geom->error) {
        // error and xjson share the same memory, so this copy covers both.
        size_t esize = strlen(geom->error)+1;
        geom2->error = tg_malloc(esize);
        if (!geom2->error) {
            goto fail;
        }
        memcpy(geom2->error, geom->error, esize);
    }
    return geom2;
fail:
    tg_geom_free(geom2);
    return NULL;
}

static struct boxed_point *boxed_point_copy(const struct boxed_point *point) {
    struct boxed_point *point2 = tg_malloc(sizeof(struct boxed_point));
    if (!point2) {
        return NULL;
    }
    memcpy(point2, point, sizeof(struct boxed_point));
    point2->head.rc = 0;
    return point2;
}

/// Copies a geometry
/// @param geom Input geometry, caller retains ownership.
/// @return A duplicate of the provided geometry.
/// @return NULL if out of memory
/// @note The caller is responsible for freeing with tg_geom_free().
/// @note This method performs a deep copy of the entire geometry to new memory.
/// @see GeometryConstructors
struct tg_geom *tg_geom_copy(const struct tg_geom *geom) {
    if (geom) {
        switch (geom->head.base) {
        case BASE_GEOM:
            return geom_copy(geom);
        case BASE_POINT:
            return (struct tg_geom*)boxed_point_copy((struct boxed_point*)geom);
        case BASE_LINE:
            return (struct tg_geom*)tg_line_copy((struct tg_line*)geom);
        case BASE_RING:
            return (struct tg_geom*)tg_ring_copy((struct tg_ring*)geom);
        case BASE_POLY:
            return (struct tg_geom*)tg_poly_copy((struct tg_poly*)geom);
        }
    }
    return NULL;
}

/// Tests whether a geometry intersects a rect.
/// @see GeometryPredicates
bool tg_geom_intersects_rect(const struct tg_geom *a, struct tg_rect b) {
    struct tg_ring *ring = stack_ring();
    rect_to_ring(b, ring);
    return tg_geom_intersects(a, (struct tg_geom*)ring);
}
