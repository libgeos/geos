#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>

#include "geos_c.h"

static int initialized = 0;
FILE * flogOut;

void
notice(const char *fmt, ...) {
    va_list ap;
    fprintf( flogOut, "NOTICE: ");
    va_start (ap, fmt);
    vfprintf( flogOut, fmt, ap);
    va_end(ap);
    fprintf( flogOut, "\n" );
}

void
log_and_exit(const char *fmt, ...) {
    va_list ap;
    fprintf( flogOut, "ERROR: ");
    va_start (ap, fmt);
    vfprintf( flogOut, fmt, ap);
    va_end(ap);
    fprintf( flogOut, "\n" );
}

int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
    if (initialized == 0) {
        flogOut = fopen("/dev/null", "wb");
        initGEOS(notice, log_and_exit);
        initialized = 1;
    }
    if (Size < 2) {
        return 0;
    }

    uint8_t sel = Data[0];
    const uint8_t *body = Data + 1;
    size_t blen = Size - 1;

    GEOSGeometry *g;
    if (sel & 1) {
        g = GEOSGeomFromWKB_buf(body, blen);
    } else {
        char *wkt = (char *) malloc(blen + 1);
        if (wkt == NULL) {
            return 0;
        }
        memcpy(wkt, body, blen);
        wkt[blen] = '\0';
        g = GEOSGeomFromWKT(wkt);
        free(wkt);
    }
    if (g == NULL) {
        return 0;
    }

    double width = ((double) Data[1] - 128.0) / 16.0;
    double tol = (double) (sel >> 1) / 8.0;
    double range = (double) blen;

    GEOSGeometry *r;

    r = GEOSBuffer(g, width, (sel & 7) + 1);
    if (r) GEOSGeom_destroy(r);
    r = GEOSBufferWithStyle(g, width, (sel & 7) + 1,
                            (sel % 3) + 1, (sel % 3) + 1, 2.0 + tol);
    if (r) GEOSGeom_destroy(r);
    r = GEOSOffsetCurve(g, width, (sel & 7) + 1, (sel % 3) + 1, 2.0 + tol);
    if (r) GEOSGeom_destroy(r);

    r = GEOSConvexHull(g);
    if (r) GEOSGeom_destroy(r);
    r = GEOSConcaveHull(g, tol > 1.0 ? 1.0 : tol, sel & 2);
    if (r) GEOSGeom_destroy(r);
    r = GEOSMinimumRotatedRectangle(g);
    if (r) GEOSGeom_destroy(r);
    r = GEOSMinimumWidth(g);
    if (r) GEOSGeom_destroy(r);

    r = GEOSSimplify(g, tol);
    if (r) GEOSGeom_destroy(r);
    r = GEOSTopologyPreserveSimplify(g, tol);
    if (r) GEOSGeom_destroy(r);

    r = GEOSDelaunayTriangulation(g, tol, sel & 1);
    if (r) GEOSGeom_destroy(r);
    r = GEOSVoronoiDiagram(g, NULL, tol, sel & 1);
    if (r) GEOSGeom_destroy(r);

    r = GEOSMakeValid(g);
    if (r) GEOSGeom_destroy(r);

    r = GEOSUnaryUnion(g);
    if (r) GEOSGeom_destroy(r);

    r = GEOSClipByRect(g, -range, -range, range, range);
    if (r) GEOSGeom_destroy(r);

    r = GEOSNode(g);
    if (r) GEOSGeom_destroy(r);
    r = GEOSBoundary(g);
    if (r) GEOSGeom_destroy(r);
    r = GEOSGetCentroid(g);
    if (r) GEOSGeom_destroy(r);
    r = GEOSPointOnSurface(g);
    if (r) GEOSGeom_destroy(r);
    r = GEOSReverse(g);
    if (r) GEOSGeom_destroy(r);

    double d;
    GEOSDistance(g, g, &d);
    GEOSCoordSequence *seq = GEOSNearestPoints(g, g);
    if (seq) GEOSCoordSeq_destroy(seq);

    double a, l, c;
    GEOSArea(g, &a);
    GEOSLength(g, &l);
    GEOSMinimumClearance(g, &c);
    GEOSisValid(g);

    GEOSGeom_destroy(g);
    return 0;
}
