#ifndef GEOS_INDEX_H
#define GEOS_INDEX_H

#include "platform.h"
#include "geom.h"

using namespace std;

class SpatialIndex {
public:
	/**
	* Adds a spatial item to the index with the given envelope
	*/
	virtual void insert(Envelope *itemEnv,void *item)=0;
	/**
	* Queries the index for all items whose envelopes intersect the given search envelope
	*
	* @param searchEnv the envelope to query for
	* @return a list of the items found by the query
	*/
	virtual vector<void*>* query(Envelope *searchEnv)=0;
};

#endif

