#include "../headers/geom.h"

namespace geos {

CoordinateListFactory* CoordinateListFactory::internalFactory=new BasicCoordinateListFactory();
//CoordinateListFactory* CoordinateListFactory::internalFactory=new PointCoordinateListFactory();

}