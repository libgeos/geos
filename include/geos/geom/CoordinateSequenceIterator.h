/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2022 ISciences, LLC
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/


#pragma once

#include <cstddef>
#include <iterator>

namespace geos {
namespace geom {

template<typename SequenceType, typename CoordinateType>
class CoordinateSequenceIterator {

public:
    using iterator_category = std::random_access_iterator_tag;
    using value_type = CoordinateType;
    using reference = CoordinateType&;
    using pointer = CoordinateType;
    using difference_type = std::ptrdiff_t;

private:
    SequenceType* m_seq;
    difference_type m_pos;

public:
    CoordinateSequenceIterator(SequenceType* seq) : m_seq(seq), m_pos(0) {}

    CoordinateSequenceIterator(SequenceType* seq, std::size_t size) : m_seq(seq), m_pos(static_cast<difference_type>(size)) {}

    reference operator*() const {
        return m_seq->template getAt<value_type>(static_cast<std::size_t>(m_pos));
    }

    pointer operator->() const {
        return &m_seq->template getAt<value_type>(static_cast<std::size_t>(m_pos));
    }

    CoordinateSequenceIterator& operator++() {
        m_pos++;
        return *this;
    }

    CoordinateSequenceIterator operator++(int) {
        CoordinateSequenceIterator ret = *this;
        m_pos++;
        return ret;
    }

    CoordinateSequenceIterator& operator--() {
        m_pos--;
        return *this;
    }

    CoordinateSequenceIterator operator--(int) {
        CoordinateSequenceIterator ret = *this;
        m_pos--;
        return ret;
    }

    difference_type operator-(const CoordinateSequenceIterator& other) const {
        return this->m_pos - other.m_pos;
    }

    CoordinateSequenceIterator operator+(difference_type n) const {
        return CoordinateSequenceIterator(m_seq, static_cast<std::size_t>(m_pos + n));
    }

    CoordinateSequenceIterator operator+=(difference_type n) {
        this->m_pos += n;
        return *this;
    }

    CoordinateSequenceIterator operator-(difference_type n) const {
        return CoordinateSequenceIterator(m_seq, static_cast<std::size_t>(m_pos - n));
    }

    CoordinateSequenceIterator operator-=(difference_type n) {
        this->m_pos -= n;
        return *this;
    }

    CoordinateType& operator[](difference_type n) const {
        return *(*this + n);
    }

    bool operator==(const CoordinateSequenceIterator& other) const {
        return this->m_pos == other.m_pos;
    }

    bool operator!=(const CoordinateSequenceIterator& other) const {
        return !(*this == other);
    }

    bool operator<(const CoordinateSequenceIterator& other) const {
        return this->m_pos < other.m_pos;
    }

    bool operator<=(const CoordinateSequenceIterator& other) const {
        return this->m_pos <= other.m_pos;
    }

    bool operator>(const CoordinateSequenceIterator& other) const {
        return this->m_pos > other.m_pos;
    }

    bool operator>=(const CoordinateSequenceIterator& other) const {
        return this->m_pos >= other.m_pos;
    }

};

}
}
