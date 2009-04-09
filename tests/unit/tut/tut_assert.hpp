#ifndef TUT_ASSERT_H_GUARD
#define TUT_ASSERT_H_GUARD

#include "tut_exception.hpp"

#if defined(TUT_USE_POSIX)
#include <errno.h>
#include <cstring>
#endif

namespace tut
{

namespace
{

/**
 * Tests provided condition.
 * Throws if false.
 */
void ensure(bool cond)
{
    if (!cond)
    {
        // TODO: default ctor?
        throw failure("");
    }
}

/**
 * Tests provided condition.
 * Throws if true.
 */
void ensure_not(bool cond)
{
    ensure(!cond);
}

/**
 * Tests provided condition.
 * Throws if false.
 */
template <typename T>
void ensure(const T msg, bool cond)
{
    if (!cond)
    {
        throw failure(msg);
    }
}

/**
 * Tests provided condition.
 * Throws if true.
 */
template <typename T>
void ensure_not(const T msg, bool cond)
{
    ensure(msg, !cond);
}

/**
 * Tests two objects for being equal.
 * Throws if false.
 *
 * NB: both T and Q must have operator << defined somewhere, or
 * client code will not compile at all!
 */
template <class T, class Q>
void ensure_equals(const char* msg, const Q& actual, const T& expected)
{
    if (expected != actual)
    {
        std::stringstream ss;
        ss << (msg ? msg : "")
            << (msg ? ":" : "")
            << " expected '"
            << expected
            << "' actual '"
            << actual
            << '\'';
        throw failure(ss.str().c_str());
    }
}

template <class T, class Q>
void ensure_equals(const Q& actual, const T& expected)
{
    ensure_equals<>(0, actual, expected);
}

/**
 * Tests two objects for being at most in given distance one from another.
 * Borders are excluded.
 * Throws if false.
 *
 * NB: T must have operator << defined somewhere, or
 * client code will not compile at all! Also, T shall have
 * operators + and -, and be comparable.
 */
template <class T>
void ensure_distance(const char* msg, const T& actual, const T& expected,
    const T& distance)
{
    if (expected-distance >= actual || expected+distance <= actual)
    {
        std::stringstream ss;
        ss << (msg ? msg : "")
            << (msg? ":" : "")
            << " expected ("
            << expected-distance
            << " - "
            << expected+distance
            << ") actual '"
            << actual
            << '\'';
        throw failure(ss.str().c_str());
    }
}

template <class T>
void ensure_distance(const T& actual, const T& expected, const T& distance)
{
    ensure_distance<>(0, actual, expected, distance);
}

void ensure_errno(const char *msg, bool cond)
{
    if(!cond)
    {
#if defined(TUT_USE_POSIX)
        char e[512];
        std::stringstream ss;
        ss << (msg ? msg : "")
            << (msg? ": " : "")
            << strerror_r(errno, e, sizeof(e));
        throw failure(ss.str().c_str());
#else
        throw failure(msg);
#endif
    }
}

/**
 * Unconditionally fails with message.
 */
void fail(const char* msg = "")
{
    throw failure(msg);
}

void fail(const std::string &msg)
{
    throw failure(msg);
}

} // end of namespace

}

#endif

