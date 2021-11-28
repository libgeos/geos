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
    size_t sep;
    for (sep = 0; sep < Size; sep ++) {
        if (Data[sep] == 0) {
            break;
        }
    }
    if (sep == Size) {
        return 0;
    }
    GEOSGeometry *g1 = GEOSGeomFromWKT(Data);

    if (g1 != NULL) {
        char * r = malloc(Size-sep);
        memcpy(r, Data+sep, Size-sep);
        GEOSGeometry *g2 = GEOSGeomFromWKT(r);
        if (g2 != NULL) {
            size_t usize;
            GEOSGeometry *g3 = GEOSIntersection(g1, g2);
            GEOSGeom_destroy(g3);
            g3 = GEOSDifference(g1, g2);
            GEOSGeom_destroy(g3);
            g3 = GEOSUnion(g1, g2);
            GEOSGeom_destroy(g3);
            unsigned char* uptr = GEOSGeomToWKB_buf(g1, &usize);
            free(uptr);
            GEOSGeom_destroy(g2);
        }
        free(r);
        GEOSGeom_destroy(g1);
    }
    return 0;
}

