//
// Test Suite for geos::util::Params class.

// tut
#include <tut/tut.hpp>

// geos
#include <geos/util/Params.h>

// std
#include <string>

namespace tut {
//
// Test Group
//

// Common data used in test cases.
struct test_params_data {

    test_params_data() {};
};

typedef test_group<test_params_data> group;
typedef group::object object;

group test_params_group("geos::util::Params");

using geos::util::Params;

//
// Test Cases
//

template<>
template<>
void object::test<1> ()
{
    Params prm;
    double d1 = 3.4, d2;
    prm.setParam("test_double", 3.4);
    bool rv = prm.getParamDouble("test_double", &d2);
    ensure_equals(rv, true);
    ensure_equals(d1, d2);
    // Wrong key should fail
    rv = prm.getParamDouble("test_wrong", &d2);
    ensure_equals(rv, false);
    // Overwrite existing value
    d1 = 5;
    prm.setParam("test_double", d1);
    rv = prm.getParamDouble("test_double", &d2);
    ensure_equals(rv, true);
    ensure_equals(d1, d2);
}

template<>
template<>
void object::test<2> ()
{
    Params prm;
    int i1 = 4, i2;
    prm.setParam("test_int", i1);
    bool rv = prm.getParamInteger("test_int", &i2);
    ensure_equals(rv, true);
    ensure_equals(i1, i2);
    // Wrong key should fail
    rv = prm.getParamInteger("test_wrong", &i2);
    ensure_equals(rv, false);
    // Overwrite existing value
    i1 = 5;
    prm.setParam("test_int", i1);
    rv = prm.getParamInteger("test_int", &i2);
    ensure_equals(rv, true);
    ensure_equals(i1, i2);
}

template<>
template<>
void object::test<3> ()
{
    Params prm;
    const char *s1 = "value";
    const char *s2;
    prm.setParam("test_str", s1);
    bool rv = prm.getParamString("test_str", &s2);
    ensure_equals(rv, true);
    ensure_equals(strlen(s1), strlen(s2));
    ensure_equals(strcmp(s1, s2), 0);
    // Wrong key should fail
    rv = prm.getParamString("test_wrong", &s2);
    ensure_equals(rv, false);
}




} // namespace tut

