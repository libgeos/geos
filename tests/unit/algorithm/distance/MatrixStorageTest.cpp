//
// Test Suite for geos::algorithm::distance::DiscreteFrechetDistance::MatrixStorage
//

#include <tut/tut.hpp>
// geos
//#include <geos/constants.h>
#include <geos/algorithm/distance/DiscreteFrechetDistance.h>
#include <geos/util.h>

// std
// #include <cmath>
// #include <string>
// #include <memory>

using geos::algorithm::distance::DiscreteFrechetDistance;


namespace tut {
//
// Test Group
//

// Test data, not used
struct test_matrixstorage_data {

    test_matrixstorage_data() {};

    void
    runOrderedTest(DiscreteFrechetDistance::MatrixStorage& mat)
    {
        mat.set(0, 0, 10);
        mat.set(0, 1, 20);
        mat.set(1, 1, 30);
        mat.set(1, 3, 40);
        mat.set(2, 2, 50);
        mat.set(2, 3, 60);
        mat.set(2, 4, 70);
        mat.set(3, 5, 80);

        ensure_equals("ordered 10", mat.get(0, 0), 10);
        ensure_equals("ordered 20", mat.get(0, 1), 20);
        ensure_equals("ordered 30", mat.get(1, 1), 30);
        ensure_equals("ordered 40", mat.get(1, 3), 40);
        ensure_equals("ordered 50", mat.get(2, 2), 50);
        ensure_equals("ordered 60", mat.get(2, 3), 60);
        ensure_equals("ordered 70", mat.get(2, 4), 70);
        ensure_equals("ordered 80", mat.get(3, 5), 80);
    }

    void
    runUnorderedTest(DiscreteFrechetDistance::MatrixStorage& mat)
    {
        mat.set(0, 0, 10);
        mat.set(3, 5, 80);
        mat.set(0, 1, 20);
        mat.set(2, 4, 70);
        mat.set(1, 1, 30);
        mat.set(2, 3, 60);
        mat.set(2, 2, 50);
        mat.set(1, 3, 40);

        ensure_equals("unordered 10", mat.get(0, 0), 10);
        ensure_equals("unordered 20", mat.get(0, 1), 20);
        ensure_equals("unordered 30", mat.get(1, 1), 30);
        ensure_equals("unordered 40", mat.get(1, 3), 40);
        ensure_equals("unordered 50", mat.get(2, 2), 50);
        ensure_equals("unordered 60", mat.get(2, 3), 60);
        ensure_equals("unordered 70", mat.get(2, 4), 70);
        ensure_equals("unordered 80", mat.get(3, 5), 80);
    }

};

typedef test_group<test_matrixstorage_data> group;
typedef group::object object;

group test_matrixstorage_group("geos::algorithm::distance::MatrixStorage");

//
// Test Cases
//

// testCsrMatrix
template<>
template<>
void object::test<1>()
{
    std::unique_ptr<DiscreteFrechetDistance::MatrixStorage> mat;
    mat = std::make_unique<DiscreteFrechetDistance::CsrMatrix>(4, 6, 0.0, 8);
    runOrderedTest(*mat);
    mat = std::make_unique<DiscreteFrechetDistance::CsrMatrix>(4, 6, 0.0, 8);
    runUnorderedTest(*mat);
}

// testHashMapMatrix
template<>
template<>
void object::test<2>()
{
    std::unique_ptr<DiscreteFrechetDistance::MatrixStorage> mat;
    mat = std::make_unique<DiscreteFrechetDistance::HashMapMatrix>(4, 6, 0.0);
    runOrderedTest(*mat);
    mat = std::make_unique<DiscreteFrechetDistance::HashMapMatrix>(4, 6, 0.0);
    runUnorderedTest(*mat);
}

// testRectMatrix
template<>
template<>
void object::test<3>()
{
    std::unique_ptr<DiscreteFrechetDistance::MatrixStorage> mat;
    mat = std::make_unique<DiscreteFrechetDistance::RectMatrix>(4, 6, 0.0);
    runOrderedTest(*mat);
    mat = std::make_unique<DiscreteFrechetDistance::RectMatrix>(4, 6, 0.0);
    runUnorderedTest(*mat);
}


} // namespace tut
