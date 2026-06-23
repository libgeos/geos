#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>

#include "geos_c.h"

static FILE * flogOut;
static GEOSGeoJSONReader *reader;

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

int LLVMFuzzerInitialize(int *argc, char ***argv) {
    (void)argc; (void)argv;
    flogOut = fopen("/dev/null", "wb");
    initGEOS(notice, log_and_exit);
    reader = GEOSGeoJSONReader_create();
    return 0;
}

int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
    if (reader == NULL) {
        return 0;
    }

    char *json = (char *) malloc(Size + 1);
    if (json == NULL) {
        return 0;
    }
    memcpy(json, Data, Size);
    json[Size] = '\0';

    GEOSGeometry *g = GEOSGeoJSONReader_readGeometry(reader, json);
    if (g != NULL) {
        GEOSGeom_destroy(g);
    }

    free(json);
    return 0;
}
