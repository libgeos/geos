#ifndef GEOS_INDEXCHAIN_H
#define GEOS_INDEXCHAIN_H

#include <memory>
#include <vector>
#include "platform.h"
#include "geom.h"

using namespace std;

namespace geos {

class indexMonotoneChain;
/**
 * The action for the internal iterator for performing
 * envelope select queries on a MonotoneChain
 */
class MonotoneChainSelectAction {
protected:
	LineSegment *seg;
public:
	MonotoneChainSelectAction();
	virtual ~MonotoneChainSelectAction();
	virtual void select(indexMonotoneChain *mc,int start);
	virtual void select(LineSegment *newSeg){}
};

/**
 * The action for the internal iterator for performing
 * overlap queries on a MonotoneChain
 */
class MonotoneChainOverlapAction {
protected:
	LineSegment *seg1;
	LineSegment *seg2;
public:
	MonotoneChainOverlapAction();
	~MonotoneChainOverlapAction();
	/**
	* This function can be overridden if the original chains are needed
	*/
	void overlap(indexMonotoneChain *mc1,int start1,indexMonotoneChain *mc2,int start2);
	void overlap(LineSegment *newSeg1,LineSegment *newSeg2){}
};


/**
 * MonotoneChains are a way of partitioning the segments of an edge to
 * allow for fast searching of intersections.
 * They have the following properties:
 * <ol>
 * <li>the segments within a monotone chain will never intersect each other
 * <li>the envelope of any contiguous subset of the segments in a monotone chain
 * is equal to the envelope of the endpoints of the subset.
 * </ol>
 * Property 1 means that there is no need to test pairs of segments from within
 * the same monotone chain for intersection.
 * Property 2 allows
 * binary search to be used to find the intersection points of two monotone chains.
 * For many types of real-world data, these properties eliminate a large number of
 * segment comparisons, producing substantial speed gains.
 * <p>
 * One of the goals of this implementation of MonotoneChains is to be
 * as space and time efficient as possible. One design choice that aids this
 * is that a MonotoneChain is based on a subarray of a list of points.
 * This means that new arrays of points (potentially very large) do not
 * have to be allocated.
 * <p>
 *
 * MonotoneChains support the following kinds of queries:
 * <ul>
 * <li>Envelope select: determine all the segments in the chain which
 * intersect a given envelope
 * <li>Overlap: determine all the pairs of segments in two chains whose
 * envelopes overlap
 * </ul>
 *
 * This implementation of MonotoneChains uses the concept of internal iterators
 * to return the resultsets for the above queries.
 * This has time and space advantages, since it
 * is not necessary to build lists of instantiated objects to represent the segments
 * returned by the query.
 * However, it does mean that the queries are not thread-safe.
 *
 * @version 1.2
 */
class indexMonotoneChain {
public:
	indexMonotoneChain(CoordinateList *newPts,int nstart,int nend);
	~indexMonotoneChain();
	Envelope* getEnvelope();
	int getStartIndex();
	int getEndIndex();
	void getLineSegment(int index,LineSegment *ls);
	/**
	* Return the subsequence of coordinates forming this chain.
	* Allocates a new array to hold the Coordinates
	*/
	CoordinateList* getCoordinates();
	/**
	* Determine all the line segments in the chain whose envelopes overlap
	* the searchEnvelope, and process them
	*/
	void select(Envelope *searchEnv,MonotoneChainSelectAction *mcs);
	void computeOverlaps(indexMonotoneChain *mc,MonotoneChainOverlapAction *mco);
private:
	void computeSelect(Envelope *searchEnv,int start0,int end0,MonotoneChainSelectAction *mcs);
	void computeOverlaps(int start0,int end0,indexMonotoneChain *mc,int start1,int end1,MonotoneChainOverlapAction *mco);
	CoordinateList *pts;
	int start, end;
	Envelope *env;
	// these envelopes are created once and reused
	Envelope *env1;
	Envelope *env2;
};

/**
 * A MonotoneChainBuilder implements functions to determine the monotone chains
 * in a sequence of points.
 */
class MonotoneChainBuilder {
public:
//	static int[] toIntArray(List list); //Not needed
	MonotoneChainBuilder(){}
	/**
	* Return a list of the monotone chains
	* for the given list of coordinates.
	*/
	static vector<indexMonotoneChain*>* getChains(CoordinateList *pts);
	/**
	* Return an array containing lists of start/end indexes of the monotone chains
	* for the given list of coordinates.
	* The last entry in the array points to the end point of the point array,
	* for use as a sentinel.
	*/
	static vector<int>* getChainStartIndices(CoordinateList *pts);
	/**
	* @return the index of the last point in the monotone chain starting at <code>start</code>.
	*/
	static int findChainEnd(CoordinateList *pts,int start);
};
}
#endif

