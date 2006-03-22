/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#ifndef GEOS_IDX_BINTREE_INTERVAL_H
#define GEOS_IDX_BINTREE_INTERVAL_H

namespace geos {
namespace index { // geos::index
namespace bintree { // geos::index::bintree

/// Represents an (1-dimensional) closed interval on the Real number line.
class Interval {

public:

	double min, max;

	Interval();

	~Interval();

	Interval(double nmin, double nmax);

	Interval(Interval *interval);

	void init(double nmin, double nmax);

	double getMin();

	double getMax();

	double getWidth();

	void expandToInclude(Interval *interval);

	bool overlaps(Interval *interval);

	bool overlaps(double nmin, double nmax);

	bool contains(Interval *interval);

	bool contains(double nmin, double nmax);

	bool contains(double p);
};

} // namespace geos::index::bintree
} // namespace geos::index
} // namespace geos

#endif // GEOS_IDX_BINTREE_INTERVAL_H

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/03/22 16:01:33  strk
 * indexBintree.h header split, classes renamed to match JTS
 *
 **********************************************************************/

