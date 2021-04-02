#include <iostream>
#include <geos/geom/PrecisionModel.h>
#include <geos/version.h>

using namespace geos::geom;

int main(int argc, char** argv) {
    PrecisionModel pm;
    std::cout << GEOS_VERSION << std::endl;
    return(0);
}
