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

#include <geos/geomgraph.h>
#include <geos/util.h>

#define DEBUG 0

namespace geos {

EdgeEndStar::EdgeEndStar():
	//edgeMap(new set<EdgeEnd*,EdgeEndLT>()),
	edgeMap()
	//edgeList(NULL)
{
	ptInAreaLocation[0]=Location::UNDEF;
	ptInAreaLocation[1]=Location::UNDEF;
}

/**
 * @return the coordinate for the node this star is based at
 */
Coordinate&
EdgeEndStar::getCoordinate()
{
	static Coordinate nullCoord(DoubleNotANumber, DoubleNotANumber, DoubleNotANumber);
	if (edgeMap.size()==0) return nullCoord;

	EdgeEndStar::iterator it=begin();
	EdgeEnd *e=*it;
	return e->getCoordinate();
}

#if 0
//vector<EdgeEnd*>*
EdgeEndStar::container &
EdgeEndStar::getEdges()
{
	return edgeMap;
#if 0
	if (edgeList==NULL)
	{
#if DEBUG
		cerr<<"EdgeEndStar["<<this<<"]::getEdges() computing edgeList"<<endl;
#endif
		edgeList=new vector<EdgeEnd*>();

		set<EdgeEnd*,EdgeEndLT>::iterator mapIter;
		set<EdgeEnd*,EdgeEndLT>::iterator begin=edgeMap.begin();
		set<EdgeEnd*,EdgeEndLT>::iterator end=edgeMap.end();

		for(mapIter=begin; mapIter!=end; ++mapIter)
		{
			EdgeEnd *e=*mapIter;
			edgeList->push_back(e);
		}
#if DEBUG
		cerr<<"EdgeEndStar["<<this<<"]::getEdges() computed edgeList at "<<edgeList<<" with size "<<edgeList->size()<<endl;
#endif
	}
#if DEBUG
	else
	{
		cerr<<"EdgeEndStar["<<this<<"]::getEdges() using cached edgeList "<<edgeList<<" with size "<<edgeList->size()<<endl;
	}
#endif
	return edgeList;
#endif
}
#endif

EdgeEnd*
EdgeEndStar::getNextCW(EdgeEnd *ee)
{
	EdgeEndStar::iterator it=find(ee);
	if ( it==end() ) return NULL;
	if ( it==begin() ) { it=end(); --it; }
	else --it;
	return *it;
#if 0
	getEdges();
	unsigned int i=0;
	for(unsigned int j=0;j<edgeList->size();j++)
	{
		//if (ee->compareTo( *(edgeList->at(j)))==0) 
		if (ee->compareTo((*edgeList)[j])==0)
		{
			i=j;
			break;
		}
	}
	unsigned int iNextCW=i-1;
	if (i==0) iNextCW=edgeList->size()-1;
	return (*edgeList)[iNextCW];
#endif
}

void
EdgeEndStar::computeLabelling(vector<GeometryGraph*> *geom)
	//throw(TopologyException *)
{
	computeEdgeEndLabels();

	// Propagate side labels  around the edges in the star
	// for each parent Geometry
	//
	// these calls can throw a TopologyException
	propagateSideLabels(0);
	propagateSideLabels(1);

	/**
	 * If there are edges that still have null labels for a geometry
	 * this must be because there are no area edges for that geometry
	 * incident on this node.
	 * In this case, to label the edge for that geometry we must test
	 * whether the edge is in the interior of the geometry.
	 * To do this it suffices to determine whether the node for the
	 * edge is in the interior of an area.
	 * If so, the edge has location INTERIOR for the geometry.
	 * In all other cases (e.g. the node is on a line, on a point, or
	 * not on the geometry at all) the edge
	 * has the location EXTERIOR for the geometry.
	 * 
	 * Note that the edge cannot be on the BOUNDARY of the geometry,
	 * since then there would have been a parallel edge from the
	 * Geometry at this node also labelled BOUNDARY
	 * and this edge would have been labelled in the previous step.
	 *
	 * This code causes a problem when dimensional collapses are present,
	 * since it may try and determine the location of a node where a
	 * dimensional collapse has occurred.
	 * The point should be considered to be on the EXTERIOR
	 * of the polygon, but locate() will return INTERIOR, since it is
	 * passed the original Geometry, not the collapsed version.
	 *
	 * If there are incident edges which are Line edges labelled BOUNDARY,
	 * then they must be edges resulting from dimensional collapses.
	 * In this case the other edges can be labelled EXTERIOR for this
	 * Geometry.
	 *
	 * MD 8/11/01 - NOT TRUE!  The collapsed edges may in fact be in the
	 * interior of the Geometry, which means the other edges should be
	 * labelled INTERIOR for this Geometry.
	 * Not sure how solve this...  Possibly labelling needs to be split
	 * into several phases:
	 * area label propagation, symLabel merging, then finally null label
	 * resolution.
	 */
	bool hasDimensionalCollapseEdge[2]={false,false};

	EdgeEndStar::iterator endIt=end();
	for (EdgeEndStar::iterator it=begin(); it!=endIt; ++it)
	{
		EdgeEnd *e=*it;
		Label *label=e->getLabel();
		for(int geomi=0; geomi<2; geomi++)
		{
			if (label->isLine(geomi) && label->getLocation(geomi)==Location::BOUNDARY)
				hasDimensionalCollapseEdge[geomi]=true;
		}
	}

	for (EdgeEndStar::iterator it=begin(); it!=end(); ++it)
	{
		EdgeEnd *e=*it;
		Label *label=e->getLabel();
		for(int geomi=0;geomi<2;geomi++){
			if (label->isAnyNull(geomi)) {
				int loc=Location::UNDEF;
				if (hasDimensionalCollapseEdge[geomi]){
					loc=Location::EXTERIOR;
				}else {
					Coordinate& p=e->getCoordinate();
					loc=getLocation(geomi,p,geom);
				}
				label->setAllLocationsIfNull(geomi,loc);
			}
		}
	}
}

void
EdgeEndStar::computeEdgeEndLabels()
{
	// Compute edge label for each EdgeEnd
	for (EdgeEndStar::iterator it=begin(); it!=end(); ++it)
	{
		EdgeEnd *e=*it;
		e->computeLabel();
	}
}

int
EdgeEndStar::getLocation(int geomIndex,
	const Coordinate& p, vector<GeometryGraph*> *geom)
{
	// compute location only on demand
	if (ptInAreaLocation[geomIndex]==Location::UNDEF)
	{
        	ptInAreaLocation[geomIndex]=SimplePointInAreaLocator::locate(p,(*geom)[geomIndex]->getGeometry());
	}
	return ptInAreaLocation[geomIndex];
}

bool EdgeEndStar::isAreaLabelsConsistent(){
	computeEdgeEndLabels();
	return checkAreaLabelsConsistent(0);
}

bool
EdgeEndStar::checkAreaLabelsConsistent(int geomIndex)
{
	// Since edges are stored in CCW order around the node,
	// As we move around the ring we move from the right to
	// the left side of the edge

	// if no edges, trivially consistent
	if (edgeMap.size()==0) return true;

	// initialize startLoc to location of last L side (if any)
	EdgeEndStar::reverse_iterator it=rbegin();

	Label *startLabel=(*it)->getLabel();
	int startLoc=startLabel->getLocation(geomIndex, Position::LEFT);
	Assert::isTrue(startLoc!=Location::UNDEF, "Found unlabelled area edge");
	int currLoc=startLoc;

	for (EdgeEndStar::iterator it=begin(); it!=end(); ++it)
	{
		EdgeEnd *e=*it;
		Label *eLabel=e->getLabel();
		// we assume that we are only checking a area
		Assert::isTrue(eLabel->isArea(geomIndex), "Found non-area edge");
		int leftLoc=eLabel->getLocation(geomIndex,Position::LEFT);
		int rightLoc=eLabel->getLocation(geomIndex,Position::RIGHT);
		// check that edge is really a boundary between inside and outside!
		if (leftLoc==rightLoc) {
			return false;
		}
		// check side location conflict
		//Assert.isTrue(rightLoc == currLoc, "side location conflict " + locStr);
		if (rightLoc!=currLoc) {
			return false;
		}
		currLoc=leftLoc;
	}
	return true;
}

void
EdgeEndStar::propagateSideLabels(int geomIndex)
	//throw(TopologyException *)
{
	// Since edges are stored in CCW order around the node,
	// As we move around the ring we move from the right to the
	// left side of the edge
	int startLoc=Location::UNDEF;

	EdgeEndStar::iterator beginIt=begin();
	EdgeEndStar::iterator endIt=end();
	EdgeEndStar::iterator it;

	// initialize loc to location of last L side (if any)
	for (EdgeEndStar::iterator it=beginIt; it!=endIt; ++it)
	{
		EdgeEnd *e=*it;
		Label *label=e->getLabel();
		if (label->isArea(geomIndex) &&
			label->getLocation(geomIndex,Position::LEFT)!=Location::UNDEF)
			startLoc=label->getLocation(geomIndex,Position::LEFT);
	}

	// no labelled sides found, so no labels to propagate
	if (startLoc==Location::UNDEF) return;

	int currLoc=startLoc;
	for (it=beginIt; it!=endIt; ++it)
	{
		EdgeEnd *e=*it;
		Label *label=e->getLabel();
		// set null ON values to be in current location
		if (label->getLocation(geomIndex,Position::ON)==Location::UNDEF)
			label->setLocation(geomIndex,Position::ON,currLoc);
		// set side labels (if any)
		// if (label.isArea())  //ORIGINAL
		if (label->isArea(geomIndex))
		{
			int leftLoc=label->getLocation(geomIndex,
				Position::LEFT);

			int rightLoc=label->getLocation(geomIndex,
				Position::RIGHT);

			// if there is a right location, that is the next
			// location to propagate
			if (rightLoc!=Location::UNDEF) {
				if (rightLoc!=currLoc)
					throw new TopologyException("side location conflict",&(e->getCoordinate()));
				if (leftLoc==Location::UNDEF) {
					Assert::shouldNeverReachHere("found single null side (at " + (e->getCoordinate()).toString() + ")");
				}
				currLoc=leftLoc;
			} else {
				/**
				 * RHS is null - LHS must be null too.
				 * This must be an edge from the other
				 * geometry, which has no location
				 * labelling for this geometry.
				 * This edge must lie wholly inside or
				 * outside the other geometry (which is
				 * determined by the current location).
				 * Assign both sides to be the current
				 * location.
				 */
				Assert::isTrue(label->getLocation(geomIndex,
					Position::LEFT)==Location::UNDEF,
					"found single null side");
				label->setLocation(geomIndex,Position::RIGHT,
					currLoc);
				label->setLocation(geomIndex,Position::LEFT,
					currLoc);
			}
		}
	}
}

string
EdgeEndStar::print()
{
	string out="EdgeEndStar:   " + getCoordinate().toString()+"\n";
	for (EdgeEndStar::iterator it=begin(); it!=end(); ++it)
	{
		EdgeEnd *e=*it;
		out+=e->print();
	}
	return out;
}

} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.11  2005/11/29 00:48:35  strk
 * Removed edgeList cache from EdgeEndRing. edgeMap is enough.
 * Restructured iterated access by use of standard ::iterator abstraction
 * with scoped typedefs.
 *
 * Revision 1.10  2005/11/21 16:03:20  strk
 *
 * Coordinate interface change:
 *         Removed setCoordinate call, use assignment operator
 *         instead. Provided a compile-time switch to
 *         make copy ctor and assignment operators non-inline
 *         to allow for more accurate profiling.
 *
 * Coordinate copies removal:
 *         NodeFactory::createNode() takes now a Coordinate reference
 *         rather then real value. This brings coordinate copies
 *         in the testLeaksBig.xml test from 654818 to 645991
 *         (tested in 2.1 branch). In the head branch Coordinate
 *         copies are 222198.
 *         Removed useless coordinate copies in ConvexHull
 *         operations
 *
 * STL containers heap allocations reduction:
 *         Converted many containers element from
 *         pointers to real objects.
 *         Made some use of .reserve() or size
 *         initialization when final container size is known
 *         in advance.
 *
 * Stateless classes allocations reduction:
 *         Provided ::instance() function for
 *         NodeFactories, to avoid allocating
 *         more then one (they are all
 *         stateless).
 *
 * HCoordinate improvements:
 *         Changed HCoordinate constructor by HCoordinates
 *         take reference rather then real objects.
 *         Changed HCoordinate::intersection to avoid
 *         a new allocation but rather return into a provided
 *         storage. LineIntersector changed to reflect
 *         the above change.
 *
 * Revision 1.9  2004/12/08 13:54:43  strk
 * gcc warnings checked and fixed, general cleanups.
 *
 * Revision 1.8  2004/11/23 19:53:06  strk
 * Had LineIntersector compute Z by interpolation.
 *
 * Revision 1.7  2004/11/22 11:34:49  strk
 * More debugging lines and comments/indentation cleanups
 *
 * Revision 1.6  2004/11/17 08:13:16  strk
 * Indentation changes.
 * Some Z_COMPUTATION activated by default.
 *
 * Revision 1.5  2004/11/01 16:43:04  strk
 * Added Profiler code.
 * Temporarly patched a bug in DoubleBits (must check drawbacks).
 * Various cleanups and speedups.
 *
 * Revision 1.4  2004/10/21 22:29:54  strk
 * Indentation changes and some more COMPUTE_Z rules
 *
 * Revision 1.3  2004/07/02 13:28:26  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.2  2004/05/03 10:43:42  strk
 * Exception specification considered harmful - left as comment.
 *
 * Revision 1.1  2004/03/19 09:48:45  ybychkov
 * "geomgraph" and "geomgraph/indexl" upgraded to JTS 1.4
 *
 * Revision 1.18  2003/11/12 15:43:38  strk
 * Added some more throw specifications
 *
 * Revision 1.17  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/

