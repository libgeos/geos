/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 * $Log$
 * Revision 1.1  2004/04/04 06:29:11  ybychkov
 * "planargraph" and "geom/utill" upgraded to JTS 1.4
 *
 *
 **********************************************************************/


#include "../headers/planargraph.h"

namespace geos {

planarGraphComponent::planarGraphComponent() {
	isMarkedVar=false;
	isVisitedVar=false;
}

/**
* Tests if a component has been visited during the course of a graph algorithm
* @return <code>true</code> if the component has been visited
*/
bool planarGraphComponent::isVisited() {
	return isVisitedVar;
}

/**
* Sets the visited flag for this component.
* @param isVisited the desired value of the visited flag
*/
void planarGraphComponent::setVisited(bool newIsVisited) { 
	isVisitedVar=newIsVisited;
}

/**
* Tests if a component has been marked at some point during the processing
* involving this graph.
* @return <code>true</code> if the component has been marked
*/
bool planarGraphComponent::isMarked() { 
	return isMarkedVar;
}

/**
* Sets the marked flag for this component.
* @param isMarked the desired value of the marked flag
*/
void planarGraphComponent::setMarked(bool newIsMarked) { 
	isMarkedVar=newIsMarked; }

}
