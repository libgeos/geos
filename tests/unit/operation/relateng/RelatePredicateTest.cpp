//
// Test Suite for geos::operation::relateng::LinearBoundary class.

#include <tut/tut.hpp>
#include <utility.h>

// geos
#include <geos/geom/Location.h>
#include <geos/geom/Dimension.h>
#include <geos/operation/relateng/TopologyPredicate.h>
#include <geos/operation/relateng/RelatePredicate.h>

// std
#include <memory>


using namespace geos::geom;
using namespace geos::operation::relateng;


namespace tut {
//
// Test Group
//

// Common data used by all tests
struct test_relatepredicate_data {

    void
    checkPredicate(TopologyPredicate& pred, const std::string& im, bool expected)
    {
        applyIM(im, pred);
        checkPred(pred, expected);
    }

    void
    checkPredicatePartial(TopologyPredicate& pred, const std::string& im, bool expected)
    {
        applyIM(im, pred);
        bool isKnown = pred.isKnown();
        ensure("predicate value is not known", isKnown);
        checkPred(pred, expected);
    }

    void
    checkPred(TopologyPredicate& pred, bool expected)
    {
        pred.finish();
        bool actual = pred.value();
        ensure_equals("checkPred", actual, expected);
    }

    static void
    applyIM(const std::string& imIn, TopologyPredicate& pred)
    {
        std::array<Location,3> locs = {
            Location::INTERIOR, Location::BOUNDARY, Location::EXTERIOR };

        const std::string& im = cleanIM(imIn);
        uint32_t i = 0;
        for (Location locA : locs) {
            for (Location locB : locs) {
                char entry = im[i++];
                if (entry == '0' || entry == '1' || entry == '2') {
                    int dim = Dimension::toDimensionValue(entry);
                    pred.updateDimension(locA, locB, dim);
                }
            }
        }
    }

    static std::string
    cleanIM(const std::string& im)
    {
        std::string str = im;
        str.erase(std::remove(str.begin(), str.end(), '.'), str.end());
        return str;
    }

    const std::string A_EXT_B_INT = "***.***.1**";
    const std::string A_INT_B_INT = "1**.***.***";
};


typedef test_group<test_relatepredicate_data> group;
typedef group::object object;

group test_relatepredicate_group("geos::operation::relateng::RelatePredicate");

//
// Test Cases
//

// testIntersects
template<>
template<>
void object::test<1> ()
{
    checkPredicate(*RelatePredicate::intersects(), A_INT_B_INT, true);
}

// testDisjoint
template<>
template<>
void object::test<2> ()
{
    checkPredicate(*RelatePredicate::intersects(), A_EXT_B_INT, false);
    checkPredicate(*RelatePredicate::disjoint(), A_EXT_B_INT, true);
}

// testCovers
template<>
template<>
void object::test<3> ()
{
    checkPredicate(*RelatePredicate::covers(), A_INT_B_INT, true);
    checkPredicate(*RelatePredicate::covers(), A_EXT_B_INT, false);
}

// testCoversFast
template<>
template<>
void object::test<4> ()
{
    checkPredicatePartial(*RelatePredicate::covers(), A_EXT_B_INT, false);
}

// testMatch
template<>
template<>
void object::test<5> ()
{
    checkPredicate(*RelatePredicate::matches("1***T*0**"), "1**.*2*.0**", true);
}


} // namespace tut
