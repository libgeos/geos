#include <iomanip>
#include <iostream>

#include <geos/version.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/io/WKTReader.h>

using namespace geos::geom;
using namespace geos::io;

int test_length() {
    GeometryFactory::Ptr factory = GeometryFactory::create();
    WKTReader reader(*factory);
    std::string wkt_a("LINESTRING(0 0, 40 0, 40 2)");
    std::unique_ptr<Geometry> geom_a(reader.read(wkt_a));
    double length = geom_a->getLength();
    std::cout << std::fixed << std::setprecision(1) << length << std::endl;
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc == 2 && argv[1][0] == '-') {
        switch (argv[1][1]) {
        case 'l':
            test_length();
            return 0;
        case 'v':
            std::cout <<  GEOS_VERSION << std::endl;
            return (0);
        }
    }
    std::cerr << "Use option -l or -v" << std::endl;
    return (1);
}
