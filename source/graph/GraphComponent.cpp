#include "../headers/graph.h"
#include "../headers/util.h"

namespace geos {

GraphComponent::GraphComponent() {
	isInResultVar = false;
	isCoveredVar = false;
	isCoveredSetVar = false;
	isVisitedVar = false;
	label=new Label();
}

GraphComponent::GraphComponent(Label* newLabel){
	isInResultVar = false;
	isCoveredVar = false;
	isCoveredSetVar = false;
	isVisitedVar = false;
	label=newLabel;
}

GraphComponent::~GraphComponent() {
//	delete label;
}

Label* GraphComponent::getLabel() {
	return label;
}

void GraphComponent::setLabel(Label* newLabel){
	this->label=newLabel;
}

void GraphComponent::setInResult(bool isInResult) {
	isInResultVar = isInResult;
}

bool GraphComponent::isInResult(){
	return isInResultVar;
}

void GraphComponent::setCovered(bool isCovered){
	isCoveredVar = isCovered;
	isCoveredSetVar = true;
}

bool GraphComponent::isCovered(){
	return isCoveredVar;
}

bool GraphComponent::isCoveredSet() {
	return isCoveredSetVar;
}

bool GraphComponent::isVisited() {
	return isVisitedVar;
}

void GraphComponent::setVisited(bool isVisited) {
	isVisitedVar = isVisited;
}

void GraphComponent::updateIM(IntersectionMatrix *im){
	Assert::isTrue(label->getGeometryCount() >= 2, "found partial label");
	computeIM(im);
}
}