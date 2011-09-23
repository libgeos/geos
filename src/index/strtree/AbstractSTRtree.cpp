/**********************************************************************
 * $Id: AbstractSTRtree.cpp 2611 2009-07-15 18:53:02Z pramsey $
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/index/strtree/AbstractSTRtree.h>
#include <geos/index/strtree/AbstractNode.h>
#include <geos/index/strtree/ItemBoundable.h>
#include <geos/index/ItemVisitor.h>
// std
#include <algorithm>
#include <vector>
#include <typeinfo>
#include <cstddef>
#include <cassert>

using namespace std;

namespace geos {
namespace index { // geos.index
namespace strtree { // geos.index.strtree


AbstractSTRtree::~AbstractSTRtree()
{
	assert(0 != itemBoundables);
    BoundableList::iterator it = itemBoundables->begin();
    BoundableList::iterator end = itemBoundables->end();
	while (it != end)
	{
		delete *it; 
        ++it;
	}
	delete itemBoundables;

	assert(0 != nodes);
	for (std::size_t i = 0, nsize = nodes->size(); i < nsize; i++)
    {
		delete (*nodes)[i];
    }
	delete nodes;
}

/*public*/
void
AbstractSTRtree::build()
{
	assert(!built);
	root=(itemBoundables->empty()?createNode(0):createHigherLevels(itemBoundables,-1));
	built=true;
}

/*protected*/
std::auto_ptr<BoundableList>
AbstractSTRtree::createParentBoundables(BoundableList* childBoundables,
		int newLevel)
{
	assert(!childBoundables->empty());
	std::auto_ptr< BoundableList > parentBoundables ( new BoundableList() );
	parentBoundables->push_back(createNode(newLevel));

	std::auto_ptr< BoundableList > sortedChildBoundables ( sortBoundables(childBoundables) );

	for (BoundableList::iterator i=sortedChildBoundables->begin(),
			e=sortedChildBoundables->end();
			i!=e; i++)
	//for(std::size_t i=0, scbsize=sortedChildBoundables->size(); i<scbsize; ++i)
	{
		Boundable *childBoundable=*i; // (*sortedChildBoundables)[i];

		AbstractNode *last = lastNode(parentBoundables.get());
		if (last->getChildBoundables()->size() == nodeCapacity)
		{
			last=createNode(newLevel);
			parentBoundables->push_back(last);
		}
		last->addChildBoundable(childBoundable);
	}
	return parentBoundables;
}

/*private*/
AbstractNode*
AbstractSTRtree::createHigherLevels(BoundableList* boundablesOfALevel, int level)
{
	assert(!boundablesOfALevel->empty());
	std::auto_ptr< BoundableList > parentBoundables (
			createParentBoundables(boundablesOfALevel,level+1)
			);

	if (parentBoundables->size()==1)
	{
		// Cast from Boundable to AbstractNode
		AbstractNode *ret = static_cast<AbstractNode*>(parentBoundables->front());
		return ret;
	}
	AbstractNode *ret = createHigherLevels(parentBoundables.get(), level+1);
	return ret;
}

/*protected*/
void
AbstractSTRtree::insert(const void* bounds,void* item)
{
	// Cannot insert items into an STR packed R-tree after it has been built
	assert(!built);
	itemBoundables->push_back(new ItemBoundable(bounds,item));
}

/*protected*/
void
AbstractSTRtree::query(const void* searchBounds, vector<void*>& matches)
{
	if (!built) build();

	if (itemBoundables->empty()) assert(root->getBounds()==NULL);

	if (getIntersectsOp()->intersects(root->getBounds(), searchBounds))
	{
		query(searchBounds,root, &matches);
	}
}

/*protected*/
void
AbstractSTRtree::query(const void* searchBounds, ItemVisitor& visitor)
{
	if (!built) build();

	if (itemBoundables->empty()) assert(root->getBounds()==NULL);
	
	if (getIntersectsOp()->intersects(root->getBounds(),searchBounds))
	{
		query(searchBounds, *root, visitor);
	}
}

/*protected*/
void
AbstractSTRtree::query(const void* searchBounds, const AbstractNode& node,
		ItemVisitor& visitor)
{

	const BoundableList& boundables = *(node.getChildBoundables());

	for (BoundableList::const_iterator i=boundables.begin(), e=boundables.end();
			i!=e; i++)
	{
		const Boundable* childBoundable = *i;
		if (!getIntersectsOp()->intersects(childBoundable->getBounds(), searchBounds)) {
			continue;
		}

		if(const AbstractNode *an=dynamic_cast<const AbstractNode*>(childBoundable))
		{
			query(searchBounds, *an, visitor);
		}
		else if (const ItemBoundable *ib=dynamic_cast<const ItemBoundable *>(childBoundable))
		{
			visitor.visitItem(ib->getItem());
		}
		else
		{
			assert(0); // unsupported childBoundable type
		}
	}
}

