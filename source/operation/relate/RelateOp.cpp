#include "opRelate.h"
#include "stdio.h"

IntersectionMatrix* RelateOp::relate(Geometry *a,Geometry *b) {
	RelateOp relOp(a,b);
	return relOp.getIntersectionMatrix();
}

RelateOp::RelateOp(Geometry *g0,Geometry *g1):GeometryGraphOperation(g0,g1) {
	relateComp=new RelateComputer(arg);
}

RelateOp::~RelateOp() {
	delete relateComp;
}

IntersectionMatrix* RelateOp::getIntersectionMatrix() {
	return relateComp->computeIM();
}
