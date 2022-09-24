/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2020-2021 Daniel Baston
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/constants.h>
#include <geos/index/strtree/TemplateSTRNode.h>
#include <geos/index/strtree/TemplateSTRNodePair.h>
#include <geos/util/IllegalArgumentException.h>
#include <geos/util.h>

#include <queue>
#include <memory>
#include <vector>

namespace geos {
namespace index {
namespace strtree {

template<typename ItemType, typename BoundsType, typename ItemDistance>
class TemplateSTRtreeDistance {
    using Node = TemplateSTRNode<ItemType, BoundsType>;
    using NodePair = TemplateSTRNodePair<ItemType, BoundsType, ItemDistance>;
    using ItemPair = std::pair<ItemType, ItemType>;

    struct PairQueueCompare {
        bool operator()(const NodePair& a, const NodePair& b) {
            return a.getDistance() > b.getDistance();
        }
    };

    using PairQueue = std::priority_queue<NodePair, std::vector<NodePair>, PairQueueCompare>;

public:
    explicit TemplateSTRtreeDistance(ItemDistance& id) : m_id(id) {}

    ItemPair nearestNeighbour(const Node& root1, const Node& root2) {
        NodePair initPair(root1, root2, m_id);
        return nearestNeighbour(initPair);
    }

    ItemPair nearestNeighbour(NodePair& initPair) {
        return nearestNeighbour(initPair, DoubleInfinity);
    }

    bool isWithinDistance(const Node& root1, const Node& root2, double maxDistance) {
        NodePair initPair(root1, root2, m_id);
        return isWithinDistance(initPair, maxDistance);
    }

private:

    ItemPair nearestNeighbour(NodePair& initPair, double maxDistance) {
        double distanceLowerBound = maxDistance;
        std::unique_ptr<NodePair> minPair;

        PairQueue priQ;
        priQ.push(initPair);

        while (!priQ.empty() && distanceLowerBound > 0) {
            NodePair pair = priQ.top();
            priQ.pop();
            double currentDistance = pair.getDistance();

            /*
             * If the distance for the first node in the queue
             * is >= the current minimum distance, all other nodes
             * in the queue must also have a greater distance.
             * So the current minDistance must be the true minimum,
             * and we are done.
             */
            if (minPair && currentDistance >= distanceLowerBound) {
                break;
            }

            /*
             * If the pair members are leaves
             * then their distance is the exact lower bound.
             * Update the distanceLowerBound to reflect this
             * (which must be smaller, due to the test
             * immediately prior to this).
             */
            if (pair.isLeaves()) {
                distanceLowerBound = currentDistance;
                if (minPair) {
                    *minPair = pair;
                } else {
                    minPair = detail::make_unique<NodePair>(pair);
                }
            } else {
                /*
                 * Otherwise, expand one side of the pair,
                 * (the choice of which side to expand is heuristically determined)
                 * and insert the new expanded pairs into the queue
                 */
                expandToQueue(pair, priQ, distanceLowerBound);
            }
        }

        if (!minPair) {
            throw util::GEOSException("Error computing nearest neighbor");
        }

        return minPair->getItems();
    }

    void expandToQueue(const NodePair& pair, PairQueue& priQ, double minDistance) {
        const Node& node1 = pair.getFirst();
        const Node& node2 = pair.getSecond();

        bool isComp1 = node1.isComposite();
        bool isComp2 = node2.isComposite();

        /**
         * HEURISTIC: If both boundables are composite,
         * choose the one with largest area to expand.
         * Otherwise, simply expand whichever is composite.
         */
        if (isComp1 && isComp2) {
            if (node1.getSize() > node2.getSize()) {
                expand(node1, node2, false, priQ, minDistance);
                return;
            } else {
                expand(node2, node1, true, priQ, minDistance);
                return;
            }
        } else if (isComp1) {
            expand(node1, node2, false, priQ, minDistance);
            return;
        } else if (isComp2) {
            expand(node2, node1, true, priQ, minDistance);
            return;
        }

        throw util::IllegalArgumentException("neither boundable is composite");

    }

    void expand(const Node &nodeComposite, const Node &nodeOther, bool isFlipped, PairQueue& priQ,
                double minDistance) {
        for (const auto *child = nodeComposite.beginChildren();
             child < nodeComposite.endChildren(); ++child) {
            NodePair sp = isFlipped ? NodePair(nodeOther, *child, m_id) : NodePair(*child, nodeOther, m_id);

            // only add to queue if this pair might contain the closest points
            if (minDistance == DoubleInfinity || sp.getDistance() < minDistance) {
                priQ.push(sp);
            }
        }
    }

    bool isWithinDistance(const NodePair& initPair, double maxDistance) {
        double distanceUpperBound = DoubleInfinity;

        // initialize search queue
        PairQueue priQ;
        priQ.push(initPair);

        while (! priQ.empty()) {
            // pop head of queue and expand one side of pair
            NodePair pair = priQ.top();
            double pairDistance = pair.getDistance();

            /**
            * If the distance for the first pair in the queue
            * is > maxDistance, all other pairs
            * in the queue must have a greater distance as well.
            * So can conclude no items are within the distance
            * and terminate with result = false
            */
            if (pairDistance > maxDistance)
                return false;

            priQ.pop();

            /**
            * If the maximum distance between the nodes
            * is less than the maxDistance,
            * than all items in the nodes must be
            * closer than the max distance.
            * Then can terminate with result = true.
            *
            * NOTE: using Envelope MinMaxDistance
            * would provide a tighter bound,
            * but not much performance improvement has been observed
            */
            if (pair.maximumDistance() <= maxDistance)
                return true;
            /**
            * If the pair items are leaves
            * then their actual distance is an upper bound.
            * Update the distanceUpperBound to reflect this
            */
            if (pair.isLeaves()) {
                // assert: currentDistance < minimumDistanceFound
                distanceUpperBound = pairDistance;
                /**
                * If the items are closer than maxDistance
                * can terminate with result = true.
                */
                if (distanceUpperBound <= maxDistance)
                return true;
            }
            else {
                /**
                * Otherwise, expand one side of the pair,
                * and insert the expanded pairs into the queue.
                * The choice of which side to expand is determined heuristically.
                */
                expandToQueue(pair, priQ, distanceUpperBound);
            }
        }
        return false;

    }


    ItemDistance& m_id;
};
}
}
}
