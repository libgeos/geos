#ifndef GEOS_CAPI_TEST_UTILS_H
#define GEOS_CAPI_TEST_UTILS_H

#include <geos_c.h>

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>




namespace capitest {

    struct test_handlers {
        static void notice(const char* fmt, ...)
        {
            std::fprintf(stdout, "NOTICE: ");

            va_list ap;
            va_start(ap, fmt);
            std::vfprintf(stdout, fmt, ap);
            va_end(ap);

            std::fprintf(stdout, "\n");
        }

        test_handlers()
        {
            initGEOS(notice, notice);
        }

        ~test_handlers()
        {
            finishGEOS();
        }
    };

}

#endif
