#include <stdio.h>
#include <stdarg.h>

#include <geos_c.h>

static void
geos_msg_handler(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
}

int test_length() {
    GEOSWKTReader* reader = GEOSWKTReader_create();
    GEOSGeometry* geom_a = GEOSWKTReader_read(reader, "LINESTRING(0 0, 40 0, 40 2)");
    double length = 0.0;
    int res = GEOSLength(geom_a, &length);
    printf("%.1f\n", length);
    return res == 1 ? 0 : 0;
}

int usage()
{
    fprintf(stderr, "Use option -l or -v\n");
    return 1;
}

int main(int argc, char *argv[]) {
    int ret = 0;
    initGEOS(geos_msg_handler, geos_msg_handler);
    if (argc == 2 && argv[1][0] == '-') {
        switch (argv[1][1]) {
        case 'l':
            ret = (test_length());
            break;
        case 'v':
            printf("%s\n", GEOSversion());
            break;
        default:
            ret = usage();
        }
    } else
        ret = usage();
    return ret;
}