/* protected */
bool
AbstractSTRtree::remove(const void* searchBounds, void* item)
{
	if (!built) build();
	if (itemBoundables->empty()) {
		assert(root->getBounds() == NULL);
	}
	if (getIntersectsOp()->intersects(root->getBounds(), searchBounds)) {
		return remove(searchBounds, *root, item);
	}
	return false;
}

/* private */
bool
AbstractSTRtree::remove(const void* searchBounds, AbstractNode& node, void* item)
{
	// first try removing item from this node
	if ( removeItem(node, item) ) return true;

	BoundableList& boundables = *(node.getChildBoundables());

	// next try removing item from lower nodes
	for (BoundableList::iterator i=boundables.begin(), e=boundables.end();
			i!=e; i++)
	{
		Boundable* childBoundable = *i;
		if (!getIntersectsOp()->intersects(childBoundable->getBounds(), searchBounds))
			continue;

		if (AbstractNode *an=dynamic_cast<AbstractNode*>(childBoundable))
		{
			// if found, record child for pruning and exit
			if ( remove(searchBounds, *an, item) )
			{
				if (an->getChildBoundables()->empty()) {
					boundables.erase(i);
				}
				return true;
			}
		}
	}

	return false;
}

/*private*/
bool
AbstractSTRtree::removeItem(AbstractNode& node, void* item)
{
	BoundableList& boundables = *(node.getChildBoundables());

	BoundableList::iterator childToRemove = boundables.end();

	for (BoundableList::iterator i=boundables.begin(),
			e=boundables.end();
			i!=e; i++)
	{
		Boundable* childBoundable = *i;
		if (ItemBoundable *ib=dynamic_cast<ItemBoundable*>(childBoundable))
		{
			if ( ib->getItem() == item) childToRemove = i;
		}
	}
	if (childToRemove != boundables.end()) {
		boundables.erase(childToRemove);
		return true;
	}
	return false;
}



/*public*/
void
AbstractSTRtree::query(const void* searchBounds,
	const AbstractNode* node, vector<void*> *matches)
{
	assert(node);

	const BoundableList& vb = *(node->getChildBoundables());


	IntersectsOp *io=getIntersectsOp();
	//std::size_t vbsize=vb.size();
	//cerr<<"AbstractSTRtree::query: childBoundables: "<<vbsize<<endl;
	for(BoundableList::const_iterator i=vb.begin(), e=vb.end();
			i!=e; ++i)
	{
		const Boundable* childBoundable=*i;
		if (!io->intersects(childBoundable->getBounds(), searchBounds))
		{
			continue;
		}

		if(const AbstractNode *an=dynamic_cast<const AbstractNode*>(childBoundable))
		{
			query(searchBounds, an, matches);
		}
		else if (const ItemBoundable *ib=dynamic_cast<const ItemBoundable *>(childBoundable))
		{
			matches->push_back(ib->getItem());
		}
		else
		{
			assert(0); // unsupported childBoundable type
		}
	}
}

void
AbstractSTRtree::iterate(ItemVisitor& visitor)
{
    for (BoundableList::const_iterator i=itemBoundables->begin(), e=itemBoundables->end();
			i!=e; i++)
	{
		const Boundable* boundable = *i;
		if (const ItemBoundable *ib=dynamic_cast<const ItemBoundable *>(boundable))
		{
			visitor.visitItem(ib->getItem());
		}
        }
}

/*protected*/
std::auto_ptr<BoundableList>
AbstractSTRtree::boundablesAtLevel(int level)
{
	std::auto_ptr<BoundableList> boundables ( new BoundableList() );
	boundablesAtLevel(level, root, boundables.get());
	return boundables;
}

/*public*/
void
AbstractSTRtree::boundablesAtLevel(int level, AbstractNode* top,
		BoundableList* boundables)
{
	assert(level>-2);
	if (top->getLevel()==level)
	{
		boundables->push_back(top);
		return;
	}

	assert(top);

	const BoundableList& vb = *(top->getChildBoundables());

	for(BoundableList::const_iterator i=vb.begin(), e=vb.end();
			i!=e; ++i)
	{
		Boundable* boundable=*i;
		if (typeid(*boundable)==typeid(AbstractNode))
		{
			boundablesAtLevel(level, (AbstractNode*)boundable,
				boundables);
		}
		else
		{
			assert(typeid(*boundable)==typeid(ItemBoundable));
			if (level==-1)
			{
				boundables->push_back(boundable);
			}
		}
	}
	return;
}

ItemsList* AbstractSTRtree::itemsTree(AbstractNode* node) 
{
    std::auto_ptr<ItemsList> valuesTreeForNode (new ItemsList());

    BoundableList::iterator end = node->getChildBoundables()->end();
    for (BoundableList::iterator i = node->getChildBoundables()->begin(); 
         i != end; ++i) 
    {
        Boundable* childBoundable = *i;
        if (dynamic_cast<AbstractNode*>(childBoundable)) {
            ItemsList* valuesTreeForChild = 
                itemsTree(static_cast<AbstractNode*>(childBoundable));
            // only add if not null (which indicates an item somewhere in this tree
            if (valuesTreeForChild != NULL)
                valuesTreeForNode->push_back_owned(valuesTreeForChild);
        }
        else if (dynamic_cast<ItemBoundable*>(childBoundable)) {
            valuesTreeForNode->push_back(
                static_cast<ItemBoundable*>(childBoundable)->getItem());
        }
        else {
            assert(!"should never be reached");
        }
    }
    if (valuesTreeForNode->empty()) 
        return NULL;

    return valuesTreeForNode.release();
}

