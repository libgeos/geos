/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/export.h>

#include <geos/geom/Coordinate.h> // for applyCoordinateFilter
#include <geos/geom/CoordinateSequenceIterator.h>

#include <vector>
#include <iosfwd> // ostream
#include <memory> // for unique_ptr typedef

// Forward declarations
namespace geos {
namespace geom {
class Envelope;
class CoordinateFilter;
}
}

namespace geos {
namespace geom { // geos::geom

/**
 * \class CoordinateSequence geom.h geos.h
 *
 * \brief
 * The internal representation of a list of coordinates inside a Geometry.
 *
 */
class GEOS_DLL CoordinateSequence {

public:

    using iterator = CoordinateSequenceIterator<CoordinateSequence, Coordinate>;
    using const_iterator = CoordinateSequenceIterator<const CoordinateSequence, const Coordinate>;

    typedef std::unique_ptr<CoordinateSequence> Ptr;

    CoordinateSequence() : dimension(0) {}

    //CoordinateSequence(const std::vector<Coordinate>& coords, std::size_t dim);

    CoordinateSequence(std::vector<Coordinate>&& coords, std::size_t dim = 0);

    CoordinateSequence(std::vector<CoordinateXY>&& coords, std::size_t dim = 0);

    CoordinateSequence(std::unique_ptr<std::vector<Coordinate>>&& coords, std::size_t dim = 0);

    CoordinateSequence(std::size_t size, std::size_t dim = 0);

    ~CoordinateSequence() = default;

    /** \brief
     * Returns a deep copy of this collection.
     */
    std::unique_ptr<CoordinateSequence> clone() const;

    /** \brief
     * Returns a read-only reference to Coordinate at position i.
     */
    const Coordinate& getAt(std::size_t i) const {
        return vect[i];
    }

    Coordinate& getAt(std::size_t i) {
        return vect[i];
    }

    /// Return last Coordinate in the sequence
    const Coordinate& back() const
    {
        return getAt(size() - 1);
    }

    /// Return first Coordinate in the sequence
    const Coordinate& front() const
    {
        return getAt(0);
    }

    const Coordinate& operator[](std::size_t i) const
    {
        return getAt(i);
    }

    Coordinate&
    operator[](std::size_t i)
    {
        return getAt(i);
    }

    Envelope getEnvelope() const;

    /** \brief
     * Write Coordinate at position i to given Coordinate.
     */
    void getAt(std::size_t i, Coordinate& c) const {
        c = getAt(i);
    }

    /** \brief
     * Returns the number of Coordinates (actual or otherwise, as
     * this implementation may not store its data in Coordinate objects).
     */
    std::size_t getSize() const {
        return size();
    }

    size_t size() const
    {
        return vect.size();
    }

    /// Pushes all Coordinates of this sequence into the provided vector.
    ///
    /// This method is a port of the toCoordinateArray() method of JTS.
    ///
    void toVector(std::vector<Coordinate>& coords) const;

    void toVector(std::vector<CoordinateXY>& coords) const;

    /// Returns <code>true</code> if list contains no coordinates.
    bool isEmpty() const {
        return vect.empty();
    }

    /// Add a Coordinate to the list
    void add(const Coordinate& c) {
        vect.push_back(c);
    }

    /**
     * \brief Add a coordinate
     * @param c the coordinate to add
     * @param allowRepeated if set to false, repeated coordinates
     *                      are collapsed
     */
    void add(const Coordinate& c, bool allowRepeated);

    /** \brief
     * Inserts the specified coordinate at the specified position in
     * this list.
     *
     * @param i the position at which to insert
     * @param coord the coordinate to insert
     * @param allowRepeated if set to false, repeated coordinates are
     *                      collapsed
     *
     * @note this is a CoordinateList interface in JTS
     */
    void add(std::size_t i, const Coordinate& coord, bool allowRepeated);

    void add(const CoordinateSequence* cl, bool allowRepeated, bool direction);

    /// Copy Coordinate c to position pos
    void setAt(const Coordinate& c, std::size_t pos) {
        vect[pos]= c;
    }

    void setAt(const CoordinateXY& c, std::size_t pos) {
        setAt(Coordinate(c), pos);
    }

    /// Get a string representation of CoordinateSequence
    std::string toString() const;

    /// Substitute Coordinate list with a copy of the given vector
    void setPoints(const std::vector<Coordinate>& v);

    /// Returns true if contains any two consecutive points
    bool hasRepeatedPoints() const;

    /// Returns lower-left Coordinate in list
    const Coordinate* minCoordinate() const;

    /** \brief
     *  Returns true if given CoordinateSequence contains
     *  any two consecutive Coordinate
     */
    static bool hasRepeatedPoints(const CoordinateSequence* cl);

