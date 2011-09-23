/**********************************************************************
 * $Id: CoordinateArraySequence.h 3255 2011-03-01 17:56:10Z mloskot $
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

#ifndef GEOS_GEOM_COORDINATEARRAYSEQUENCE_H
#define GEOS_GEOM_COORDINATEARRAYSEQUENCE_H

#include <geos/export.h>
#include <vector>

#include <geos/geom/CoordinateSequence.h>

#include <geos/inline.h>

// Forward declarations
namespace geos {
	namespace geom { 
		class Coordinate;
	}
}


namespace geos {
namespace geom { // geos.geom

/// The default implementation of CoordinateSequence
class GEOS_DLL CoordinateArraySequence : public CoordinateSequence {
public:

    CoordinateArraySequence(const CoordinateArraySequence &cl);

	CoordinateSequence *clone() const;

	//const Coordinate& getCoordinate(int pos) const;
	const Coordinate& getAt(std::size_t pos) const;

	/// Copy Coordinate at position i to Coordinate c
	virtual void getAt(std::size_t i, Coordinate& c) const;

	//int size() const;
	size_t getSize() const;

	// @deprecated
	const std::vector<Coordinate>* toVector() const;

	// See dox in CoordinateSequence.h
	void toVector(std::vector<Coordinate>&) const;

	/// Construct an empty sequence
	CoordinateArraySequence();

	/// Construct sequence taking ownership of given Coordinate vector
	CoordinateArraySequence(std::vector<Coordinate> *coords,
                                std::size_t dimension = 0);
        
	/// Construct sequence allocating space for n coordinates
	CoordinateArraySequence(std::size_t n, std::size_t dimension = 0);

	~CoordinateArraySequence();

	bool isEmpty() const { return empty(); }

	bool empty() const { return vect->empty(); }

	/// Reset this CoordinateArraySequence to the empty state
	void clear() { vect->clear(); }

	void add(const Coordinate& c);

	virtual void add(const Coordinate& c, bool allowRepeated);

	/** \brief
	 * Inserts the specified coordinate at the specified position in
	 * this list.
	 *
	 * @param i the position at which to insert
	 * @param coord the coordinate to insert
	 * @param allowRepeated if set to false, repeated coordinates are
	 *                      collapsed
	 *
	 * NOTE: this is a CoordinateList interface in JTS
	 */
	virtual void add(std::size_t i, const Coordinate& coord, bool allowRepeated);

	void setAt(const Coordinate& c, std::size_t pos);

	void deleteAt(std::size_t pos);

	std::string toString() const;

	void setPoints(const std::vector<Coordinate> &v);

	double getOrdinate(std::size_t index,
			size_t ordinateIndex) const;

	void setOrdinate(std::size_t index, std::size_t ordinateIndex,
			double value);

	void expandEnvelope(Envelope &env) const;

    std::size_t getDimension() const;

	void apply_rw(const CoordinateFilter *filter); 

	void apply_ro(CoordinateFilter *filter) const; 

	virtual CoordinateSequence& removeRepeatedPoints();

private:
	std::vector<Coordinate> *vect;
    mutable std::size_t dimension;
};

/// This is for backward API compatibility
typedef CoordinateArraySequence DefaultCoordinateSequence;

} // namespace geos.geom
} // namespace geos

//#ifdef GEOS_INLINE
//# include "geos/geom/CoordinateArraySequence.inl"
//#endif

#endif // ndef GEOS_GEOM_COORDINATEARRAYSEQUENCE_H

/**********************************************************************
 * $Log$
 * Revision 1.4  2006/06/12 10:10:39  strk
 * Fixed getGeometryN() to take size_t rather then int, changed unsigned int parameters to size_t.
 *
 * Revision 1.3  2006/05/03 08:58:34  strk
 * added new non-static CoordinateSequence::removeRepeatedPoints() mutator.
 *
 * Revision 1.2  2006/03/24 09:52:41  strk
 * USE_INLINE => GEOS_INLINE
 *
 * Revision 1.1  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/
