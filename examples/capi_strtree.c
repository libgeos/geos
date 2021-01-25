/*
* # GEOS C API example 3
*
* Build a spatial index and search it for a
* nearest pair.
*/

/* To print to stdout */
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

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
geos_message_handler(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vprintf (fmt, ap);
    va_end(ap);
}

/*
* Generate a random point in the range of
* POINT(0..range, 0..range). Caller must
* free.
*/
static GEOSGeometry*
geos_random_point(double range)
{
    double x = range * rand() / RAND_MAX;
    double y = range * rand() / RAND_MAX;
    /* Make a point in the point grid */
    return GEOSGeom_createPointFromXY(x, y);
}


int main()
{
    /* Send notice and error messages to our stdout handler */
    initGEOS(geos_message_handler, geos_message_handler);

    /* How many points to add to our random field */
    const size_t npoints = 10000;
    /* The coordinate range of the field (0->100.0) */
    const double range = 100.0;

    /*
    * The tree doesn't take ownership of inputs just
    * holds references, so we keep our point field
    * handy in an array
    */
    GEOSGeometry* geoms[npoints];
    /*
    * The create parameter for the tree is not the
    * number of inputs, it is the number of entries
    * per node. 10 is a good default number to use.
    */
    GEOSSTRtree* tree = GEOSSTRtree_create(10);
    for (size_t i = 0; i < npoints; i++) {
        /* Make a random point */
        GEOSGeometry* geom = geos_random_point(range);
        /* Store away a reference so we can free it after */
        geoms[i] = geom;
        /* Add an entry for it to the tree */
        GEOSSTRtree_insert(tree, geom, geom);
    }

    /* Random point to compare to the field */
    GEOSGeometry* geom_random = geos_random_point(range);
    /* Nearest point in the field to our test point */
    const GEOSGeometry* geom_nearest = GEOSSTRtree_nearest(tree, geom_random);

    /* Convert results to WKT */
    GEOSWKTWriter* writer = GEOSWKTWriter_create();
    /* Trim trailing zeros off output */
    GEOSWKTWriter_setTrim(writer, 1);
    GEOSWKTWriter_setRoundingPrecision(writer, 3);
    char* wkt_random = GEOSWKTWriter_write(writer, geom_random);
    char* wkt_nearest = GEOSWKTWriter_write(writer, geom_nearest);
    GEOSWKTWriter_destroy(writer);

    /* Print answer */
    printf(" Random Point: %s\n", wkt_random);
    printf("Nearest Point: %s\n", wkt_nearest);

    /* Clean up all allocated objects */
    /* Destroying tree does not destroy inputs */
    GEOSSTRtree_destroy(tree);
    GEOSGeom_destroy(geom_random);
    /* Destroy all the points in our random field */
    for (size_t i = 0; i < npoints; i++) {
        GEOSGeom_destroy(geoms[i]);
    }
    /*
    * Don't forget to free memory allocated by the
    * printing functions!
    */
    GEOSFree(wkt_random);
    GEOSFree(wkt_nearest);

    /* Clean up the global context */
    finishGEOS();

    /* Done */
    return 0;
}
