/*
* # GEOS C API example 2
*
* Thread-safe version of example 1. Uses the
* re-entrant API.
* Reads two WKT representations and calculates the
* intersection, prints it out, and cleans up.
*/

/* To print to stdout */
#include <stdio.h>
#include <stdarg.h>

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


int main()
{
    /*
    * Each thread using the re-entrant API must get its
    * own context handle
    */
    GEOSContextHandle_t context = GEOS_init_r();
    /*
    * The notice/error handlers route message back to the calling
    * application. Here they just print to stdout.
    */
    GEOSContext_setNoticeHandler_r(context, geos_message_handler);
    GEOSContext_setErrorHandler_r(context, geos_message_handler);

    /* Two squares that overlap */
    const char* wkt_a = "POLYGON((0 0, 10 0, 10 10, 0 10, 0 0))";
    const char* wkt_b = "POLYGON((5 5, 15 5, 15 15, 5 15, 5 5))";

    /* Read the WKT into geometry objects */
    GEOSWKTReader* reader = GEOSWKTReader_create_r(context);
    GEOSGeometry* geom_a = GEOSWKTReader_read_r(context, reader, wkt_a);
    GEOSGeometry* geom_b = GEOSWKTReader_read_r(context, reader, wkt_b);

    /* Calculate the intersection */
    GEOSGeometry* inter = GEOSIntersection_r(context, geom_a, geom_b);

    /* Convert result to WKT */
    GEOSWKTWriter* writer = GEOSWKTWriter_create_r(context);
    /* Trim trailing zeros off output */
    GEOSWKTWriter_setTrim(writer, 1);
    char* wkt_inter = GEOSWKTWriter_write_r(context, writer, inter);

    /* Print answer */
    printf("Geometry A:         %s\n", wkt_a);
    printf("Geometry B:         %s\n", wkt_b);
    printf("Intersection(A, B): %s\n", wkt_inter);

    /* Clean up everything we allocated */
    GEOSWKTReader_destroy_r(context, reader);
    GEOSWKTWriter_destroy_r(context, writer);
    GEOSGeom_destroy_r(context, geom_a);
    GEOSGeom_destroy_r(context, geom_b);
    GEOSGeom_destroy_r(context, inter);
    GEOSFree_r(context, wkt_inter);

    /* Clean up the global context */
    GEOS_finish_r(context);

    /* Done */
    return 0;
}
