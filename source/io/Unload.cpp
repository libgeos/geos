#include "../headers/unload.h"
#include "../headers/opValid.h"
#include "../headers/graph.h"
#include "../headers/operation.h"
#include "../headers/opRelate.h"
#include "../headers/indexQuadtree.h"

namespace geos {

Unload::Unload(void)
{
}

Unload::~Unload(void)
{
}

void Unload::Release()
{
	delete CoordinateListFactory::internalFactory;
	delete Geometry::cgAlgorithms;
	delete Geometry::geometryChangedFilter;
	delete IsValidOp::cga;
	delete PlanarGraph::cga;
	delete PlanarGraph::li;
	delete GeometryGraphOperation::cga;
	delete GeometryGraphOperation::li;
	delete RelateComputer::li;
	delete RelateComputer::ptLocator;
	delete EdgeEnd::cga;
	delete QuadTreeRoot::origin;
}
}
