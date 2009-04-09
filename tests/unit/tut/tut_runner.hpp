#ifndef TUT_RUNNER_H_GUARD
#define TUT_RUNNER_H_GUARD

#include <string>
#include <vector>
#include "tut_exception.hpp"

namespace tut
{

/**
 * Interface.
 * Test group operations.
 */
struct group_base
{
    virtual ~group_base()
    {
    }

    // execute tests iteratively
    virtual void rewind() = 0;
    virtual test_result run_next() = 0;

    // execute one test
    virtual test_result run_test(int n) = 0;
};


/**
 * Test runner callback interface.
 * Can be implemented by caller to update
 * tests results in real-time. User can implement
 * any of callback methods, and leave unused
 * in default implementation.
 */
struct callback
{
    /**
     * Virtual destructor is a must for subclassed types.
     */
    virtual ~callback()
    {
    }

    /**
     * Called when new test run started.
     */
    virtual void run_started()
    {
    }

    /**
     * Called when a group started
     * @param name Name of the group
     */
    virtual void group_started(const std::string& /*name*/)
    {
    }

    /**
     * Called when a test finished.
     * @param tr Test results.
     */
    virtual void test_completed(const test_result& /*tr*/)
    {
    }

    /**
     * Called when a group is completed
     * @param name Name of the group
     */
    virtual void group_completed(const std::string& /*name*/)
    {
    }

    /**
     * Called when all tests in run completed.
     */
    virtual void run_completed()
    {
    }
};

/**
 * Typedef for runner::list_groups()
 */
typedef std::vector<std::string> groupnames;

/**
 * Test runner.
 */
class test_runner
{

public:

    /**
     * Constructor
     */
    test_runner()
        : callback_(&default_callback_)
    {
    }

    /**
     * Stores another group for getting by name.
     */
    void register_group(const std::string& name, group_base* gr)
    {
        if (gr == 0)
        {
            throw tut_error("group shall be non-null");
        }

        if (groups_.find(name) != groups_.end())
        {
            std::string msg("attempt to add already existent group " + name);
            // this exception terminates application so we use cerr also
            // TODO: should this message appear in stream?
            std::cerr << msg << std::endl;
            throw tut_error(msg);
        }

        groups_.insert( std::make_pair(name, gr) );
    }

    /**
     * Stores callback object.
     */
    void set_callback(callback* cb)
    {
        callback_ = cb == 0 ? &default_callback_ : cb;
    }

    /**
     * Returns callback object.
     */
    callback& get_callback() const
    {
        return *callback_;
    }

    /**
     * Returns list of known test groups.
     */
    const groupnames list_groups() const
    {
        groupnames ret;
        const_iterator i = groups_.begin();
        const_iterator e = groups_.end();
        while (i != e)
        {
            ret.push_back(i->first);
            ++i;
        }
        return ret;
    }

    /**
     * Runs all tests in all groups.
     * @param callback Callback object if exists; null otherwise
     */
    void run_tests() const
    {
        callback_->run_started();

        const_iterator i = groups_.begin();
        const_iterator e = groups_.end();
        while (i != e)
        {
            callback_->group_started(i->first);
            try
            {
                run_all_tests_in_group_(i);
            }
            catch (const no_more_tests&)
            {
                callback_->group_completed(i->first);
            }

            ++i;
        }

        callback_->run_completed();
    }

    /**
     * Runs all tests in specified group.
     */
    void run_tests(const std::string& group_name) const
    {
        callback_->run_started();

        const_iterator i = groups_.find(group_name);
        if (i == groups_.end())
        {
            callback_->run_completed();
            throw no_such_group(group_name);
        }

        callback_->group_started(group_name);
        try
        {
            run_all_tests_in_group_(i);
        }
        catch (const no_more_tests&)
        {
            // ok
        }

        callback_->group_completed(group_name);
        callback_->run_completed();
    }

    /**
     * Runs one test in specified group.
     */
    test_result run_test(const std::string& group_name, int n) const
    {
        callback_->run_started();

        const_iterator i = groups_.find(group_name);
        if (i == groups_.end())
        {
            callback_->run_completed();
            throw no_such_group(group_name);
        }

        callback_->group_started(group_name);
        try
        {
            test_result tr = i->second->run_test(n);
            callback_->test_completed(tr);
            callback_->group_completed(group_name);
            callback_->run_completed();
            return tr;
        }
        catch (const beyond_last_test&)
        {
            callback_->group_completed(group_name);
            callback_->run_completed();
            throw;
        }
        catch (const no_such_test&)
        {
            callback_->group_completed(group_name);
            callback_->run_completed();
            throw;
        }
    }

protected:

    typedef std::map<std::string, group_base*> groups;
    typedef groups::iterator iterator;
    typedef groups::const_iterator const_iterator;
    groups groups_;

    callback  default_callback_;
    callback* callback_;


private:

    void run_all_tests_in_group_(const_iterator i) const
    {
        i->second->rewind();
        for ( ;; )
        {
            test_result tr = i->second->run_next();
            callback_->test_completed(tr);

            if (tr.result == test_result::ex_ctor)
            {
                throw no_more_tests();
            }
        }
    }
};

/**
 * Singleton for test_runner implementation.
 * Instance with name runner_singleton shall be implemented
 * by user.
 */
class test_runner_singleton
{
public:

    static test_runner& get()
    {
        static test_runner tr;
        return tr;
    }
};

extern test_runner_singleton runner;

}

#endif
