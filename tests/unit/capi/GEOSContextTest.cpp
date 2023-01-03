#include <tut/tut.hpp>
// geos
#include <geos_c.h>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

struct test_geoscontext_data : public capitest::utility {

        static void count_notice(const char* fmt, ...)
        {
            (void) fmt;
            num_notice++;
        }

        static void count_error(const char* fmt, ...)
        {
            (void) fmt;
            num_error++;
        }

        void provokeError(GEOSContextHandle_t& context) {
            GEOSGeom_createEmptyCollection_r(context, 999999); // Produce an error
        }

        void provokeNotice(GEOSContextHandle_t& context) {
            GEOSGeometry* g = fromWKT("POLYGON ((0 0, 1 0, 0 1, 1 1, 0 0))");
            GEOSisValid_r(context, g); // Produce a notice
            GEOSGeom_destroy(g);
        }

        static int num_notice;
        static int num_error;
};

int test_geoscontext_data::num_notice = 0;
int test_geoscontext_data::num_error = 0;

typedef test_group<test_geoscontext_data> group;
typedef group::object object;

group test_geoscontext("capi::GEOSContext");

// Test "new" style error and notice handlers
template<>
template<>
void object::test<1>()
{
    GEOSContextHandle_t context = GEOS_init_r();

    std::string errorMsg;
    std::string noticeMsg;

    GEOSContext_setErrorMessageHandler_r(context, [](const char* message, void* userdata) {
        (void) message;
        std::string& msg = *static_cast<std::string*>(userdata);
        msg.append("error");
    }, &errorMsg);

    GEOSContext_setNoticeMessageHandler_r(context, [](const char* message, void* userdata) {
        (void) message;
        std::string& msg = *static_cast<std::string*>(userdata);
        msg.append("notice");
    }, &noticeMsg);

    provokeError(context);
    provokeNotice(context);

    ensure_equals(errorMsg, "error");
    ensure_equals(noticeMsg, "notice");

    finishGEOS_r(context);
}

// Test "old" style error and notice handlers
template<>
template<>
void object::test<2>()
{
    GEOSContextHandle_t context = GEOS_init_r();

    GEOSContext_setErrorHandler_r(context, count_error);
    GEOSContext_setNoticeHandler_r(context, count_notice);

    ensure_equals(num_error, 0);
    ensure_equals(num_notice, 0);

    provokeError(context);
    ensure_equals(num_error, 1);
    ensure_equals(num_notice, 0);

    provokeNotice(context);
    ensure_equals(num_error, 1);
    ensure_equals(num_notice, 1);

    finishGEOS_r(context);
}

} // namespace tut

