/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 * Copyright (C) 2005 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <cassert>

#include <geos/geomgraph.h>
#include <geos/util.h>

namespace geos {
namespace geomgraph { // geos.geomgraph

GraphComponent::GraphComponent():
	label(NULL),
	isInResultVar(false),
	isCoveredVar(false),
	isCoveredSetVar(false),
	isVisitedVar(false)
{
}

GraphComponent::GraphComponent(Label* newLabel):
	label(newLabel),
	isInResultVar(false),
	isCoveredVar(false),
	isCoveredSetVar(false),
	isVisitedVar(false)
{
}

GraphComponent::~GraphComponent()
{
	delete label;
}

Label *
GraphComponent::getLabel()
{
	return label;
}

void
GraphComponent::setLabel(Label* newLabel)
{
	delete label;
	label=newLabel;
}

void
GraphComponent::setCovered(bool isCovered)
{
	isCoveredVar = isCovered;
	isCoveredSetVar = true;
}

void
GraphComponent::updateIM(IntersectionMatrix *im)
{
	assert(label->getGeometryCount() >= 2); // found partial label
	computeIM(im);
}

} // namespace geos.geomgraph
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.6  2006/03/06 19:40:46  strk
 * geos::util namespace. New GeometryCollection::iterator interface, many cleanups.
 *
 * Revision 1.5  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.4  2005/11/16 15:49:54  strk
 * Reduced gratuitous heap allocations.
 *
 * Revision 1.3  2005/11/14 18:14:04  strk
 * Reduced heap allocations made by TopologyLocation and Label objects.
 * Enforced const-correctness on GraphComponent.
 * Cleanups.
 *
 * Revision 1.2  2004/07/02 13:28:26  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.1  2004/03/19 09:48:45  ybychkov
 * "geomgraph" and "geomgraph/indexl" upgraded to JTS 1.4
 *
 * Revision 1.9  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/

