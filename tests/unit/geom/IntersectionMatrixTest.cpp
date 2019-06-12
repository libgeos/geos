//
// Test Suite for geos::geom::IntersectionMatrix class.

#include <tut/tut.hpp>
// geos
#include <geos/geom/IntersectionMatrix.h>
#include <geos/geom/Dimension.h>
#include <geos/geom/Location.h>
// std
#include <memory>
#include <string>
#include <sstream>

namespace tut {
//
// Test Group
//

// Common data used by test cases
struct test_intersectionmatrix_data {
    typedef geos::geom::IntersectionMatrix* MatrixPtr;
    typedef geos::geom::IntersectionMatrix const* MatrixCPtr;

    const std::string pattern_false_;
    geos::geom::IntersectionMatrix im_;

    test_intersectionmatrix_data()
        : pattern_false_("FFFFFFFFF"), im_(pattern_false_)
    {}
private:
    // Declare type as noncopyable
    test_intersectionmatrix_data(const test_intersectionmatrix_data& other) = delete;
    test_intersectionmatrix_data& operator=(const test_intersectionmatrix_data& rhs) = delete;
};

typedef test_group<test_intersectionmatrix_data> group;
typedef group::object object;

group test_intersectionmatrix_group("geos::geom::IntersectionMatrix");

//
// Test Cases
//

// Test of default constructor
template<>
template<>
void object::test<1>
()
{
    ensure_equals(im_.toString(), pattern_false_);
}

// Test of user's constructor
template<>
template<>
void object::test<2>
()
{
    const std::string pattern("012*TF012");
    geos::geom::IntersectionMatrix im(pattern);

    ensure_equals(im.toString(), pattern);
}

// Test of copy constructor
template<>
template<>
void object::test<3>
()
{
    geos::geom::IntersectionMatrix copy(im_);

    ensure_equals(copy.toString(), im_.toString());
}

// Test nr 2 of copy constructor
template<>
template<>
void object::test<4>
()
{
    const std::string pattern("012*TF012");
    geos::geom::IntersectionMatrix examplar(pattern);

    ensure_equals(examplar.toString(), pattern);

    geos::geom::IntersectionMatrix copy(examplar);

    ensure_equals(copy.toString(), examplar.toString());
}

// Test of matches(int actualDimensionValue, char requiredDimensionSymbol)
template<>
template<>
void object::test<5>
()
{
    const int dummyAny = 999;
    ensure(im_.matches(dummyAny, '*'));

    ensure(im_.matches(geos::geom::Dimension::P, '0'));

    ensure(im_.matches(geos::geom::Dimension::True, 'T'));
    ensure(!im_.matches(geos::geom::Dimension::False, 'T'));
    ensure(!im_.matches(geos::geom::Dimension::DONTCARE, 'T'));
}

// Test of matches(const std::string& requiredDimensionSymbols)
template<>
template<>
void object::test<6>
()
{
    const std::string pattern("012*TF012");
    ensure(im_.matches(pattern_false_));
    ensure(!im_.matches(pattern));
}

// Test of matches(const std::string& actualDimensionSymbols,
//				   const std::string& requiredDimensionSymbols)
template<>
template<>
void object::test<7>
()
{
    ensure(im_.matches(pattern_false_, pattern_false_));
}

// Test of matches(const std::string& actualDimensionSymbols,
//				   const std::string& requiredDimensionSymbols)
template<>
template<>
void object::test<8>
()
{
    const std::string pattern("012*TF012");
    ensure(!im_.matches(pattern_false_, pattern));
}

// Test of matches(const std::string& actualDimensionSymbols,
//				   const std::string& requiredDimensionSymbols)
template<>
template<>
void object::test<9>
()
{
    const std::string pattern("012*TF012");
    ensure(im_.matches(pattern, pattern));
}

// add(IntersectionMatrix *im)
template<>
template<>
void object::test<10>
()
{
    // Create local matrix
    const std::string pattern("012*TF012");
    geos::geom::IntersectionMatrix im_local(pattern);

    ensure_equals(im_local.toString(), pattern);

    // Add local matrix to the matrix with 'FFFFFFFFF' pattern
    im_local.add(&im_);

    // Check pattern of local matrix after addition
    const std::string pattern_result("012FFF012");
    ensure_equals(im_local.toString(), pattern_result);
    ensure(im_local.matches(pattern_result));
}


// Test of set(int row, int column, int dimensionValue);
template<>
template<>
void object::test<11>
()
{
    using geos::geom::Location;

    // 'im_' is initialized with 'FFFFFFFFF'
    ensure_equals(im_.toString(), pattern_false_);

    // Do some changes and checks
    im_.set(Location::INTERIOR, Location::INTERIOR, geos::geom::Dimension::P);

    ensure_equals(im_.toString(), "0FFFFFFFF");

}

// Test of set(const std::string& dimensionSymbols);
template<>
template<>
void object::test<12>
()
{
    // 'im_' is initialized with 'FFFFFFFFF'
    ensure_equals(im_.toString(), pattern_false_);

    const std::string pattern("012*TF012");
    im_.set(pattern);

    ensure_equals(im_.toString(), pattern);
}

// Test of setAll(int dimensionValue);
template<>
template<>
void object::test<13>
()
{
    // 'im_' is initialized with 'FFFFFFFFF'
    ensure_equals(im_.toString(), pattern_false_);

    const std::string pattern("000000000");
    im_.setAll(geos::geom::Dimension::P);

    ensure_equals(im_.toString(), pattern);
}

// Test of setAtLeast(std::string minimumDimensionSymbols)
template<>
template<>
void object::test<14>
()
{
    // 'im_' is initialized with 'FFFFFFFFF'
    ensure_equals(im_.toString(), pattern_false_);

    // 1. No change expected
    im_.setAtLeast(pattern_false_);
    ensure_equals(im_.toString(), pattern_false_);

    // 2. Set new pattern
    const std::string pattern_new("FT*012FT*");
    const std::string pattern_expected("FFF012FFF");

    im_.setAtLeast(pattern_new);
    ensure(im_.toString() != pattern_false_);
    ensure_equals(im_.toString(), pattern_expected);

    // 3. Set old pattern, no change expected
    im_.setAtLeast(pattern_false_);
    ensure_equals(im_.toString(), pattern_expected);
}

// Test of setAtLeast(int row, int column, int minimumDimensionValue);
template<>
template<>
void object::test<15>
()
{
    using geos::geom::Location;

    // 'im_' is initialized with 'FFFFFFFFF'
    ensure_equals(im_.toString(), pattern_false_);

    // 1. No change expected
    im_.setAtLeast(Location::INTERIOR, Location::INTERIOR, geos::geom::Dimension::False);
    ensure_equals(im_.toString(), pattern_false_);

    // 2. Change is expected
    im_.setAtLeast(Location::INTERIOR, Location::INTERIOR, geos::geom::Dimension::A);
    ensure_equals(im_.toString(), "2FFFFFFFF");
}


// Test of setAtLeastIfValid(int row, int column, int minimumDimensionValue)
template<>
template<>
void object::test<16>
()
{
    using geos::geom::Location;

    // 'im_' is initialized with 'FFFFFFFFF'
    ensure_equals(im_.toString(), pattern_false_);

    // 1. No change expected
    im_.setAtLeast(Location::INTERIOR, Location::INTERIOR, geos::geom::Dimension::A);
    ensure(im_.toString() != pattern_false_);
    ensure_equals(im_.get(Location::INTERIOR, Location::INTERIOR), geos::geom::Dimension::A);
}

// Test of get(int row, int column)
template<>
template<>
void object::test<17>
()
{
    using geos::geom::Location;

    // Test on the original p0attern 'FFFFFFFFF' of the 'im_' object
    ensure_equals(im_.toString(), pattern_false_);
    ensure_equals(im_.get(Location::INTERIOR, Location::INTERIOR), geos::geom::Dimension::False);

    // Change first value and test again
    im_.setAtLeast(Location::INTERIOR, Location::INTERIOR, geos::geom::Dimension::A);
    ensure_equals(im_.get(Location::INTERIOR, Location::INTERIOR), geos::geom::Dimension::A);

    // Change last value and test again
    im_.setAtLeast(Location::EXTERIOR, Location::EXTERIOR, geos::geom::Dimension::L);
    ensure_equals(im_.get(Location::EXTERIOR, Location::EXTERIOR), geos::geom::Dimension::L);
}


// Test of isContains()
template<>
template<>
void object::test<18>
()
{
    // Test on the FALSE matrix
    ensure_equals(im_.toString(),  pattern_false_);
    ensure(!im_.isContains());

    // 1. Test 'minimum' version of 'contains' matrix
    const std::string pattern_minimum("T*****FF*");
    geos::geom::IntersectionMatrix minimum(pattern_minimum);
    ensure_equals(minimum.toString(), pattern_minimum);
    ensure(minimum.isContains());

    // 2. Test other version of  valid 'contains' matrix
    const std::string pattern_contains("T00000FF0");
    geos::geom::IntersectionMatrix contains(pattern_contains);
    ensure_equals(contains.toString(),  pattern_contains);
    ensure(contains.isContains());

    // 3. Test 'not-contains' version of 'contains' matrix
    const std::string pattern_notcontains("T0F00F000");
    geos::geom::IntersectionMatrix notcontains(pattern_notcontains);
    ensure_equals(notcontains.toString(),  pattern_notcontains);
    ensure(!notcontains.isContains());
}

// Test of isCrosses(int dimensionOfGeometryA, int dimensionOfGeometryB)
template<>
template<>
void object::test<19>
()
{
    using geos::geom::Dimension;

    // Test on the FALSE matrix
    ensure_equals(im_.toString(),  pattern_false_);
    ensure(!im_.isCrosses(Dimension::L, Dimension::L));

    // P/L: dim(I(a) * I(b)) AND dim(I(a) * E(b))
    im_.set("TFTFFFFFF");
    ensure(im_.isCrosses(Dimension::P, Dimension::L));

    // L/P: dim(I(a) * I(b)) AND dim(I(a) * E(b))
    im_.set("TFTFFFFFF");
    ensure(!im_.isCrosses(Dimension::L, Dimension::P));

    // P/L: dim(I(a) * I(b)) AND dim(E(a) * I(b))
    im_.set("TFFFFFTFF");
    ensure(!im_.isCrosses(Dimension::P, Dimension::L));

    // L/P: dim(I(a) * I(b)) AND dim(E(a) * I(b))
    im_.set("TFFFFFTFF");
    ensure(im_.isCrosses(Dimension::L, Dimension::P));

    // L/L: dim(I(a) * I(b)) == 0
    im_.set("0FFFFFFFF");
    ensure(im_.isCrosses(Dimension::L, Dimension::L));

    // L/L: dim(I(a) * I(b)) == 1
    im_.set("1FFFFFFFF");
    ensure(!im_.isCrosses(Dimension::L, Dimension::L));
}

// Test of isDisjoint()
template<>
template<>
void object::test<20>
()
{
    // Test on the FALSE matrix
    ensure_equals(im_.toString(),  pattern_false_);
    ensure(im_.isDisjoint());

    im_.set("FF*FF****");
    ensure(im_.isDisjoint());

    im_.set("FF1FF2T*0");
    ensure(im_.isDisjoint());

    im_.set("*F*FF****");
    ensure(!im_.isDisjoint());
}

// Test of isEquals(int dimensionOfGeometryA, int dimensionOfGeometryB)
template<>
template<>
void object::test<21>
()
{
    using geos::geom::Dimension;

    // Test on the FALSE matrix
    ensure_equals(im_.toString(),  pattern_false_);
    ensure(!im_.isEquals(Dimension::L, Dimension::L));

    // Test geometries with all dimensions
    im_.set("0FFFFFFF2");
    ensure(im_.isEquals(Dimension::P, Dimension::P));
    im_.set("1FFF0FFF2");
    ensure(im_.isEquals(Dimension::L, Dimension::L));
    im_.set("2FFF1FFF2");
    ensure(im_.isEquals(Dimension::A, Dimension::A));
    im_.set("0F0FFFFF2");
    ensure(!im_.isEquals(Dimension::P, Dimension::P));
    im_.set("1FFF1FFF2");
    ensure(im_.isEquals(Dimension::L, Dimension::L));
    im_.set("2FFF1*FF2");
    ensure(!im_.isEquals(Dimension::A, Dimension::A));
    im_.set("0FFFFFFF2");
    ensure(!im_.isEquals(Dimension::P, Dimension::L));
    im_.set("1FFF0FFF2");
    ensure(!im_.isEquals(Dimension::L, Dimension::A));
    im_.set("2FFF1FFF2");
    ensure(!im_.isEquals(Dimension::A, Dimension::P));
}

// Test of isIntersects()
template<>
template<>
void object::test<22>
()
{
    // Test on the FALSE matrix
    ensure_equals(im_.toString(),  pattern_false_);
    ensure(!im_.isIntersects());

    im_.set("FF*FF****");
    ensure(!im_.isIntersects());
    im_.set("FF1FF2T*0");
    ensure(!im_.isIntersects());
    im_.set("*F*FF****");
    ensure(im_.isIntersects());
}

// Test of isOverlaps(int dimensionOfGeometryA, int dimensionOfGeometryB)
template<>
template<>
void object::test<23>
()
{
    using geos::geom::Dimension;

    // Test on the FALSE matrix
    ensure_equals(im_.toString(),  pattern_false_);
    ensure(!im_.isEquals(Dimension::L, Dimension::L));

    im_.set("2*2***2**");
    ensure(im_.isOverlaps(Dimension::P, Dimension::P));
    im_.set("2*2***2**");
    ensure(im_.isOverlaps(Dimension::A, Dimension::A));
    im_.set("2*2***2**");
    ensure(!im_.isOverlaps(Dimension::P, Dimension::A));
    im_.set("2*2***2**");
    ensure(!im_.isOverlaps(Dimension::L, Dimension::L));
    im_.set("1*2***2**");
    ensure(im_.isOverlaps(Dimension::L, Dimension::L));
    im_.set("0FFFFFFF2");
    ensure(!im_.isOverlaps(Dimension::P, Dimension::P));
    im_.set("1FFF0FFF2");
    ensure(!im_.isOverlaps(Dimension::L, Dimension::L));
    im_.set("2FFF1FFF2");
    ensure(!im_.isOverlaps(Dimension::A, Dimension::A));
}

// Test of isTouches(int dimensionOfGeometryA, int dimensionOfGeometryB)
template<>
template<>
void object::test<24>
()
{
    using geos::geom::Dimension;

    // Test on the FALSE matrix
    ensure_equals(im_.toString(),  pattern_false_);
    ensure(!im_.isTouches(Dimension::P, Dimension::P));

    im_.set("FT*******");
    ensure(im_.isTouches(Dimension::P, Dimension::A));
    im_.set("FT*******");
    ensure(im_.isTouches(Dimension::A, Dimension::P));
    im_.set("FT*******");
    ensure(!im_.isTouches(Dimension::P, Dimension::P));
}

// Test of isWithin()
template<>
template<>
void object::test<25>
()
{
    // Test on the FALSE matrix
    ensure_equals(im_.toString(),  pattern_false_);
    ensure(!im_.isWithin());

    im_.set("T0F00F000");
    ensure(im_.isWithin());
    im_.set("T00000FF0");
    ensure(!im_.isWithin());
}


// Test of IntersectionMatrix* transpose()
template<>
template<>
void object::test<26>
()
{
    using geos::geom::IntersectionMatrix;

    IntersectionMatrix input("012*TF012");
    IntersectionMatrix transposed(input);

    // transpose() returns 'this' pointer to transposed object
    MatrixCPtr output = transposed.transpose();
    ensure(output != nullptr);

    // TODO - mloskot - waiting for equality operators
    //ensure_equals( (*output), input );

    ensure_equals("0*01T12F2", transposed.toString());

    ensure_equals("012*TF012", input.toString());
}

// Test of toString() and output operator
template<>
template<>
void object::test<27>
()
{
    ensure(im_.toString() != "0*01T12F2");
    ensure_equals("FFFFFFFFF", im_.toString());

    std::stringstream ss;
    ss << im_;

    ensure(ss.str() != "0*01T12F2");
    ensure_equals("FFFFFFFFF", ss.str());
}

// Test of isCovers()
template<>
template<>
void object::test<28>
()
{
    using geos::geom::IntersectionMatrix;

    ensure(IntersectionMatrix("T*****FF*").isCovers());
    ensure(IntersectionMatrix("*T****FF*").isCovers());
    ensure(IntersectionMatrix("***T**FF*").isCovers());
    ensure(IntersectionMatrix("****T*FF*").isCovers());
}

// Test of isCoveredBy()
template<>
template<>
void object::test<29>
()
{
    using geos::geom::IntersectionMatrix;

    ensure(IntersectionMatrix("T*F**F***").isCoveredBy());
    ensure(IntersectionMatrix("*TF**F***").isCoveredBy());
    ensure(IntersectionMatrix("**FT*F***").isCoveredBy());
    ensure(IntersectionMatrix("**F*TF***").isCoveredBy());
}

} // namespace tut

