#include "../headers/geom.h"

CoordinateListFactory* CoordinateListFactory::internalFactory=new BasicCoordinateListFactory();
//CoordinateListFactory* CoordinateListFactory::internalFactory=new PointCoordinateListFactory();