    /** \brief
     *  Returns either the given CoordinateSequence if its length
     *  is greater than the given amount, or an empty CoordinateSequence.
     */
    static CoordinateSequence* atLeastNCoordinatesOrNothing(std::size_t n,
            CoordinateSequence* c);

    /// Return position of a Coordinate
    //
    /// or numeric_limits<std::size_t>::max() if not found
    ///
    static std::size_t indexOf(const Coordinate* coordinate,
                               const CoordinateSequence* cl);

    /**
     * \brief
     * Returns true if the two arrays are identical, both null,
     * or pointwise equal
     */
    static bool equals(const CoordinateSequence* cl1,
                       const CoordinateSequence* cl2);

    /// Scroll given CoordinateSequence so to start with given Coordinate.
    static void scroll(CoordinateSequence* cl, const Coordinate* firstCoordinate);

    /** \brief
     * Determines which orientation of the {@link Coordinate} array
     * is (overall) increasing.
     *
     * In other words, determines which end of the array is "smaller"
     * (using the standard ordering on {@link Coordinate}).
     * Returns an integer indicating the increasing direction.
     * If the sequence is a palindrome, it is defined to be
     * oriented in a positive direction.
     *
     * @param pts the array of Coordinates to test
     * @return <code>1</code> if the array is smaller at the start
     * or is a palindrome,
     * <code>-1</code> if smaller at the end
     *
     * NOTE: this method is found in CoordinateArrays class for JTS
     */
    static int increasingDirection(const CoordinateSequence& pts);


    /** \brief
    * Tests whether an a {@link CoordinateSequence} forms a ring,
    * by checking length and closure. Self-intersection is not checked.
    *
    * @return true if the coordinate form a ring.
    */
    bool isRing() const;

    /// Reverse Coordinate order in given CoordinateSequence
    static void reverse(CoordinateSequence* cl);

    /// Standard ordinate index values
    enum { X, Y, Z, M };

    /**
     * Returns the dimension (number of ordinates in each coordinate)
     * for this sequence.
     *
     * @return the dimension of the sequence.
     */
    std::size_t getDimension() const;

    bool hasZ() const {
        return getDimension() > 2;
    }

    /**
     * Returns the ordinate of a coordinate in this sequence.
     * Ordinate indices 0 and 1 are assumed to be X and Y.
     * Ordinates indices greater than 1 have user-defined semantics
     * (for instance, they may contain other dimensions or measure values).
     *
     * @param index  the coordinate index in the sequence
     * @param ordinateIndex the ordinate index in the coordinate
     *                      (in range [0, dimension-1])
     */
    double getOrdinate(std::size_t index, std::size_t ordinateIndex) const;

    /**
     * Returns ordinate X (0) of the specified coordinate.
     *
     * @param index
     * @return the value of the X ordinate in the index'th coordinate
     */
    double getX(std::size_t index) const
    {
        return getOrdinate(index, X);
    }

    /**
     * Returns ordinate Y (1) of the specified coordinate.
     *
     * @param index
     * @return the value of the Y ordinate in the index'th coordinate
     */
    double getY(std::size_t index) const
    {
        return getOrdinate(index, Y);
    }


    /**
     * Sets the value for a given ordinate of a coordinate in this sequence.
     *
     * @param index  the coordinate index in the sequence
     * @param ordinateIndex the ordinate index in the coordinate
     *                      (in range [0, dimension-1])
     * @param value  the new ordinate value
     */
    void setOrdinate(std::size_t index, std::size_t ordinateIndex, double value);

    /**
     * Expands the given Envelope to include the coordinates in the
     * sequence.
     * @param env the envelope to expand
     */
    void expandEnvelope(Envelope& env) const;

    void apply_rw(const CoordinateFilter* filter);
    void apply_ro(CoordinateFilter* filter) const;

    void clear();

    void closeRing();

    /** \brief
     * Apply a filter to each Coordinate of this sequence.
     * The filter is expected to provide a .filter(Coordinate&)
     * method.
     *
     * TODO: accept a Functor instead, will be more flexible.
     *       actually, define iterators on Geometry
     */
    template <class T>
    void
    applyCoordinateFilter(T& f)
    {
        Coordinate c;
        for(std::size_t i = 0, n = size(); i < n; ++i) {
            getAt(i, c);
            f.filter(c);
            setAt(c, i);
        }
    }

    iterator begin();

    iterator end();

    const_iterator cbegin() const;

    const_iterator cend() const;
private:
    std::vector<Coordinate> vect;
    mutable std::size_t dimension;
};

GEOS_DLL std::ostream& operator<< (std::ostream& os, const CoordinateSequence& cs);

GEOS_DLL bool operator== (const CoordinateSequence& s1, const CoordinateSequence& s2);

GEOS_DLL bool operator!= (const CoordinateSequence& s1, const CoordinateSequence& s2);

} // namespace geos::geom
} // namespace geos

