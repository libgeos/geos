---
title: "C API Programming"
date: 2021-10-04T14:21:00-07:00
draft: false
weight: 40
---

Most programs using GEOS use the C API, rather than building against the C++ headers. The C API offers several benefits:

* Stable API, that preserves behaviour and function naming over multiple releases.
* Stable ABI, allowing new binaries to be dropped into place without requiring a rebuild of dependent applications.
* Simple access pattern, using the [simple features model](https://en.wikipedia.org/wiki/Simple_Features) as the basis for most operations.

In exchange for this simplicity and stability, the C API has a few requirements from application authors:

* Explicit memory management. If you create a GEOS object with a GEOS function, you must free it using the appropriate GEOS destructor.

The C API is contained in the [geos_c.h](../../doxygen/geos__c_8h.html) header file.

## Building a Program

The simplest GEOS C API application needs to include the API header, declare a message handler, initialize the GEOS globals, and link to the GEOS C library when built.

```c
/* geos_hello_world.c */

#include <stdio.h>  /* for printf */
#include <stdarg.h> /* for va_list */

/* Only the CAPI header is required */
#include <geos_c.h>

/*
* GEOS requires two message handlers to return
* error and notice message to the calling program.
*
*   typedef void(* GEOSMessageHandler) (const char *fmt,...)
*
* Here we stub out an example that just prints the
* messages to stdout.
*/
static void
geos_msg_handler(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vprintf (fmt, ap);
    va_end(ap);
}

int main()
{
    /* Send notice and error messages to the terminal */
    initGEOS(geos_msg_handler, geos_msg_handler);

    /* Read WKT into geometry object */
    GEOSWKTReader* reader = GEOSWKTReader_create();
    GEOSGeometry* geom_a = GEOSWKTReader_read(reader, "POINT(1 1)");

    /* Convert result to WKT */
    GEOSWKTWriter* writer = GEOSWKTWriter_create();
    char* wkt = GEOSWKTWriter_write(writer, geom_a);
    printf("Geometry: %s\n", wkt);

    /* Clean up allocated objects */
    GEOSWKTReader_destroy(reader);
    GEOSWKTWriter_destroy(writer);
    GEOSGeom_destroy(geom_a);
    GEOSFree(wkt);

    /* Clean up the global context */
    finishGEOS();
    return 0;
}
```

When compiling the program, remember to link in the GEOS C library.

```bash
cc geos_hello_world.c -o geos_hello_world -l geos_c
```

## Reentrant/Threadsafe API

GEOS functions provide reentrant variants, indicated by an `_r` suffix. The reentrant functions work the same as their regular counterparts, but they have an extra parameter, a `GEOSContextHandle_t`.

The `GEOSContextHandle_t` carries a thread-local state that is equivalent to the state initialized by the `initGEOS()` call in the simple example above.

To use the reentrant API, call `GEOS_init_r()` instead of `initGEOS()` to create a context local to your thread. Each thread that will be running GEOS operations should create its own context prior to working with the GEOS API.

In this example the overall structure of the code is identical, but the reentrant variants are used, and the preamble and cleanup are slightly different.

```c
/* geos_hello_world.c */

#include <stdio.h>  /* for printf */
#include <stdarg.h> /* for va_list */

/* Only the CAPI header is required */
#include <geos_c.h>

static void
geos_msg_handler(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vprintf (fmt, ap);
    va_end(ap);
}

int main()
{
    /* Send notice and error messages to the terminal */
    GEOSContextHandle_t ctx = GEOS_init_r ();
    GEOSContext_setNoticeHandler_r(ctx, geos_msg_handler);
    GEOSContext_setErrorHandler_r(ctx, geos_msg_handler);

    /* Read WKT into geometry object */
    GEOSWKTReader* reader = GEOSWKTReader_create_r(ctx);
    GEOSGeometry* geom_a = GEOSWKTReader_read_r(ctx, reader, "POINT(1 1)");

    /* Convert result to WKT */
    GEOSWKTWriter* writer = GEOSWKTWriter_create_r(ctx);
    char* wkt = GEOSWKTWriter_write_r(ctx, writer, geom_a);
    printf("Geometry: %s\n", wkt);

    /* Clean up allocated objects */
    GEOSWKTReader_destroy_r(ctx, reader);
    GEOSWKTWriter_destroy_r(ctx, writer);
    GEOSGeom_destroy_r(ctx, geom_a);
    GEOSFree_r(ctx, wkt);

    /* Clean up the global context */
    GEOS_finish_r(ctx);
    return 0;
}
```

## Object Model

The `GEOSCoordSequence` and `GEOSGeometry` objects are at the heart of the GEOS object model.

### GEOSCoordSequence

`GEOSCoordSequence` is an ordered list of coordinates.
Coordinates are 2 (XY) or 3 (XYZ) dimensional.

There are a number of ways to make a `GEOSCoordSequence`.  You can create a `GEOSCoordSequence` by creating a blank one and then setting the coordinate values.

```c
double xList[] = {1.0, 2.0, 3.0};
double yList[] = {3.0, 2.0, 1.0};
size_t seqSize = 3;
size_t seqDims = 2;

GEOSCoordSequence* seq = GEOSCoordSeq_create(seqSize, seqDims);

for (size_t i = 0; i < seqSize; i++) {
    seq->setXY(i, xList[i], yList[i]);
}
GEOSCoordSeq_destroy(seq);
```

You can also create a `GEOSCoordSequence` and initialize it from coordinate arrays.

```c
double xList[] = {1.0, 2.0, 3.0};
double yList[] = {3.0, 2.0, 1.0};
size_t seqSize = 3;

GEOSCoordSequence* seq = GEOSCoordSeq_copyFromArrays(
    xList,
    yList,
    NULL,  /* Zs */
    NULL,  /* Ms */
    seqSize);

GEOSCoordSeq_destroy(seq);
```

Finally, you can create a `GEOSCoordSequence` and initialize it from a coordinate buffer (an array of double in coordinate order, eg: `XYXYXY`).

```c
/* Coordinates in a buffer (X,Y, X,Y, X,Y) */
double coordBuf[] = {1.0,3.0, 2.0,2.0, 3.0,1.0};
size_t seqSize = 3;

GEOSCoordSequence* seq = GEOSCoordSeq_copyFromBuffer(
    coordBuf,
    seqSize,
    0, /* hasZ */
    0  /* hasM */
    );

GEOSCoordSeq_destroy(seq);
```

Note that while you can reclaim the memory for a `GEOSCoordSequence` directly using `GEOSCoordSeq_destroy()`, you usually **will not have to** since creating a `GEOSGeometry` with a `GEOSCoordSequence` hands ownership of the sequence to the new geometry.

When writing data back from GEOS to whatever application you are using, you have the option of using a standard serialization format like WKB (see below) or by writing back to arrays or buffers.

* `GEOSCoordSeq_copyToArrays()`
* `GEOSCoordSeq_copyToBuffer()`

Using the array or buffer methods can often be **faster** than using direct coordinate reading or serialization formats, if the target structures use coordinate arrays or XY binary buffers.

### GEOSGeometry

The fundamental structure of the GEOS C API is `GEOSGeometry`.
`GEOSGeometry` is a generic type that can be a Point, LineString, Polygon, MultiPoint, MultiLineString, MultiPolygon, or GeometryCollection.
Most functions in the GEOS C API have a `GEOSGeometry` as a parameter or return type.
When `GEOSGeometry` values have been created they must be deallocated using `GEOSGeom_destroy()`.

There are many constructors for `GEOSGeometry`:

* `GEOSGeom_createPoint()`
* `GEOSGeom_createPointFromXY()`
* `GEOSGeom_createLinearRing()`
* `GEOSGeom_createLineString()`
* `GEOSGeom_createPolygon()`
* `GEOSGeom_createCollection()`
* `GEOSGeom_createEmptyPoint()`
* `GEOSGeom_createEmptyLineString()`
* `GEOSGeom_createEmptyPolygon()`
* `GEOSGeom_createEmptyCollection()`

The `createEmpty` functions take no arguments and return geometries that are "empty".
Empty geometries represent a (typed) empty set of space.
For example, the intersection of two **disjoint polygons** is a "empty polygon".

The `GEOSGeom_createPoint()`, `GEOSGeom_createLinearRing()` and `GEOSGeom_createLineString()` functions accept a single `GEOSCoordSequence` and take ownership of that sequence,
so freeing the geometry with `GEOSGeom_destroy()` frees all the allocated memory.

```c
double coordBuf[] = {1.0,3.0, 2.0,2.0, 3.0,1.0};
size_t seqSize = 3;
GEOSCoordSequence* seq = GEOSCoordSeq_copyFromBuffer(
    coordBuf, seqSize, 0, 0);

/* Takes ownership of sequence */
GEOSGeometry* geom = GEOSGeom_createLineString(seq);

/* Frees all memory */
GEOSGeom_destroy(geom);
```

The `GEOSGeom_createPolygon()` and `GEOSGeom_createCollection()` functions both require an array of inputs:

* an array of inner ring `GEOSCoordSequence` to create polygons; and,
* an array of `GEOSGeometry` to create collections.

As in the other creation functions, ownership of the contained objects is transferred to the new geometry. However, ownership of the array that **holds** the contained objects is not transferred.

```c
/* Two points in an array */
size_t npoints = 2;
GEOSGeometry** points = malloc(sizeof(GEOSGeometry*) * npoints);
points[0] = GEOSGeom_createPointFromXY(0.0, 0.0);
points[1] = GEOSGeom_createPointFromXY(0.0, 0.0);

/* takes ownership of the points in the array */
/* but not the array itself */
GEOSGeometry* collection = GEOSGeom_createCollection(
    GEOS_MULTIPOINT,  /* collection type */
    points,           /* geometry array */
    npoints);

/* frees collection and contained points */
GEOSGeom_destroy(collection);

/* frees the containing array */
free(points);
```

### Readers and Writers

The examples above build `GEOSCoordSequence`s from arrays of double, and `GEOSGeometry` from coordinate sequences, but it is also possible to directly read from and write to standard geometry formats:

* Well-Known Text ([WKT]({{< ref "/specifications/wkt" >}}))
* Well-Known Binary ([WKB]({{< ref "/specifications/wkb" >}}))
* [GeoJSON](https://datatracker.ietf.org/doc/html/rfc7946)

For example, reading and writing WKT:

```c
const char* wkt_in = "POLYGON((0 0, 10 0, 10 10, 0 10, 0 0))";

/* Read the WKT into geometry object */
GEOSWKTReader* reader = GEOSWKTReader_create();
GEOSGeometry* geom = GEOSWKTReader_read(reader, wkt_in);

/* Convert geometry back to WKT */
GEOSWKTWriter* writer = GEOSWKTWriter_create();
/* Trim trailing zeros off output; only needed before GEOS 3.12 */
GEOSWKTWriter_setTrim(writer, 1);
char* wkt_out = GEOSWKTWriter_write(writer, geom);

/* Clean up everything we allocated */
GEOSWKTReader_destroy(reader);
GEOSGeom_destroy(geom);
/* Use GEOSFree() to free memory allocated inside GEOS~ */
GEOSFree(wkt_out);
```

Note that the output WKT string is freed using `GEOSFree()`, not the system `free()`. This ensures that the same library that allocates the memory also frees it, which is important for some platforms (Windows primarily).

For more information about the specific options available for each format, see the documentation for the various readers and writers.

* `GEOSWKTReader` / `GEOSWKTWriter`
* `GEOSWKBReader` / `GEOSWKBWriter`
* `GEOSGeoJSONReader` / `GEOSGeoJSONWriter`

For a complete example using a reader and writer, see [capi_read.c](https://github.com/libgeos/geos/blob/main/examples/capi_read.c).

### Prepared Geometry

The GEOS "prepared geometry" is conceptually similar to a database "prepared statement": by doing up-front work to create an optimized object, you reap a performance benefit when executing repeated function calls on that object.

Prepared geometries contain internal indexes that make calls to the "spatial predicate" functions like `GEOSPreparedIntersects()` and `GEOSPreparedContains()` much much faster. These are functions that take in two geometries and return true or false.

If you are going to be making repeated calls to predicates on the same geometry, using a prepared geometry could be a big performance boost, at the cost of a little extra complexity.

```c
/* One concave polygon */
const char* wkt = "POLYGON ((189 115, 200 170, 130 170, 35 242, 156 215, 210 290, 274 256, 360 190, 267 215, 300 50, 200 60, 189 115))";

/* Read the WKT into geometry objects */
GEOSWKTReader* reader = GEOSWKTReader_create();
GEOSGeometry* geom = GEOSWKTReader_read(reader, wkt);
GEOSWKTReader_destroy(reader);

/* Prepare the geometry */
const GEOSPreparedGeometry* prep_geom = GEOSPrepare(geom);

/* Make a point to test */
GEOSGeometry* pt = GEOSGeom_createPointFromXY(190, 200);

/* Check if the point and polygon intersect */
if (GEOSPreparedIntersects(prep_geom, pt)) {
    /* done something ... */
}

/* Note that both prepared and original geometry are destroyed */
GEOSPreparedGeom_destroy(prep_geom);
GEOSGeom_destroy(geom);
GEOSGeom_destroy(pt);
```

For a complete example of using prepared geometry to accelerate multiple predicate tests, see the [capi_prepared.c](https://github.com/libgeos/geos/blob/main/examples/capi_prepared.c) example.


### STRTree Index

The STRTree index allows you to create, populate, and query a spatial index. Like most spatial indexes, the STRTree is based on [indexing rectangles](https://en.wikipedia.org/wiki/R-tree). For a complete example using a reader and writer, see [capi_strtree.c](https://github.com/libgeos/geos/blob/main/examples/capi_strtree.c).

When you build an index, you will usually insert an "item" -- some kind of `struct` that you are interested in indexing -- and an associated bounds for that item, in the form of a `GEOSGeometry`. The geometry does not need to be rectangular, a rectangular bounding box will be automatically calculated for the geometry.

Once you have built an STRTree, you have two basic ways to query:

* Find the **nearest item** to a query geometry, using `GEOSSTRtree_nearest_generic()`
* Find all the **items that intersect** with a query rectangle, using `GEOSSTRtree_query()`

Build the tree by creating it, then inserting items.

```c
/*
* An application will want to index items, which have
* some attributes and a geometry part.
*/
typedef struct
{
    GEOSGeometry* geom;
    size_t id;
} item;

/*
* The tree doesn't take ownership of inputs just
* holds references, so we keep our point field
* handy in an array
*/
item* items[nItems];

/*
* The create parameter for the tree is not the
* number of inputs, it is the number of entries
* per node. 10 is a good default number to use.
*/
GEOSSTRtree* tree = GEOSSTRtree_create(10);

for (size_t i = 0; i < nItems; i++) {
    /* Make a random point */
    item* obj = random_item(range);
    /* Store away a reference so we can free it after */
    items[i] = obj;
    /* Add an entry for it to the tree */
    GEOSSTRtree_insert(tree, obj->geom, obj);
}
```

Note that the index does **not take ownership** of the inserted `GEOSGeometry` or the item, it just stores pointers. So remember to keep a list of the items you create in order to free them at the end of your process.

Once the tree is built, you can query it.

The generic **nearest-neighbor query** uses a callback to check the actual distance between the search item and the indexed item. In this way it can filter through the many candidate nearest nodes in the index to find the **actual** nearest node.

```c
/*
* Item distance callback for GEOSSTRtree_nearest_generic()
*/
int
itemDistanceCallback(const void* item1, const void* item2, double* distance, void* userdata)
{
    item_t* obj1 = (item_t*)item1;
    item_t* obj2 = (item_t*)item2;
    return GEOSDistance(obj1->geom, obj2->geom, distance);
}
```

The query call requires the tree, the item driving the search (so it can be fed into the callback), the geometry driving the search (because the library doesn't know how to extract the geometry from the item *a priori*), the callback, and whatever extra information want sent into the callback.

```c
/* Random item to query the index with */
item_t* item_random = random_item(range);

/* Nearest item in the index to our random item */
const item_t* item_nearest = GEOSSTRtree_nearest_generic(
    tree,                 // STRTree to query
    item_random,          // Item to use in search
    item_random->geom,    // Geometry to seed search
    itemDistanceCallback, // Callback to process nearest object
    NULL);                // Userdata to hand to the callback
```

The **query by rectangle** function also uses a callback, which could be used to exactly test for a spatial relationship (intersects, contains, etc), for all the index nodes that meet the rough "bounds interact" filter the index applies.

```c
/*
* Item query callback for GEOSSTRtree_query()
*/
void
itemQueryCallback(void* item, void* userdata)
{
    double x, y;
    item_t* i = (item_t*)item;
    GEOSGeomGetX(i->geom, &x);
    GEOSGeomGetY(i->geom, &y);
    printf("Found item %10zu at (%g, %g)\n", i->id, x, y);
}
```

This example just prints out every candidate that passes the index filter.

```c
/* Set up a query rectangle for index query */
const char* wkt_bounds = "POLYGON((20 20, 22 20, 22 22, 20 22, 20 20))";
GEOSGeometry* geom_query = GEOSWKTReader_read(reader, wkt_bounds);

/* Find all items that touch the bounds */
/* For non-rectangular query geometry, this will be an over-determined set */
GEOSSTRtree_query(
    tree,              // STRTree to query
    geom_query,        // GEOSGeometry query bounds
    itemQueryCallback, // Callback to process index entries that pass query
    NULL);             // Userdata to hand to the callback
```

The query itself just uses the tree, the query bounds geometry, the callback, and optional user data. You could use the user data to pass in an array to write results out to, or a prepared geometry to use for exact tests.

