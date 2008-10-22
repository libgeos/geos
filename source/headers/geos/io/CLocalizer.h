#include <string>

namespace geos {
namespace io {

/**
 * \class CLocalizer io.h geos.h
 */
class CLocalizer
{
public:

    CLocalizer();
    ~CLocalizer();

private:

    std::string saved_locale;
};

} // namespace io
} // namespace geos

