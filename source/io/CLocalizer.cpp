#include <geos/io/CLocalizer.h>

#include <string>

using namespace std;

namespace geos {
namespace io {

CLocalizer::CLocalizer()
{
    outer_locale = strdup(std::setlocale(LC_NUMERIC, NULL));
    if (std::setlocale(LC_NUMERIC, "C") == NULL)
    {
        if (outer_locale != NULL)
        {
            free(outer_locale);
            outer_locale = NULL;
        }
    }
}

CLocalizer::~CLocalizer()
{
    if (outer_locale != NULL)
    {
        std::setlocale(LC_NUMERIC, outer_locale);
        free(outer_locale);
        outer_locale = NULL;
    }
}

} // namespace geos.io
} // namespace geos
