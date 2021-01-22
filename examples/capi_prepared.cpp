/*
* # GEOS C API example 2
*
* Reads one geometry and does a high-performance
* prepared geometry operations to place random
* points inside it.
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


int main()
{
    /* Send notice and error messages to our stdout handler */
    initGEOS(geos_message_handler, geos_message_handler);

    /* One concave polygon */
    const char* wkt = "POLYGON ((189 115, 200 170, 130 170, 35 242, 156 215, 210 290, 274 256, 360 190, 267 215, 300 50, 200 60, 189 115))";

    /* Read the WKT into geometry objects */
    GEOSWKTReader* reader = GEOSWKTReader_create();
    GEOSGeometry* geom = GEOSWKTReader_read(reader, wkt);
    GEOSWKTReader_destroy(reader);

    /* Check for parse success */
    if (!geom) {
        finishGEOS();
        return 1;
    }

    /* Prepare the geometry */
    const GEOSPreparedGeometry* prep_geom = GEOSPrepare(geom);

    /* Read bounds of geometry */
    double xmin, xmax, ymin, ymax;
    GEOSGeom_getXMin(geom, &xmin);
    GEOSGeom_getXMax(geom, &xmax);
    GEOSGeom_getYMin(geom, &ymin);
    GEOSGeom_getYMax(geom, &ymax);

    /*
    * Set up the point generator
    * Generate all the points in the bounding box
    * of the input polygon.
    */
    const int steps = 10;
    double xstep = (xmax - xmin) / steps;
    double ystep = (ymax - ymin) / steps;

    /* Place to hold points to output */
    GEOSGeometry** geoms = (GEOSGeometry**)malloc(steps*steps*sizeof(GEOSGeometry*));
    size_t ngeoms = 0;

    /*
    * Test all the points in the polygon bounding box
    * and only keep those that intersect the actual polygon
    */
    int i, j;
    for (i = 0; i < steps; i++) {
        for (j = 0; j < steps; j++) {
            /* Make a point in the point grid */
            GEOSGeometry* pt = GEOSGeom_createPointFromXY(
                                    xmin + xstep*i,
                                    ymin + ystep*j);
            /* Check if the point and polygon intersect */
            if (GEOSPreparedIntersects(prep_geom, pt)) {
                /* Save the ones that do */
                geoms[ngeoms++] = pt;
            }
            else {
                /* Clean up the ones that don't */
                GEOSGeom_destroy(pt);
            }

        }
    }

    /* Put the successful geoms inside a geometry for WKT output */
    GEOSGeometry* result = GEOSGeom_createCollection(GEOS_MULTIPOINT, geoms, ngeoms);

    /*
    * The GEOSGeom_createCollection() only takes ownership of the
    * geometries, not the array container, so we can free the container
    * now
    */
    free(geoms);

    /* Convert result to WKT */
    GEOSWKTWriter* writer = GEOSWKTWriter_create();
    /* Trim trailing zeros off output */
    GEOSWKTWriter_setTrim(writer, 1);
    GEOSWKTWriter_setRoundingPrecision(writer, 3);
    char* wkt_result = GEOSWKTWriter_write(writer, result);
    GEOSWKTWriter_destroy(writer);

    /* Print answer */
    printf("Input Polygon:\n");
    printf("%s\n\n", wkt);
    printf("Output Points:\n");
    printf("%s\n\n", wkt_result);

    /* Clean up everything we allocated */
    GEOSPreparedGeom_destroy(prep_geom);
    GEOSGeom_destroy(geom);
    GEOSGeom_destroy(result);
    GEOSFree(wkt_result);

    /* Clean up the global context */
    finishGEOS();

    /* Done */
    return 0;
}