ItemsList* AbstractSTRtree::itemsTree()
{
    if (!built) { 
        build(); 
    }

    ItemsList* valuesTree (itemsTree(root));
    if (valuesTree == NULL)
        return new ItemsList();

    return valuesTree;
}

} // namespace geos.index.strtree
} // namespace geos.index
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.30  2006/06/12 10:49:43  strk
 * unsigned int => std::size_t
 *
 * Revision 1.29  2006/03/21 10:47:34  strk
 * indexStrtree.h split
 *
 * Revision 1.28  2006/03/03 10:46:21  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.27  2006/02/23 11:54:20  strk
 * - MCIndexPointSnapper
 * - MCIndexSnapRounder
 * - SnapRounding BufferOp
 * - ScaledNoder
 * - GEOSException hierarchy cleanups
 * - SpatialIndex memory-friendly query interface
 * - GeometryGraph::getBoundaryNodes memory-friendly
 * - NodeMap::getBoundaryNodes memory-friendly
 * - Cleanups in geomgraph::Edge
 * - Added an XML test for snaprounding buffer (shows leaks, working on it)
 *
 * Revision 1.26  2006/02/20 21:04:37  strk
 * - namespace geos::index
 * - SpatialIndex interface synced
 *
 * Revision 1.25  2006/02/20 10:14:18  strk
 * - namespaces geos::index::*
 * - Doxygen documentation cleanup
 *
 * Revision 1.24  2005/02/15 17:15:13  strk
 * Inlined most Envelope methods, reserved() memory for some vectors when
 * the usage was known a priori.
 *
 * Revision 1.23  2005/01/31 15:41:03  strk
 * Small optimizations.
 *
 * Revision 1.22  2004/12/08 13:54:43  strk
 * gcc warnings checked and fixed, general cleanups.
 *
 * Revision 1.21  2004/11/08 15:58:13  strk
 * More performance tuning.
 *
 * Revision 1.20  2004/11/04 19:08:07  strk
 * Cleanups, initializers list, profiling.
 *
 * Revision 1.19  2004/11/01 16:43:04  strk
 * Added Profiler code.
 * Temporarly patched a bug in DoubleBits (must check drawbacks).
 * Various cleanups and speedups.
 *
 * Revision 1.18  2004/07/27 16:35:46  strk
 * Geometry::getEnvelopeInternal() changed to return a const Envelope *.
 * This should reduce object copies as once computed the envelope of a
 * geometry remains the same.
 *
 * Revision 1.17  2004/07/13 08:33:52  strk
 * Added missing virtual destructor to virtual classes.
 * Fixed implicit unsigned int -> int casts
 *
 * Revision 1.16  2004/07/02 13:28:27  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.15  2004/05/06 15:00:59  strk
 * Boundable destructor made virtual.
 * Added vector <AbstractNode *> *nodes member in AbstractSTRTree,
 * used to keep track of created node to cleanly delete them at
 * destruction time.
 *
 * Revision 1.14  2004/05/06 08:59:19  strk
 * memory leak fixed
 *
 * Revision 1.13  2004/05/05 17:42:06  strk
 * AbstractNode destructor made virtual. AbstractNode::bounds made protected.
 * SIRAbstractNode and STRAbstractNode destructors added to get rid of
 * AbstractNode::bounds in the right way (is a void * casted to appropriate
 * Class in the subClasses).
 *
 * Revision 1.12  2004/05/03 22:56:44  strk
 * leaks fixed, exception specification omitted.
 *
 * Revision 1.11  2004/05/03 16:29:21  strk
 * Added sortBoundables(const vector<Boundable *>) pure virtual in AbstractSTRtree,
 * implemented in SIRtree and STRtree. Comparator funx made static in STRtree.cpp
 * and SIRtree.cpp.
 *
 * Revision 1.10  2004/05/03 13:17:55  strk
 * Fixed comparator function to express StrictWeakOrdering.
 *
 * Revision 1.9  2004/04/28 14:58:47  strk
 * Made AbstractSTRtree::query use dynamic_cast<> to simulate java's
 * instanceof. Previous typeid(*) use missed to catch an STRAbstractNode
 * as a class derived from AbstractNode. Still have to check if this
 * is the correct semantic with Martin, but at least lots of SIGABORT
 * are no more raised.
 *
 * Revision 1.8  2004/04/26 12:37:19  strk
 * Some leaks fixed.
 *
 * Revision 1.7  2004/04/14 12:28:43  strk
 * shouldNeverReachHere exceptions made more verbose
 *
 * Revision 1.6  2004/03/25 02:23:55  ybychkov
 * All "index/" packages upgraded to JTS 1.4
 *
 * Revision 1.5  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/

