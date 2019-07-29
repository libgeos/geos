#include <geos/geom/GeometryFactory.h>
using namespace geos::geom;

int main()
{
    GeometryFactory::Ptr factory = GeometryFactory::create();
    (void)factory;
}