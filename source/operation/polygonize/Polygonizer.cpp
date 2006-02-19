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

#include <geos/opPolygonize.h>
#include <typeinfo>

//#define DEBUG 1

namespace geos {
namespace operation { // geos.operation
namespace polygonize { // geos.operation.polygonize

Polygonizer::LineStringAdder::LineStringAdder(Polygonizer *p):
	pol(p)
{
}

void
Polygonizer::LineStringAdder::filter_rw(Geometry *g)
{
	LineString *ls = dynamic_cast<LineString *>(g);
	if ( ls ) pol->add(ls);
}


/*
 * Create a polygonizer with the same GeometryFactory
 * as the input Geometry
 */
Polygonizer::Polygonizer():
	lineStringAdder(new Polygonizer::LineStringAdder(this)),
	graph(NULL),
	dangles(NULL),
	cutEdges(NULL),
	invalidRingLines(NULL),
	holeList(NULL),
	shellList(NULL),
	polyList(NULL)
{
}

Polygonizer::~Polygonizer()
{
	delete lineStringAdder;
	delete dangles;
	delete cutEdges;
	delete graph;

	delete holeList;
	delete shellList;
	if ( invalidRingLines )
	{
		for (unsigned int i=0, n=invalidRingLines->size(); i<n; ++i)
			delete (*invalidRingLines)[i];
		delete invalidRingLines;
	}
	if ( polyList )
	{
		for (unsigned int i=0, n=polyList->size(); i<n; ++i)
			delete (*polyList)[i];
		delete polyList;
	}
}

/*
 * Add a collection of geometries to be polygonized.
 * May be called multiple times.
 * Any dimension of Geometry may be added;
 * the constituent linework will be extracted and used
 *
 * @param geomList a list of {@link Geometry}s with linework to be polygonized
 */
void
Polygonizer::add(vector<Geometry*> *geomList)
{
	for(unsigned int i=0, n=geomList->size(); i<n; ++i)
	{
		Geometry *geometry=(*geomList)[i];
		add(geometry);
	}
}

/*
 * Add a geometry to the linework to be polygonized.
 * May be called multiple times.
 * Any dimension of Geometry may be added;
 * the constituent linework will be extracted and used
 *
 * @param g a Geometry with linework to be polygonized
 */
void
Polygonizer::add(Geometry *g)
{
	g->apply_rw(lineStringAdder);
}

/*
 * Add a linestring to the graph of polygon edges.
 *
 * @param line the LineString to add
 */
void
Polygonizer::add(LineString *line)
{
	// create a new graph using the factory from the input Geometry
	if (graph==NULL)
		graph=new PolygonizeGraph(line->getFactory());
	graph->addEdge(line);
}

/*
 * Gets the list of polygons formed by the polygonization.
 * @return a collection of Polygons
 */
vector<Polygon*>*
Polygonizer::getPolygons()
{
	polygonize();
	vector<Polygon *> *ret = polyList;
	polyList = NULL;
	return ret;
}

/*
 * Get the list of dangling lines found during polygonization.
 * @return a collection of dangles LineStrings from input.
 */
vector<const LineString*>*
Polygonizer::getDangles()
{
	polygonize();
	return dangles;
}

/*
 * Get the list of cut edges found during polygonization.
 * @return a collection of the input {@LineStrings} which are cut edges
 */
vector<const LineString*>*
Polygonizer::getCutEdges()
{
	polygonize();
	return cutEdges;
}

/*
 * Get the list of lines forming invalid rings found during polygonization.
 * @return a collection of the input {@LineStrings} which form invalid rings
 */
vector<LineString*>*
Polygonizer::getInvalidRingLines()
{
	polygonize();
	vector<LineString*> *ret = invalidRingLines;
	invalidRingLines = NULL;
	return ret;
}

/*
 * Perform the polygonization, if it has not already been carried out.
 */
void
Polygonizer::polygonize()
{
	// check if already computed
	if (polyList!=NULL) return;

	polyList=new vector<Polygon*>();

	if (graph==NULL) // No valid geometries added
	{
		return; 
	}

	dangles=graph->deleteDangles();
	cutEdges=graph->deleteCutEdges();
	vector<polygonizeEdgeRing*> *edgeRingList=graph->getEdgeRings();
#if DEBUG
	cerr<<"Polygonizer::polygonize(): "<<edgeRingList->size()<<" edgeRings in graph"<<endl;
#endif
	vector<polygonizeEdgeRing*> *validEdgeRingList=new vector<polygonizeEdgeRing*>();
	invalidRingLines=new vector<LineString*>();
	findValidRings(edgeRingList, validEdgeRingList, invalidRingLines);
#if DEBUG
	cerr<<"                           "<<validEdgeRingList->size()<<" valid"<<endl;
	cerr<<"                           "<<invalidRingLines->size()<<" invalid"<<endl;
#endif
	delete edgeRingList;

	findShellsAndHoles(validEdgeRingList);
#if DEBUG
	cerr<<"                           "<<holeList->size()<<" holes"<<endl;
	cerr<<"                           "<<shellList->size()<<" shells"<<endl;
#endif

	assignHolesToShells(holeList, shellList);

	for (unsigned int i=0, n=shellList->size(); i<n; ++i)
	{
		polygonizeEdgeRing *er=(*shellList)[i];
		polyList->push_back(er->getPolygon());
	}
	delete validEdgeRingList;
}

void
Polygonizer::findValidRings(vector<polygonizeEdgeRing*> *edgeRingList, vector<polygonizeEdgeRing*> *validEdgeRingList, vector<LineString*> *invalidRingList)
{
	for (unsigned int i=0, n=edgeRingList->size(); i<n; ++i)
	{
		polygonizeEdgeRing *er=(*edgeRingList)[i];
		if (er->isValid())
			validEdgeRingList->push_back(er);
		else
		{
			invalidRingList->push_back(er->getLineString());
		}
	}
}

void
Polygonizer::findShellsAndHoles(vector<polygonizeEdgeRing*> *edgeRingList)
{
	holeList=new vector<polygonizeEdgeRing*>();
	shellList=new vector<polygonizeEdgeRing*>();
	for (unsigned int i=0, n=edgeRingList->size(); i<n; ++i)
	{
		polygonizeEdgeRing *er=(*edgeRingList)[i];
		if (er->isHole())
			holeList->push_back(er);
		else
			shellList->push_back(er);
	}
}

void
Polygonizer::assignHolesToShells(vector<polygonizeEdgeRing*> *holeList,vector<polygonizeEdgeRing*> *shellList)
{
	for (unsigned int i=0, n=holeList->size(); i<n; ++i)
	{
		polygonizeEdgeRing *holeER=(*holeList)[i];
		assignHoleToShell(holeER,shellList);
	}
}

void
Polygonizer::assignHoleToShell(polygonizeEdgeRing *holeER,
		vector<polygonizeEdgeRing*> *shellList)
{
	polygonizeEdgeRing *shell=polygonizeEdgeRing::findEdgeRingContaining(holeER, shellList);

	if (shell!=NULL)
		shell->addHole(holeER->getRingOwnership());
}


} // namespace geos.operation.polygonize
} // namespace geos.operation
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.10  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.9  2005/12/09 10:32:28  strk
 * Cleaned up debugging line left over from previous commit
 *
 * Revision 1.8  2005/12/09 10:03:46  strk
 * Fixed a bug making PolygonizeGraph choking on invalid LineStrings.
 * Minor optimizations in Polygonizer loops.
 *
 * Revision 1.7  2005/06/17 15:08:06  strk
 * Polygonizer segfault fix
 *
 * Revision 1.6  2004/12/08 13:54:44  strk
 * gcc warnings checked and fixed, general cleanups.
 *
 * Revision 1.5  2004/10/27 13:57:07  strk
 * Added some debugging lines (disabled by default)
 *
 * Revision 1.4  2004/10/26 16:09:21  strk
 * Some more intentation and envelope equality check fix.
 *
 * Revision 1.3  2004/10/19 19:51:14  strk
 * Fixed many leaks and bugs in Polygonizer.
 * Output still bogus.
 *
 * Revision 1.2  2004/07/02 13:28:29  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.1  2004/04/08 04:53:56  ybychkov
 * "operation/polygonize" ported from JTS 1.4
 *
 *
 **********************************************************************/
