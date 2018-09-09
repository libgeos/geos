#ifndef TUT_EXCEPTION_H_GUARD
#define TUT_EXCEPTION_H_GUARD

#include <stdexcept>
#include "tut_result.hpp"

namespace tut
{

/**
 * The base for all TUT exceptions.
 */
struct tut_error : public std::exception
{
    explicit tut_error(const std::string& msg)
        : err_msg(msg)
    {
    }

    virtual test_result::result_type result() const
    {
        return test_result::ex;
    }

    virtual std::string type() const
    {
        return "tut::tut_error";
    }

    const char* what() const throw() override
    {
        return err_msg.c_str();
    }

    ~tut_error() throw() override
    {
    }

private:
    void operator=(const tut_error &);

    const std::string err_msg;
};

/**
 * Group not found exception.
 */
struct no_such_group : public tut_error
{
    explicit no_such_group(const std::string& grp)
        : tut_error(grp)
    {
    }

    std::string type() const override
    {
        return "tut::no_such_group";
    }

    ~no_such_group() throw() override
    {
    }
};

/**
 * Test not found exception.
 */
struct no_such_test : public tut_error
{
    explicit no_such_test(const std::string& grp)
        : tut_error(grp)
    {
    }

    std::string type() const override
    {
        return "tut::no_such_test";
    }

    ~no_such_test() throw() override
    {
    }
};

/**
 * Internal exception to be throwed when
 * test constructor has failed.
 */
struct bad_ctor : public tut_error
{
    explicit bad_ctor(const std::string& msg)
        : tut_error(msg)
    {
    }

    test_result::result_type result() const override
    {
        return test_result::ex_ctor;
    }

    std::string type() const override
    {
        return "tut::bad_ctor";
    }

    ~bad_ctor() throw() override
    {
    }
};

/**
 * Exception to be throwed when ensure() fails or fail() called.
 */
struct failure : public tut_error
{
    explicit failure(const std::string& msg)
        : tut_error(msg)
    {
    }

    test_result::result_type result() const override
    {
        return test_result::fail;
    }

    std::string type() const override
    {
        return "tut::failure";
    }

    ~failure() throw() override
    {
    }
};

/**
 * Exception to be throwed when test desctructor throwed an exception.
 */
struct warning : public tut_error
{
    explicit warning(const std::string& msg)
        : tut_error(msg)
    {
    }

    test_result::result_type result() const override
    {
        return test_result::warn;
    }

    std::string type() const override
    {
        return "tut::warning";
    }

    ~warning() throw() override
    {
    }
};

/**
 * Exception to be throwed when test issued SEH (Win32)
 */
struct seh : public tut_error
{
    explicit seh(const std::string& msg)
        : tut_error(msg)
    {
    }

    test_result::result_type result() const override
    {
        return test_result::term;
    }

    std::string type() const override
    {
        return "tut::seh";
    }

    ~seh() throw() override
    {
    }
};

/**
 * Exception to be throwed when child processes fail.
 */
struct rethrown : public failure
{
    explicit rethrown(const test_result &p_result)
        : failure(p_result.message), tr(p_result)
    {
    }

    test_result::result_type result() const override
    {
        return test_result::rethrown;
    }

    std::string type() const override
    {
        return "tut::rethrown";
    }

    ~rethrown() throw() override
    {
    }

    const test_result tr;
};

struct skipped : public tut_error
{
    explicit skipped(const std::string& msg)
        : tut_error(msg)
    {
    }

    test_result::result_type result() const override
    {
        return test_result::skipped;
    }

    std::string type() const override
    {
        return "tut::skipped";
    }

    ~skipped() throw() override
    {
    }
};

}

#endif
