/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2018-2025 ISciences, LLC
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <cstring>
#include <iomanip>
#include <memory>
#include <string>
#include <vector>

namespace geos::operation::grid {

template<typename T>
class Matrix
{

  public:
    using value_type = T;

    Matrix(size_t rows, size_t cols, std::shared_ptr<T[]> data)
        : m_data{ data }
        , m_rows{ rows }
        , m_cols{ cols }
    {}

    Matrix(size_t rows, size_t cols)
      : m_rows{ rows }
      , m_cols{ cols }
    {
        if (m_rows > 0 && m_cols > 0) {
            // new T[]() initializes to zero
            m_data = std::shared_ptr<T[]>(new T[m_rows * m_cols]());
        }
    }

    Matrix(size_t rows, size_t cols, T value)
      : m_rows{ rows }
      , m_cols{ cols }
    {
        if (m_rows > 0 && m_cols > 0) {
            // new T[] does not initialize
            m_data = std::shared_ptr<T[]>(new T[m_rows * m_cols]);
        }

        std::fill(m_data.get(), m_data.get() + m_rows * m_cols, value);
    }

    explicit Matrix(const std::vector<std::vector<T>>& data)
      : m_rows{ data.size() }
      , m_cols{ data[0].size() }
    {
        m_data = std::shared_ptr<T[]>(new T[m_rows * m_cols]());

        auto lastpos = m_data.get();
        for (auto& row : data) {
            lastpos = std::copy(row.begin(), row.end(), lastpos);
        }
    }

    Matrix(Matrix<T>&& m) noexcept
      : m_rows{ m.getNumRows() }
      , m_cols{ m.getNumCols() }
    {
        m_data = std::move(m.m_data);
    }

    T& operator()(size_t row, size_t col)
    {
        check(row, col);
        return m_data.get()[row * m_cols + col];
    }

    const T& operator()(size_t row, size_t col) const
    {
        check(row, col);
        return m_data.get()[row * m_cols + col];
    }

    bool operator!=(const Matrix<T>& other) const {
        return !(*this == other);
    }

    bool operator==(const Matrix<T>& other) const
    {
        if (m_rows != other.m_rows) {
            return false;
        }
        if (m_cols != other.m_cols) {
            return false;
        }

        return 0 == memcmp(m_data.get(), other.m_data.get(), m_rows * m_cols * sizeof(T));
    }

    void increment(size_t row, size_t col, const T& val)
    {
        check(row, col);
        m_data.get()[row * m_cols + col] += val;
    }

    size_t getNumRows() const { return m_rows; }
    size_t getNumCols() const { return m_cols; }

    T* row(size_t row)
    {
        return &(m_data[row * m_cols]);
    }

    T* data()
    {
        return m_data.get();
    }

    const T* data() const
    {
        return m_data.get();
    }

    T* release()
    {
        return m_data.release();
    }

    const T* begin() const {
        return m_data.get();
    }

    const T* end() const {
        return m_data.get() + m_rows * m_cols;
    }

#ifdef MATRIX_CHECK_BOUNDS
    void check(size_t row, size_t col) const
    {
        if (row + 1 > m_rows) {
            throw std::out_of_range("Row " + std::to_string(row) + " is out of range.");
        }
        if (col + 1 > m_cols) {
            throw std::out_of_range("Col " + std::to_string(col) + " is out of range.");
        }
    }
#else
    void check(size_t, size_t) const
    {
    }
#endif

  private:
    std::shared_ptr<T[]> m_data;

    size_t m_rows;
    size_t m_cols;
};

template<typename T>
std::ostream&
operator<<(std::ostream& os, const Matrix<T>& m)
{
    for (size_t i = 0; i < m.getNumRows(); i++) {
        for (size_t j = 0; j < m.getNumCols(); j++) {
            if (m(i, j) != 0) {
                os << std::right << std::fixed << std::setw(10) << std::setprecision(6) << m(i, j) << " ";
            } else {
                os << "           ";
            }
        }
        os << std::endl;
    }

    return os;
}

}
